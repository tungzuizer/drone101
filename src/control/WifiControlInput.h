#ifndef WIFI_CONTROL_INPUT_H
#define WIFI_CONTROL_INPUT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <atomic>
#include "ControlInputSource.h"
#include "SerialControlInput.h"
#include "../config.h"

// Cấu trúc Telemetry truyền từ Core 1 sang Core 0 để gửi về điện thoại & Web Tuner
struct TelemetryPayload {
    float roll;
    float pitch;
    float yaw;
    float rateRoll;
    float ratePitch;
    float rateYaw;
    float throttle;
    float altitude;
    float batteryVoltage;
    float ax, ay, az;
    uint16_t m1;
    uint16_t m2;
    uint16_t m3;
    uint16_t m4;
    bool isArmed;
    uint8_t failsafeState;
    uint32_t flightLoopTimeUs;
    bool imuOk;
    bool baroOk;
    bool magOk;
    bool pcaOk;
};

class WifiControlInput : public ControlInputSource {
public:
    WifiControlInput(uint16_t httpPort = WIFI_HTTP_PORT, uint16_t wsPort = WIFI_WS_PORT);
    ~WifiControlInput() override;

    // Khởi tạo Wi-Fi SoftAP, WebServer và WebSocketsServer
    bool begin() override;

    // Gọi tại Core 1 (250Hz): Đọc atomic snapshot dữ liệu điều khiển từ Core 0
    void update() override;

    // Lấy dữ liệu điều khiển hiện tại (được gọi từ MotorMixer & Failsafe)
    const ControlData& getControlData() const override { return localData_; }

    // Gọi từ Core 1: Đẩy dữ liệu Telemetry vào Mailbox để gửi về điện thoại
    void updateTelemetry(const TelemetryPayload& telemetry);

    // Gọi từ Core 0: Xử lý HTTP Client và WebSockets events (Non-blocking)
    void processNetwork();

    // Gán tham chiếu đến SerialControlInput để chuyển tiếp lệnh GCS từ WiFi
    void setGcsForwarder(SerialControlInput* gcsTarget) { gcsTarget_ = gcsTarget; }

    // Kiểm tra có điện thoại đang kết nối WebSocket VÀ gửi gói điều khiển $C,... hay không
    // (Phân biệt Phone Cockpit thực sự vs GCS Tuner chỉ gửi text commands)
    bool isPhoneConnected() const { return phoneControlActive_.load(); }

    // Kiểm tra có bất kỳ client WebSocket nào đang kết nối (bao gồm cả Tuner)
    bool isAnyClientConnected() const { return clientConnected_.load(); }
    uint8_t getConnectedClientCount() const { return connectedClientsCount_.load(); }

    // Reset về trạng thái an toàn
    void resetToSafeState();

private:
    uint16_t httpPort_;
    uint16_t wsPort_;
    SerialControlInput* gcsTarget_ = nullptr;

    WebServer server_;
    WebSocketsServer wsServer_;

    // =========================================================================
    // LOCK-FREE DOUBLE BUFFER (CORE 0 -> CORE 1: UPLINK CONTROL DATA)
    // =========================================================================
    ControlData controlBufferPool_[2];
    std::atomic<uint8_t> controlReadIdx_;
    ControlData localData_; // Bản sao làm việc cục bộ của Core 1

    // =========================================================================
    // LOCK-FREE TELEMETRY BUFFER (CORE 1 -> CORE 0: DOWNLINK TELEMETRY)
    // =========================================================================
    TelemetryPayload telemBufferPool_[2];
    std::atomic<uint8_t> telemReadIdx_;

    std::atomic<bool> clientConnected_;
    std::atomic<bool> phoneControlActive_;  // true chỉ khi nhận được gói $C,... từ Phone Cockpit
    std::atomic<uint8_t> connectedClientsCount_;
    uint32_t lastTelemBroadcastMs_;

    // Callback xử lý WebSocket Event
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

    // Giải mã gói tin điều khiển từ điện thoại: $C,thr,roll,pitch,yaw,arm,mode,kill
    void parseControlPacket(const char* packet, size_t length);

    // Truyền Telemetry về tất cả các điện thoại đang kết nối
    void broadcastTelemetry();

    // Khởi tạo các Route cho WebServer
    void setupWebRoutes();
};

#endif // WIFI_CONTROL_INPUT_H
