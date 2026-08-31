#ifndef WEB_COCKPIT_HTML_H
#define WEB_COCKPIT_HTML_H

#include <Arduino.h>

// =============================================================================
// GIAO DIỆN WEB COCKPIT ĐIỀU KHIỂN DRONE TRÊN ĐIỆN THOẠI (HTML5/CSS3/JS INLINE)
// Tích hợp: 3D Quadcopter HUD Hi-DPI, Hoạt ảnh 60fps mượt mà, Gợi ý bay AI,
// Hiệu ứng âm thanh Web Audio & Rung Haptic phản hồi xúc giác.
// Lưu trữ trực tiếp trong Flash PROGMEM của ESP32-S3 (Zero-Dependency)
// =============================================================================

const char PAGE_COCKPIT_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <title>ESP32-S3 Drone Cockpit Pro</title>
  <style>
    :root {
      --bg: #06090e;
      --card: rgba(13, 20, 32, 0.90);
      --card-inner: rgba(7, 11, 18, 0.75);
      --border: rgba(0, 229, 255, 0.25);
      --border-bright: rgba(0, 229, 255, 0.7);
      --accent: #00e5ff;
      --accent-glow: rgba(0, 229, 255, 0.45);
      --warn: #ffab00;
      --warn-glow: rgba(255, 171, 0, 0.45);
      --danger: #ff1744;
      --danger-glow: rgba(255, 23, 68, 0.5);
      --success: #00e676;
      --success-glow: rgba(0, 230, 118, 0.45);
      --text: #f1f5f9;
      --text-dim: #94a3b8;
      --font-mono: "SF Mono", Consolas, Monaco, "Roboto Mono", monospace;
    }
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-select: none;
      -webkit-touch-callout: none;
    }
    body {
      background: var(--bg);
      background-image:
        radial-gradient(circle at 50% 10%, rgba(0, 229, 255, 0.08) 0%, transparent 65%),
        linear-gradient(180deg, #090e17 0%, #040608 100%);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      height: 100vh;
      height: 100dvh;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      touch-action: none;
    }

    /* TOP STATUS BAR */
    header {
      background: var(--card);
      backdrop-filter: blur(10px);
      -webkit-backdrop-filter: blur(10px);
      border-bottom: 1px solid var(--border);
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 4px 10px;
      font-size: 11px;
      z-index: 10;
      gap: 6px;
    }
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 4px;
      padding: 3px 8px;
      border-radius: 12px;
      font-weight: 800;
      letter-spacing: 0.5px;
      text-transform: uppercase;
      font-size: 10px;
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    }
    .badge-dot {
      width: 6px;
      height: 6px;
      border-radius: 50%;
      background: currentColor;
      box-shadow: 0 0 8px currentColor;
    }
    .badge-danger { background: rgba(255,23,68,0.2); color: var(--danger); border: 1px solid var(--danger); box-shadow: 0 0 8px var(--danger-glow); }
    .badge-success { background: rgba(0,230,118,0.2); color: var(--success); border: 1px solid var(--success); box-shadow: 0 0 12px var(--success-glow); }
    .badge-warn { background: rgba(255,171,0,0.2); color: var(--warn); border: 1px solid var(--warn); }
    .badge-info { background: rgba(0,229,255,0.18); color: var(--accent); border: 1px solid var(--border); }

    .telemetry-row {
      display: flex;
      gap: 6px;
      font-family: var(--font-mono);
      font-size: 11px;
      color: #94a3b8;
      overflow-x: auto;
    }
    .telemetry-item {
      background: rgba(0,0,0,0.4);
      padding: 2px 6px;
      border-radius: 4px;
      border: 1px solid rgba(255,255,255,0.08);
      white-space: nowrap;
    }
    .telemetry-item span { color: var(--accent); font-weight: 700; }

    /* SMART FLIGHT ADVISOR BANNER */
    .smart-advisor {
      background: linear-gradient(90deg, rgba(0, 229, 255, 0.15) 0%, rgba(13, 20, 32, 0.95) 50%, rgba(0, 229, 255, 0.15) 100%);
      border-bottom: 1px solid var(--border);
      padding: 4px 10px;
      font-size: 11px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      z-index: 9;
      color: #e0f2fe;
    }
    .advisor-msg {
      display: flex;
      align-items: center;
      gap: 6px;
      font-weight: 600;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }
    .advisor-btn {
      background: rgba(0, 229, 255, 0.18);
      border: 1px solid var(--border-bright);
      color: var(--accent);
      padding: 3px 9px;
      border-radius: 6px;
      font-size: 10px;
      font-weight: 800;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 4px;
      box-shadow: 0 0 8px rgba(0,229,255,0.2);
    }
    .advisor-btn:active { background: var(--accent); color: #000; }

    /* MAIN COCKPIT VIEWPORT */
    .cockpit-container {
      flex: 1;
      display: grid;
      grid-template-columns: 1fr 190px 1fr;
      padding: 6px 8px;
      gap: 8px;
      position: relative;
      overflow: hidden;
    }
    @media (max-width: 680px) {
      .cockpit-container {
        grid-template-columns: 1fr 165px 1fr;
      }
    }

    /* JOYSTICK PANELS */
    .stick-panel {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      position: relative;
      overflow: hidden;
      box-shadow: inset 0 0 30px rgba(0,0,0,0.7);
    }
    .stick-title {
      position: absolute;
      top: 6px;
      font-size: 10px;
      color: var(--text-dim);
      font-weight: 800;
      text-transform: uppercase;
      letter-spacing: 0.8px;
    }
    .stick-readout {
      position: absolute;
      bottom: 6px;
      font-family: var(--font-mono);
      font-size: 10px;
      color: var(--accent);
      background: rgba(0,0,0,0.7);
      border: 1px solid rgba(0,229,255,0.3);
      padding: 2px 8px;
      border-radius: 6px;
      box-shadow: 0 0 10px rgba(0,0,0,0.5);
    }
    .joystick-zone {
      width: 190px;
      height: 190px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(0,229,255,0.04) 0%, rgba(13,20,32,0.8) 65%, rgba(0,229,255,0.15) 100%);
      border: 2px solid var(--border);
      position: relative;
      display: flex;
      align-items: center;
      justify-content: center;
      touch-action: none;
      box-shadow: 0 0 20px rgba(0,229,255,0.08);
    }
    .joystick-zone::before {
      content: '';
      position: absolute;
      width: 100%;
      height: 1px;
      background: linear-gradient(90deg, transparent, rgba(0,229,255,0.4), transparent);
    }
    .joystick-zone::after {
      content: '';
      position: absolute;
      height: 100%;
      width: 1px;
      background: linear-gradient(180deg, transparent, rgba(0,229,255,0.4), transparent);
    }
    .joystick-ring-inner {
      position: absolute;
      width: 95px;
      height: 95px;
      border-radius: 50%;
      border: 1px dashed rgba(0,229,255,0.3);
      pointer-events: none;
    }
    .joystick-nipple {
      width: 62px;
      height: 62px;
      border-radius: 50%;
      background: radial-gradient(circle at 35% 35%, #00e5ff 0%, #00838f 60%, #004d40 100%);
      box-shadow: 0 0 25px var(--accent-glow), 0 4px 12px rgba(0,0,0,0.9);
      border: 2px solid #e0f7fa;
      position: absolute;
      pointer-events: none;
      transform: translate(0px, 0px);
      transition: transform 0.04s ease-out;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .joystick-nipple-center {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: #fff;
      box-shadow: 0 0 10px #fff;
    }

    /* CENTER HUD: 3D ATTITUDE DISPLAY */
    .center-hud {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: space-between;
      padding: 6px 4px;
      gap: 4px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.5);
    }
    .hud-canvas-box {
      width: 136px;
      height: 136px;
      border-radius: 50%;
      border: 2px solid var(--border-bright);
      background: #000;
      position: relative;
      overflow: hidden;
      box-shadow: 0 0 20px rgba(0,229,255,0.25), inset 0 0 15px rgba(0,0,0,0.9);
    }
    #hudCanvas {
      width: 100%;
      height: 100%;
      display: block;
    }

    /* THROTTLE PROFILE SELECTOR */
    .rate-limit-box {
      display: flex;
      background: #090e17;
      border: 1px solid var(--border);
      border-radius: 6px;
      overflow: hidden;
      width: 95%;
    }
    .rate-btn {
      flex: 1;
      padding: 5px 0;
      font-size: 10px;
      font-weight: 800;
      color: var(--text-dim);
      background: transparent;
      border: none;
      cursor: pointer;
      text-align: center;
      transition: all 0.2s;
    }
    .rate-btn.active {
      background: var(--accent);
      color: #000;
      box-shadow: 0 0 10px var(--accent-glow);
    }

    /* MOTOR POWER & PROPELLER INDICATORS */
    .motor-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 4px;
      width: 100%;
      padding: 0 2px;
    }
    .motor-card {
      background: var(--card-inner);
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 6px;
      padding: 3px 4px;
      display: flex;
      flex-direction: column;
      gap: 2px;
    }
    .motor-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 9px;
      color: #94a3b8;
    }
    .prop-icon {
      display: inline-block;
      width: 11px;
      height: 11px;
      border: 1px solid var(--accent);
      border-radius: 50%;
      position: relative;
    }
    .prop-icon::after {
      content: '';
      position: absolute;
      top: 4px; left: 0; right: 0; height: 1.5px;
      background: var(--accent);
    }
    .prop-spinning {
      animation: spin 0.3s linear infinite;
    }
    @keyframes spin {
      0% { transform: rotate(0deg); }
      100% { transform: rotate(360deg); }
    }
    .motor-progress {
      height: 5px;
      background: #111827;
      border-radius: 2px;
      overflow: hidden;
      border: 1px solid rgba(255,255,255,0.05);
    }
    .motor-fill {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, var(--accent), var(--success));
      border-radius: 2px;
      transition: width 0.08s ease;
    }
    .motor-fill.warn { background: linear-gradient(90deg, var(--warn), var(--danger)); }

    /* FOOTER CONTROLS */
    footer {
      background: var(--card);
      backdrop-filter: blur(10px);
      -webkit-backdrop-filter: blur(10px);
      border-top: 1px solid var(--border);
      padding: 6px 12px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      z-index: 10;
    }
    .btn {
      padding: 8px 12px;
      border-radius: 8px;
      border: 1px solid var(--border);
      background: #111a28;
      color: #fff;
      font-weight: 800;
      font-size: 11px;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 4px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      transition: all 0.2s;
    }
    .btn:active { transform: scale(0.95); }
    .btn-danger {
      background: linear-gradient(135deg, #c62828, #ff1744);
      border-color: #ff5252;
      box-shadow: 0 0 12px var(--danger-glow);
    }
    .btn-danger:active { box-shadow: 0 0 25px var(--danger); }
    .btn-mode {
      border-color: var(--border-bright);
      background: rgba(0, 229, 255, 0.15);
      color: var(--accent);
    }

    /* SLIDE-TO-ARM CONTROL */
    .arm-slider-container {
      flex: 1;
      max-width: 260px;
      height: 40px;
      background: #080d16;
      border: 2px solid var(--border);
      border-radius: 20px;
      position: relative;
      display: flex;
      align-items: center;
      padding: 2px;
      overflow: hidden;
      box-shadow: inset 0 0 12px rgba(0,0,0,0.85);
    }
    .arm-slider-track {
      position: absolute;
      left: 0; top: 0; bottom: 0;
      width: 0%;
      background: linear-gradient(90deg, rgba(255,23,68,0.25), rgba(0,230,118,0.35));
      pointer-events: none;
      transition: width 0.05s;
    }
    .arm-slider-text {
      width: 100%;
      text-align: center;
      font-size: 11px;
      font-weight: 800;
      color: var(--text-dim);
      letter-spacing: 1px;
      text-transform: uppercase;
      pointer-events: none;
      z-index: 1;
    }
    .arm-thumb {
      width: 36px;
      height: 36px;
      border-radius: 50%;
      background: linear-gradient(135deg, #ff1744, #b71c1c);
      box-shadow: 0 0 14px var(--danger-glow);
      border: 2px solid #ff8a80;
      position: absolute;
      left: 2px;
      cursor: grab;
      display: flex;
      align-items: center;
      justify-content: center;
      color: #fff;
      font-weight: bold;
      font-size: 14px;
      z-index: 2;
      touch-action: none;
      transition: left 0.05s, background 0.3s;
    }
    .arm-thumb.armed {
      background: linear-gradient(135deg, #00e676, #00c853);
      box-shadow: 0 0 16px var(--success-glow);
      border-color: #b9f6ca;
    }

    /* MODAL GỢI Ý & HƯỚNG DẪN */
    .modal-backdrop {
      position: fixed;
      top: 0; left: 0; right: 0; bottom: 0;
      background: rgba(0,0,0,0.88);
      backdrop-filter: blur(8px);
      -webkit-backdrop-filter: blur(8px);
      z-index: 999;
      display: none;
      align-items: center;
      justify-content: center;
      padding: 16px;
    }
    .modal-backdrop.show { display: flex; }
    .modal-card {
      background: #0f172a;
      border: 1px solid var(--accent);
      box-shadow: 0 0 35px rgba(0,229,255,0.3);
      border-radius: 14px;
      width: 100%;
      max-width: 480px;
      max-height: 85vh;
      overflow-y: auto;
      padding: 16px;
      display: flex;
      flex-direction: column;
      gap: 12px;
    }
    .modal-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      border-bottom: 1px solid rgba(255,255,255,0.12);
      padding-bottom: 8px;
    }
    .modal-title {
      font-size: 14px;
      font-weight: 800;
      color: var(--accent);
      display: flex;
      align-items: center;
      gap: 6px;
    }
    .modal-close {
      background: transparent;
      border: none;
      color: #94a3b8;
      font-size: 18px;
      cursor: pointer;
      padding: 4px;
    }
    .tip-box {
      background: rgba(0,229,255,0.08);
      border-left: 3px solid var(--accent);
      padding: 8px 10px;
      border-radius: 0 6px 6px 0;
      font-size: 12px;
      line-height: 1.45;
    }
    .checklist-item {
      display: flex;
      align-items: center;
      gap: 8px;
      font-size: 12px;
      padding: 4px 0;
    }
    .check-icon { color: var(--success); font-weight: bold; }
  </style>
</head>
<body>

  <!-- TOP HEADER -->
  <header>
    <div style="display:flex; align-items:center; gap:6px;">
      <div id="armBadge" class="badge badge-danger"><span class="badge-dot"></span><span id="armTxt">DISARMED</span></div>
      <div id="modeBadge" class="badge badge-info">ANGLE</div>
      <div id="wsBadge" class="badge badge-warn"><span class="badge-dot"></span><span id="wsTxt">CONNECTING</span></div>
    </div>
    <div class="telemetry-row">
      <div class="telemetry-item">R:<span id="valRoll">0.0°</span></div>
      <div class="telemetry-item">P:<span id="valPitch">0.0°</span></div>
      <div class="telemetry-item">Y:<span id="valYaw">0.0°</span></div>
      <div class="telemetry-item">ALT:<span id="valAlt">0.0m</span></div>
      <div class="telemetry-item">BAT:<span id="valBat">12.0V</span></div>
    </div>
  </header>

  <!-- SMART FLIGHT ADVISOR (AI COACH) -->
  <div class="smart-advisor">
    <div class="advisor-msg" id="advisorText">
      <span>💡</span> <span id="advisorMsgContent">Đang khởi tạo cảm biến IMU & kết nối...</span>
    </div>
    <button class="advisor-btn" onclick="toggleGuideModal(true)">
      <span>📖 GỢI Ý & GUIDE</span>
    </button>
  </div>

  <!-- MAIN COCKPIT VIEW -->
  <div class="cockpit-container">

    <!-- LEFT JOYSTICK: THROTTLE + YAW -->
    <div class="stick-panel">
      <div class="stick-title">CẦN TRÁI: GA (Y) / YAW (X)</div>
      <div class="joystick-zone" id="zoneLeft">
        <div class="joystick-ring-inner"></div>
        <div class="joystick-nipple" id="nippleLeft">
          <div class="joystick-nipple-center"></div>
        </div>
      </div>
      <div class="stick-readout" id="readoutLeft">THR: 0% | YAW: 0°/s</div>
    </div>

    <!-- CENTER HUD: 3D ATTITUDE + PROPELLER BARS -->
    <div class="center-hud">
      <div class="hud-canvas-box">
        <canvas id="hudCanvas"></canvas>
      </div>

      <!-- THROTTLE PROFILE SELECTOR -->
      <div class="rate-limit-box">
        <button class="rate-btn active" onclick="setThrottleCap(30, this)">30%</button>
        <button class="rate-btn" onclick="setThrottleCap(60, this)">60%</button>
        <button class="rate-btn" onclick="setThrottleCap(100, this)">MAX</button>
      </div>

      <!-- MOTOR OUTPUT POWER & PROPELLERS -->
      <div class="motor-grid">
        <div class="motor-card">
          <div class="motor-header">
            <span>M1 (FR)</span>
            <span class="prop-icon" id="prop1"></span>
            <span id="m1Txt">0%</span>
          </div>
          <div class="motor-progress"><div class="motor-fill" id="m1Bar"></div></div>
        </div>
        <div class="motor-card">
          <div class="motor-header">
            <span>M2 (FL)</span>
            <span class="prop-icon" id="prop2"></span>
            <span id="m2Txt">0%</span>
          </div>
          <div class="motor-progress"><div class="motor-fill" id="m2Bar"></div></div>
        </div>
        <div class="motor-card">
          <div class="motor-header">
            <span>M3 (RR)</span>
            <span class="prop-icon" id="prop3"></span>
            <span id="m3Txt">0%</span>
          </div>
          <div class="motor-progress"><div class="motor-fill" id="m3Bar"></div></div>
        </div>
        <div class="motor-card">
          <div class="motor-header">
            <span>M4 (RL)</span>
            <span class="prop-icon" id="prop4"></span>
            <span id="m4Txt">0%</span>
          </div>
          <div class="motor-progress"><div class="motor-fill" id="m4Bar"></div></div>
        </div>
      </div>
    </div>

    <!-- RIGHT JOYSTICK: PITCH + ROLL -->
    <div class="stick-panel">
      <div class="stick-title">CẦN PHẢI: PITCH (Y) / ROLL (X)</div>
      <div class="joystick-zone" id="zoneRight">
        <div class="joystick-ring-inner"></div>
        <div class="joystick-nipple" id="nippleRight">
          <div class="joystick-nipple-center"></div>
        </div>
      </div>
      <div class="stick-readout" id="readoutRight">ROLL: 0.0° | PITCH: 0.0°</div>
    </div>

  </div>

  <!-- FOOTER: SAFETY CONTROLS & ARM SLIDER -->
  <footer>
    <button class="btn btn-danger" onclick="triggerEmergencyKill()">
      ⚠️ KILL
    </button>

    <!-- SLIDE TO ARM -->
    <div class="arm-slider-container" id="armContainer">
      <div class="arm-slider-track" id="armTrack"></div>
      <div class="arm-slider-text" id="armText">TRƯỢT ĐỂ ARM ➔</div>
      <div class="arm-thumb" id="armThumb">🔒</div>
    </div>

    <button class="btn btn-mode" onclick="toggleFlightMode()">
      <span id="btnModeText">ANGLE</span>
    </button>
  </footer>

  <!-- MODAL GỢI Ý & HƯỚNG DẪN CẤT CÁNH -->
  <div class="modal-backdrop" id="guideModal" onclick="if(event.target===this)toggleGuideModal(false)">
    <div class="modal-card">
      <div class="modal-header">
        <div class="modal-title"><span>🎯</span> GỢI Ý & QUY TRÌNH BAY AN TOÀN</div>
        <button class="modal-close" onclick="toggleGuideModal(false)">✕</button>
      </div>

      <div class="tip-box">
        <b>💡 Checklist 5 bước cất cánh:</b>
        <div class="checklist-item"><span class="check-icon">1.</span> Đặt drone trên mặt phẳng, cách người ≥ 3 mét.</div>
        <div class="checklist-item"><span class="check-icon">2.</span> Kiểm tra Roll & Pitch trên HUD cân bằng quanh 0.0°.</div>
        <div class="checklist-item"><span class="check-icon">3.</span> Chọn mức ga <b>30%</b> an toàn cho người mới.</div>
        <div class="checklist-item"><span class="check-icon">4.</span> Kéo thanh trượt ARM hết sang phải ➔ Nghe tiếng bíp xác nhận.</div>
        <div class="checklist-item"><span class="check-icon">5.</span> Đẩy cần ga trái từ từ lên 25% để drone nhấc nhẹ chân đáp.</div>
      </div>

      <div class="tip-box" style="border-left-color: var(--warn);">
        <b>⚡ Gợi ý ứng biến khi bay:</b><br>
        • <b>Khi gió tạt hoặc lệch hướng:</b> Dùng cần phải (Roll/Pitch) nhấp nhả nhẹ để giữ drone cố định một chỗ.<br>
        • <b>Khi drone mất kiểm soát:</b> Bấm ngay nút đỏ <b>⚠️ KILL</b> để ngắt motor lập tức tránh va đập.<br>
        • <b>Khi điện thoại chuyển app / tắt màn:</b> Hệ thống tự động kích hoạt Failsafe ngắt ga.
      </div>

      <button class="btn" style="background:var(--accent); color:#000; justify-content:center;" onclick="toggleGuideModal(false)">
        ĐÃ HIỂU - SẴN SÀNG BAY
      </button>
    </div>
  </div>

  <!-- JAVASCRIPT: WEBSOCKET, TOUCH ENGINE, 3D HUD & ADVISOR -->
  <script>
    // State Variables
    let controlState = {
      throttle: 0.0,
      roll: 0.0,
      pitch: 0.0,
      yaw: 0.0,
      arm: 0,
      flightMode: 0, // 0=ANGLE, 1=ACRO, 2=ALT_HOLD
      kill: 0
    };
    let telemetryData = {
      roll: 0, pitch: 0, yaw: 0, alt: 0, vbat: 12.0,
      m1: 1000, m2: 1000, m3: 1000, m4: 1000,
      armed: 0, fsState: 0
    };
    // LERP Smooth State for 60FPS Fluid Animations
    let smoothAttitude = { roll: 0, pitch: 0, yaw: 0 };
    let rotorAngle = 0;

    let throttleCap = 30.0;
    let ws = null;
    let sendInterval = null;

    // Web Audio Synthesizer (Zero-Dependency Sound Effects)
    let audioCtx = null;
    function initAudio() {
      if (!audioCtx) {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)();
      }
    }
    function playBeep(freq, duration, type='sine') {
      try {
        initAudio();
        if (audioCtx.state === 'suspended') audioCtx.resume();
        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();
        osc.type = type;
        osc.frequency.setValueAtTime(freq, audioCtx.currentTime);
        gain.gain.setValueAtTime(0.15, audioCtx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + duration);
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        osc.start();
        osc.stop(audioCtx.currentTime + duration);
      } catch (e) {}
    }
    function triggerHaptic(ms=20) {
      if (navigator.vibrate) {
        try { navigator.vibrate(ms); } catch (e) {}
      }
    }

    // =========================================================================
    // 1. WEBSOCKET ENGINE
    // =========================================================================
    function connectWebSocket() {
      const wsUrl = "ws://" + (window.location.hostname || "192.168.4.1") + ":81/";
      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        document.getElementById('wsBadge').className = 'badge badge-success';
        document.getElementById('wsTxt').innerText = 'CONNECTED';
        playBeep(880, 0.1);
        updateAdvisor();
      };

      ws.onclose = () => {
        document.getElementById('wsBadge').className = 'badge badge-danger';
        document.getElementById('wsTxt').innerText = 'DISCONNECTED';
        setTimeout(connectWebSocket, 1000);
        updateAdvisor();
      };

      ws.onmessage = (event) => {
        if (event.data.startsWith('$T,')) {
          const p = event.data.substring(3).split(',');
          if (p.length >= 11) {
            telemetryData.roll = parseFloat(p[0]);
            telemetryData.pitch = parseFloat(p[1]);
            telemetryData.yaw = parseFloat(p[2]);
            telemetryData.alt = parseFloat(p[3]);
            telemetryData.vbat = parseFloat(p[4]);
            telemetryData.m1 = parseInt(p[5]);
            telemetryData.m2 = parseInt(p[6]);
            telemetryData.m3 = parseInt(p[7]);
            telemetryData.m4 = parseInt(p[8]);
            telemetryData.armed = parseInt(p[9]);
            telemetryData.fsState = parseInt(p[10]);

            renderTelemetry();
          }
        }
      };
    }

    function renderTelemetry() {
      document.getElementById('valRoll').innerText = telemetryData.roll.toFixed(1) + '°';
      document.getElementById('valPitch').innerText = telemetryData.pitch.toFixed(1) + '°';
      document.getElementById('valYaw').innerText = telemetryData.yaw.toFixed(1) + '°';
      document.getElementById('valAlt').innerText = telemetryData.alt.toFixed(1) + 'm';
      document.getElementById('valBat').innerText = telemetryData.vbat.toFixed(1) + 'V';

      const batEl = document.getElementById('valBat');
      if (telemetryData.vbat < 10.5) batEl.style.color = 'var(--danger)';
      else if (telemetryData.vbat < 11.1) batEl.style.color = 'var(--warn)';
      else batEl.style.color = 'var(--accent)';

      const armBadge = document.getElementById('armBadge');
      if (telemetryData.armed === 1) {
        armBadge.className = 'badge badge-success';
        document.getElementById('armTxt').innerText = 'ARMED';
      } else {
        armBadge.className = 'badge badge-danger';
        document.getElementById('armTxt').innerText = 'DISARMED';
      }

      updateMotorGauge('m1', 'prop1', telemetryData.m1);
      updateMotorGauge('m2', 'prop2', telemetryData.m2);
      updateMotorGauge('m3', 'prop3', telemetryData.m3);
      updateMotorGauge('m4', 'prop4', telemetryData.m4);

      updateAdvisor();
    }

    function updateMotorGauge(id, propId, pulseUs) {
      const pct = Math.max(0, Math.min(100, Math.round((pulseUs - 1000) / 10)));
      document.getElementById(id + 'Txt').innerText = pct + '%';
      const bar = document.getElementById(id + 'Bar');
      bar.style.width = pct + '%';
      if (pct > 80) bar.classList.add('warn'); else bar.classList.remove('warn');

      const prop = document.getElementById(propId);
      if (pulseUs > 1050) prop.classList.add('prop-spinning');
      else prop.classList.remove('prop-spinning');
    }

    function startControlUplink() {
      if (sendInterval) clearInterval(sendInterval);
      sendInterval = setInterval(() => {
        if (ws && ws.readyState === WebSocket.OPEN) {
          const packet = `$C,${controlState.throttle.toFixed(1)},${controlState.roll.toFixed(1)},${controlState.pitch.toFixed(1)},${controlState.yaw.toFixed(1)},${controlState.arm},${controlState.flightMode},${controlState.kill}\n`;
          ws.send(packet);
          controlState.kill = 0;
        }
      }, 25);
    }

    // =========================================================================
    // 2. SMART FLIGHT ADVISOR LOGIC
    // =========================================================================
    function updateAdvisor() {
      const adv = document.getElementById('advisorMsgContent');
      if (!ws || ws.readyState !== WebSocket.OPEN) {
        adv.innerText = "Mất kết nối Wi-Fi! Hãy kết nối vào mạng ESP32-DRONE-FC.";
        return;
      }
      if (telemetryData.vbat < 10.5 && telemetryData.vbat > 5.0) {
        adv.innerHTML = "<span style='color:var(--danger)'>⚠️ Pin yếu (< 10.5V)! Hãy hạ ga và đáp drone ngay lập tức!</span>";
        return;
      }
      if (Math.abs(telemetryData.roll) > 25 || Math.abs(telemetryData.pitch) > 25) {
        adv.innerHTML = "<span style='color:var(--warn)'>⚠️ Góc nghiêng lớn! Giảm cần lái để drone tự lấy lại thăng bằng.</span>";
        return;
      }
      if (controlState.arm === 0) {
        if (controlState.throttle > 1.0) {
          adv.innerText = "⚠️ Hãy hạ cần ga về 0% trước khi trượt thanh ARM.";
        } else {
          adv.innerText = "💡 Hệ thống an toàn. Trượt thanh ARM sang phải để khởi động motor.";
        }
      } else {
        if (controlState.throttle <= 0.5) {
          adv.innerText = "🚀 Đã ARM! Motor quay Idle. Đẩy cần ga trái từ từ lên 25% để cất cánh.";
        } else if (controlState.throttle < 40) {
          adv.innerText = "✈️ Đang bay Hover. Dùng cần phải nhấp nhả nhẹ để giữ thăng bằng vị trí.";
        } else {
          adv.innerText = "⚡ Đang ở mức ga cao. Chú ý giữ không gian bay an toàn.";
        }
      }
    }

    // =========================================================================
    // 3. VIRTUAL JOYSTICK ENGINE (TOUCH + HAPTIC)
    // =========================================================================
    function setupJoystick(zoneId, nippleId, onMove, onEnd, isThrottleStick = false) {
      const zone = document.getElementById(zoneId);
      const nipple = document.getElementById(nippleId);
      const maxRadius = 65;
      let activeTouchId = null;

      function handleStart(e) {
        e.preventDefault();
        initAudio();
        const touch = e.changedTouches[0];
        activeTouchId = touch.identifier;
        triggerHaptic(10);
        updatePosition(touch.clientX, touch.clientY);
      }

      function handleMove(e) {
        e.preventDefault();
        for (let i = 0; i < e.changedTouches.length; i++) {
          if (e.changedTouches[i].identifier === activeTouchId) {
            updatePosition(e.changedTouches[i].clientX, e.changedTouches[i].clientY);
            break;
          }
        }
      }

      function handleEnd(e) {
        e.preventDefault();
        for (let i = 0; i < e.changedTouches.length; i++) {
          if (e.changedTouches[i].identifier === activeTouchId) {
            activeTouchId = null;
            if (!isThrottleStick) {
              nipple.style.transform = 'translate(0px, 0px)';
              onEnd();
            } else {
              nipple.style.transform = `translate(0px, ${lastThrottleY}px)`;
              onMove(0, currentThrottlePct);
            }
            triggerHaptic(8);
            break;
          }
        }
      }

      let lastThrottleY = maxRadius;
      let currentThrottlePct = 0.0;

      function updatePosition(clientX, clientY) {
        const rect = zone.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;
        let dx = clientX - centerX;
        let dy = clientY - centerY;

        let distance = Math.sqrt(dx * dx + dy * dy);
        if (distance > maxRadius) {
          dx = (dx / distance) * maxRadius;
          dy = (dy / distance) * maxRadius;
        }

        nipple.style.transform = `translate(${dx}px, ${dy}px)`;

        if (isThrottleStick) {
          lastThrottleY = dy;
          const normY = (-dy + maxRadius) / (2 * maxRadius);
          currentThrottlePct = Math.max(0, Math.min(throttleCap, normY * throttleCap));
          const normX = dx / maxRadius;
          const yawRate = normX * 180.0;
          onMove(yawRate, currentThrottlePct);
        } else {
          const normX = dx / maxRadius;
          const normY = -dy / maxRadius;
          onMove(normX * 45.0, normY * 45.0);
        }
      }

      zone.addEventListener('touchstart', handleStart, { passive: false });
      zone.addEventListener('touchmove', handleMove, { passive: false });
      zone.addEventListener('touchend', handleEnd, { passive: false });
      zone.addEventListener('touchcancel', handleEnd, { passive: false });
    }

    // Khởi tạo Joystick
    setupJoystick('zoneLeft', 'nippleLeft', (yaw, thr) => {
      controlState.yaw = yaw;
      controlState.throttle = thr;
      document.getElementById('readoutLeft').innerText = `THR: ${thr.toFixed(0)}% | YAW: ${yaw.toFixed(0)}°/s`;
      updateAdvisor();
    }, () => {
      controlState.yaw = 0.0;
    }, true);

    setupJoystick('zoneRight', 'nippleRight', (roll, pitch) => {
      controlState.roll = roll;
      controlState.pitch = pitch;
      document.getElementById('readoutRight').innerText = `ROLL: ${roll.toFixed(1)}° | PITCH: ${pitch.toFixed(1)}°`;
    }, () => {
      controlState.roll = 0.0;
      controlState.pitch = 0.0;
      document.getElementById('readoutRight').innerText = `ROLL: 0.0° | PITCH: 0.0°`;
    }, false);

    // =========================================================================
    // 4. SLIDE-TO-ARM CONTROLLER
    // =========================================================================
    const armThumb = document.getElementById('armThumb');
    const armContainer = document.getElementById('armContainer');
    const armTrack = document.getElementById('armTrack');
    let isDraggingArm = false;

    armThumb.addEventListener('touchstart', (e) => {
      initAudio();
      if (controlState.arm === 1) {
        controlState.arm = 0;
        controlState.throttle = 0.0;
        armThumb.className = 'arm-thumb';
        armThumb.style.left = '2px';
        armTrack.style.width = '0%';
        armThumb.innerText = '🔒';
        document.getElementById('armText').innerText = 'TRƯỢT ĐỂ ARM ➔';
        playBeep(440, 0.15, 'triangle');
        triggerHaptic(30);
        updateAdvisor();
        return;
      }
      isDraggingArm = true;
      triggerHaptic(15);
    }, { passive: false });

    window.addEventListener('touchmove', (e) => {
      if (!isDraggingArm) return;
      const touch = e.touches[0];
      const rect = armContainer.getBoundingClientRect();
      let dx = touch.clientX - rect.left - 18;
      const maxSlide = rect.width - 40;
      dx = Math.max(2, Math.min(maxSlide, dx));
      armThumb.style.left = dx + 'px';
      armTrack.style.width = (dx / maxSlide * 100) + '%';

      if (dx >= maxSlide - 4) {
        isDraggingArm = false;
        controlState.arm = 1;
        armThumb.className = 'arm-thumb armed';
        armThumb.innerText = '🔓';
        document.getElementById('armText').innerText = 'CHẠM ĐỂ DISARM';
        playBeep(659, 0.08);
        setTimeout(() => playBeep(880, 0.15), 90);
        triggerHaptic([30, 50, 40]);
        updateAdvisor();
      }
    }, { passive: false });

    window.addEventListener('touchend', () => {
      if (isDraggingArm) {
        isDraggingArm = false;
        if (controlState.arm === 0) {
          armThumb.style.left = '2px';
          armTrack.style.width = '0%';
        }
      }
    });

    // =========================================================================
    // 5. SAFETY & ACTIONS
    // =========================================================================
    function triggerEmergencyKill() {
      initAudio();
      controlState.kill = 1;
      controlState.arm = 0;
      controlState.throttle = 0.0;
      armThumb.className = 'arm-thumb';
      armThumb.style.left = '2px';
      armTrack.style.width = '0%';
      armThumb.innerText = '🔒';
      document.getElementById('armText').innerText = 'TRƯỢT ĐỂ ARM ➔';
      playBeep(250, 0.4, 'sawtooth');
      triggerHaptic([80, 50, 80]);
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(`$C,0.0,0.0,0.0,0.0,0,0,1\n`);
      }
      alert("⚠️ ĐÃ KÍCH HOẠT DỪNG KHẨN CẤP!");
      updateAdvisor();
    }

    function toggleFlightMode() {
      initAudio();
      controlState.flightMode = (controlState.flightMode + 1) % 3;
      const modes = ['ANGLE', 'ACRO', 'ALT_HOLD'];
      const mName = modes[controlState.flightMode];
      document.getElementById('modeBadge').innerText = mName;
      document.getElementById('btnModeText').innerText = mName;
      playBeep(520, 0.08);
      triggerHaptic(15);
    }

    function setThrottleCap(cap, btn) {
      initAudio();
      throttleCap = cap;
      document.querySelectorAll('.rate-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      playBeep(587, 0.06);
      triggerHaptic(10);
    }

    function toggleGuideModal(show) {
      initAudio();
      const m = document.getElementById('guideModal');
      if (show) {
        m.classList.add('show');
        playBeep(650, 0.06);
      } else {
        m.classList.remove('show');
      }
    }

    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        controlState.throttle = 0.0;
        controlState.roll = 0.0;
        controlState.pitch = 0.0;
        controlState.yaw = 0.0;
        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(`$C,0.0,0.0,0.0,0.0,0,0,0\n`);
        }
      }
    });

    // =========================================================================
    // 6. HIGH-DEFINITION 3D DRONE ATTITUDE & AVIONICS HUD CANVAS (60 FPS)
    // =========================================================================
    const canvas = document.getElementById('hudCanvas');
    const ctx = canvas.getContext('2d');

    function setupCanvasHiDPI() {
      const dpr = window.devicePixelRatio || 2;
      const rect = canvas.getBoundingClientRect();
      const size = rect.width || 136;
      canvas.width = size * dpr;
      canvas.height = size * dpr;
      ctx.scale(dpr, dpr);
    }

    function project3D(x, y, z, rollRad, pitchRad) {
      // 1. Pitch Rotation around X axis
      const cosP = Math.cos(pitchRad);
      const sinP = Math.sin(pitchRad);
      const y1 = y * cosP - z * sinP;
      const z1 = y * sinP + z * cosP;

      // 2. Roll Rotation around Y axis
      const cosR = Math.cos(rollRad);
      const sinR = Math.sin(rollRad);
      const x2 = x * cosR + z1 * sinR;
      const z2 = -x * sinR + z1 * cosR;

      // Isometric / Orthographic perspective with depth foreshortening
      return { x: x2, y: y1, z: z2 };
    }

    function drawHud3D() {
      // LERP Smoothing for fluid 60FPS experience
      smoothAttitude.roll  += (telemetryData.roll  - smoothAttitude.roll)  * 0.25;
      smoothAttitude.pitch += (telemetryData.pitch - smoothAttitude.pitch) * 0.25;
      smoothAttitude.yaw   += (telemetryData.yaw   - smoothAttitude.yaw)   * 0.25;

      const rect = canvas.getBoundingClientRect();
      const size = rect.width || 136;
      const cx = size / 2;
      const cy = size / 2;
      const rollRad = (-smoothAttitude.roll * Math.PI) / 180;
      const pitchRad = (smoothAttitude.pitch * Math.PI) / 180;
      const pitchOffset = Math.max(-42, Math.min(42, smoothAttitude.pitch * 1.15));

      // Propeller spin increment based on average motor PWM
      const avgPulse = (telemetryData.m1 + telemetryData.m2 + telemetryData.m3 + telemetryData.m4) / 4.0;
      if (avgPulse > 1050) {
        rotorAngle += (avgPulse - 1000) * 0.0012;
      }

      ctx.clearRect(0, 0, size, size);

      // -----------------------------------------------------------------------
      // A. ARTIFICIAL HORIZON (AVIONICS SKY / GROUND DISC)
      // -----------------------------------------------------------------------
      ctx.save();
      ctx.beginPath();
      ctx.arc(cx, cy, cx - 2, 0, Math.PI * 2);
      ctx.clip();

      ctx.translate(cx, cy);
      ctx.rotate(rollRad);

      // Sky Hemisphere
      const skyGrad = ctx.createLinearGradient(0, -cy + pitchOffset, 0, pitchOffset);
      skyGrad.addColorStop(0, '#0284c7');
      skyGrad.addColorStop(1, '#38bdf8');
      ctx.fillStyle = skyGrad;
      ctx.fillRect(-size, -size * 2 + pitchOffset, size * 2, size * 2);

      // Earth Hemisphere
      const gndGrad = ctx.createLinearGradient(0, pitchOffset, 0, cy + pitchOffset);
      gndGrad.addColorStop(0, '#78350f');
      gndGrad.addColorStop(1, '#291104');
      ctx.fillStyle = gndGrad;
      ctx.fillRect(-size, pitchOffset, size * 2, size * 2);

      // Glowing Horizon Line
      ctx.strokeStyle = '#ffffff';
      ctx.lineWidth = 2;
      ctx.shadowColor = '#00e5ff';
      ctx.shadowBlur = 6;
      ctx.beginPath();
      ctx.moveTo(-size, pitchOffset);
      ctx.lineTo(size, pitchOffset);
      ctx.stroke();
      ctx.shadowBlur = 0;

      // Pitch Ladder Lines (+20°, +10°, -10°, -20°)
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.85)';
      ctx.lineWidth = 1.2;
      ctx.font = '8px var(--font-mono)';
      ctx.fillStyle = '#ffffff';
      ctx.textAlign = 'center';

      [-30, -20, -10, 10, 20, 30].forEach(deg => {
        const yPos = pitchOffset - deg * 1.15;
        if (deg > 0) {
          // Positive Pitch (Solid Ladder)
          ctx.beginPath();
          ctx.moveTo(-16, yPos); ctx.lineTo(-6, yPos);
          ctx.moveTo(6, yPos);   ctx.lineTo(16, yPos);
          ctx.stroke();
          ctx.fillText(deg + '°', -22, yPos + 3);
          ctx.fillText(deg + '°', 22, yPos + 3);
        } else {
          // Negative Pitch (Dashed Ladder)
          ctx.setLineDash([3, 3]);
          ctx.beginPath();
          ctx.moveTo(-16, yPos); ctx.lineTo(-6, yPos);
          ctx.moveTo(6, yPos);   ctx.lineTo(16, yPos);
          ctx.stroke();
          ctx.setLineDash([]);
          ctx.fillText(Math.abs(deg) + '°', -22, yPos + 3);
          ctx.fillText(Math.abs(deg) + '°', 22, yPos + 3);
        }
      });

      ctx.restore();

      // -----------------------------------------------------------------------
      // B. 3D ISOMETRIC QUADCOPTER VISUALIZATION
      // -----------------------------------------------------------------------
      ctx.save();
      ctx.translate(cx, cy);

      const armSpan = 30; // Boom Arm length in 3D
      // 3D Arms: M1(Front-Right), M2(Front-Left), M3(Rear-Right), M4(Rear-Left)
      const pCenter = project3D(0, 0, 0, rollRad, pitchRad);
      const pFR = project3D( armSpan * 0.72, -armSpan * 0.72, 0, rollRad, pitchRad); // M1
      const pFL = project3D(-armSpan * 0.72, -armSpan * 0.72, 0, rollRad, pitchRad); // M2
      const pRR = project3D( armSpan * 0.72,  armSpan * 0.72, 0, rollRad, pitchRad); // M3
      const pRL = project3D(-armSpan * 0.72,  armSpan * 0.72, 0, rollRad, pitchRad); // M4
      const pNose = project3D(0, -armSpan * 0.95, -4, rollRad, pitchRad);            // Front Arrow

      // 1. Drone Carbon Arms (Thick 3D Beams)
      ctx.strokeStyle = '#0f172a';
      ctx.lineWidth = 4.5;
      ctx.beginPath();
      ctx.moveTo(pFL.x, pFL.y); ctx.lineTo(pRR.x, pRR.y);
      ctx.moveTo(pFR.x, pFR.y); ctx.lineTo(pRL.x, pRL.y);
      ctx.stroke();

      ctx.strokeStyle = '#38bdf8';
      ctx.lineWidth = 2.2;
      ctx.beginPath();
      ctx.moveTo(pFL.x, pFL.y); ctx.lineTo(pRR.x, pRR.y);
      ctx.moveTo(pFR.x, pFR.y); ctx.lineTo(pRL.x, pRL.y);
      ctx.stroke();

      // 2. High-Visibility Forward Heading Arrow (Mũi trước Drone màu cam)
      ctx.fillStyle = '#ff6d00';
      ctx.shadowColor = '#ff6d00';
      ctx.shadowBlur = 8;
      ctx.beginPath();
      ctx.moveTo(pNose.x, pNose.y);
      ctx.lineTo(pFL.x * 0.45, pFL.y * 0.45);
      ctx.lineTo(pFR.x * 0.45, pFR.y * 0.45);
      ctx.closePath();
      ctx.fill();
      ctx.shadowBlur = 0;

      // 3. Center Flight Controller Canopy
      ctx.fillStyle = '#0284c7';
      ctx.beginPath();
      ctx.arc(pCenter.x, pCenter.y, 5.5, 0, Math.PI * 2);
      ctx.fill();
      ctx.fillStyle = '#ffffff';
      ctx.beginPath();
      ctx.arc(pCenter.x, pCenter.y, 2, 0, Math.PI * 2);
      ctx.fill();

      // 4. 3D Rotating Rotor Discs (M1..M4)
      const rotors = [
        { p: pFR, label: '1', isFront: true,  pulse: telemetryData.m1 },
        { p: pFL, label: '2', isFront: true,  pulse: telemetryData.m2 },
        { p: pRR, label: '3', isFront: false, pulse: telemetryData.m3 },
        { p: pRL, label: '4', isFront: false, pulse: telemetryData.m4 },
      ];

      rotors.forEach((r, idx) => {
        // Rotor Pod
        ctx.fillStyle = r.isFront ? '#ff3d00' : '#00e5ff';
        ctx.beginPath();
        ctx.arc(r.p.x, r.p.y, 3, 0, Math.PI * 2);
        ctx.fill();

        // 3D Spinning Propeller Ring
        ctx.strokeStyle = telemetryData.armed ? (r.pulse > 1050 ? '#00e676' : '#ffab00') : '#ff1744';
        ctx.lineWidth = 1.6;
        ctx.beginPath();
        ctx.ellipse(r.p.x, r.p.y, 10, 4.5, rollRad, 0, Math.PI * 2);
        ctx.stroke();

        // Spinning Blades in 3D
        if (telemetryData.armed && r.pulse > 1050) {
          const bladeAng = rotorAngle * (idx % 2 === 0 ? 1 : -1) + (idx * Math.PI / 2);
          const bx = Math.cos(bladeAng) * 9;
          const by = Math.sin(bladeAng) * 4;
          ctx.strokeStyle = 'rgba(255, 255, 255, 0.85)';
          ctx.lineWidth = 1.2;
          ctx.beginPath();
          ctx.moveTo(r.p.x - bx, r.p.y - by);
          ctx.lineTo(r.p.x + bx, r.p.y + by);
          ctx.stroke();
        }
      });

      ctx.restore();

      // -----------------------------------------------------------------------
      // C. AIRCRAFT FIXED RETICLE & ROLL BEZEL TICK MARKS
      // -----------------------------------------------------------------------
      // Fixed Crosshairs
      ctx.strokeStyle = '#facc15';
      ctx.lineWidth = 2;
      ctx.shadowColor = '#facc15';
      ctx.shadowBlur = 4;

      ctx.beginPath();
      // Left Wing
      ctx.moveTo(cx - 24, cy); ctx.lineTo(cx - 8, cy); ctx.lineTo(cx - 8, cy + 4);
      // Right Wing
      ctx.moveTo(cx + 24, cy); ctx.lineTo(cx + 8, cy); ctx.lineTo(cx + 8, cy + 4);
      // Center Dot
      ctx.arc(cx, cy, 1.5, 0, Math.PI * 2);
      ctx.stroke();
      ctx.shadowBlur = 0;

      // Top Roll Pointer Triangle
      ctx.fillStyle = '#facc15';
      ctx.beginPath();
      ctx.moveTo(cx, 4);
      ctx.lineTo(cx - 4, 10);
      ctx.lineTo(cx + 4, 10);
      ctx.closePath();
      ctx.fill();

      // Numerical Roll / Pitch Mini Badges inside HUD
      ctx.font = 'bold 8px var(--font-mono)';
      ctx.fillStyle = 'rgba(0, 229, 255, 0.9)';
      ctx.textAlign = 'left';
      ctx.fillText('R:' + telemetryData.roll.toFixed(1) + '°', 6, size - 6);
      ctx.textAlign = 'right';
      ctx.fillText('P:' + telemetryData.pitch.toFixed(1) + '°', size - 6, size - 6);

      requestAnimationFrame(drawHud3D);
    }

    // Khởi động toàn bộ khi load trang
    window.addEventListener('load', () => {
      setupCanvasHiDPI();
      window.addEventListener('resize', setupCanvasHiDPI);
      connectWebSocket();
      startControlUplink();
      requestAnimationFrame(drawHud3D);
      updateAdvisor();
    });
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_COCKPIT_HTML_H
