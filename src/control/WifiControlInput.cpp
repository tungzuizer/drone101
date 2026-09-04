#include "WifiControlInput.h"
#include "WebCockpitHtml.h"
#include <stdio.h>
#include <string.h>

WifiControlInput::WifiControlInput(uint16_t httpPort, uint16_t wsPort)
    : httpPort_(httpPort),
      wsPort_(wsPort),
      server_(httpPort),
      wsServer_(wsPort),
      controlReadIdx_(0),
      telemReadIdx_(0),
      clientConnected_(false),
      phoneControlActive_(false),
      connectedClientsCount_(0),
      lastTelemBroadcastMs_(0) {
    resetToSafeState();
    localData_ = controlBufferPool_[0];
}

WifiControlInput::~WifiControlInput() {
    wsServer_.close();
    server_.stop();
    WiFi.softAPdisconnect(true);
}

void WifiControlInput::resetToSafeState() {
    for (int i = 0; i < 2; i++) {
        controlBufferPool_[i].throttle = 0.0f;
        controlBufferPool_[i].roll = 0.0f;
        controlBufferPool_[i].pitch = 0.0f;
        controlBufferPool_[i].yaw = 0.0f;
        controlBufferPool_[i].flightMode = MODE_ANGLE;
        controlBufferPool_[i].armSwitch = false;
        controlBufferPool_[i].aux1 = false;
        controlBufferPool_[i].aux2 = false;
        controlBufferPool_[i].isConnected = false;
        controlBufferPool_[i].lastPacketTimeMs = 0;

        memset(&telemBufferPool_[i], 0, sizeof(TelemetryPayload));
    }
    phoneControlActive_.store(false, std::memory_order_relaxed);
}

bool WifiControlInput::begin() {
    resetToSafeState();

    Serial.println("\n[WIFI] Đang khởi tạo Wi-Fi Soft-AP...");
    WiFi.mode(WIFI_AP);

    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    WiFi.softAPConfig(local_IP, gateway, subnet);
    bool apOk = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CLIENTS);

    if (!apOk) {
        Serial.println("[WIFI ERROR] Không thể khởi tạo Wi-Fi Soft-AP!");
        return false;
    }

    // Giảm công suất phát RF về 8.5dBm sau khi Soft-AP đã bật để tránh sụt áp cổng USB
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    Serial.printf("[WIFI OK] SSID: %s | Mật khẩu: %s\n", WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.printf("[WIFI OK] IP Drone: http://%s (Cổng Web: %d, WS: %d)\n",
                  WiFi.softAPIP().toString().c_str(), httpPort_, wsPort_);

    // Cấu hình WebServer
    setupWebRoutes();
    server_.begin(httpPort_);
    Serial.println("[WIFI OK] Web Server HTTP đã khởi động.");

    // Cấu hình WebSockets Server
    wsServer_.begin();
    wsServer_.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        handleWebSocketEvent(num, type, payload, length);
    });
    Serial.println("[WIFI OK] WebSockets Server đã khởi động.");

    return true;
}

void WifiControlInput::setupWebRoutes() {
    // Trang chủ giao diện Web Cockpit
    server_.on("/", HTTP_GET, [this]() {
        server_.send_P(200, "text/html", PAGE_COCKPIT_HTML);
    });

    // Hỗ trợ Captive Portal tự động mở trang điều khiển trên Android & iOS
    server_.on("/generate_204", HTTP_GET, [this]() {
        server_.send_P(200, "text/html", PAGE_COCKPIT_HTML);
    });
    server_.on("/hotspot-detect.html", HTTP_GET, [this]() {
        server_.send_P(200, "text/html", PAGE_COCKPIT_HTML);
    });
    server_.on("/canonical.html", HTTP_GET, [this]() {
        server_.send_P(200, "text/html", PAGE_COCKPIT_HTML);
    });

    // Bắt toàn bộ các request khác chuyển hướng về Cockpit
    server_.onNotFound([this]() {
        server_.send_P(200, "text/html", PAGE_COCKPIT_HTML);
    });
}

void WifiControlInput::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            clientConnected_.store(true);
            connectedClientsCount_.fetch_add(1);
            IPAddress ip = wsServer_.remoteIP(num);
            Serial.printf("[WS CONNECT] Client kết nối: [#%u] từ IP: %s (chờ xác định Phone/GCS)\n", num, ip.toString().c_str());
            break;
        }

        case WStype_DISCONNECTED: {
            uint8_t prev = connectedClientsCount_.load();
            while (prev > 0 && !connectedClientsCount_.compare_exchange_weak(prev, prev - 1)) {
                // CAS loop đảm bảo an toàn thread-safe không bị underflow
            }
            uint8_t nextCount = connectedClientsCount_.load();
            Serial.printf("[WS DISCONNECT] Client ngắt kết nối: [#%u]. Số kết nối còn lại: %u\n", num, nextCount);

            if (nextCount == 0) {
                clientConnected_.store(false);
                phoneControlActive_.store(false);
                resetToSafeState();
            }
            break;
        }

        case WStype_TEXT: {
            if (length > 0 && payload != nullptr) {
                parseControlPacket((const char*)payload, length);
            }
            break;
        }

        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        case WStype_PING:
        case WStype_PONG:
            break;
    }
}

void WifiControlInput::parseControlPacket(const char* packet, size_t length) {
    if (length < 3 || length > 128) return;

    char buf[129];
    memcpy(buf, packet, length);
    buf[length] = '\0';

    // Gói tin điều khiển từ Phone Cockpit: $C,throttle,roll,pitch,yaw,arm,mode,kill
    if (buf[0] == '$' && buf[1] == 'C' && buf[2] == ',') {
        float thr = 0.0f, roll = 0.0f, pitch = 0.0f, yaw = 0.0f;
        int arm = 0, mode = 0, kill = 0;

        int parsed = sscanf(buf + 3, "%f,%f,%f,%f,%d,%d,%d",
                            &thr, &roll, &pitch, &yaw, &arm, &mode, &kill);

        if (parsed >= 6) {
            // Giới hạn an toàn các kênh điều khiển
            if (thr < 0.0f) thr = 0.0f;
            if (thr > 100.0f) thr = 100.0f;

            if (roll < -45.0f) roll = -45.0f;
            if (roll > 45.0f) roll = 45.0f;

            if (pitch < -45.0f) pitch = -45.0f;
            if (pitch > 45.0f) pitch = 45.0f;

            if (yaw < -180.0f) yaw = -180.0f;
            if (yaw > 180.0f) yaw = 180.0f;

            FlightMode fMode = MODE_ANGLE;
            if (mode == 1) fMode = MODE_ACRO;
            else if (mode == 2) fMode = MODE_ALT_HOLD;
            else if (mode == 3) fMode = MODE_POS_HOLD;

            // Nếu người dùng bấm nút Dừng khẩn cấp trên điện thoại
            bool isKill = (kill == 1);

            // Ghi dữ liệu vào Buffer chờ (Lock-Free Double Buffering)
            uint8_t writeIdx = 1 - controlReadIdx_.load(std::memory_order_relaxed);
            ControlData& target = controlBufferPool_[writeIdx];

            target.throttle = isKill ? 0.0f : thr;
            target.roll = isKill ? 0.0f : roll;
            target.pitch = isKill ? 0.0f : pitch;
            target.yaw = isKill ? 0.0f : yaw;
            target.armSwitch = isKill ? false : (arm == 1);
            target.flightMode = fMode;
            target.aux1 = false;
            target.aux2 = false;
            target.isConnected = true;
            target.lastPacketTimeMs = millis();

            // Cập nhật chỉ số đọc cho Core 1
            controlReadIdx_.store(writeIdx, std::memory_order_release);
            clientConnected_.store(true, std::memory_order_relaxed);
            phoneControlActive_.store(true, std::memory_order_relaxed);  // Đánh dấu đây là Phone Cockpit thật sự
        }
    } else if (gcsTarget_ != nullptr) {
        gcsTarget_->processExternalLine(buf);
        // Không log PING/HEARTBEAT để tránh spam Serial
        if (strncasecmp(buf, "PING", 4) != 0 && strncasecmp(buf, "HEARTBEAT", 9) != 0) {
            Serial.printf("[WS GCS] Lệnh WiFi: %s\n", buf);
        }
    }
}

// Gọi từ Core 1 (250Hz): Lấy dữ liệu điều khiển tức thời không bị block
void WifiControlInput::update() {
    uint8_t currentIdx = controlReadIdx_.load(std::memory_order_acquire);
    localData_ = controlBufferPool_[currentIdx];
}

// Gọi từ Core 1 (250Hz): Cập nhật dữ liệu Telemetry vào Mailbox
void WifiControlInput::updateTelemetry(const TelemetryPayload& telemetry) {
    uint8_t writeIdx = 1 - telemReadIdx_.load(std::memory_order_relaxed);
    telemBufferPool_[writeIdx] = telemetry;
    telemReadIdx_.store(writeIdx, std::memory_order_release);
}

// Gọi từ Core 0: Xử lý Web và phát Telemetry về điện thoại
void WifiControlInput::processNetwork() {
    server_.handleClient();
    wsServer_.loop();

    // Truyền Telemetry về điện thoại theo chu kỳ quy định (25Hz = 40ms)
    uint32_t now = millis();
    const uint32_t intervalMs = 1000 / WIFI_TELEMETRY_RATE_HZ;

    if (now - lastTelemBroadcastMs_ >= intervalMs) {
        lastTelemBroadcastMs_ = now;
        if (clientConnected_.load(std::memory_order_relaxed)) {
            broadcastTelemetry();
        }
    }
}

void WifiControlInput::broadcastTelemetry() {
    uint8_t readIdx = telemReadIdx_.load(std::memory_order_acquire);
    TelemetryPayload t = telemBufferPool_[readIdx];

    // Gửi gói tin Telemetry đầy đủ $TEL (tương thích Web Tuner)
    char msg[256];
    snprintf(msg, sizeof(msg),
             "$TEL,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%u,%u,%u,%u,%.2f,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f",
             t.roll, t.pitch, t.yaw,
             t.rateRoll, t.ratePitch, t.rateYaw,
             t.throttle,
             t.m1, t.m2, t.m3, t.m4,
             t.altitude,
             t.isArmed ? 1 : 0,
             (int)t.failsafeState,
             t.imuOk ? 1 : 0,
             t.baroOk ? 1 : 0,
             t.magOk ? 1 : 0,
             t.pcaOk ? 1 : 0,
             t.ax, t.ay, t.az);

    wsServer_.broadcastTXT(msg);
}
