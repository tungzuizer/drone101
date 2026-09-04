/**
 * Build Script for Web Tuner (index.html)
 * Importers/callers: Invoked via Node.js CLI: node tools/tuner/build_tuner.js
 * Affected API: tools/tuner/index.html
 * Data Schemas: GCS Serial Commands, Telemetry $TEL parser, Betaflight Rates, 3D Mesh Arrays
 * User request: "cải thiên lên web tun nữa và phàna ở giữa đồ thi sóng và rc input không cuộn chuột được", "thêm chức năng phóng to drone để quat sát tốt hơn", "thêm cả các chức năng để tun drone 1 cách chuyên nghiệp hơn"
 */

const fs = require('fs');
const path = require('path');

const indexPath = path.join(__dirname, 'index.html');
const currentHtml = fs.readFileSync(indexPath, 'utf8');

// Extract DRONE_V, DRONE_F, DRONE_FN arrays from current HTML
const vMatch = currentHtml.match(/const DRONE_V\s*=\s*new Float32Array\(\[[\s\S]*?\]\);/);
const fMatch = currentHtml.match(/const DRONE_F\s*=\s*new Uint16Array\(\[[\s\S]*?\]\);/);
const fnMatch = currentHtml.match(/const DRONE_FN\s*=\s*new Float32Array\(\[[\s\S]*?\]\);/);

if (!vMatch || !fMatch || !fnMatch) {
    console.error("Could not extract DRONE 3D model arrays!");
    process.exit(1);
}

const droneModelData = `
        // === DRONE 3D MODEL DATA (embedded from drone3d.obj) ===
        ${vMatch[0]}
        ${fMatch[0]}
        ${fnMatch[0]}
`;

console.log("Extracted model data successfully. Model size:", droneModelData.length);

const generatedHtml = `<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-S3 Pro Drone GCS & Flight Dynamics Tuner</title>
    <style>
        :root {
            --bg-base: #060913;
            --bg-panel: #0b1120;
            --bg-card: #111a2e;
            --bg-input: #080d1a;
            --bg-hover: #1e293b;
            --border: #1e293b;
            --border-bright: #334155;

            --cyan: #00f3ff;
            --cyan-glow: rgba(0, 243, 255, 0.25);
            --blue: #38bdf8;
            --green: #10b981;
            --green-glow: rgba(16, 185, 129, 0.25);
            --amber: #f59e0b;
            --amber-glow: rgba(245, 158, 11, 0.25);
            --red: #ef4444;
            --red-glow: rgba(239, 68, 68, 0.35);
            --purple: #a855f7;
            --purple-glow: rgba(168, 85, 247, 0.25);

            --text: #f8fafc;
            --text-muted: #94a3b8;
            --text-dim: #64748b;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'SF Pro Display', Helvetica, Arial, sans-serif;
        }

        html, body {
            background-color: var(--bg-base);
            color: var(--text);
            height: 100vh;
            width: 100vw;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            user-select: none;
        }

        /* --- Header Navigation Bar --- */
        header {
            background: linear-gradient(180deg, #10172a 0%, var(--bg-panel) 100%);
            border-bottom: 1px solid var(--border);
            padding: 8px 14px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            height: 52px;
            flex-shrink: 0;
            z-index: 10;
        }

        .brand-box {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .brand-logo {
            width: 34px;
            height: 34px;
            background: linear-gradient(135deg, #0284c7, var(--cyan));
            border-radius: 7px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 18px;
            box-shadow: 0 0 10px var(--cyan-glow);
        }

        .brand-title {
            font-size: 13px;
            font-weight: 800;
            letter-spacing: 0.6px;
            background: linear-gradient(90deg, #fff, var(--cyan));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .brand-sub {
            font-size: 10px;
            color: var(--text-muted);
        }

        .header-actions {
            display: flex;
            align-items: center;
            gap: 6px;
        }

        /* --- Button System --- */
        .btn {
            background: var(--bg-card);
            color: var(--text);
            border: 1px solid var(--border-bright);
            padding: 5px 10px;
            border-radius: 5px;
            font-size: 11px;
            font-weight: 600;
            cursor: pointer;
            display: inline-flex;
            align-items: center;
            gap: 5px;
            transition: all 0.15s ease;
        }

        .btn:hover:not(:disabled) {
            background: var(--bg-hover);
            border-color: var(--cyan);
            box-shadow: 0 0 8px var(--cyan-glow);
        }

        .btn:active:not(:disabled) {
            transform: scale(0.98);
        }

        .btn:disabled {
            opacity: 0.4;
            cursor: not-allowed;
        }

        .btn-primary {
            background: linear-gradient(135deg, #0284c7, #0369a1);
            border-color: #38bdf8;
            color: white;
        }
        .btn-primary:hover:not(:disabled) {
            background: linear-gradient(135deg, #0369a1, #0284c7);
            box-shadow: 0 0 10px var(--cyan-glow);
        }

        .btn-success {
            background: linear-gradient(135deg, #059669, #047857);
            border-color: #34d399;
            color: white;
        }
        .btn-success:hover:not(:disabled) {
            box-shadow: 0 0 10px var(--green-glow);
        }

        .btn-danger {
            background: linear-gradient(135deg, #dc2626, #b91c1c);
            border-color: #f87171;
            color: white;
        }
        .btn-danger:hover:not(:disabled) {
            box-shadow: 0 0 12px var(--red-glow);
        }

        .btn-warning {
            background: linear-gradient(135deg, #d97706, #b45309);
            border-color: #fbbf24;
            color: white;
        }

        .btn-purple {
            background: linear-gradient(135deg, #7e22ce, #6b21a8);
            border-color: #c084fc;
            color: white;
        }
        .btn-purple:hover:not(:disabled) {
            box-shadow: 0 0 10px var(--purple-glow);
        }

        .status-pill {
            display: inline-flex;
            align-items: center;
            gap: 5px;
            padding: 3px 8px;
            border-radius: 20px;
            font-size: 10px;
            font-weight: 700;
            background: rgba(239, 68, 68, 0.12);
            color: var(--red);
            border: 1px solid rgba(239, 68, 68, 0.3);
        }

        .status-pill.connected {
            background: rgba(16, 185, 129, 0.12);
            color: var(--green);
            border-color: rgba(16, 185, 129, 0.3);
        }

        .status-dot {
            width: 6px;
            height: 6px;
            border-radius: 50%;
            background: currentColor;
            box-shadow: 0 0 6px currentColor;
        }

        /* --- Workspace Grid with Seamless Column Scrolling --- */
        .app-layout {
            display: grid;
            grid-template-columns: 350px 1fr 390px;
            gap: 10px;
            padding: 10px;
            flex: 1;
            min-height: 0;
            overflow: hidden;
        }

        .col {
            display: flex;
            flex-direction: column;
            gap: 10px;
            min-height: 0;
            height: 100%;
        }

        /* Essential scroll fix for middle and side columns */
        .col.scrollable {
            overflow-y: auto !important;
            overflow-x: hidden;
            padding-right: 5px;
            scroll-behavior: smooth;
        }

        .col.scrollable::-webkit-scrollbar {
            width: 5px;
        }
        .col.scrollable::-webkit-scrollbar-track {
            background: rgba(10, 15, 29, 0.5);
            border-radius: 4px;
        }
        .col.scrollable::-webkit-scrollbar-thumb {
            background: var(--border-bright);
            border-radius: 4px;
        }
        .col.scrollable::-webkit-scrollbar-thumb:hover {
            background: var(--cyan);
        }

        .panel {
            background: var(--bg-panel);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 9px 11px;
            display: flex;
            flex-direction: column;
            position: relative;
            flex-shrink: 0;
        }

        .panel-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 7px;
            padding-bottom: 5px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.06);
            font-size: 11px;
            font-weight: 800;
            text-transform: uppercase;
            letter-spacing: 0.6px;
            color: var(--text-muted);
        }

        .tag {
            font-size: 9px;
            padding: 2px 5px;
            border-radius: 3px;
            font-weight: 700;
            font-family: monospace;
        }
        .tag-cyan { background: rgba(0, 243, 255, 0.15); color: var(--cyan); border: 1px solid rgba(0, 243, 255, 0.3); }
        .tag-green { background: rgba(16, 185, 129, 0.15); color: var(--green); border: 1px solid rgba(16, 185, 129, 0.3); }
        .tag-amber { background: rgba(245, 158, 11, 0.15); color: var(--amber); border: 1px solid rgba(245, 158, 11, 0.3); }
        .tag-red { background: rgba(239, 68, 68, 0.15); color: var(--red); border: 1px solid rgba(239, 68, 68, 0.3); }
        .tag-purple { background: rgba(168, 85, 247, 0.15); color: var(--purple); border: 1px solid rgba(168, 85, 247, 0.3); }

        /* --- 3D Attitude Canvas Container & Orbit/Zoom Controls --- */
        #attitudeContainer {
            width: 100%;
            height: 340px;
            background: #03060f;
            border-radius: 6px;
            position: relative;
            overflow: hidden;
            border: 1px solid var(--border);
            cursor: grab;
        }
        #attitudeContainer:active {
            cursor: grabbing;
        }

        #attitudeCanvas {
            width: 100%;
            height: 100%;
            display: block;
        }

        .attitude-toolbar {
            position: absolute;
            top: 6px;
            right: 6px;
            display: flex;
            align-items: center;
            gap: 3px;
            z-index: 5;
            background: rgba(11, 17, 32, 0.90);
            padding: 3px 5px;
            border-radius: 5px;
            border: 1px solid var(--border);
            backdrop-filter: blur(4px);
            max-width: 95%;
            flex-wrap: wrap;
        }

        .tool-btn {
            background: var(--bg-card);
            border: 1px solid var(--border-bright);
            color: var(--text-muted);
            padding: 0 5px;
            min-width: 22px;
            height: 20px;
            border-radius: 3px;
            font-size: 9px;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            font-family: monospace;
            font-weight: bold;
            transition: all 0.15s ease;
        }
        .tool-btn:hover {
            border-color: var(--cyan);
            color: var(--cyan);
            background: var(--bg-hover);
        }
        .tool-btn.active {
            border-color: var(--cyan);
            color: #03060f;
            background: var(--cyan);
            box-shadow: 0 0 8px var(--cyan-glow);
        }

        .hud-mode-group {
            display: inline-flex;
            background: rgba(3, 6, 15, 0.75);
            border: 1px solid var(--border);
            border-radius: 4px;
            padding: 2px;
            gap: 2px;
        }

        .hud-mode-btn {
            background: transparent;
            border: none;
            color: var(--text-muted);
            padding: 2px 7px;
            border-radius: 3px;
            font-size: 10px;
            font-weight: bold;
            cursor: pointer;
            font-family: inherit;
            transition: all 0.15s ease;
            display: flex;
            align-items: center;
            gap: 3px;
        }
        .hud-mode-btn:hover {
            color: var(--cyan);
            background: rgba(0, 243, 255, 0.1);
        }
        .hud-mode-btn.active {
            background: var(--cyan);
            color: #03060f;
            box-shadow: 0 0 8px var(--cyan-glow);
        }

        .metric-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 5px;
            margin-top: 5px;
        }

        .metric-box {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 5px;
            padding: 4px 6px;
            text-align: center;
        }

        .metric-label {
            font-size: 9px;
            color: var(--text-dim);
            text-transform: uppercase;
            font-weight: 700;
        }

        .metric-val {
            font-size: 13px;
            font-weight: 800;
            font-family: 'SF Mono', Consolas, monospace;
            margin-top: 2px;
        }

        /* --- Motor Pulse Micro-Bars --- */
        .motor-bar-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 5px;
            margin-top: 5px;
        }

        .m-box {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 5px;
            padding: 4px 2px;
            text-align: center;
        }
        .m-box-title {
            font-size: 9px;
            color: var(--text-muted);
            font-weight: bold;
        }
        .m-box-pulse {
            font-size: 11px;
            font-weight: bold;
            color: var(--cyan);
            font-family: monospace;
            margin: 2px 0;
        }
        .m-bar-bg {
            height: 4px;
            background: #080d1a;
            border-radius: 2px;
            overflow: hidden;
            margin-top: 2px;
        }
        .m-bar-fill {
            height: 100%;
            background: var(--cyan);
            width: 0%;
            transition: width 0.05s linear;
        }

        /* --- PID Cards & Controls --- */
        .pid-card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 6px;
            padding: 7px 9px;
            margin-bottom: 6px;
        }

        .pid-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 4px;
        }

        .pid-title {
            font-size: 11px;
            font-weight: 800;
            color: var(--cyan);
        }

        .pid-inputs {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 5px;
        }

        .pid-field {
            display: flex;
            flex-direction: column;
            gap: 2px;
        }

        .pid-field label {
            font-size: 9px;
            color: var(--text-dim);
            font-weight: 700;
        }

        .pid-num-row {
            display: flex;
            align-items: center;
            background: var(--bg-input);
            border: 1px solid var(--border-bright);
            border-radius: 4px;
            overflow: hidden;
        }

        .pid-num-row input {
            width: 100%;
            background: transparent;
            border: none;
            color: var(--text);
            font-family: monospace;
            font-size: 11px;
            font-weight: 700;
            padding: 3px 2px;
            text-align: center;
            outline: none;
        }

        .step-btn {
            background: var(--bg-hover);
            border: none;
            color: var(--text-muted);
            width: 16px;
            font-size: 10px;
            cursor: pointer;
            height: 20px;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .step-btn:hover {
            color: var(--cyan);
            background: #334155;
        }

        /* --- Oscilloscope --- */
        .scope-container {
            width: 100%;
            height: 220px;
            position: relative;
            background: #040711;
            border-radius: 6px;
            overflow: hidden;
            border: 1px solid var(--border);
        }

        #scopeCanvas {
            width: 100%;
            height: 100%;
            display: block;
        }

        .scope-toolbar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 3px 6px;
            background: rgba(11, 17, 32, 0.85);
            border-bottom: 1px solid var(--border);
            font-size: 10px;
        }

        .scope-channels {
            display: flex;
            gap: 7px;
            flex-wrap: wrap;
        }

        .channel-toggle {
            display: flex;
            align-items: center;
            gap: 3px;
            cursor: pointer;
            font-size: 10px;
            color: var(--text-muted);
        }
        .channel-toggle input {
            accent-color: var(--cyan);
            cursor: pointer;
        }

        /* --- Rate Curves & Virtual RC Stick Container --- */
        .rc-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
        }

        .stick-box {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 6px;
            padding: 6px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .stick-pad {
            width: 130px;
            height: 130px;
            background: #070c18;
            border: 1px solid var(--border-bright);
            border-radius: 50%;
            position: relative;
            margin: 4px 0;
            cursor: crosshair;
            touch-action: none;
        }

        .stick-cross-h, .stick-cross-v {
            position: absolute;
            background: rgba(255, 255, 255, 0.08);
        }
        .stick-cross-h { top: 50%; left: 10%; right: 10%; height: 1px; }
        .stick-cross-v { left: 50%; top: 10%; bottom: 10%; width: 1px; }

        .stick-knob {
            width: 22px;
            height: 22px;
            background: linear-gradient(135deg, var(--cyan), #0284c7);
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            box-shadow: 0 0 8px var(--cyan-glow);
            pointer-events: none;
        }

        /* --- Terminal / Console --- */
        .terminal-box {
            height: 130px;
            background: #03060f;
            border-radius: 6px;
            padding: 6px 8px;
            font-family: 'SF Mono', Consolas, 'Courier New', monospace;
            font-size: 11px;
            color: #34d399;
            overflow-y: auto;
            white-space: pre-wrap;
            border: 1px solid var(--border);
            line-height: 1.35;
        }

        .term-input-row {
            display: flex;
            gap: 6px;
            margin-top: 5px;
        }

        .term-input {
            flex: 1;
            background: var(--bg-input);
            border: 1px solid var(--border-bright);
            border-radius: 4px;
            color: white;
            padding: 4px 8px;
            font-family: monospace;
            font-size: 11px;
            outline: none;
        }
        .term-input:focus {
            border-color: var(--cyan);
        }

        .quick-chips {
            display: flex;
            gap: 4px;
            flex-wrap: wrap;
            margin-top: 4px;
        }
        .chip {
            background: var(--bg-card);
            border: 1px solid var(--border);
            color: var(--text-muted);
            padding: 2px 5px;
            border-radius: 3px;
            font-size: 9px;
            font-family: monospace;
            cursor: pointer;
        }
        .chip:hover {
            color: var(--cyan);
            border-color: var(--cyan);
        }

        /* Toggle Switch UI */
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 34px;
            height: 18px;
        }
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider-switch {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #334155;
            transition: .25s;
            border-radius: 18px;
        }
        .slider-switch:before {
            position: absolute;
            content: "";
            height: 12px;
            width: 12px;
            left: 3px;
            bottom: 3px;
            background-color: white;
            transition: .25s;
            border-radius: 50%;
        }
        input:checked + .slider-switch {
            background-color: var(--cyan);
            box-shadow: 0 0 8px var(--cyan-glow);
        }
        input:checked + .slider-switch:before {
            transform: translateX(16px);
        }

        /* Modal Popup System */
        .modal-overlay {
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background: rgba(0, 0, 0, 0.82);
            display: none;
            align-items: center;
            justify-content: center;
            z-index: 999;
            backdrop-filter: blur(6px);
        }
        .modal-card {
            background: var(--bg-panel);
            border: 1px solid var(--cyan);
            border-radius: 8px;
            width: 520px;
            max-width: 92vw;
            padding: 14px;
            box-shadow: 0 0 25px rgba(0, 243, 255, 0.25);
        }
        .modal-card-lg {
            background: var(--bg-panel);
            border: 1px solid var(--cyan);
            border-radius: 10px;
            width: 820px;
            max-width: 95vw;
            max-height: 88vh;
            display: flex;
            flex-direction: column;
            padding: 16px;
            box-shadow: 0 0 35px rgba(0, 243, 255, 0.3);
            overflow: hidden;
        }
        .modal-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 12px;
            font-weight: 800;
            color: var(--cyan);
            font-size: 13px;
            padding-bottom: 8px;
            border-bottom: 1px solid var(--border);
        }
        .modal-body-scroll {
            overflow-y: auto;
            overflow-x: hidden;
            flex: 1;
            padding-right: 6px;
        }
        .modal-body-scroll::-webkit-scrollbar {
            width: 5px;
        }
        .modal-body-scroll::-webkit-scrollbar-thumb {
            background: var(--border-bright);
            border-radius: 4px;
        }

        /* --- Tab System for Modals --- */
        .tab-bar {
            display: flex;
            gap: 5px;
            border-bottom: 1px solid var(--border);
            margin-bottom: 12px;
            padding-bottom: 6px;
        }
        .tab-btn {
            background: var(--bg-card);
            border: 1px solid var(--border);
            color: var(--text-muted);
            padding: 5px 10px;
            border-radius: 5px;
            font-size: 11px;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.15s ease;
        }
        .tab-btn:hover {
            background: var(--bg-hover);
            color: var(--text);
            border-color: var(--cyan);
        }
        .tab-btn.active {
            background: rgba(0, 243, 255, 0.15);
            color: var(--cyan);
            border-color: var(--cyan);
            box-shadow: 0 0 8px var(--cyan-glow);
        }
        .tab-pane {
            display: none;
        }
        .tab-pane.active {
            display: block;
        }

        /* --- Guide Callout Boxes & Steps --- */
        .guide-step-card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 6px;
            padding: 10px 12px;
            margin-bottom: 10px;
            position: relative;
        }
        .guide-step-num {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            width: 22px;
            height: 22px;
            border-radius: 50%;
            background: var(--cyan);
            color: #03060f;
            font-weight: 900;
            font-size: 11px;
            margin-right: 6px;
        }
        .guide-callout {
            background: rgba(245, 158, 11, 0.1);
            border-left: 3px solid var(--amber);
            padding: 8px 10px;
            border-radius: 0 4px 4px 0;
            font-size: 11px;
            color: #fde68a;
            margin: 8px 0;
        }
        .guide-callout-info {
            background: rgba(0, 243, 255, 0.08);
            border-left: 3px solid var(--cyan);
            padding: 8px 10px;
            border-radius: 0 4px 4px 0;
            font-size: 11px;
            color: #bae6fd;
            margin: 8px 0;
        }
        .guide-callout-danger {
            background: rgba(239, 68, 68, 0.12);
            border-left: 3px solid var(--red);
            padding: 8px 10px;
            border-radius: 0 4px 4px 0;
            font-size: 11px;
            color: #fca5a5;
            margin: 8px 0;
        }

        /* --- Preset Cards --- */
        .preset-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 8px;
            margin: 8px 0;
        }
        .preset-card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 6px;
            padding: 9px;
            cursor: pointer;
            transition: all 0.2s ease;
        }
        .preset-card:hover {
            border-color: var(--cyan);
            background: var(--bg-hover);
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(0, 243, 255, 0.15);
        }
        .preset-title {
            font-size: 12px;
            font-weight: 800;
            color: var(--cyan);
            margin-bottom: 3px;
        }
        .preset-desc {
            font-size: 10px;
            color: var(--text-muted);
            line-height: 1.3;
            margin-bottom: 6px;
        }
        .preset-pids {
            font-family: monospace;
            font-size: 9px;
            color: var(--text-dim);
            background: #040711;
            padding: 4px;
            border-radius: 3px;
        }

        /* --- Troubleshooting Matrix Table --- */
        .trouble-table {
            width: 100%;
            border-collapse: collapse;
            font-size: 11px;
            margin-top: 6px;
        }
        .trouble-table th {
            background: #090e1a;
            color: var(--cyan);
            text-align: left;
            padding: 6px 8px;
            border: 1px solid var(--border);
            font-weight: 700;
        }
        .trouble-table td {
            padding: 6px 8px;
            border: 1px solid var(--border);
            vertical-align: top;
            color: var(--text-muted);
        }
        .trouble-table tr:nth-child(even) td {
            background: rgba(255, 255, 255, 0.02);
        }

        .code-block {
            background: #050811;
            border: 1px solid var(--border);
            border-radius: 4px;
            padding: 8px;
            color: #e2e8f0;
            font-family: monospace;
            font-size: 10px;
            max-height: 220px;
            overflow-y: auto;
            white-space: pre;
            user-select: text;
        }

        /* Responsive Breakpoints */
        @media (max-width: 1200px) {
            .app-layout { grid-template-columns: 330px 1fr; }
        }
        @media (max-width: 850px) {
            .app-layout { grid-template-columns: 1fr; }
            body { overflow-y: auto; }
            header { flex-wrap: wrap; height: auto; padding: 8px; gap: 6px; }
        }
    </style>
</head>
<body>

    <!-- Header Navigation & Connection -->
    <header>
        <div class="brand-box">
            <div class="brand-logo">🚁</div>
            <div>
                <div class="brand-title">ESP32-S3 PRO FLIGHT GCS</div>
                <div class="brand-sub">Dual-Core 240MHz · 250Hz Cascaded PID · 3D Attitude HUD</div>
            </div>
        </div>

        <div class="header-actions">
            <div id="connStatus" class="status-pill">
                <span class="status-dot"></span>
                <span id="connLabel">DISCONNECTED</span>
            </div>

            <button id="btnConnectSerial" class="btn btn-primary" onclick="toggleSerial()">
                <span>🔌</span> Web Serial
            </button>
            <button id="btnConnectWifi" class="btn btn-purple" onclick="toggleWifi()">
                <span>📡</span> Wi-Fi WS
            </button>
            <button id="btnArm" class="btn btn-danger" onclick="toggleArm()">
                <span>⚠️</span> ARM MOTORS
            </button>
            <button class="btn btn-primary" onclick="openPidGuideModal()" style="background: linear-gradient(135deg, #0284c7, #0891b2); border-color: #00f3ff;">
                <span>📖</span> Hướng Dẫn Tun PID
            </button>
            <button class="btn btn-purple" onclick="openTuningAssistantModal()">
                <span>🎛️</span> Trợ Lý Tun Preset
            </button>
            <button class="btn btn-warning" onclick="sendCalib('GYRO')">
                <span>⚖️</span> Calib Gyro
            </button>
            <button class="btn" onclick="openConfigModal()">
                <span>📋</span> Export Cfg
            </button>
        </div>
    </header>

    <!-- Main Workspace Layout -->
    <div class="app-layout">

        <!-- COLUMN 1: 3D Flight Dynamics & Sensors (Scrollable) -->
        <div class="col scrollable">
            <!-- 3D Drone & PFD Flight Dynamics Panel -->
            <div class="panel">
                <div class="panel-header">
                    <div style="display: flex; align-items: center; gap: 8px; flex-wrap: wrap;">
                        <span>Attitude HUD</span>
                        <div class="hud-mode-group">
                            <button class="hud-mode-btn active" id="modeBtnPfd" onclick="setHudMode('pfd')" title="Chân trời nhân tạo phi công (Aviation PFD / Horizon)">✈️ PFD Chân Trời</button>
                            <button class="hud-mode-btn" id="modeBtn3d" onclick="setHudMode('3d')" title="Mô hình 3D Mesh Drone">🚁 3D Drone</button>
                            <button class="hud-mode-btn" id="modeBtnSplit" onclick="setHudMode('split')" title="Hiển thị song song PFD & 3D">⚡ Song Song</button>
                        </div>
                    </div>
                    <span class="tag tag-cyan" id="fpsTag">60 FPS</span>
                </div>

                <div id="attitudeContainer">
                    <canvas id="attitudeCanvas"></canvas>
                    <div class="attitude-toolbar" id="viewToolbar">
                        <button class="tool-btn active" id="btnViewIso" onclick="setCameraPreset('iso')" title="Góc nhìn Isometric 3/4">ISO</button>
                        <button class="tool-btn" id="btnViewTop" onclick="setCameraPreset('top')" title="Góc nhìn từ trên xuống (Top-Down)">TOP</button>
                        <button class="tool-btn" id="btnViewFpv" onclick="setCameraPreset('fpv')" title="Góc nhìn phi công theo sau đuôi (FPV Chase)">FPV</button>
                        <button class="tool-btn" id="btnViewFront" onclick="setCameraPreset('front')" title="Góc nhìn chính diện (Front)">FRONT</button>
                        <button class="tool-btn" id="btnViewRear" onclick="setCameraPreset('rear')" title="Góc nhìn từ sau (Rear)">REAR</button>
                        <button class="tool-btn" id="btnViewLeft" onclick="setCameraPreset('left')" title="Góc nhìn bên trái (Left)">LEFT</button>
                        <button class="tool-btn" id="btnViewRight" onclick="setCameraPreset('right')" title="Góc nhìn bên phải (Right)">RIGHT</button>
                        <button class="tool-btn" id="btnViewBottom" onclick="setCameraPreset('bottom')" title="Góc nhìn từ dưới bụng lên (Bottom)">BTM</button>
                        <div style="width: 1px; height: 14px; background: var(--border); margin: 0 2px;"></div>
                        <button class="tool-btn" onclick="zoomDrone(1.2)" title="Phóng to Drone (Zoom In)">+</button>
                        <button class="tool-btn" onclick="zoomDrone(0.83)" title="Thu nhỏ Drone (Zoom Out)">-</button>
                        <button class="tool-btn" onclick="resetDroneView()" title="Đặt lại góc nhìn mặc định">⟲</button>
                    </div>
                </div>

                <!-- Orientation Metrics -->
                <div class="metric-grid">
                    <div class="metric-box">
                        <div class="metric-label">Roll (Trục X)</div>
                        <div class="metric-val" id="valRoll" style="color: #38bdf8;">0.0°</div>
                    </div>
                    <div class="metric-box">
                        <div class="metric-label">Pitch (Trục Y)</div>
                        <div class="metric-val" id="valPitch" style="color: #a855f7;">0.0°</div>
                    </div>
                    <div class="metric-box">
                        <div class="metric-label">Yaw (Trục Z)</div>
                        <div class="metric-val" id="valYaw" style="color: #34d399;">0.0°</div>
                    </div>
                </div>
            </div>

            <!-- Environmental & Battery Panel -->
            <div class="panel">
                <div class="panel-header">
                    <span>Barometer & Telemetry</span>
                    <span class="tag tag-green" id="baroTag">BMP280 OK</span>
                </div>

                <div class="metric-grid">
                    <div class="metric-box">
                        <div class="metric-label">Độ cao ước tính</div>
                        <div class="metric-val" id="valAltitude" style="color: #fbbf24;">0.00 m</div>
                    </div>
                    <div class="metric-box">
                        <div class="metric-label">Vận tốc đứng</div>
                        <div class="metric-val" id="valVario" style="color: #38bdf8;">+0.0 m/s</div>
                    </div>
                    <div class="metric-box">
                        <div class="metric-label">Điện áp Pin 3S</div>
                        <div class="metric-val" id="valBatt" style="color: #34d399;">12.4 V</div>
                    </div>
                </div>
            </div>

            <!-- Quad-X Motor Micro-Bars -->
            <div class="panel">
                <div class="panel-header">
                    <span>Motor Pulse Outputs (PCA9685)</span>
                    <span class="tag tag-cyan">50 Hz PWM</span>
                </div>

                <div class="motor-bar-grid">
                    <div class="m-box">
                        <div class="m-box-title">M1 (FR-CCW)</div>
                        <div class="m-box-pulse" id="pM1">1000µs</div>
                        <div class="m-bar-bg"><div id="barM1" class="m-bar-fill"></div></div>
                    </div>
                    <div class="m-box">
                        <div class="m-box-title">M2 (FL-CW)</div>
                        <div class="m-box-pulse" id="pM2">1000µs</div>
                        <div class="m-bar-bg"><div id="barM2" class="m-bar-fill"></div></div>
                    </div>
                    <div class="m-box">
                        <div class="m-box-title">M3 (RR-CW)</div>
                        <div class="m-box-pulse" id="pM3">1000µs</div>
                        <div class="m-bar-bg"><div id="barM3" class="m-bar-fill"></div></div>
                    </div>
                    <div class="m-box">
                        <div class="m-box-title">M4 (RL-CCW)</div>
                        <div class="m-box-pulse" id="pM4">1000µs</div>
                        <div class="m-bar-bg"><div id="barM4" class="m-bar-fill"></div></div>
                    </div>
                </div>
            </div>

            <!-- Hardware Health Summary -->
            <div class="panel">
                <div class="panel-header">
                    <span>Hardware Health Status</span>
                    <button class="btn" style="padding: 2px 6px; font-size: 9px;" onclick="sendCmd('SCAN_I2C')">Scan I2C</button>
                </div>

                <div style="display: flex; justify-content: space-between; font-size: 10px; font-family: monospace; color: var(--text-muted);">
                    <div>MPU6050: <b id="stImu" style="color: #34d399;">OK</b></div>
                    <div>BMP280: <b id="stBaro" style="color: #34d399;">OK</b></div>
                    <div>MAG: <b id="stMag" style="color: #34d399;">OK</b></div>
                    <div>PCA9685: <b id="stPca" style="color: #34d399;">OK</b></div>
                </div>
            </div>
        </div>

        <!-- COLUMN 2: Real-time Analysis & Control (Fully Scrollable Fix) -->
        <div class="col scrollable">
            <!-- Realtime Waveform Oscilloscope -->
            <div class="panel">
                <div class="panel-header">
                    <span>Real-time Waveform Scope</span>
                    <div class="scope-channels">
                        <label class="channel-toggle"><input type="checkbox" id="chRoll" checked> Roll</label>
                        <label class="channel-toggle"><input type="checkbox" id="chPitch" checked> Pitch</label>
                        <label class="channel-toggle"><input type="checkbox" id="chYaw"> Yaw</label>
                        <label class="channel-toggle"><input type="checkbox" id="chGyro"> Gyro</label>
                        <label class="channel-toggle"><input type="checkbox" id="chMotors"> Motors</label>
                    </div>
                </div>

                <div class="scope-container">
                    <canvas id="scopeCanvas"></canvas>
                </div>

                <div class="scope-toolbar">
                    <div>Timebase: <b>50ms/div</b> | Scale: <b>±45°</b></div>
                    <div style="display: flex; gap: 4px;">
                        <button class="btn" style="padding: 2px 6px; font-size: 9px;" onclick="toggleScopePause()" id="btnScopePause">Pause</button>
                        <button class="btn" style="padding: 2px 6px; font-size: 9px;" onclick="clearScope()">Clear</button>
                    </div>
                </div>
            </div>

            <!-- Virtual RC Transmitter & Flight Mode Dispatcher -->
            <div class="panel">
                <div class="panel-header">
                    <span>Virtual RC Transmitter & Mode Control</span>
                    <span class="tag tag-amber" id="modeTag">MODE: ANGLE</span>
                </div>

                <div class="rc-grid">
                    <!-- Left Gimbal: Throttle & Yaw -->
                    <div class="stick-box">
                        <div style="font-size: 10px; font-weight: bold; color: var(--cyan);">THROTTLE / YAW (Mode 2)</div>
                        <div class="stick-pad" id="stickLeft">
                            <div class="stick-cross-h"></div>
                            <div class="stick-cross-v"></div>
                            <div class="stick-knob" id="knobLeft"></div>
                        </div>
                        <div style="display: flex; justify-content: space-between; width: 100%; font-size: 10px; font-family: monospace;">
                            <span>Thr: <b id="valRcThr">0%</b></span>
                            <span>Yaw: <b id="valRcYaw">0°/s</b></span>
                        </div>
                    </div>

                    <!-- Right Gimbal: Roll & Pitch -->
                    <div class="stick-box">
                        <div style="font-size: 10px; font-weight: bold; color: var(--blue);">ROLL / PITCH</div>
                        <div class="stick-pad" id="stickRight">
                            <div class="stick-cross-h"></div>
                            <div class="stick-cross-v"></div>
                            <div class="stick-knob" id="knobRight"></div>
                        </div>
                        <div style="display: flex; justify-content: space-between; width: 100%; font-size: 10px; font-family: monospace;">
                            <span>Roll: <b id="valRcRoll">0.0°</b></span>
                            <span>Pitch: <b id="valRcPitch">0.0°</b></span>
                        </div>
                    </div>
                </div>

                <!-- Flight Mode Selector Buttons -->
                <div style="display: grid; grid-template-columns: repeat(4, 1fr); gap: 4px; margin-top: 8px;">
                    <button class="btn btn-primary" onclick="setFlightMode('ANGLE')">ANGLE (Cân bằng)</button>
                    <button class="btn" onclick="setFlightMode('ACRO')">ACRO (Rate)</button>
                    <button class="btn" onclick="setFlightMode('ALT_HOLD')">ALT HOLD (Giữ cao)</button>
                    <button class="btn" onclick="setFlightMode('POS_HOLD')">POS HOLD (GPS)</button>
                </div>
            </div>

            <!-- Betaflight Rates & Expo Visualizer -->
            <div class="panel">
                <div class="panel-header">
                    <span>Betaflight Rates & Expo Visualizer</span>
                    <span class="tag tag-purple" id="maxRateTag">Max: 667 °/s</span>
                </div>

                <div style="display: grid; grid-template-columns: 1fr 140px; gap: 8px; align-items: center;">
                    <div style="height: 120px; background: #040711; border: 1px solid var(--border); border-radius: 5px; position: relative;">
                        <canvas id="rateCurveCanvas" style="width: 100%; height: 100%; display: block;"></canvas>
                    </div>

                    <div style="display: flex; flex-direction: column; gap: 6px;">
                        <div>
                            <div style="font-size: 9px; color: var(--text-dim); font-weight: bold;">RC RATE: <span id="lblRcRate">1.00</span></div>
                            <input type="range" id="inpRcRate" min="0.2" max="2.5" step="0.05" value="1.00" style="width: 100%;" oninput="updateRatesPreview()">
                        </div>
                        <div>
                            <div style="font-size: 9px; color: var(--text-dim); font-weight: bold;">SUPER RATE: <span id="lblSuperRate">0.70</span></div>
                            <input type="range" id="inpSuperRate" min="0.0" max="0.90" step="0.02" value="0.70" style="width: 100%;" oninput="updateRatesPreview()">
                        </div>
                        <div>
                            <div style="font-size: 9px; color: var(--text-dim); font-weight: bold;">EXPO: <span id="lblExpo">0.15</span></div>
                            <input type="range" id="inpExpo" min="0.0" max="0.80" step="0.05" value="0.15" style="width: 100%;" oninput="updateRatesPreview()">
                        </div>
                        <button class="btn btn-purple" style="width: 100%; justify-content: center; padding: 3px;" onclick="sendRates()">Gửi Rates</button>
                    </div>
                </div>
            </div>

            <!-- Interactive Serial GCS Console -->
            <div class="panel">
                <div class="panel-header">
                    <span>GCS Serial Communication Console</span>
                    <button class="btn" style="padding: 2px 6px; font-size: 9px;" onclick="clearTerm()">Clear</button>
                </div>

                <div id="termBox" class="terminal-box"></div>

                <div class="term-input-row">
                    <input type="text" id="termInput" class="term-input" placeholder="Nhập lệnh GCS (VD: PING, SET AIRMODE 1, GET VERSION)..." onkeydown="handleTermKey(event)">
                    <button class="btn btn-primary" onclick="sendManualTerm()">Gửi</button>
                </div>

                <div class="quick-chips">
                    <span class="chip" onclick="sendCmd('PING')">PING</span>
                    <span class="chip" onclick="sendCmd('GET VERSION')">VERSION</span>
                    <span class="chip" onclick="sendCmd('SET AIRMODE 1')">AIRMODE ON</span>
                    <span class="chip" onclick="sendCmd('SET AIRMODE 0')">AIRMODE OFF</span>
                    <span class="chip" onclick="sendCmd('SET MODE ANGLE')">MODE ANGLE</span>
                    <span class="chip" onclick="sendCmd('SET MODE ACRO')">MODE ACRO</span>
                    <span class="chip" onclick="sendCmd('SCAN_I2C')">SCAN I2C</span>
                </div>
            </div>
        </div>

        <!-- COLUMN 3: Flight Dynamics & Algorithm Pro Tuning (Scrollable) -->
        <div class="col scrollable">
            <!-- Advanced Flight Dynamics Features (AirMode, TPA, Estimator) -->
            <div class="panel">
                <div class="panel-header">
                    <span>Advanced Flight Dynamics Algorithms</span>
                    <span class="tag tag-green">PRO ALGORITHMS</span>
                </div>

                <!-- AirMode Toggle -->
                <div style="display: flex; justify-content: space-between; align-items: center; background: var(--bg-card); padding: 6px 8px; border-radius: 5px; border: 1px solid var(--border); margin-bottom: 6px;">
                    <div>
                        <div style="font-size: 11px; font-weight: bold; color: var(--cyan);">AirMode (Zero-Throttle Stabilization)</div>
                        <div style="font-size: 9px; color: var(--text-dim);">Giữ 100% lực điều khiển bẻ góc khi hạ ga về 0% để nhào lộn</div>
                    </div>
                    <label class="toggle-switch">
                        <input type="checkbox" id="chkAirMode" onchange="toggleAirMode(this.checked)">
                        <span class="slider-switch"></span>
                    </label>
                </div>

                <!-- TPA (Throttle PID Attenuation) -->
                <div style="background: var(--bg-card); padding: 6px 8px; border-radius: 5px; border: 1px solid var(--border); margin-bottom: 6px;">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
                        <div style="font-size: 11px; font-weight: bold; color: var(--amber);">Throttle PID Attenuation (TPA)</div>
                        <button class="btn btn-warning" style="padding: 2px 6px; font-size: 9px;" onclick="sendTpa()">Áp dụng TPA</button>
                    </div>
                    <div style="font-size: 9px; color: var(--text-dim); margin-bottom: 5px;">Tự động giảm PID ở dải ga cao để triệt tiêu dao động cộng hưởng động cơ</div>
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 6px;">
                        <div>
                            <div style="font-size: 9px; color: var(--text-dim); font-weight: bold;">TPA Rate (Giảm %): <span id="lblTpaRate">20%</span></div>
                            <input type="range" id="inpTpaRate" min="0" max="0.7" step="0.05" value="0.20" style="width: 100%;" oninput="document.getElementById('lblTpaRate').innerText=Math.round(this.value*100)+'%'">
                        </div>
                        <div>
                            <div style="font-size: 9px; color: var(--text-dim); font-weight: bold;">Breakpoint (Điểm bắt đầu): <span id="lblTpaBp">50%</span></div>
                            <input type="range" id="inpTpaBp" min="0.2" max="0.8" step="0.05" value="0.50" style="width: 100%;" oninput="document.getElementById('lblTpaBp').innerText=Math.round(this.value*100)+'%'">
                        </div>
                    </div>
                </div>

                <!-- Attitude Estimator Selection (Madgwick vs Mahony) -->
                <div style="background: var(--bg-card); padding: 6px 8px; border-radius: 5px; border: 1px solid var(--border);">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
                        <div style="font-size: 11px; font-weight: bold; color: var(--purple);">Attitude Estimator Fusion</div>
                        <button class="btn btn-purple" style="padding: 2px 6px; font-size: 9px;" onclick="sendEstimator()">Cập nhật AHRS</button>
                    </div>
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 6px; align-items: center;">
                        <div>
                            <label style="font-size: 9px; color: var(--text-dim); font-weight: bold;">Thuật toán:</label>
                            <select id="selEstimator" style="width: 100%; background: var(--bg-input); color: white; border: 1px solid var(--border-bright); border-radius: 4px; padding: 3px; font-size: 10px;">
                                <option value="MADGWICK">Madgwick AHRS (Gradient)</option>
                                <option value="MAHONY">Mahony (Complementary)</option>
                            </select>
                        </div>
                        <div>
                            <div style="font-size: 9px; color: var(--text-dim); font-weight: bold;">Gain (Beta/Kp): <span id="lblGain">0.040</span></div>
                            <input type="range" id="inpGain" min="0.01" max="0.20" step="0.005" value="0.040" style="width: 100%;" oninput="document.getElementById('lblGain').innerText=parseFloat(this.value).toFixed(3)">
                        </div>
                    </div>
                </div>
            </div>

            <!-- Digital Signal Processing & Biquad Filters -->
            <div class="panel">
                <div class="panel-header">
                    <span>DSP Noise Filtering (Biquad Butterworth)</span>
                    <button class="btn btn-primary" style="padding: 2px 6px; font-size: 9px;" onclick="sendFilters()">Gửi Filters</button>
                </div>

                <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 6px;">
                    <div class="pid-field">
                        <label>Gyro LPF (Hz)</label>
                        <input type="number" id="fltGyroLpf" value="90" class="term-input" style="text-align: center;">
                    </div>
                    <div class="pid-field">
                        <label>D-Term LPF (Hz)</label>
                        <input type="number" id="fltDtermLpf" value="70" class="term-input" style="text-align: center;">
                    </div>
                    <div class="pid-field">
                        <label>Notch Center (Hz)</label>
                        <input type="number" id="fltNotch" value="180" class="term-input" style="text-align: center;">
                    </div>
                </div>
            </div>

            <!-- Cascaded Dual-Loop PID Tuning & Assistant -->
            <div class="panel">
                <div class="panel-header">
                    <span>Cascaded PID Flight Tuning</span>
                    <div style="display: flex; gap: 4px;">
                        <button class="btn btn-primary" style="padding: 2px 6px; font-size: 9px;" onclick="openPidGuideModal()">📖 Hướng Dẫn</button>
                        <button class="btn btn-purple" style="padding: 2px 6px; font-size: 9px;" onclick="openTuningAssistantModal()">🎛️ Presets</button>
                        <button class="btn btn-success" style="padding: 2px 6px; font-size: 9px;" onclick="sendAllPids()">Ghi Toàn Bộ PID</button>
                    </div>
                </div>

                <!-- Quick Presets Selector & Master Multiplier Bar -->
                <div style="background: var(--bg-card); border: 1px solid var(--border); border-radius: 6px; padding: 7px 9px; margin-bottom: 6px;">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 5px;">
                        <span style="font-size: 10px; font-weight: 800; color: var(--cyan);">CHỌN CẤU HÌNH PRESET NHANH:</span>
                        <select id="selQuickPreset" style="background: var(--bg-input); color: #fff; border: 1px solid var(--border-bright); border-radius: 4px; padding: 2px 6px; font-size: 10px;" onchange="applyPidPreset(this.value)">
                            <option value="freestyle">5-inch Freestyle / Acro (Default)</option>
                            <option value="cinewhoop">Cinewhoop / Smooth 4S (Mát máy)</option>
                            <option value="longrange">Long Range / 7-inch Cruiser</option>
                            <option value="racer">Racer / High Agility (Gắt)</option>
                            <option value="safe_test">Indoor / Desk Safe Test</option>
                        </select>
                    </div>

                    <!-- Master PID Slider -->
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-top: 4px;">
                        <div>
                            <div style="display: flex; justify-content: space-between; font-size: 9px; color: var(--text-dim); font-weight: bold;">
                                <span>Master Scale (Độ cứng):</span>
                                <span id="lblMasterScale" style="color: var(--cyan);">1.00x</span>
                            </div>
                            <input type="range" id="inpMasterScale" min="0.5" max="1.8" step="0.05" value="1.00" style="width: 100%;" oninput="applyMasterMultiplier(parseFloat(this.value))">
                        </div>
                        <div>
                            <div style="display: flex; justify-content: space-between; font-size: 9px; color: var(--text-dim); font-weight: bold;">
                                <span>P/D Balance (Cân bằng P-D):</span>
                                <span id="lblPdBalance" style="color: var(--purple);">Neutral</span>
                            </div>
                            <input type="range" id="inpPdBalance" min="-0.3" max="0.3" step="0.05" value="0.00" style="width: 100%;" oninput="applyPdBalance(parseFloat(this.value))">
                        </div>
                    </div>
                </div>

                <!-- 1-Click Quick Problem Solvers -->
                <div style="background: #060b17; border: 1px dashed var(--border-bright); border-radius: 6px; padding: 6px 8px; margin-bottom: 6px;">
                    <div style="font-size: 9px; font-weight: 800; color: var(--amber); margin-bottom: 4px;">⚡ BẮT BỆNH & KHẮC PHỤC 1-CHẠM (QUICK DIAGNOSTIC):</div>
                    <div style="display: flex; flex-wrap: wrap; gap: 4px;">
                        <button class="chip" style="color: #fca5a5;" onclick="applyDiagnosticFix('high_freq_shake')" title="Giảm D 15%, hạ D-LPF xuống 70Hz">🔍 Rung cao tần khi ga lớn</button>
                        <button class="chip" style="color: #fde68a;" onclick="applyDiagnosticFix('drift_yaw')" title="Tăng I 20%, tăng P 10%">🔍 Trôi góc / Bồng bềnh</button>
                        <button class="chip" style="color: #cbd5e1;" onclick="applyDiagnosticFix('bounceback')" title="Tăng D 12% để dập tắt dao động nhả cần">🔍 Nảy mũi khi dừng lộn</button>
                        <button class="chip" style="color: #f87171;" onclick="applyDiagnosticFix('hot_motors')" title="Hạ D 25% và giảm D-LPF xuống 60Hz để hạ nhiệt động cơ">🔍 Motor quá nóng</button>
                        <button class="chip" style="color: #67e8f9;" onclick="applyDiagnosticFix('sluggish_stick')" title="Tăng Kff (Feedforward) 25% cho phản hồi bám sát ngón tay">🔍 Phản ứng cần lái trễ</button>
                    </div>
                </div>

                <!-- Outer Angle Loop (Kp Only) -->
                <div class="pid-card">
                    <div class="pid-header">
                        <span class="pid-title">1. Vòng Ngoài: Euler Angle Loop (Tự cân bằng)</span>
                        <span style="font-size: 9px; color: var(--text-dim);">Angle Error → Target Rate</span>
                    </div>
                    <div class="pid-inputs">
                        <div class="pid-field">
                            <label>Roll Angle Kp</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('roll_ang_p', -0.1)">-</button>
                                <input type="number" id="roll_ang_p" value="4.50" step="0.1">
                                <button class="step-btn" onclick="stepPid('roll_ang_p', 0.1)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Pitch Angle Kp</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('pitch_ang_p', -0.1)">-</button>
                                <input type="number" id="pitch_ang_p" value="4.50" step="0.1">
                                <button class="step-btn" onclick="stepPid('pitch_ang_p', 0.1)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Max Rate Limit</label>
                            <div class="pid-num-row">
                                <input type="number" id="max_rate_lim" value="300" step="10" disabled>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Inner Rate Loop: ROLL -->
                <div class="pid-card">
                    <div class="pid-header">
                        <span class="pid-title">2. Vòng Trong: Roll Rate PID (250Hz Gyro Loop)</span>
                        <button class="btn" style="padding: 1px 5px; font-size: 9px;" onclick="sendPidAxis('ROLL')">Gửi Roll</button>
                    </div>
                    <div class="pid-inputs">
                        <div class="pid-field">
                            <label>Kp (Phản ứng tức thì)</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('roll_rate_p', -0.05)">-</button>
                                <input type="number" id="roll_rate_p" value="1.20" step="0.05">
                                <button class="step-btn" onclick="stepPid('roll_rate_p', 0.05)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Ki (Triệt tiêu gió lệch)</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('roll_rate_i', -0.005)">-</button>
                                <input type="number" id="roll_rate_i" value="0.040" step="0.005">
                                <button class="step-btn" onclick="stepPid('roll_rate_i', 0.005)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Kd (Chống lố dao động)</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('roll_rate_d', -0.005)">-</button>
                                <input type="number" id="roll_rate_d" value="0.035" step="0.005">
                                <button class="step-btn" onclick="stepPid('roll_rate_d', 0.005)">+</button>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Inner Rate Loop: PITCH -->
                <div class="pid-card">
                    <div class="pid-header">
                        <span class="pid-title">3. Vòng Trong: Pitch Rate PID</span>
                        <button class="btn" style="padding: 1px 5px; font-size: 9px;" onclick="sendPidAxis('PITCH')">Gửi Pitch</button>
                    </div>
                    <div class="pid-inputs">
                        <div class="pid-field">
                            <label>Kp</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('pitch_rate_p', -0.05)">-</button>
                                <input type="number" id="pitch_rate_p" value="1.20" step="0.05">
                                <button class="step-btn" onclick="stepPid('pitch_rate_p', 0.05)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Ki</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('pitch_rate_i', -0.005)">-</button>
                                <input type="number" id="pitch_rate_i" value="0.040" step="0.005">
                                <button class="step-btn" onclick="stepPid('pitch_rate_i', 0.005)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Kd</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('pitch_rate_d', -0.005)">-</button>
                                <input type="number" id="pitch_rate_d" value="0.035" step="0.005">
                                <button class="step-btn" onclick="stepPid('pitch_rate_d', 0.005)">+</button>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Inner Rate Loop: YAW -->
                <div class="pid-card">
                    <div class="pid-header">
                        <span class="pid-title">4. Vòng Trong: Yaw Rate PID (Mô-men xoắn)</span>
                        <button class="btn" style="padding: 1px 5px; font-size: 9px;" onclick="sendPidAxis('YAW')">Gửi Yaw</button>
                    </div>
                    <div class="pid-inputs">
                        <div class="pid-field">
                            <label>Kp</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('yaw_rate_p', -0.1)">-</button>
                                <input type="number" id="yaw_rate_p" value="2.50" step="0.1">
                                <button class="step-btn" onclick="stepPid('yaw_rate_p', 0.1)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Ki</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('yaw_rate_i', -0.01)">-</button>
                                <input type="number" id="yaw_rate_i" value="0.080" step="0.01">
                                <button class="step-btn" onclick="stepPid('yaw_rate_i', 0.01)">+</button>
                            </div>
                        </div>
                        <div class="pid-field">
                            <label>Kd</label>
                            <div class="pid-num-row">
                                <button class="step-btn" onclick="stepPid('yaw_rate_d', -0.005)">-</button>
                                <input type="number" id="yaw_rate_d" value="0.000" step="0.005">
                                <button class="step-btn" onclick="stepPid('yaw_rate_d', 0.005)">+</button>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Motor Bench Tester & ESC Calibrator -->
            <div class="panel">
                <div class="panel-header">
                    <span>Motor Bench Tester (Bàn Test An Toàn)</span>
                    <span class="tag tag-red">⚠️ THÁO CÁNH QUẠT</span>
                </div>

                <div style="font-size: 9px; color: var(--text-dim); margin-bottom: 6px;">Kéo thanh trượt để quay thử từng động cơ (Tối đa 15% ga). Khi thả chuột, thanh trượt sẽ tự động ngắt về 0%.</div>

                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 6px;">
                    <div>
                        <div style="font-size: 9px; font-weight: bold; color: var(--cyan);">M1 (FR): <span id="lblTestM1">0%</span></div>
                        <input type="range" id="testM1" min="0" max="15" value="0" style="width: 100%;" oninput="runMotorTest(1, this.value)" onmouseup="stopMotorTest(1)" ontouchend="stopMotorTest(1)">
                    </div>
                    <div>
                        <div style="font-size: 9px; font-weight: bold; color: var(--cyan);">M2 (FL): <span id="lblTestM2">0%</span></div>
                        <input type="range" id="testM2" min="0" max="15" value="0" style="width: 100%;" oninput="runMotorTest(2, this.value)" onmouseup="stopMotorTest(2)" ontouchend="stopMotorTest(2)">
                    </div>
                    <div>
                        <div style="font-size: 9px; font-weight: bold; color: var(--cyan);">M3 (RR): <span id="lblTestM3">0%</span></div>
                        <input type="range" id="testM3" min="0" max="15" value="0" style="width: 100%;" oninput="runMotorTest(3, this.value)" onmouseup="stopMotorTest(3)" ontouchend="stopMotorTest(3)">
                    </div>
                    <div>
                        <div style="font-size: 9px; font-weight: bold; color: var(--cyan);">M4 (RL): <span id="lblTestM4">0%</span></div>
                        <input type="range" id="testM4" min="0" max="15" value="0" style="width: 100%;" oninput="runMotorTest(4, this.value)" onmouseup="stopMotorTest(4)" ontouchend="stopMotorTest(4)">
                    </div>
                </div>
            </div>
        </div>

    </div>

    <!-- Modal 1: PID Tuning Handbook & Guide (Sổ Tay Hướng Dẫn Chi Tiết) -->
    <div id="pidGuideModal" class="modal-overlay">
        <div class="modal-card-lg">
            <div class="modal-header">
                <div style="display: flex; align-items: center; gap: 8px;">
                    <span style="font-size: 16px;">📖</span>
                    <span>SỔ TAY & HƯỚNG DẪN TUN PID DRONE (CHUẨN CHUYÊN NGHIỆP)</span>
                </div>
                <button class="btn" onclick="closePidGuideModal()">✕</button>
            </div>

            <!-- Tab Bar -->
            <div class="tab-bar">
                <button class="tab-btn active" onclick="switchPidGuideTab('tabSteps')">🌟 Quy Trình 6 Bước Chuẩn</button>
                <button class="tab-btn" onclick="switchPidGuideTab('tabTrouble')">📊 Ma Trận Bắt Bệnh & Khắc Phục</button>
                <button class="tab-btn" onclick="switchPidGuideTab('tabMath')">📐 Ý Nghĩa Tham Số & Công Thức</button>
                <button class="tab-btn" onclick="switchPidGuideTab('tabSafety')">🛡️ Cảnh Báo An Toàn & Test Bench</button>
            </div>

            <!-- Tab Contents -->
            <div class="modal-body-scroll">
                <!-- TAB 1: QUY TRÌNH 6 BƯỚC CHUẨN -->
                <div id="tabSteps" class="tab-pane active">
                    <div class="guide-callout-info">
                        💡 <b>Nguyên lý cốt lõi:</b> Drone sử dụng <b>Cascaded PID (2 vòng lặp lồng nhau)</b>. Luôn luôn tinh chỉnh <b>Vòng trong (Rate Gyro Loop - 250Hz)</b> trước, sau đó mới tinh chỉnh <b>Vòng ngoài (Angle Level Loop - 50Hz)</b>.
                    </div>

                    <div class="guide-step-card">
                        <div style="display: flex; align-items: center; margin-bottom: 6px;">
                            <span class="guide-step-num">1</span>
                            <span style="font-size: 12px; font-weight: 800; color: var(--cyan);">Chuẩn bị phần cứng & Hiệu chuẩn mặt phẳng</span>
                        </div>
                        <ul style="padding-left: 20px; font-size: 11px; line-height: 1.5; color: var(--text-muted);">
                            <li><b>Tháo toàn bộ cánh quạt</b> khi kết nối máy tính hoặc kiểm tra động cơ trên bàn test.</li>
                            <li>Đảm bảo trọng tâm (CG) của Pin LiPo nằm chính xác tại điểm giao thoa giữa 4 động cơ.</li>
                            <li>Đặt drone lên mặt bàn phẳng tuyệt đối và bấm <b>[⚖️ Calib Gyro]</b> để triệt tiêu độ lệch cảm biến IMU.</li>
                        </ul>
                    </div>

                    <div class="guide-step-card">
                        <div style="display: flex; align-items: center; margin-bottom: 6px;">
                            <span class="guide-step-num">2</span>
                            <span style="font-size: 12px; font-weight: 800; color: var(--cyan);">Tune Rate P (Hệ số tỷ lệ Kp - Độ nhạy phản xạ)</span>
                        </div>
                        <ul style="padding-left: 20px; font-size: 11px; line-height: 1.5; color: var(--text-muted);">
                            <li>Bắt đầu với thông số an toàn: <code>Roll/Pitch Rate Kp = 0.90</code>, <code>Ki = 0.030</code>, <code>Kd = 0.020</code>.</li>
                            <li>Bay thử và tăng dần Kp mỗi lần <b>+0.10</b> cho đến khi cảm thấy drone giữ góc đầm và phản ứng bám sát theo cần lái.</li>
                            <li><b>Dấu hiệu Kp quá cao:</b> Drone xuất hiện hiện tượng rung giật nhanh li ti (High-frequency oscillation) khi bay lướt. Khi thấy dấu hiệu này, hãy <b>hạ Kp xuống 10% - 15%</b>.</li>
                        </ul>
                    </div>

                    <div class="guide-step-card">
                        <div style="display: flex; align-items: center; margin-bottom: 6px;">
                            <span class="guide-step-num">3</span>
                            <span style="font-size: 12px; font-weight: 800; color: var(--cyan);">Tune Rate D (Hệ số vi phân Kd - Giảm chấn & Chống quá đà)</span>
                        </div>
                        <ul style="padding-left: 20px; font-size: 11px; line-height: 1.5; color: var(--text-muted);">
                            <li>Tăng dần Kd mỗi lần <b>+0.005</b>. Mục tiêu của D là dập tắt dao động vượt lố (Overshoot) và chống nảy ngược mũi (Bounceback) khi thả mạnh cần lái về vị trí giữa.</li>
                            <li><b style="color: var(--amber);">CẢNH BÁO QUAN TRỌNG:</b> Kd càng cao sẽ càng khuếch đại nhiễu từ động cơ, làm motor bị nóng rát. Sau mỗi lần bay test 30 giây, hãy đáp xuống và dùng tay chạm vào 4 motor để kiểm tra nhiệt độ (nếu ấm nhẹ &lt; 50°C là an toàn).</li>
                        </ul>
                    </div>

                    <div class="guide-step-card">
                        <div style="display: flex; align-items: center; margin-bottom: 6px;">
                            <span class="guide-step-num">4</span>
                            <span style="font-size: 12px; font-weight: 800; color: var(--cyan);">Tune Rate I (Hệ số tích phân Ki - Chống trôi góc & Giữ hướng)</span>
                        </div>
                        <ul style="padding-left: 20px; font-size: 11px; line-height: 1.5; color: var(--text-muted);">
                            <li>Tăng Ki mỗi lần <b>+0.005</b> (thông thường từ <code>0.035 - 0.060</code>).</li>
                            <li>Ki giúp drone giữ nguyên góc nghiêng chính xác tuyệt đối khi bạn buông cần lái, không bị gió tạt làm đổi hướng hoặc bị lún mũi khi đổi tốc độ.</li>
                        </ul>
                    </div>

                    <div class="guide-step-card">
                        <div style="display: flex; align-items: center; margin-bottom: 6px;">
                            <span class="guide-step-num">5</span>
                            <span style="font-size: 12px; font-weight: 800; color: var(--cyan);">Tune Trục Yaw (Kp, Ki) & Feedforward (Kff)</span>
                        </div>
                        <ul style="padding-left: 20px; font-size: 11px; line-height: 1.5; color: var(--text-muted);">
                            <li>Trục Yaw điều khiển bằng mô-men xoắn phản lực (Torque reaction) của 4 cánh quạt ngược chiều nhau, do đó không có lực đẩy trực tiếp: <code>Yaw Kp = 2.0 - 3.2</code>, <code>Ki = 0.06 - 0.10</code>, và <code>Kd = 0.00</code>.</li>
                            <li>Nếu cảm thấy drone bị trễ 1 nhịp khi bạn gạt nhanh cần lái $\to$ Tăng hệ số <b>Feedforward (Kff)</b> lên để drone bắt kịp phản xạ ngón tay tức thời.</li>
                        </ul>
                    </div>

                    <div class="guide-step-card">
                        <div style="display: flex; align-items: center; margin-bottom: 6px;">
                            <span class="guide-step-num">6</span>
                            <span style="font-size: 12px; font-weight: 800; color: var(--cyan);">Tinh chỉnh TPA và Vòng ngoài Angle Kp</span>
                        </div>
                        <ul style="padding-left: 20px; font-size: 11px; line-height: 1.5; color: var(--text-muted);">
                            <li><b>TPA (Throttle PID Attenuation):</b> Nếu drone bay ở ga thấp rất êm nhưng khi ép ga 70% - 100% bị rần rần rung lắc $\to$ Kích hoạt TPA với Rate = 25%, Breakpoint = 50%.</li>
                            <li><b>Angle Kp:</b> Khi chuyển sang chế độ bay tự cân bằng (ANGLE Mode), điều chỉnh <code>Roll/Pitch Angle Kp = 4.0 - 5.5</code> để drone tự động trả về vị trí cân bằng phẳng một cách mượt mà và dứt khoát.</li>
                        </ul>
                    </div>
                </div>

                <!-- TAB 2: MA TRẬN BẮT BỆNH & KHẮC PHỤC SỰ CỐ -->
                <div id="tabTrouble" class="tab-pane">
                    <div class="guide-callout">
                        ⚠️ <b>Bảng chẩn đoán triệu chứng:</b> Tra cứu nhanh các hiện tượng bất thường khi bay và biện pháp xử lý chuẩn xác.
                    </div>

                    <table class="trouble-table">
                        <thead>
                            <tr>
                                <th style="width: 28%;">Hiện tượng thực tế</th>
                                <th style="width: 32%;">Nguyên nhân cốt lõi</th>
                                <th style="width: 40%;">Cách khắc phục cụ thể</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                <td style="color: #fca5a5; font-weight: bold;">Rung cao tần (Fast Shaking)<br><span style="font-size: 9px; color: var(--text-dim);">Tiếng motor rít chói tai</span></td>
                                <td>Hệ số <b>Kd quá cao</b> hoặc <b>Kp quá cao</b>; nhiễu cơ học từ động cơ lọt vào Gyro.</td>
                                <td>1. Giảm <b>Kd</b> xuống 15% - 20%.<br>2. Giảm <b>Kp</b> xuống 10%.<br>3. Hạ tần số <b>D-Term LPF</b> từ 90Hz xuống 70Hz.</td>
                            </tr>
                            <tr>
                                <td style="color: #fde68a; font-weight: bold;">Bồng bềnh / Lắc lư chậm (Slow Wobble)<br><span style="font-size: 9px; color: var(--text-dim);">Drone lảo đảo như say sóng</span></td>
                                <td>Hệ số <b>Kp quá thấp</b> (thiếu lực phản ứng) hoặc <b>Ki quá thấp</b> (thiếu lực giữ góc).</td>
                                <td>1. Tăng <b>Rate Kp</b> thêm +0.15 đến +0.25.<br>2. Tăng <b>Rate Ki</b> thêm +0.010.</td>
                            </tr>
                            <tr>
                                <td style="color: #93c5fd; font-weight: bold;">Nảy ngược mũi (Bounceback)<br><span style="font-size: 9px; color: var(--text-dim);">Sau khi lộn nhào và nhả cần</span></td>
                                <td>Thiếu lực cản giảm chấn <b>Kd</b> hoặc tỷ lệ P/D bị lệch.</td>
                                <td>1. Tăng <b>Kd</b> thêm +0.005.<br>2. Nếu Kp đang quá cao, hãy hạ nhẹ Kp xuống 5%.</td>
                            </tr>
                            <tr>
                                <td style="color: #f87171; font-weight: bold;">Động cơ quá nóng (&gt; 65°C)<br><span style="font-size: 9px; color: var(--text-dim);">Có mùi khét nhẹ sau 1 phút bay</span></td>
                                <td>Kd quá cao hoặc bộ lọc DSP chưa lọc hết rung động cơ khí của cánh quạt.</td>
                                <td>1. <b>Hạ ngay Kd xuống 25% - 30%</b>.<br>2. Giảm tần số cắt <b>D-Term LPF</b> xuống 60Hz.<br>3. Cân bằng lại cánh quạt (hoặc thay cánh mới).</td>
                            </tr>
                            <tr>
                                <td style="color: #fbbf24; font-weight: bold;">Rung khi ép ga tối đa (Full Throttle)<br><span style="font-size: 9px; color: var(--text-dim);">Ga &gt; 70% bị rần rần</span></td>
                                <td>Lực đẩy cực đại làm tăng độ nhạy vòng lặp kín (Loop Gain saturation).</td>
                                <td>1. Bật tính năng <b>TPA (Throttle PID Attenuation)</b>.<br>2. Đặt <b>TPA Rate = 0.25 - 0.35</b>, <b>Breakpoint = 0.45 - 0.50</b>.</td>
                            </tr>
                            <tr>
                                <td style="color: #c084fc; font-weight: bold;">Lún mũi khi tăng ga đột ngột (Punchout Dip)</td>
                                <td>Pin sụt áp tức thời và mất lực giữ góc khi thay đổi ga lớn.</td>
                                <td>1. Bật chế độ <b>AirMode ON</b>.<br>2. Tăng <b>Rate Ki</b> thêm +0.010.</td>
                            </tr>
                            <tr>
                                <td style="color: #67e8f9; font-weight: bold;">Phản hồi cần lái bị trễ (Sluggish)</td>
                                <td>Thiếu thành phần bù trước gia tốc (Feedforward).</td>
                                <td>1. Tăng hệ số <b>Feedforward (Kff)</b> từ 0.00 lên <code>0.40 - 0.80</code>.</td>
                            </tr>
                        </tbody>
                    </table>
                </div>

                <!-- TAB 3: Ý NGHĨA THAM SỐ & KIẾN TRÚC TOÁN HỌC -->
                <div id="tabMath" class="tab-pane">
                    <div class="guide-callout-info">
                        📐 <b>Kiến trúc điều khiển Cascaded 2-Loop:</b>
                    </div>

                    <div style="background: #040711; border: 1px solid var(--border); border-radius: 6px; padding: 10px; font-size: 11px; font-family: monospace; color: #e2e8f0; line-height: 1.6; margin-bottom: 10px;">
                        [Người lái / Cần RC] → [Vòng ngoài Euler Angle (50Hz)] → Sinh ra Target Angular Rate (°/s)<br>
                        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;↓<br>
                        [IMU Gyro MPU6050] &nbsp;→ [Vòng trong Gyro Rate PID (250Hz)] → [P + I + D + FF] × TPA Factor<br>
                        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;↓<br>
                        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Quad-X Motor Mixer] → 4x ESC PCA9685 (PWM 50Hz)
                    </div>

                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 8px;">
                        <div class="guide-step-card">
                            <b style="color: var(--cyan);">P (Proportional - Tỷ lệ):</b>
                            <div style="font-size: 10px; color: var(--text-muted); margin-top: 4px;">
                                $P_{term} = K_p \times e(t)$<br>
                                Tạo ra mô-men phản kháng ngay tức thì tỷ lệ với sai số góc. P càng cao, drone càng cứng cáp và phản ứng nhanh.
                            </div>
                        </div>

                        <div class="guide-step-card">
                            <b style="color: var(--green);">I (Integral - Tích phân):</b>
                            <div style="font-size: 10px; color: var(--text-muted); margin-top: 4px;">
                                $I_{term} = K_i \int e(t) dt$<br>
                                Tích lũy sai số theo thời gian để triệt tiêu hoàn toàn sai số xác lập, giúp chống trôi góc khi có gió hoặc lệch trọng tâm.
                            </div>
                        </div>

                        <div class="guide-step-card">
                            <b style="color: var(--amber);">D (Derivative on Measurement - Vi phân):</b>
                            <div style="font-size: 10px; color: var(--text-muted); margin-top: 4px;">
                                $D_{term} = -K_d \times \frac{d(Gyro)}{dt}$<br>
                                Đo vận tốc biến thiên của cảm biến để phanh hãm dao động quán tính, chống nảy ngược và dập tắt overshoot.
                            </div>
                        </div>

                        <div class="guide-step-card">
                            <b style="color: var(--purple);">Feedforward (Kff - Bù trước quán tính):</b>
                            <div style="font-size: 10px; color: var(--text-muted); margin-top: 4px;">
                                $FF_{term} = K_{ff} \times \frac{d(Setpoint)}{dt}$<br>
                                Bơm thêm lực tức thời ngay khi cần lái di chuyển, loại bỏ độ trễ phản hồi của vòng lặp kín thông thường.
                            </div>
                        </div>
                    </div>
                </div>

                <!-- TAB 4: CẢNH BÁO AN TOÀN & TEST BENCH -->
                <div id="tabSafety" class="tab-pane">
                    <div class="guide-callout-danger">
                        🚨 <b>QUY TẮC AN TOÀN SỐNG CÒN KHI LÀM VIỆC VỚI QUADCOPTER:</b>
                    </div>

                    <div class="guide-step-card">
                        <ol style="padding-left: 20px; font-size: 11px; line-height: 1.7; color: #f8fafc;">
                            <li><b style="color: #f87171;">LUÔN THÁO CÁNH QUẠT</b> khi cắm cáp USB vào máy tính để nạp code hoặc điều chỉnh PID trên bàn làm việc.</li>
                            <li>Không bao giờ đứng trong bán kính quay của cánh quạt khi cắm nguồn Pin LiPo.</li>
                            <li>Luôn cài đặt công tắc <b>NGẮT KHẨN CẤP (DISARM)</b> trên tay cầm điều khiển hoặc sử dụng phím <code>Space</code> trên giao diện Web GCS.</li>
                            <li>Đảm bảo thứ tự và chiều quay động cơ Quad-X:
                                <br>&nbsp;&nbsp;• <b>M1 (Front-Right)</b>: Quay ngược chiều kim đồng hồ (CCW).
                                <br>&nbsp;&nbsp;• <b>M2 (Front-Left)</b>: Quay thuận chiều kim đồng hồ (CW).
                                <br>&nbsp;&nbsp;• <b>M3 (Rear-Right)</b>: Quay thuận chiều kim đồng hồ (CW).
                                <br>&nbsp;&nbsp;• <b>M4 (Rear-Left)</b>: Quay ngược chiều kim đồng hồ (CCW).
                            </li>
                            <li>Khi bay thử nghiệm lần đầu ngoài trời: chọn không gian thoáng đãng (bãi cỏ), không có người và vật cản trong bán kính 10 mét.</li>
                        </ol>
                    </div>
                </div>
            </div>

            <div style="display: flex; justify-content: flex-end; gap: 6px; margin-top: 10px; padding-top: 8px; border-top: 1px solid var(--border);">
                <button class="btn btn-primary" onclick="closePidGuideModal()">Đã Hiểu & Đóng Hướng Dẫn</button>
            </div>
        </div>
    </div>

    <!-- Modal 2: Tuning Assistant & Presets (Trợ Lý Tun Nâng Cao) -->
    <div id="tuningAssistantModal" class="modal-overlay">
        <div class="modal-card-lg">
            <div class="modal-header">
                <div style="display: flex; align-items: center; gap: 8px;">
                    <span style="font-size: 16px;">🎛️</span>
                    <span>TRỢ LÝ TINH CHỈNH PID & BỘ CẤU HÌNH PRESETS MẪU</span>
                </div>
                <button class="btn" onclick="closeTuningAssistantModal()">✕</button>
            </div>

            <div class="modal-body-scroll">
                <div class="guide-callout-info">
                    ✨ Chọn một cấu hình Preset phù hợp với kích thước khung drone và phong cách bay của bạn. Các tham số PID, TPA, Filter và Rates sẽ tự động được điền và tối ưu hóa.
                </div>

                <div class="preset-grid">
                    <!-- Preset 1: Freestyle -->
                    <div class="preset-card" onclick="applyPidPreset('freestyle')">
                        <div class="preset-title">🚁 1. 5-inch Freestyle / Acro (Default)</div>
                        <div class="preset-desc">Cân bằng hoàn hảo giữa độ nhạy phản xạ và độ êm ái. Phù hợp cho nhào lộn, bay tự do và cản gió tốt.</div>
                        <div class="preset-pids">
                            Roll/Pitch: P=1.20 | I=0.040 | D=0.035<br>
                            Yaw: P=2.50 | I=0.080 | D=0.000<br>
                            TPA: 20% @ 50% ga | Angle Kp: 4.50
                        </div>
                    </div>

                    <!-- Preset 2: Cinewhoop -->
                    <div class="preset-card" onclick="applyPidPreset('cinewhoop')">
                        <div class="preset-title">🎬 2. Cinewhoop / Smooth 4S (Mát máy)</div>
                        <div class="preset-desc">Dành cho drone quay phim mượt mà, hạ thấp D để motor không bị nóng, P êm ái chống rung camera.</div>
                        <div class="preset-pids">
                            Roll/Pitch: P=0.95 | I=0.035 | D=0.022<br>
                            Yaw: P=2.00 | I=0.060 | D=0.000<br>
                            TPA: 30% @ 45% ga | Angle Kp: 3.80
                        </div>
                    </div>

                    <!-- Preset 3: Long Range -->
                    <div class="preset-card" onclick="applyPidPreset('longrange')">
                        <div class="preset-title">🏔️ 3. Long Range / 7-inch Cruiser</div>
                        <div class="preset-desc">Dành cho drone khung lớn, tải nặng. Tăng I-term để chống gió giật mạnh, giữ đường bay thẳng tắp.</div>
                        <div class="preset-pids">
                            Roll/Pitch: P=1.45 | I=0.055 | D=0.045<br>
                            Yaw: P=3.00 | I=0.100 | D=0.000<br>
                            TPA: 25% @ 40% ga | Angle Kp: 5.20
                        </div>
                    </div>

                    <!-- Preset 4: Racer -->
                    <div class="preset-card" onclick="applyPidPreset('racer')">
                        <div class="preset-title">⚡ 4. Racer / High Agility (Gắt)</div>
                        <div class="preset-desc">Tốc độ phản xạ cực nhanh, ôm cua góc hẹp chính xác tuyệt đối. Đòi hỏi motor và cánh quạt chất lượng cao.</div>
                        <div class="preset-pids">
                            Roll/Pitch: P=1.60 | I=0.045 | D=0.040<br>
                            Yaw: P=3.20 | I=0.090 | D=0.000<br>
                            TPA: 35% @ 55% ga | Angle Kp: 5.50
                        </div>
                    </div>

                    <!-- Preset 5: Desk Test Safe -->
                    <div class="preset-card" onclick="applyPidPreset('safe_test')">
                        <div class="preset-title">🛡️ 5. Indoor / Desk Safe Test</div>
                        <div class="preset-desc">Thông số cực kỳ dịu dàng, an toàn tuyệt đối cho người mới bắt đầu bay thử nghiệm trong phòng hoặc test bàn.</div>
                        <div class="preset-pids">
                            Roll/Pitch: P=0.70 | I=0.025 | D=0.015<br>
                            Yaw: P=1.50 | I=0.040 | D=0.000<br>
                            TPA: 10% @ 60% ga | Angle Kp: 3.00
                        </div>
                    </div>
                </div>

                <div class="guide-step-card" style="margin-top: 10px;">
                    <div style="font-size: 11px; font-weight: bold; color: var(--cyan); margin-bottom: 6px;">🎛️ BỘ ĐIỀU KHIỂN TỔNG THỂ (MASTER MULTIPLIERS):</div>
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;">
                        <div>
                            <div style="display: flex; justify-content: space-between; font-size: 10px; color: var(--text-muted); font-weight: bold;">
                                <span>Master PID Scale (Tỷ lệ nhân tổng):</span>
                                <span id="lblAssistantMasterScale" style="color: var(--cyan); font-weight: 800;">1.00x</span>
                            </div>
                            <input type="range" id="inpAssistantMasterScale" min="0.5" max="1.8" step="0.05" value="1.00" style="width: 100%; margin-top: 4px;" oninput="applyMasterMultiplier(parseFloat(this.value))">
                            <div style="font-size: 9px; color: var(--text-dim); margin-top: 2px;">Tăng/giảm toàn bộ độ cứng của drone mà không làm lệch tỷ lệ P/I/D.</div>
                        </div>

                        <div>
                            <div style="display: flex; justify-content: space-between; font-size: 10px; color: var(--text-muted); font-weight: bold;">
                                <span>P-D Balance (Cân bằng P và D):</span>
                                <span id="lblAssistantPdBalance" style="color: var(--purple); font-weight: 800;">Neutral</span>
                            </div>
                            <input type="range" id="inpAssistantPdBalance" min="-0.3" max="0.3" step="0.05" value="0.00" style="width: 100%; margin-top: 4px;" oninput="applyPdBalance(parseFloat(this.value))">
                            <div style="font-size: 9px; color: var(--text-dim); margin-top: 2px;">Kéo sang trái để máy êm hơn (nhiều D), sang phải để phản xạ bén hơn (nhiều P).</div>
                        </div>
                    </div>
                </div>
            </div>

            <div style="display: flex; justify-content: space-between; align-items: center; margin-top: 10px; padding-top: 8px; border-top: 1px solid var(--border);">
                <button class="btn btn-success" onclick="sendAllPids(); closeTuningAssistantModal();">💾 Áp Dụng & Ghi Xuống Drone</button>
                <button class="btn" onclick="closeTuningAssistantModal()">Đóng</button>
            </div>
        </div>
    </div>

    <!-- Export C++ Config Modal -->
    <div id="cfgModal" class="modal-overlay">
        <div class="modal-card">
            <div class="modal-header">
                <span>EXPORT C++ CONFIG HEADER (src/config.h)</span>
                <button class="btn" onclick="closeConfigModal()">✕</button>
            </div>
            <div style="font-size: 10px; color: var(--text-muted); margin-bottom: 6px;">Sao chép các hằng số này dán vào <code>src/config.h</code> để lưu vĩnh viễn cấu hình PID và thuật toán:</div>
            <div id="cfgCode" class="code-block"></div>
            <div style="display: flex; justify-content: flex-end; gap: 6px; margin-top: 10px;">
                <button class="btn btn-primary" onclick="copyConfigCode()">Sao chép mã C++</button>
                <button class="btn" onclick="closeConfigModal()">Đóng</button>
            </div>
        </div>
    </div>

    <script>
        ${droneModelData}

        // === GLOBAL APPLICATION STATE ===
        const state = {
            serialPort: null,
            serialReader: null,
            serialWriter: null,
            ws: null,
            isConnected: false,
            isArmed: false,
            airMode: false,
            flightMode: 'ANGLE',
            roll: 0.0,
            pitch: 0.0,
            yaw: 0.0,
            alt: 0.0,
            vario: 0.0,
            batt: 12.4,
            motors: [1000, 1000, 1000, 1000],
            gyro: [0, 0, 0],
            accel: [0, 0, 1.0],
            rc: { thr: 0, roll: 0, pitch: 0, yaw: 0 },
            rates: { rcRate: 1.0, superRate: 0.70, expo: 0.15 }
        };

        // === 3D & PFD ATTITUDE RENDERING ENGINE WITH PRESETS, ORBIT & SMOOTH LERP ===
        const attitudeCanvas = document.getElementById('attitudeCanvas');
        const attitudeCtx = attitudeCanvas.getContext('2d');
        const attitudeContainer = document.getElementById('attitudeContainer');

        let hudMode = 'pfd'; // 'pfd' | '3d' | 'split'

        function setHudMode(mode) {
            hudMode = mode;
            document.querySelectorAll('.hud-mode-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            const btnId = 'modeBtn' + (mode === '3d' ? '3d' : (mode.charAt(0).toUpperCase() + mode.slice(1)));
            const activeBtn = document.getElementById(btnId);
            if (activeBtn) activeBtn.classList.add('active');

            const viewToolbar = document.getElementById('viewToolbar');
            if (viewToolbar) {
                viewToolbar.style.display = (mode === 'pfd') ? 'none' : 'flex';
            }
        }

        let droneZoom = 1.0;
        let targetZoom = 1.0;
        let camPitch = 0.35; // rad
        let camYaw = 0.0;    // rad
        let targetCamPitch = 0.35;
        let targetCamYaw = 0.0;
        let cameraMode = 'orbit'; // 'orbit' | 'fpv'
        let currentPreset = 'iso';
        let isOrbiting = false;
        let lastMouseX = 0;
        let lastMouseY = 0;

        const CAMERA_PRESETS = {
            iso:    { pitch: 0.35,  yaw: 0.0,            zoom: 1.0,  mode: 'orbit', label: 'ISOMETRIC (3/4)' },
            top:    { pitch: 1.38,  yaw: 0.0,            zoom: 1.05, mode: 'orbit', label: 'TOP-DOWN (TRÊN XUỐNG)' },
            fpv:    { pitch: 0.12,  yaw: 0.0,            zoom: 1.25, mode: 'fpv',   label: 'FPV PILOT CHASE' },
            front:  { pitch: 0.0,   yaw: Math.PI,        zoom: 1.1,  mode: 'orbit', label: 'FRONT (CHÍNH DIỆN)' },
            rear:   { pitch: 0.0,   yaw: 0.0,            zoom: 1.1,  mode: 'orbit', label: 'REAR (PHÍA SAU)' },
            left:   { pitch: 0.0,   yaw: -Math.PI / 2,   zoom: 1.1,  mode: 'orbit', label: 'LEFT (BÊN TRÁI)' },
            right:  { pitch: 0.0,   yaw: Math.PI / 2,    zoom: 1.1,  mode: 'orbit', label: 'RIGHT (BÊN PHẢI)' },
            bottom: { pitch: -1.35, yaw: 0.0,            zoom: 1.05, mode: 'orbit', label: 'BOTTOM (DƯỚI BỤNG)' }
        };

        function setCameraPreset(presetName) {
            const p = CAMERA_PRESETS[presetName];
            if (!p) return;
            currentPreset = presetName;
            targetCamPitch = p.pitch;
            targetCamYaw = p.yaw;
            targetZoom = p.zoom;
            cameraMode = p.mode;

            // Update UI buttons active state
            document.querySelectorAll('#viewToolbar .tool-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            const btn = document.getElementById('btnView' + presetName.charAt(0).toUpperCase() + presetName.slice(1));
            if (btn) btn.classList.add('active');
        }

        function resizeAttitudeCanvas() {
            const rect = attitudeContainer.getBoundingClientRect();
            if (rect.width > 0 && rect.height > 0) {
                attitudeCanvas.width = rect.width;
                attitudeCanvas.height = rect.height;
            }
        }
        window.addEventListener('resize', resizeAttitudeCanvas);
        resizeAttitudeCanvas();

        // Mouse Drag Orbit Controls
        attitudeContainer.addEventListener('mousedown', (e) => {
            isOrbiting = true;
            lastMouseX = e.clientX;
            lastMouseY = e.clientY;
        });

        window.addEventListener('mousemove', (e) => {
            if (!isOrbiting) return;
            const dx = e.clientX - lastMouseX;
            const dy = e.clientY - lastMouseY;
            lastMouseX = e.clientX;
            lastMouseY = e.clientY;

            targetCamYaw += dx * 0.01;
            targetCamPitch += dy * 0.01;
            if (targetCamPitch > 1.45) targetCamPitch = 1.45;
            if (targetCamPitch < -1.45) targetCamPitch = -1.45;

            // If user manually orbits, switch mode to orbit and clear active preset highlight
            currentPreset = 'custom';
            cameraMode = 'orbit';
            document.querySelectorAll('#viewToolbar .tool-btn').forEach(btn => {
                btn.classList.remove('active');
            });
        });

        window.addEventListener('mouseup', () => {
            isOrbiting = false;
        });

        // Mouse Wheel Zoom inside 3D Canvas (Does NOT scroll page)
        attitudeContainer.addEventListener('wheel', (e) => {
            e.preventDefault();
            if (e.deltaY < 0) {
                targetZoom *= 1.08;
            } else {
                targetZoom *= 0.92;
            }
            if (targetZoom > 3.5) targetZoom = 3.5;
            if (targetZoom < 0.3) targetZoom = 0.3;
        }, { passive: false });

        function zoomDrone(factor) {
            targetZoom *= factor;
            if (targetZoom > 3.5) targetZoom = 3.5;
            if (targetZoom < 0.3) targetZoom = 0.3;
        }

        function resetDroneView() {
            setCameraPreset('iso');
        }

        function lerpAngle(current, target, factor) {
            let diff = (target - current) % (Math.PI * 2);
            if (diff < -Math.PI) diff += Math.PI * 2;
            if (diff > Math.PI) diff -= Math.PI * 2;
            return current + diff * factor;
        }

        // 3D Matrix Math Helpers
        function createRotationMatrix(rollDeg, pitchDeg, yawDeg) {
            const r = rollDeg * Math.PI / 180;
            const p = pitchDeg * Math.PI / 180;
            const y = yawDeg * Math.PI / 180;

            const cr = Math.cos(r), sr = Math.sin(r);
            const cp = Math.cos(p), sp = Math.sin(p);
            const cy = Math.cos(y), sy = Math.sin(y);

            // ZYX Euler Rotation Matrix
            return [
                cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr,
                sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr,
                -sp,   cp*sr,            cp*cr
            ];
        }

        // === AVIATION PRIMARY FLIGHT DISPLAY (PFD / ARTIFICIAL HORIZON) ===
        function renderPFD(ctx, vx, vy, vw, vh) {
            if (vw <= 0 || vh <= 0) return;

            ctx.save();
            ctx.beginPath();
            ctx.rect(vx, vy, vw, vh);
            ctx.clip();

            const cx = vx + vw / 2;
            const cy = vy + vh / 2;
            const roll = state.roll;
            const pitch = state.pitch;
            const yaw = state.yaw;
            const rollRad = (roll * Math.PI) / 180;
            const pitchPxPerDeg = (vh * 0.78) / 60; // 60 deg vertical FOV
            const pitchOffset = pitch * pitchPxPerDeg;

            // 1. Artificial Horizon Sky & Ground Gradients (Rotated & Translated)
            ctx.save();
            ctx.translate(cx, cy);
            ctx.rotate(-rollRad);
            ctx.translate(0, pitchOffset);

            // Sky Gradient (Cyan / Deep Navy Blue)
            const skyGrad = ctx.createLinearGradient(0, -1500, 0, 0);
            skyGrad.addColorStop(0, '#0a2540');
            skyGrad.addColorStop(0.55, '#0284c7');
            skyGrad.addColorStop(1, '#38bdf8');
            ctx.fillStyle = skyGrad;
            ctx.fillRect(-2000, -2000, 4000, 2000);

            // Ground Gradient (Warm Earth Brown / Dark Charcoal)
            const gndGrad = ctx.createLinearGradient(0, 0, 0, 1500);
            gndGrad.addColorStop(0, '#5c3a21');
            gndGrad.addColorStop(0.35, '#382010');
            gndGrad.addColorStop(1, '#120a05');
            ctx.fillStyle = gndGrad;
            ctx.fillRect(-2000, 0, 4000, 2000);

            // White Horizon Line
            ctx.strokeStyle = '#ffffff';
            ctx.lineWidth = 2.2;
            ctx.beginPath();
            ctx.moveTo(-2000, 0);
            ctx.lineTo(2000, 0);
            ctx.stroke();

            // 2. Pitch Ladder (Thang chia độ chúi / ngóc)
            const pitchAngles = [-80, -70, -60, -50, -45, -40, -35, -30, -25, -20, -15, -10, -5, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 60, 70, 80];
            const gap = 38;
            const tickLen = 7;

            for (let i = 0; i < pitchAngles.length; i++) {
                const P = pitchAngles[i];
                const py = -P * pitchPxPerDeg;
                if (py < -vh * 1.5 || py > vh * 1.5) continue;

                const isMajor = (Math.abs(P) % 10 === 0);
                const rungWidth = isMajor ? 82 : 52;

                if (P > 0) {
                    // Vùng Trời (Positive Pitch - Ngóc): Solid cyan with downward ticks
                    ctx.strokeStyle = '#38bdf8';
                    ctx.fillStyle = '#38bdf8';
                    ctx.lineWidth = isMajor ? 2.0 : 1.2;
                    ctx.setLineDash([]);

                    ctx.beginPath();
                    ctx.moveTo(-rungWidth / 2, py + tickLen);
                    ctx.lineTo(-rungWidth / 2, py);
                    ctx.lineTo(-gap / 2, py);
                    ctx.moveTo(gap / 2, py);
                    ctx.lineTo(rungWidth / 2, py);
                    ctx.lineTo(rungWidth / 2, py + tickLen);
                    ctx.stroke();
                } else {
                    // Vùng Đất (Negative Pitch - Chúi): Dashed amber with upward ticks
                    ctx.strokeStyle = '#f59e0b';
                    ctx.fillStyle = '#f59e0b';
                    ctx.lineWidth = isMajor ? 2.0 : 1.2;

                    ctx.setLineDash([5, 4]);
                    ctx.beginPath();
                    ctx.moveTo(-rungWidth / 2, py);
                    ctx.lineTo(-gap / 2, py);
                    ctx.moveTo(gap / 2, py);
                    ctx.lineTo(rungWidth / 2, py);
                    ctx.stroke();

                    ctx.setLineDash([]);
                    ctx.beginPath();
                    ctx.moveTo(-rungWidth / 2, py);
                    ctx.lineTo(-rungWidth / 2, py - tickLen);
                    ctx.moveTo(rungWidth / 2, py);
                    ctx.lineTo(rungWidth / 2, py - tickLen);
                    ctx.stroke();
                }

                ctx.font = 'bold 9px monospace';
                ctx.textAlign = 'right';
                ctx.textBaseline = 'middle';
                ctx.fillText(Math.abs(P).toString(), -rungWidth / 2 - 5, py);
                ctx.textAlign = 'left';
                ctx.fillText(Math.abs(P).toString(), rungWidth / 2 + 5, py);
            }

            // Zenith (+90°) & Nadir (-90°)
            const zY = -90 * pitchPxPerDeg;
            ctx.fillStyle = '#38bdf8';
            ctx.beginPath(); ctx.arc(0, zY, 5, 0, Math.PI * 2); ctx.fill();
            ctx.font = 'bold 9px monospace'; ctx.textAlign = 'center'; ctx.fillText('+90° ZENITH', 0, zY - 10);

            const nY = 90 * pitchPxPerDeg;
            ctx.fillStyle = '#f59e0b';
            ctx.beginPath(); ctx.arc(0, nY, 5, 0, Math.PI * 2); ctx.fill();
            ctx.fillText('-90° NADIR', 0, nY + 14);

            ctx.restore(); // Back to unrotated screen coordinates

            // 3. Roll Bank Scale Arc & Dynamic Sky Pointer (Top Fixed Arc)
            const rollRadius = Math.min(vw, vh) * 0.38;
            ctx.save();
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.65)';
            ctx.lineWidth = 1.5;
            ctx.beginPath();
            ctx.arc(cx, cy, rollRadius, -Math.PI / 2 - Math.PI / 3, -Math.PI / 2 + Math.PI / 3);
            ctx.stroke();

            const rollTicks = [-60, -45, -30, -20, -10, 0, 10, 20, 30, 45, 60];
            for (let i = 0; i < rollTicks.length; i++) {
                const ang = rollTicks[i];
                const rad = -Math.PI / 2 + ang * Math.PI / 180;
                const isMajor = (Math.abs(ang) === 30 || Math.abs(ang) === 60 || ang === 0);
                const is45 = (Math.abs(ang) === 45);
                const tLen = isMajor ? 10 : (is45 ? 8 : 5);

                const x1 = cx + Math.cos(rad) * rollRadius;
                const y1 = cy + Math.sin(rad) * rollRadius;
                const x2 = cx + Math.cos(rad) * (rollRadius + tLen);
                const y2 = cy + Math.sin(rad) * (rollRadius + tLen);

                ctx.beginPath();
                ctx.moveTo(x1, y1);
                ctx.lineTo(x2, y2);
                ctx.stroke();

                if (isMajor && ang !== 0) {
                    ctx.font = 'bold 8px monospace';
                    ctx.fillStyle = '#ffffff';
                    ctx.textAlign = 'center';
                    ctx.textBaseline = 'middle';
                    const tx = cx + Math.cos(rad) * (rollRadius + tLen + 7);
                    const ty = cy + Math.sin(rad) * (rollRadius + tLen + 7);
                    ctx.fillText(Math.abs(ang).toString(), tx, ty);
                }
            }

            // Dynamic Roll Pointer (Sky Pointer)
            const pRad = -Math.PI / 2 - rollRad;
            const px = cx + Math.cos(pRad) * rollRadius;
            const py = cy + Math.sin(pRad) * rollRadius;
            ctx.save();
            ctx.translate(px, py);
            ctx.rotate(pRad + Math.PI / 2);
            ctx.fillStyle = '#facc15';
            ctx.strokeStyle = '#000000';
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(0, 0);
            ctx.lineTo(-6, -10);
            ctx.lineTo(6, -10);
            ctx.closePath();
            ctx.fill();
            ctx.stroke();
            ctx.restore();
            ctx.restore();

            // 4. Fixed Central Aircraft Boresight Reticle & Procedural 3D Vector Quad-X Drone Silhouette
            ctx.save();
            ctx.strokeStyle = '#facc15';
            ctx.fillStyle = '#facc15';
            ctx.lineWidth = 2.5;

            // Left Wingbar
            ctx.beginPath();
            ctx.moveTo(cx - 52, cy);
            ctx.lineTo(cx - 18, cy);
            ctx.lineTo(cx - 18, cy + 6);
            ctx.stroke();

            // Right Wingbar
            ctx.beginPath();
            ctx.moveTo(cx + 52, cy);
            ctx.lineTo(cx + 18, cy);
            ctx.lineTo(cx + 18, cy + 6);
            ctx.stroke();

            // Center Pip
            ctx.beginPath();
            ctx.arc(cx, cy, 3, 0, Math.PI * 2);
            ctx.fill();

            // Procedural Vector Quadcopter
            const armLen = 26;
            const propRadius = 11;
            const armAngle = Math.PI / 4;
            const now = Date.now() * 0.04;

            const corners = [
                { id: 'M1', x: Math.cos(armAngle),  y: -Math.sin(armAngle), color: '#10b981' },
                { id: 'M2', x: -Math.cos(armAngle), y: -Math.sin(armAngle), color: '#10b981' },
                { id: 'M3', x: Math.cos(armAngle),  y: Math.sin(armAngle),  color: '#f59e0b' },
                { id: 'M4', x: -Math.cos(armAngle), y: Math.sin(armAngle),  color: '#f59e0b' }
            ];

            // Drone Center Hub
            ctx.fillStyle = 'rgba(11, 17, 32, 0.9)';
            ctx.strokeStyle = 'rgba(0, 243, 255, 0.7)';
            ctx.lineWidth = 1.2;
            ctx.beginPath();
            ctx.arc(cx, cy, 7, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();

            for (let i = 0; i < corners.length; i++) {
                const c = corners[i];
                const mx = cx + c.x * armLen;
                const my = cy + c.y * armLen;

                // Arm
                ctx.strokeStyle = c.color;
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.moveTo(cx, cy);
                ctx.lineTo(mx, my);
                ctx.stroke();

                // Spinning Prop Disc
                ctx.save();
                ctx.fillStyle = (c.color === '#10b981') ? 'rgba(16, 185, 129, 0.25)' : 'rgba(245, 158, 11, 0.25)';
                ctx.strokeStyle = c.color;
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.arc(mx, my, propRadius, 0, Math.PI * 2);
                ctx.fill();
                ctx.stroke();

                // Rotating blade
                const spin = now + i * Math.PI / 2;
                ctx.beginPath();
                ctx.moveTo(mx - Math.cos(spin) * (propRadius - 2), my - Math.sin(spin) * (propRadius - 2));
                ctx.lineTo(mx + Math.cos(spin) * (propRadius - 2), my + Math.sin(spin) * (propRadius - 2));
                ctx.stroke();

                // Motor Label
                ctx.font = 'bold 7px monospace';
                ctx.fillStyle = '#ffffff';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText(c.id, mx, my);
                ctx.restore();
            }

            // Front Nose Arrow
            ctx.fillStyle = '#10b981';
            ctx.beginPath();
            ctx.moveTo(cx, cy - 14);
            ctx.lineTo(cx - 4, cy - 9);
            ctx.lineTo(cx + 4, cy - 9);
            ctx.closePath();
            ctx.fill();
            ctx.restore();

            // 5. Top Heading Compass Ribbon
            const hx = vx + 10, hy = vy + 6, hw = vw - 20, hh = 26;
            ctx.save();
            ctx.beginPath();
            ctx.rect(hx, hy, hw, hh);
            ctx.clip();

            ctx.fillStyle = 'rgba(6, 9, 19, 0.88)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.9)';
            ctx.lineWidth = 1;
            ctx.fillRect(hx, hy, hw, hh);
            ctx.strokeRect(hx, hy, hw, hh);

            const hcx = hx + hw / 2;
            const hdgPxPerDeg = hw / 80;
            const normYaw = (yaw % 360 + 360) % 360;

            for (let d = Math.floor(normYaw - 45); d <= Math.ceil(normYaw + 45); d++) {
                const deg = (d % 360 + 360) % 360;
                const dx = hcx + (d - normYaw) * hdgPxPerDeg;
                if (dx < hx - 10 || dx > hx + hw + 10) continue;

                const isMajor = (deg % 15 === 0);
                const isFive = (deg % 5 === 0);

                if (isFive) {
                    ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)';
                    ctx.lineWidth = isMajor ? 1.5 : 1;
                    ctx.beginPath();
                    ctx.moveTo(dx, hy + hh);
                    ctx.lineTo(dx, hy + hh - (isMajor ? 8 : 4));
                    ctx.stroke();
                }

                if (isMajor) {
                    let label = deg.toString().padStart(3, '0');
                    let col = '#94a3b8';
                    if (deg === 0)   { label = 'N';  col = '#ef4444'; }
                    if (deg === 45)  { label = 'NE'; col = '#38bdf8'; }
                    if (deg === 90)  { label = 'E';  col = '#38bdf8'; }
                    if (deg === 135) { label = 'SE'; col = '#38bdf8'; }
                    if (deg === 180) { label = 'S';  col = '#10b981'; }
                    if (deg === 225) { label = 'SW'; col = '#f59e0b'; }
                    if (deg === 270) { label = 'W';  col = '#f59e0b'; }
                    if (deg === 315) { label = 'NW'; col = '#f59e0b'; }

                    ctx.font = 'bold 8px monospace';
                    ctx.fillStyle = col;
                    ctx.textAlign = 'center';
                    ctx.fillText(label, dx, hy + 10);
                }
            }

            // Lubber Line Pointer
            ctx.fillStyle = '#facc15';
            ctx.beginPath();
            ctx.moveTo(hcx, hy + hh);
            ctx.lineTo(hcx - 4, hy + hh - 6);
            ctx.lineTo(hcx + 4, hy + hh - 6);
            ctx.closePath();
            ctx.fill();
            ctx.restore();

            // Heading Readout Tag
            ctx.save();
            ctx.fillStyle = 'rgba(11, 17, 32, 0.92)';
            ctx.strokeStyle = '#facc15';
            ctx.lineWidth = 1;
            ctx.fillRect(hcx - 26, hy + hh + 2, 52, 14);
            ctx.strokeRect(hcx - 26, hy + hh + 2, 52, 14);
            ctx.font = 'bold 9px monospace';
            ctx.fillStyle = '#facc15';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(normYaw.toFixed(0).padStart(3, '0') + '°', hcx, hy + hh + 9);
            ctx.restore();

            // 6. Right Altimeter Tape (BMP280 Barometer)
            const altX = vx + vw - 48, altY = vy + 38, altW = 42, altH = vh - 78;
            ctx.save();
            ctx.fillStyle = 'rgba(6, 9, 19, 0.85)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.9)';
            ctx.lineWidth = 1;
            ctx.fillRect(altX, altY, altW, altH);
            ctx.strokeRect(altX, altY, altW, altH);

            const curAlt = state.alt || 0.0;
            const altPxPerM = altH / 12; // 12 meter visible range
            const altCenterY = altY + altH / 2;

            for (let m = Math.floor(curAlt - 6); m <= Math.ceil(curAlt + 6); m++) {
                const dy = altCenterY - (m - curAlt) * altPxPerM;
                if (dy < altY || dy > altY + altH) continue;

                const isMajorM = (m % 2 === 0);
                ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)';
                ctx.lineWidth = isMajorM ? 1.5 : 1;
                ctx.beginPath();
                ctx.moveTo(altX, dy);
                ctx.lineTo(altX + (isMajorM ? 8 : 4), dy);
                ctx.stroke();

                if (isMajorM) {
                    ctx.font = '8px monospace';
                    ctx.fillStyle = '#94a3b8';
                    ctx.textAlign = 'right';
                    ctx.fillText(m.toString(), altX + altW - 4, dy + 3);
                }
            }

            // Altimeter Digital Readout Box
            ctx.fillStyle = '#0b1120';
            ctx.strokeStyle = '#00f3ff';
            ctx.lineWidth = 1.2;
            ctx.fillRect(altX - 4, altCenterY - 9, altW + 6, 18);
            ctx.strokeRect(altX - 4, altCenterY - 9, altW + 6, 18);
            ctx.font = 'bold 9px monospace';
            ctx.fillStyle = '#00f3ff';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(curAlt.toFixed(1) + 'm', altX + altW / 2 - 1, altCenterY);
            ctx.restore();

            // 7. Left Throttle Tape
            const thrX = vx + 6, thrY = vy + 38, thrW = 42, thrH = vh - 78;
            ctx.save();
            ctx.fillStyle = 'rgba(6, 9, 19, 0.85)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.9)';
            ctx.lineWidth = 1;
            ctx.fillRect(thrX, thrY, thrW, thrH);
            ctx.strokeRect(thrX, thrY, thrW, thrH);

            const thrPct = Math.max(0, Math.min(100, state.rc.thr || 0));
            const fillH = (thrPct / 100) * (thrH - 4);
            const thrFillY = thrY + thrH - 2 - fillH;

            const thrGrad = ctx.createLinearGradient(0, thrY + thrH, 0, thrY);
            thrGrad.addColorStop(0, '#10b981');
            thrGrad.addColorStop(0.6, '#f59e0b');
            thrGrad.addColorStop(1, '#ef4444');
            ctx.fillStyle = thrGrad;
            ctx.fillRect(thrX + 2, thrFillY, 6, fillH);

            // Throttle Ticks
            for (let t = 0; t <= 100; t += 25) {
                const ty = thrY + thrH - 2 - (t / 100) * (thrH - 4);
                ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)';
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(thrX + 8, ty);
                ctx.lineTo(thrX + 13, ty);
                ctx.stroke();

                ctx.font = '7px monospace';
                ctx.fillStyle = '#94a3b8';
                ctx.textAlign = 'right';
                ctx.fillText(t + '%', thrX + thrW - 2, ty + 2);
            }

            // Throttle Digital Readout Box
            const thrCenterY = thrY + thrH / 2;
            ctx.fillStyle = '#0b1120';
            ctx.strokeStyle = '#10b981';
            ctx.lineWidth = 1.2;
            ctx.fillRect(thrX - 2, thrCenterY - 9, thrW + 4, 18);
            ctx.strokeRect(thrX - 2, thrCenterY - 9, thrW + 4, 18);
            ctx.font = 'bold 8px monospace';
            ctx.fillStyle = '#10b981';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText('THR ' + thrPct.toFixed(0) + '%', thrX + thrW / 2, thrCenterY);
            ctx.restore();

            // 8. Flight State & Digital Metrics (Corner Badges)
            ctx.save();
            ctx.fillStyle = 'rgba(11, 17, 32, 0.88)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.9)';
            ctx.lineWidth = 1;
            ctx.fillRect(vx + 6, vy + vh - 40, 160, 34);
            ctx.strokeRect(vx + 6, vy + vh - 40, 160, 34);

            ctx.font = '8px monospace';
            ctx.fillStyle = '#38bdf8';
            ctx.fillText('R (Roll): ' + (roll >= 0 ? '+' : '') + roll.toFixed(1) + '°', vx + 12, vy + vh - 28);
            ctx.fillStyle = '#f59e0b';
            ctx.fillText('P (Pitch): ' + (pitch >= 0 ? '+' : '') + pitch.toFixed(1) + '°', vx + 12, vy + vh - 18);
            ctx.fillStyle = '#10b981';
            ctx.fillText('Y (Yaw): ' + (yaw >= 0 ? '+' : '') + yaw.toFixed(1) + '°', vx + 12, vy + vh - 8);

            // Top-Left Badges
            ctx.fillStyle = 'rgba(11, 17, 32, 0.88)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.9)';
            ctx.fillRect(vx + 6, vy + 6, 85, 26);
            ctx.strokeRect(vx + 6, vy + 6, 85, 26);

            ctx.font = 'bold 8px monospace';
            ctx.fillStyle = state.isArmed ? '#ef4444' : '#10b981';
            ctx.fillText(state.isArmed ? '● ARMED' : '○ DISARMED', vx + 12, vy + 16);
            ctx.fillStyle = '#00f3ff';
            ctx.fillText('MODE: ' + (state.flightMode || 'ANGLE'), vx + 12, vy + 26);

            ctx.restore();
            ctx.restore();
        }

        // === 3D MESH MODEL RENDERING ENGINE ===
        function render3DMesh(ctx, vx, vy, vw, vh) {
            if (vw <= 0 || vh <= 0) return;

            ctx.save();
            ctx.beginPath();
            ctx.rect(vx, vy, vw, vh);
            ctx.clip();

            // Smooth Camera Interpolation (Lerp)
            if (cameraMode === 'fpv') {
                targetCamYaw = (-state.yaw * Math.PI / 180);
            }
            camPitch += (targetCamPitch - camPitch) * 0.18;
            camYaw = lerpAngle(camYaw, targetCamYaw, 0.18);
            droneZoom += (targetZoom - droneZoom) * 0.18;

            const cx = vx + vw / 2;
            const cy = vy + vh / 2;
            const baseScale = Math.min(vw, vh) * 0.38 * droneZoom;

            // 1. Draw Horizon Background Artificial Ground Grid
            ctx.save();
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.4)';
            ctx.lineWidth = 1;
            const gridPitch = Math.sin(camPitch);
            const gridCenterY = cy + gridPitch * 100;

            for (let i = -5; i <= 5; i++) {
                ctx.beginPath();
                ctx.moveTo(vx, gridCenterY + i * 20);
                ctx.lineTo(vx + vw, gridCenterY + i * 20);
                ctx.stroke();
            }
            ctx.restore();

            // 2. Compute Combined Rotation: Camera Orbit * Drone Attitude
            const droneRot = createRotationMatrix(state.roll, -state.pitch, -state.yaw);

            // Camera Orbit Matrix
            const cosP = Math.cos(camPitch), sinP = Math.sin(camPitch);
            const cosY = Math.cos(camYaw), sinY = Math.sin(camYaw);

            // 3D Point Projection Helper (Body frame or World frame)
            function projectPoint(px, py, pz, isWorld = false) {
                let rx, ry, rz;
                if (isWorld) {
                    rx = px; ry = py; rz = pz;
                } else {
                    rx = droneRot[0]*px + droneRot[1]*py + droneRot[2]*pz;
                    ry = droneRot[3]*px + droneRot[4]*py + droneRot[5]*pz;
                    rz = droneRot[6]*px + droneRot[7]*py + droneRot[8]*pz;
                }
                const ox = cosY*rx - sinY*rz;
                const oz = sinY*rx + cosY*rz;
                const oy = cosP*ry - sinP*oz;
                const finalZ = sinP*ry + cosP*oz;
                return {
                    x: cx + ox * baseScale,
                    y: cy - oy * baseScale,
                    z: finalZ
                };
            }

            // Draw 3D Compass Ground Ring under the Drone (World Frame)
            ctx.save();
            const groundZ = -0.45;
            const ringRadius = 1.35;
            ctx.strokeStyle = 'rgba(51, 65, 85, 0.4)';
            ctx.lineWidth = 1;
            ctx.beginPath();
            const ringSteps = 36;
            for (let i = 0; i <= ringSteps; i++) {
                const angle = (i / ringSteps) * Math.PI * 2;
                const pt = projectPoint(Math.cos(angle) * ringRadius, Math.sin(angle) * ringRadius, groundZ, true);
                if (i === 0) ctx.moveTo(pt.x, pt.y);
                else ctx.lineTo(pt.x, pt.y);
            }
            ctx.stroke();

            // Ground Cardinal Directions
            const groundCenter = projectPoint(0, 0, groundZ, true);
            const ptNorth = projectPoint(0, ringRadius, groundZ, true);
            const ptSouth = projectPoint(0, -ringRadius, groundZ, true);
            const ptEast  = projectPoint(ringRadius, 0, groundZ, true);
            const ptWest  = projectPoint(-ringRadius, 0, groundZ, true);

            ctx.strokeStyle = 'rgba(16, 185, 129, 0.3)';
            ctx.beginPath(); ctx.moveTo(groundCenter.x, groundCenter.y); ctx.lineTo(ptNorth.x, ptNorth.y); ctx.stroke();
            ctx.strokeStyle = 'rgba(245, 158, 11, 0.3)';
            ctx.beginPath(); ctx.moveTo(groundCenter.x, groundCenter.y); ctx.lineTo(ptSouth.x, ptSouth.y); ctx.stroke();
            ctx.strokeStyle = 'rgba(239, 68, 68, 0.3)';
            ctx.beginPath(); ctx.moveTo(groundCenter.x, groundCenter.y); ctx.lineTo(ptEast.x, ptEast.y); ctx.stroke();
            ctx.strokeStyle = 'rgba(56, 189, 248, 0.3)';
            ctx.beginPath(); ctx.moveTo(groundCenter.x, groundCenter.y); ctx.lineTo(ptWest.x, ptWest.y); ctx.stroke();
            ctx.restore();

            // Project 3D Model Vertices
            const numVerts = DRONE_V.length / 3;
            const projVerts = new Float32Array(numVerts * 3);

            for (let i = 0; i < numVerts; i++) {
                const vx_ = DRONE_V[i * 3];
                const vy_ = DRONE_V[i * 3 + 1];
                const vz_ = DRONE_V[i * 3 + 2];

                // Apply Drone Rotation
                const rx = droneRot[0]*vx_ + droneRot[1]*vy_ + droneRot[2]*vz_;
                const ry = droneRot[3]*vx_ + droneRot[4]*vy_ + droneRot[5]*vz_;
                const rz = droneRot[6]*vx_ + droneRot[7]*vy_ + droneRot[8]*vz_;

                // Apply Camera Orbit
                const ox = cosY*rx - sinY*rz;
                const oz = sinY*rx + cosY*rz;
                const oy = cosP*ry - sinP*oz;
                const finalZ = sinP*ry + cosP*oz;

                projVerts[i * 3]     = cx + ox * baseScale;
                projVerts[i * 3 + 1] = cy - oy * baseScale;
                projVerts[i * 3 + 2] = finalZ;
            }

            // 3. Render 3D Model Faces with Depth & Directional Light
            const numFaces = DRONE_F.length / 3;
            const faceList = [];

            const lightDir = [0.4, 0.7, 0.6];
            const lightLen = Math.hypot(...lightDir);
            const lx = lightDir[0]/lightLen, ly = lightDir[1]/lightLen, lz = lightDir[2]/lightLen;

            for (let f = 0; f < numFaces; f++) {
                const i0 = DRONE_F[f * 3];
                const i1 = DRONE_F[f * 3 + 1];
                const i2 = DRONE_F[f * 3 + 2];

                const z0 = projVerts[i0 * 3 + 2];
                const z1 = projVerts[i1 * 3 + 2];
                const z2 = projVerts[i2 * 3 + 2];
                const avgZ = (z0 + z1 + z2) / 3;

                // Normal shading
                const nx = DRONE_FN[f * 3];
                const ny = DRONE_FN[f * 3 + 1];
                const nz = DRONE_FN[f * 3 + 2];

                // Rotate normal by drone rotation
                const rnx = droneRot[0]*nx + droneRot[1]*ny + droneRot[2]*nz;
                const rny = droneRot[3]*nx + droneRot[4]*ny + droneRot[5]*nz;
                const rnz = droneRot[6]*nx + droneRot[7]*ny + droneRot[8]*nz;

                const dot = Math.max(0.15, rnx*lx + rny*ly + rnz*lz);
                faceList.push({ f, avgZ, dot, i0, i1, i2 });
            }

            // Painter's algorithm sort (back to front)
            faceList.sort((a, b) => a.avgZ - b.avgZ);

            ctx.lineWidth = 0.5;
            for (let i = 0; i < faceList.length; i++) {
                const item = faceList[i];
                const i0 = item.i0, i1 = item.i1, i2 = item.i2;

                const x0 = projVerts[i0 * 3], y0 = projVerts[i0 * 3 + 1];
                const x1 = projVerts[i1 * 3], y1 = projVerts[i1 * 3 + 1];
                const x2 = projVerts[i2 * 3], y2 = projVerts[i2 * 3 + 1];

                const brightness = Math.round(item.dot * 210 + 35);
                const isFront = (item.f % 4 === 0);

                if (isFront) {
                    ctx.fillStyle = 'rgb(' + Math.round(brightness * 0.1) + ', ' + Math.round(brightness * 0.85) + ', ' + brightness + ')';
                    ctx.strokeStyle = 'rgba(0, 243, 255, 0.4)';
                } else {
                    ctx.fillStyle = 'rgb(' + Math.round(brightness * 0.4) + ', ' + Math.round(brightness * 0.5) + ', ' + Math.round(brightness * 0.7) + ')';
                    ctx.strokeStyle = 'rgba(148, 163, 184, 0.15)';
                }

                ctx.beginPath();
                ctx.moveTo(x0, y0);
                ctx.lineTo(x1, y1);
                ctx.lineTo(x2, y2);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();
            }

            // 4. Draw 3D Body Frame Coordinate Axes (X: Phải/Trái, Y: Trước/Sau, Z: Trên/Dưới)
            function draw3DArrow(startX, startY, startZ, endX, endY, endZ, color, label, isDashed = false) {
                const p0 = projectPoint(startX, startY, startZ, false);
                const p1 = projectPoint(endX, endY, endZ, false);

                ctx.save();
                ctx.strokeStyle = color;
                ctx.fillStyle = color;
                ctx.lineWidth = 2;
                if (isDashed) ctx.setLineDash([4, 3]);

                // Line
                ctx.beginPath();
                ctx.moveTo(p0.x, p0.y);
                ctx.lineTo(p1.x, p1.y);
                ctx.stroke();

                // Arrowhead
                ctx.setLineDash([]);
                const headAngle = Math.atan2(p1.y - p0.y, p1.x - p0.x);
                const headLen = 8;
                ctx.beginPath();
                ctx.moveTo(p1.x, p1.y);
                ctx.lineTo(p1.x - headLen * Math.cos(headAngle - Math.PI / 6), p1.y - headLen * Math.sin(headAngle - Math.PI / 6));
                ctx.lineTo(p1.x - headLen * Math.cos(headAngle + Math.PI / 6), p1.y - headLen * Math.sin(headAngle + Math.PI / 6));
                ctx.closePath();
                ctx.fill();

                // Text Badge in 3D Space
                ctx.font = 'bold 9px monospace';
                const textOffset = 12;
                const tx = p1.x + Math.cos(headAngle) * textOffset;
                const ty = p1.y + Math.sin(headAngle) * textOffset;

                // Background pill
                const textMetrics = ctx.measureText(label);
                ctx.fillStyle = 'rgba(11, 17, 32, 0.85)';
                ctx.fillRect(tx - textMetrics.width / 2 - 3, ty - 6, textMetrics.width + 6, 12);
                ctx.strokeStyle = color;
                ctx.lineWidth = 1;
                ctx.strokeRect(tx - textMetrics.width / 2 - 3, ty - 6, textMetrics.width + 6, 12);

                ctx.fillStyle = color;
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText(label, tx, ty);
                ctx.restore();
            }

            // TRỤC X: Phải (+X) / Trái (-X) [Màu đỏ #ef4444]
            draw3DArrow(0, 0, 0,  1.35, 0, 0, '#ef4444', '+X: PHẢI (RIGHT)');
            draw3DArrow(0, 0, 0, -1.35, 0, 0, '#f87171', '-X: TRÁI (LEFT)', true);

            // TRỤC Y: Trước (+Y) / Sau (-Y) [Màu xanh lá #10b981 / Cam #f59e0b]
            draw3DArrow(0, 0, 0, 0,  1.35, 0, '#10b981', '+Y: TRƯỚC (FRONT)');
            draw3DArrow(0, 0, 0, 0, -1.35, 0, '#f59e0b', '-Y: SAU (BACK)', true);

            // TRỤC Z: Trên (+Z) / Dưới (-Z) [Màu xanh cyan #00f3ff]
            draw3DArrow(0, 0, 0, 0, 0,  1.10, '#00f3ff', '+Z: TRÊN (UP)');
            draw3DArrow(0, 0, 0, 0, 0, -1.10, '#818cf8', '-Z: DƯỚI (DOWN)', true);

            // 5. Draw 3D Motor Badges at 4 Quad-X Corners
            function drawMotorBadge(x, y, z, label) {
                const pt = projectPoint(x, y, z, false);
                ctx.save();
                ctx.fillStyle = 'rgba(11, 17, 32, 0.9)';
                ctx.strokeStyle = 'rgba(0, 243, 255, 0.5)';
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.arc(pt.x, pt.y, 11, 0, Math.PI * 2);
                ctx.fill();
                ctx.stroke();

                ctx.font = 'bold 8px monospace';
                ctx.fillStyle = '#00f3ff';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText(label, pt.x, pt.y - 1);
                ctx.restore();
            }

            drawMotorBadge( 0.72,  0.72, 0.05, 'M1');
            drawMotorBadge(-0.72,  0.72, 0.05, 'M2');
            drawMotorBadge( 0.72, -0.72, 0.05, 'M3');
            drawMotorBadge(-0.72, -0.72, 0.05, 'M4');

            // 6. Interactive 3D Coordinate Gizmo Triad in Bottom-Left Corner
            const gx = vx + 45, gy = vy + vh - 45;
            const gLen = 28;

            function drawGizmoAxis(ax, ay, az, color, label) {
                const ox = cosY*ax - sinY*az;
                const oz = sinY*ax + cosY*az;
                const oy = cosP*ay - sinP*oz;

                const ex = gx + ox * gLen;
                const ey = gy - oy * gLen;

                ctx.save();
                ctx.strokeStyle = color;
                ctx.fillStyle = color;
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.moveTo(gx, gy);
                ctx.lineTo(ex, ey);
                ctx.stroke();

                ctx.beginPath();
                ctx.arc(ex, ey, 4, 0, Math.PI * 2);
                ctx.fill();

                ctx.font = 'bold 9px monospace';
                ctx.fillStyle = color;
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText(label, ex + (ex - gx) * 0.3, ey + (ey - gy) * 0.3);
                ctx.restore();
            }

            ctx.save();
            ctx.fillStyle = 'rgba(6, 9, 19, 0.75)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.8)';
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.arc(gx, gy, 36, 0, Math.PI * 2);
            ctx.fill();
            ctx.stroke();

            drawGizmoAxis(1, 0, 0, '#ef4444', 'X');
            drawGizmoAxis(0, 1, 0, '#10b981', 'Y');
            drawGizmoAxis(0, 0, 1, '#00f3ff', 'Z');
            ctx.restore();

            // 7. Live Telemetry & Viewport HUD Overlay (Top-Left)
            ctx.save();
            ctx.fillStyle = 'rgba(11, 17, 32, 0.85)';
            ctx.strokeStyle = 'rgba(30, 41, 59, 0.9)';
            ctx.lineWidth = 1;
            ctx.fillRect(vx + 8, vy + 8, 195, 72);
            ctx.strokeRect(vx + 8, vy + 8, 195, 72);

            ctx.font = 'bold 9px monospace';
            if (state.isConnected) {
                ctx.fillStyle = '#10b981';
                ctx.fillText('● 100% REALTIME TELEMETRY', vx + 14, vy + 20);
            } else {
                ctx.fillStyle = '#ef4444';
                ctx.fillText('○ CHỜ KẾT NỐI (DISCONNECTED)', vx + 14, vy + 20);
            }

            ctx.font = '9px monospace';
            ctx.fillStyle = '#ef4444';
            ctx.fillText('Trục X (Roll): ' + (state.roll >= 0 ? '+' : '') + state.roll.toFixed(1) + '° [Phải/Trái]', vx + 14, vy + 32);
            ctx.fillStyle = '#10b981';
            ctx.fillText('Trục Y (Pitch): ' + (state.pitch >= 0 ? '+' : '') + state.pitch.toFixed(1) + '° [Trước/Sau]', vx + 14, vy + 44);
            ctx.fillStyle = '#00f3ff';
            ctx.fillText('Trục Z (Yaw): ' + (state.yaw >= 0 ? '+' : '') + state.yaw.toFixed(1) + '° [Hướng Mũi]', vx + 14, vy + 56);

            const presetLabel = CAMERA_PRESETS[currentPreset] ? CAMERA_PRESETS[currentPreset].label : 'CUSTOM ORBIT';
            ctx.fillStyle = '#f59e0b';
            ctx.fillText('Góc nhìn: ' + presetLabel + ' (' + (droneZoom * 100).toFixed(0) + '%)', vx + 14, vy + 68);
            ctx.restore();

            ctx.restore();
        }

        // === MASTER ATTITUDE HUD ROUTER (PFD / 3D / SPLIT) ===
        function renderAttitude() {
            const ctx = attitudeCtx;
            const w = attitudeCanvas.width;
            const h = attitudeCanvas.height;
            if (w === 0 || h === 0) return;

            ctx.clearRect(0, 0, w, h);

            if (hudMode === 'pfd') {
                renderPFD(ctx, 0, 0, w, h);
            } else if (hudMode === '3d') {
                render3DMesh(ctx, 0, 0, w, h);
            } else if (hudMode === 'split') {
                const halfW = Math.floor(w / 2);
                renderPFD(ctx, 0, 0, halfW, h);
                render3DMesh(ctx, halfW, 0, w - halfW, h);

                // Split Divider Line with Cyberpunk Glow
                ctx.save();
                ctx.strokeStyle = 'rgba(0, 243, 255, 0.5)';
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(halfW, 0);
                ctx.lineTo(halfW, h);
                ctx.stroke();
                ctx.restore();
            }
        }

        // === REALTIME OSCILLOSCOPE ENGINE ===
        const scopeCanvas = document.getElementById('scopeCanvas');
        const scopeCtx = scopeCanvas.getContext('2d');
        const scopeData = {
            roll: [],
            pitch: [],
            yaw: [],
            gyro: [],
            motors: []
        };
        const MAX_SCOPE_SAMPLES = 240;
        let isScopePaused = false;

        function resizeScopeCanvas() {
            const container = scopeCanvas.parentElement;
            if (container) {
                scopeCanvas.width = container.clientWidth;
                scopeCanvas.height = container.clientHeight;
            }
        }
        window.addEventListener('resize', resizeScopeCanvas);
        resizeScopeCanvas();

        function addScopeSample() {
            if (isScopePaused) return;

            scopeData.roll.push(state.roll);
            scopeData.pitch.push(state.pitch);
            scopeData.yaw.push(state.yaw);
            scopeData.gyro.push(state.gyro[0]);
            scopeData.motors.push((state.motors[0] - 1000) / 10);

            if (scopeData.roll.length > MAX_SCOPE_SAMPLES) {
                scopeData.roll.shift();
                scopeData.pitch.shift();
                scopeData.yaw.shift();
                scopeData.gyro.shift();
                scopeData.motors.shift();
            }
        }

        function renderScope() {
            const ctx = scopeCtx;
            const w = scopeCanvas.width;
            const h = scopeCanvas.height;
            if (w === 0 || h === 0) return;

            ctx.clearRect(0, 0, w, h);

            // Scope Grid Background
            ctx.strokeStyle = '#0e172a';
            ctx.lineWidth = 1;
            const midY = h / 2;

            for (let y = 0; y < h; y += 25) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(w, y);
                ctx.stroke();
            }
            for (let x = 0; x < w; x += 35) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, h);
                ctx.stroke();
            }

            // Zero Center Line
            ctx.strokeStyle = '#1e293b';
            ctx.lineWidth = 1.5;
            ctx.beginPath();
            ctx.moveTo(0, midY);
            ctx.lineTo(w, midY);
            ctx.stroke();

            // Trace Channels
            function drawTrace(arr, color, scale) {
                if (arr.length < 2) return;
                ctx.strokeStyle = color;
                ctx.lineWidth = 1.6;
                ctx.beginPath();
                const step = w / (MAX_SCOPE_SAMPLES - 1);

                for (let i = 0; i < arr.length; i++) {
                    const x = i * step;
                    const y = midY - (arr[i] * scale);
                    if (i === 0) ctx.moveTo(x, y);
                    else ctx.lineTo(x, y);
                }
                ctx.stroke();
            }

            const scale = (h / 2) / 45.0; // ±45° range
            if (document.getElementById('chRoll').checked) drawTrace(scopeData.roll, '#38bdf8', scale);
            if (document.getElementById('chPitch').checked) drawTrace(scopeData.pitch, '#a855f7', scale);
            if (document.getElementById('chYaw').checked) drawTrace(scopeData.yaw, '#34d399', scale);
            if (document.getElementById('chGyro').checked) drawTrace(scopeData.gyro, '#f59e0b', scale * 0.3);
            if (document.getElementById('chMotors').checked) drawTrace(scopeData.motors, '#ef4444', scale * 0.4);
        }

        function toggleScopePause() {
            isScopePaused = !isScopePaused;
            document.getElementById('btnScopePause').innerText = isScopePaused ? 'Resume' : 'Pause';
        }

        function clearScope() {
            scopeData.roll = [];
            scopeData.pitch = [];
            scopeData.yaw = [];
            scopeData.gyro = [];
            scopeData.motors = [];
        }

        // === BETAFLIGHT RATES & EXPO VISUALIZER ===
        const rateCanvas = document.getElementById('rateCurveCanvas');
        const rateCtx = rateCanvas.getContext('2d');

        function resizeRateCanvas() {
            if (rateCanvas.parentElement) {
                rateCanvas.width = rateCanvas.parentElement.clientWidth;
                rateCanvas.height = rateCanvas.parentElement.clientHeight;
            }
        }
        window.addEventListener('resize', resizeRateCanvas);
        resizeRateCanvas();

        function calculateBetaflightRate(stick, rcRate, superRate, expo) {
            const absStick = Math.abs(stick);
            const stickCubed = stick * stick * stick;
            const rcFactor = stick * (1.0 - expo) + stickCubed * expo;
            let superFactor = 1.0;
            const denom = 1.0 - (absStick * superRate);
            if (denom > 0.01) superFactor = 1.0 / denom;
            return rcFactor * rcRate * superFactor * 200.0;
        }

        function updateRatesPreview() {
            const rcRate = parseFloat(document.getElementById('inpRcRate').value);
            const superRate = parseFloat(document.getElementById('inpSuperRate').value);
            const expo = parseFloat(document.getElementById('inpExpo').value);

            document.getElementById('lblRcRate').innerText = rcRate.toFixed(2);
            document.getElementById('lblSuperRate').innerText = superRate.toFixed(2);
            document.getElementById('lblExpo').innerText = expo.toFixed(2);

            const maxRate = calculateBetaflightRate(1.0, rcRate, superRate, expo);
            document.getElementById('maxRateTag').innerText = \`Max: \${Math.round(maxRate)} °/s\`;

            renderRateCurve(rcRate, superRate, expo, maxRate);
        }

        function renderRateCurve(rcRate, superRate, expo, maxRate) {
            const ctx = rateCtx;
            const w = rateCanvas.width;
            const h = rateCanvas.height;
            if (w === 0 || h === 0) return;

            ctx.clearRect(0, 0, w, h);

            // Grid
            ctx.strokeStyle = '#1e293b';
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(0, h/2); ctx.lineTo(w, h/2);
            ctx.moveTo(w/2, 0); ctx.lineTo(w/2, h);
            ctx.stroke();

            // Rate Curve
            ctx.strokeStyle = '#a855f7';
            ctx.lineWidth = 2;
            ctx.beginPath();

            const maxRateCap = Math.max(800, maxRate);
            for (let i = 0; i <= w; i++) {
                const stick = ((i / w) * 2.0) - 1.0; // [-1.0, 1.0]
                const degRate = calculateBetaflightRate(stick, rcRate, superRate, expo);
                const py = (h / 2) - (degRate / maxRateCap) * (h / 2);
                if (i === 0) ctx.moveTo(i, py);
                else ctx.lineTo(i, py);
            }
            ctx.stroke();

            // Real-time Virtual Stick Cursor
            const curStick = state.rc.roll / 45.0; // normalized
            const curX = ((curStick + 1.0) / 2.0) * w;
            const curRate = calculateBetaflightRate(curStick, rcRate, superRate, expo);
            const curY = (h / 2) - (curRate / maxRateCap) * (h / 2);

            ctx.fillStyle = '#00f3ff';
            ctx.beginPath();
            ctx.arc(curX, curY, 4, 0, Math.PI * 2);
            ctx.fill();
        }

        // === VIRTUAL RC STICK CONTROLS ===
        function setupStickPad(padId, knobId, isThrottleCenterZero, onChange) {
            const pad = document.getElementById(padId);
            const knob = document.getElementById(knobId);
            let active = false;

            function updateFromEvent(e) {
                const rect = pad.getBoundingClientRect();
                const clientX = e.touches ? e.touches[0].clientX : e.clientX;
                const clientY = e.touches ? e.touches[0].clientY : e.clientY;

                let nx = ((clientX - rect.left) / rect.width) * 2 - 1;
                let ny = ((clientY - rect.top) / rect.height) * 2 - 1;

                nx = Math.max(-1, Math.min(1, nx));
                ny = Math.max(-1, Math.min(1, ny));

                knob.style.left = ((nx + 1) / 2 * 100) + '%';
                knob.style.top = ((ny + 1) / 2 * 100) + '%';

                onChange(nx, -ny);
            }

            pad.addEventListener('mousedown', (e) => { active = true; updateFromEvent(e); });
            pad.addEventListener('touchstart', (e) => { active = true; updateFromEvent(e); }, { passive: false });

            window.addEventListener('mousemove', (e) => { if (active) updateFromEvent(e); });
            window.addEventListener('touchmove', (e) => { if (active) updateFromEvent(e); }, { passive: false });

            window.addEventListener('mouseup', () => {
                if (!active) return;
                active = false;
                if (!isThrottleCenterZero) {
                    // Reset to center
                    knob.style.left = '50%';
                    knob.style.top = '50%';
                    onChange(0, 0);
                } else {
                    // Reset yaw to center, keep throttle
                    knob.style.left = '50%';
                    onChange(0, null);
                }
            });
            window.addEventListener('touchend', () => {
                if (!active) return;
                active = false;
                if (!isThrottleCenterZero) {
                    knob.style.left = '50%';
                    knob.style.top = '50%';
                    onChange(0, 0);
                }
            });
        }

        // Setup Left Stick (Throttle & Yaw)
        setupStickPad('stickLeft', 'knobLeft', true, (x, y) => {
            state.rc.yaw = x * 180.0; // ±180°/s
            if (y !== null) {
                state.rc.thr = Math.max(0, Math.min(100, (y + 1) / 2 * 100));
            }
            document.getElementById('valRcThr').innerText = Math.round(state.rc.thr) + '%';
            document.getElementById('valRcYaw').innerText = Math.round(state.rc.yaw) + '°/s';
            sendRcData();
        });

        // Setup Right Stick (Roll & Pitch)
        setupStickPad('stickRight', 'knobRight', false, (x, y) => {
            state.rc.roll = x * 45.0;  // ±45°
            state.rc.pitch = y * 45.0; // ±45°
            document.getElementById('valRcRoll').innerText = state.rc.roll.toFixed(1) + '°';
            document.getElementById('valRcPitch').innerText = state.rc.pitch.toFixed(1) + '°';
            sendRcData();
        });

        // Keyboard Controls
        window.addEventListener('keydown', (e) => {
            if (document.activeElement.tagName === 'INPUT') return;
            if (e.key === ' ' || e.code === 'Space') {
                e.preventDefault();
                disarmEmergency();
            }
        });

        // === SERIAL COMMUNICATION & PROTOCOL ($TEL Telemetry) ===
        async function toggleSerial() {
            if (state.isConnected && state.serialPort) {
                await disconnectSerial();
                return;
            }

            if (!navigator.serial) {
                alert("Trình duyệt của bạn không hỗ trợ Web Serial API! Vui lòng dùng Google Chrome, Microsoft Edge hoặc Opera.");
                return;
            }

            try {
                const port = await navigator.serial.requestPort();
                await port.open({ baudRate: 115200 });

                state.serialPort = port;
                state.isConnected = true;
                setConnectedUI(true, 'SERIAL 115200');

                startSerialReadLoop();
                startHeartbeat();
            } catch (err) {
                logTerm("[LỖI KẾT NỐI SERIAL] " + err.message);
            }
        }

        async function disconnectSerial() {
            stopHeartbeat();
            if (state.serialReader) {
                await state.serialReader.cancel();
                state.serialReader = null;
            }
            if (state.serialPort) {
                await state.serialPort.close();
                state.serialPort = null;
            }
            state.isConnected = false;
            setConnectedUI(false, 'DISCONNECTED');
        }

        async function startSerialReadLoop() {
            const textDecoder = new TextDecoderStream();
            const readableStreamClosed = state.serialPort.readable.pipeTo(textDecoder.writable);
            const reader = textDecoder.readable.getReader();
            state.serialReader = reader;

            let lineBuffer = "";

            try {
                while (true) {
                    const { value, done } = await reader.read();
                    if (done) break;
                    if (value) {
                        lineBuffer += value;
                        const lines = lineBuffer.split(/\\r?\\n/);
                        lineBuffer = lines.pop(); // Keep partial line

                        for (const line of lines) {
                            if (line.trim()) processIncomingLine(line.trim());
                        }
                    }
                }
            } catch (err) {
                logTerm("[MẤT KẾT NỐI SERIAL] " + err.message);
            } finally {
                reader.releaseLock();
            }
        }

        // Wi-Fi WebSocket Connection
        function toggleWifi() {
            if (state.ws && state.isConnected) {
                state.ws.close();
                return;
            }
            const ip = prompt("Nhập địa chỉ IP của ESP32-S3 WebSocket:", "192.168.4.1");
            if (!ip) return;

            const wsUrl = \`ws://\${ip}/ws\`;
            logTerm("[WI-FI] Đang kết nối tới " + wsUrl);

            const ws = new WebSocket(wsUrl);
            ws.onopen = () => {
                state.ws = ws;
                state.isConnected = true;
                setConnectedUI(true, \`WI-FI WS (\${ip})\`);
                startHeartbeat();
            };
            ws.onmessage = (e) => {
                if (e.data) processIncomingLine(e.data.trim());
            };
            ws.onerror = (e) => logTerm("[LỖI WI-FI WS]");
            ws.onclose = () => {
                stopHeartbeat();
                state.ws = null;
                state.isConnected = false;
                setConnectedUI(false, 'DISCONNECTED');
            };
        }

        // Protocol Parser ($TEL Telemetry Packet - 21 Fields)
        // Format: $TEL,roll,pitch,yaw,rateRoll,ratePitch,rateYaw,throttle,m1,m2,m3,m4,altitude,armed,fsState,imuOk,baroOk,magOk,pcaOk,ax,ay,az
        function processIncomingLine(line) {
            if (line.startsWith('$TEL,')) {
                const parts = line.split(',');
                if (parts.length >= 14) {
                    state.roll = parseFloat(parts[1]) || 0;
                    state.pitch = parseFloat(parts[2]) || 0;
                    state.yaw = parseFloat(parts[3]) || 0;
                    state.gyro[0] = parseFloat(parts[4]) || 0; // rateRoll
                    state.gyro[1] = parseFloat(parts[5]) || 0; // ratePitch
                    state.gyro[2] = parseFloat(parts[6]) || 0; // rateYaw
                    const throttle = parseFloat(parts[7]) || 0;
                    state.motors[0] = parseInt(parts[8]) || 1000;
                    state.motors[1] = parseInt(parts[9]) || 1000;
                    state.motors[2] = parseInt(parts[10]) || 1000;
                    state.motors[3] = parseInt(parts[11]) || 1000;
                    state.alt = parseFloat(parts[12]) || 0;
                    state.isArmed = (parseInt(parts[13]) === 1);

                    if (parts.length >= 19) {
                        state.fsState = parseInt(parts[14]) || 0;
                        state.imuOk = (parseInt(parts[15]) === 1);
                        state.baroOk = (parseInt(parts[16]) === 1);
                        state.magOk = (parseInt(parts[17]) === 1);
                        state.pcaOk = (parseInt(parts[18]) === 1);
                    }
                    if (parts.length >= 22) {
                        state.accel[0] = parseFloat(parts[19]) || 0;
                        state.accel[1] = parseFloat(parts[20]) || 0;
                        state.accel[2] = parseFloat(parts[21]) || 1.0;
                    }

                    updateTelemetryUI();
                    addScopeSample();
                }
            } else {
                logTerm(line);
            }
        }

        function updateTelemetryUI() {
            document.getElementById('valRoll').innerText = state.roll.toFixed(1) + '°';
            document.getElementById('valPitch').innerText = state.pitch.toFixed(1) + '°';
            document.getElementById('valYaw').innerText = state.yaw.toFixed(1) + '°';

            document.getElementById('valAltitude').innerText = state.alt.toFixed(2) + ' m';
            document.getElementById('valVario').innerText = (state.vario >= 0 ? '+' : '') + state.vario.toFixed(1) + ' m/s';
            document.getElementById('valBatt').innerText = state.batt.toFixed(1) + ' V';

            // Motors
            for (let i = 0; i < 4; i++) {
                const pulse = state.motors[i];
                document.getElementById(\`pM\${i+1}\`).innerText = pulse + 'µs';
                const pct = Math.max(0, Math.min(100, (pulse - 1000) / 10));
                document.getElementById(\`barM\${i+1}\`).style.width = pct + '%';
            }

            // Hardware Health Status Badges
            if (state.imuOk !== undefined) {
                const el = document.getElementById('stImu');
                el.innerText = state.imuOk ? 'OK' : 'FAIL';
                el.style.color = state.imuOk ? '#34d399' : '#ef4444';
            }
            if (state.baroOk !== undefined) {
                const el = document.getElementById('stBaro');
                el.innerText = state.baroOk ? 'OK' : 'FAIL';
                el.style.color = state.baroOk ? '#34d399' : '#ef4444';
            }
            if (state.magOk !== undefined) {
                const el = document.getElementById('stMag');
                el.innerText = state.magOk ? 'OK' : 'FAIL';
                el.style.color = state.magOk ? '#34d399' : '#ef4444';
            }
            if (state.pcaOk !== undefined) {
                const el = document.getElementById('stPca');
                el.innerText = state.pcaOk ? 'OK' : 'FAIL';
                el.style.color = state.pcaOk ? '#34d399' : '#ef4444';
            }

            // Arm Button State
            const btnArm = document.getElementById('btnArm');
            if (state.isArmed) {
                btnArm.className = "btn btn-danger";
                btnArm.innerHTML = "<span>🛑</span> DISARM MOTORS";
            } else {
                btnArm.className = "btn btn-primary";
                btnArm.innerHTML = "<span>⚠️</span> ARM MOTORS";
            }
        }

        // Send Commands to Firmware
        async function sendRaw(cmdStr) {
            const line = cmdStr.endsWith('\\n') ? cmdStr : (cmdStr + '\\n');
            if (state.serialPort && state.serialPort.writable) {
                const writer = state.serialPort.writable.getWriter();
                const encoder = new TextEncoder();
                await writer.write(encoder.encode(line));
                writer.releaseLock();
            } else if (state.ws && state.ws.readyState === WebSocket.OPEN) {
                state.ws.send(line);
            }
        }

        function sendCmd(cmd) {
            logTerm("> " + cmd);
            sendRaw(cmd);
        }

        function sendRcData() {
            if (!state.isConnected) return;
            const cmd = \`SET RC \${state.rc.thr.toFixed(1)} \${state.rc.roll.toFixed(1)} \${state.rc.pitch.toFixed(1)} \${state.rc.yaw.toFixed(1)}\`;
            sendRaw(cmd);
        }

        function setFlightMode(mode) {
            state.flightMode = mode;
            document.getElementById('modeTag').innerText = 'MODE: ' + mode;
            sendCmd('SET MODE ' + mode);
        }

        function toggleArm() {
            if (state.isArmed) {
                sendCmd('DISARM');
            } else {
                sendCmd('ARM');
            }
        }

        function disarmEmergency() {
            sendCmd('DISARM');
            state.rc.thr = 0;
            document.getElementById('valRcThr').innerText = '0%';
            logTerm("[KHẨN CẤP] ĐÃ NGẮT TOÀN BỘ ĐỘNG CƠ (DISARM)!");
        }

        function sendCalib(sensor) {
            sendCmd('CALIB ' + sensor);
        }

        function toggleAirMode(enable) {
            state.airMode = enable;
            sendCmd('SET AIRMODE ' + (enable ? '1' : '0'));
        }

        function sendTpa() {
            const rate = parseFloat(document.getElementById('inpTpaRate').value);
            const bp = parseFloat(document.getElementById('inpTpaBp').value);
            sendCmd(\`SET TPA \${rate.toFixed(2)} \${bp.toFixed(2)}\`);
        }

        function sendEstimator() {
            const algo = document.getElementById('selEstimator').value;
            const gain = parseFloat(document.getElementById('inpGain').value);
            sendCmd(\`SET ESTIMATOR \${algo} \${gain.toFixed(3)}\`);
        }

        function sendFilters() {
            const gyro = document.getElementById('fltGyroLpf').value;
            const dterm = document.getElementById('fltDtermLpf').value;
            const notch = document.getElementById('fltNotch').value;
            sendCmd(\`SET FILTER GYRO_LPF \${gyro}\`);
            sendCmd(\`SET FILTER DTERM \${dterm}\`);
            sendCmd(\`SET FILTER NOTCH \${notch}\`);
        }

        function sendRates() {
            const rcRate = parseFloat(document.getElementById('inpRcRate').value);
            const superRate = parseFloat(document.getElementById('inpSuperRate').value);
            const expo = parseFloat(document.getElementById('inpExpo').value);
            sendCmd(\`SET RATES \${rcRate.toFixed(2)} \${superRate.toFixed(2)} \${expo.toFixed(2)}\`);
        }

        function sendPidAxis(axis) {
            let p, i, d;
            if (axis === 'ROLL') {
                p = document.getElementById('roll_rate_p').value;
                i = document.getElementById('roll_rate_i').value;
                d = document.getElementById('roll_rate_d').value;
            } else if (axis === 'PITCH') {
                p = document.getElementById('pitch_rate_p').value;
                i = document.getElementById('pitch_rate_i').value;
                d = document.getElementById('pitch_rate_d').value;
            } else if (axis === 'YAW') {
                p = document.getElementById('yaw_rate_p').value;
                i = document.getElementById('yaw_rate_i').value;
                d = document.getElementById('yaw_rate_d').value;
            }
            sendCmd(\`SET PID \${axis} \${p} \${i} \${d}\`);
        }

        function sendAllPids() {
            sendPidAxis('ROLL');
            sendPidAxis('PITCH');
            sendPidAxis('YAW');
        }

        function stepPid(inputId, step) {
            const input = document.getElementById(inputId);
            let val = parseFloat(input.value) + step;
            if (step >= 0.05 || step <= -0.05) {
                input.value = val.toFixed(2);
            } else {
                input.value = val.toFixed(3);
            }
        }

        function runMotorTest(mNum, percent) {
            document.getElementById(\`lblTestM\${mNum}\`).innerText = percent + '%';
            sendCmd(\`TEST M\${mNum} \${percent}\`);
        }

        function stopMotorTest(mNum) {
            document.getElementById(\`testM\${mNum}\`).value = 0;
            document.getElementById(\`lblTestM\${mNum}\`).innerText = '0%';
            sendCmd(\`TEST M\${mNum} 0\`);
        }

        function logTerm(msg) {
            const box = document.getElementById('termBox');
            box.innerText += msg + '\\n';
            box.scrollTop = box.scrollHeight;
        }

        function clearTerm() {
            document.getElementById('termBox').innerText = '';
        }

        function handleTermKey(e) {
            if (e.key === 'Enter') sendManualTerm();
        }

        function sendManualTerm() {
            const input = document.getElementById('termInput');
            const cmd = input.value.trim();
            if (cmd) {
                sendCmd(cmd);
                input.value = '';
            }
        }

        function setConnectedUI(connected, label) {
            const pill = document.getElementById('connStatus');
            const lbl = document.getElementById('connLabel');
            if (connected) {
                pill.className = "status-pill connected";
                lbl.innerText = label;
            } else {
                pill.className = "status-pill";
                lbl.innerText = label;
            }
        }

        let heartbeatTimer = null;

        function startHeartbeat() {
            stopHeartbeat();
            heartbeatTimer = setInterval(() => {
                if (state.isConnected) {
                    sendRaw('PING');
                }
            }, 500);
        }

        function stopHeartbeat() {
            if (heartbeatTimer) {
                clearInterval(heartbeatTimer);
                heartbeatTimer = null;
            }
        }

        // === PID PRESETS & TUNING ASSISTANT LOGIC ===
        const PID_PRESETS = {
            freestyle: {
                name: '5-inch Freestyle / Acro (Default)',
                roll_ang_p: 4.50, pitch_ang_p: 4.50,
                roll_p: 1.20, roll_i: 0.040, roll_d: 0.035,
                pitch_p: 1.20, pitch_i: 0.040, pitch_d: 0.035,
                yaw_p: 2.50, yaw_i: 0.080, yaw_d: 0.000,
                tpa_rate: 0.20, tpa_bp: 0.50,
                rc_rate: 1.00, super_rate: 0.70, expo: 0.15,
                gyro_lpf: 90, dterm_lpf: 70
            },
            cinewhoop: {
                name: 'Cinewhoop / Smooth 4S (Mát máy)',
                roll_ang_p: 3.80, pitch_ang_p: 3.80,
                roll_p: 0.95, roll_i: 0.035, roll_d: 0.022,
                pitch_p: 0.95, pitch_i: 0.035, pitch_d: 0.022,
                yaw_p: 2.00, yaw_i: 0.060, yaw_d: 0.000,
                tpa_rate: 0.30, tpa_bp: 0.45,
                rc_rate: 0.85, super_rate: 0.55, expo: 0.25,
                gyro_lpf: 80, dterm_lpf: 60
            },
            longrange: {
                name: 'Long Range / 7-inch Cruiser',
                roll_ang_p: 5.20, pitch_ang_p: 5.20,
                roll_p: 1.45, roll_i: 0.055, roll_d: 0.045,
                pitch_p: 1.45, pitch_i: 0.055, pitch_d: 0.045,
                yaw_p: 3.00, yaw_i: 0.100, yaw_d: 0.000,
                tpa_rate: 0.25, tpa_bp: 0.40,
                rc_rate: 0.90, super_rate: 0.60, expo: 0.20,
                gyro_lpf: 85, dterm_lpf: 65
            },
            racer: {
                name: 'Racer / High Agility (Gắt)',
                roll_ang_p: 5.50, pitch_ang_p: 5.50,
                roll_p: 1.60, roll_i: 0.045, roll_d: 0.040,
                pitch_p: 1.60, pitch_i: 0.045, pitch_d: 0.040,
                yaw_p: 3.20, yaw_i: 0.090, yaw_d: 0.000,
                tpa_rate: 0.35, tpa_bp: 0.55,
                rc_rate: 1.20, super_rate: 0.78, expo: 0.10,
                gyro_lpf: 110, dterm_lpf: 85
            },
            safe_test: {
                name: 'Indoor / Desk Safe Test',
                roll_ang_p: 3.00, pitch_ang_p: 3.00,
                roll_p: 0.70, roll_i: 0.025, roll_d: 0.015,
                pitch_p: 0.70, pitch_i: 0.025, pitch_d: 0.015,
                yaw_p: 1.50, yaw_i: 0.040, yaw_d: 0.000,
                tpa_rate: 0.10, tpa_bp: 0.60,
                rc_rate: 0.70, super_rate: 0.40, expo: 0.30,
                gyro_lpf: 75, dterm_lpf: 55
            }
        };

        let currentBasePreset = JSON.parse(JSON.stringify(PID_PRESETS.freestyle));

        function applyPidPreset(presetKey) {
            const p = PID_PRESETS[presetKey];
            if (!p) return;

            currentBasePreset = JSON.parse(JSON.stringify(p));

            // Sync Dropdown
            const sel = document.getElementById('selQuickPreset');
            if (sel) sel.value = presetKey;

            // Apply to form inputs
            document.getElementById('roll_ang_p').value = p.roll_ang_p.toFixed(2);
            document.getElementById('pitch_ang_p').value = p.pitch_ang_p.toFixed(2);

            document.getElementById('roll_rate_p').value = p.roll_p.toFixed(2);
            document.getElementById('roll_rate_i').value = p.roll_i.toFixed(3);
            document.getElementById('roll_rate_d').value = p.roll_d.toFixed(3);

            document.getElementById('pitch_rate_p').value = p.pitch_p.toFixed(2);
            document.getElementById('pitch_rate_i').value = p.pitch_i.toFixed(3);
            document.getElementById('pitch_rate_d').value = p.pitch_d.toFixed(3);

            document.getElementById('yaw_rate_p').value = p.yaw_p.toFixed(2);
            document.getElementById('yaw_rate_i').value = p.yaw_i.toFixed(3);
            document.getElementById('yaw_rate_d').value = p.yaw_d.toFixed(3);

            // TPA
            document.getElementById('inpTpaRate').value = p.tpa_rate;
            document.getElementById('lblTpaRate').innerText = Math.round(p.tpa_rate * 100) + '%';
            document.getElementById('inpTpaBp').value = p.tpa_bp;
            document.getElementById('lblTpaBp').innerText = Math.round(p.tpa_bp * 100) + '%';

            // Filters
            document.getElementById('fltGyroLpf').value = p.gyro_lpf;
            document.getElementById('fltDtermLpf').value = p.dterm_lpf;

            // Rates
            document.getElementById('inpRcRate').value = p.rc_rate;
            document.getElementById('inpSuperRate').value = p.super_rate;
            document.getElementById('inpExpo').value = p.expo;
            updateRatesPreview();

            // Reset Sliders
            document.getElementById('inpMasterScale').value = 1.0;
            document.getElementById('lblMasterScale').innerText = '1.00x';
            if (document.getElementById('inpAssistantMasterScale')) {
                document.getElementById('inpAssistantMasterScale').value = 1.0;
                document.getElementById('lblAssistantMasterScale').innerText = '1.00x';
            }
            document.getElementById('inpPdBalance').value = 0.0;
            document.getElementById('lblPdBalance').innerText = 'Neutral';
            if (document.getElementById('inpAssistantPdBalance')) {
                document.getElementById('inpAssistantPdBalance').value = 0.0;
                document.getElementById('lblAssistantPdBalance').innerText = 'Neutral';
            }

            logTerm(\`[TUNING PRESET] Đã nạp cấu hình: \${p.name}\`);
        }

        function applyMasterMultiplier(scale) {
            document.getElementById('lblMasterScale').innerText = scale.toFixed(2) + 'x';
            if (document.getElementById('lblAssistantMasterScale')) {
                document.getElementById('lblAssistantMasterScale').innerText = scale.toFixed(2) + 'x';
                document.getElementById('inpAssistantMasterScale').value = scale;
            }
            document.getElementById('inpMasterScale').value = scale;

            const base = currentBasePreset;
            document.getElementById('roll_rate_p').value = (base.roll_p * scale).toFixed(2);
            document.getElementById('roll_rate_i').value = (base.roll_i * scale).toFixed(3);
            document.getElementById('roll_rate_d').value = (base.roll_d * scale).toFixed(3);

            document.getElementById('pitch_rate_p').value = (base.pitch_p * scale).toFixed(2);
            document.getElementById('pitch_rate_i').value = (base.pitch_i * scale).toFixed(3);
            document.getElementById('pitch_rate_d').value = (base.pitch_d * scale).toFixed(3);

            document.getElementById('yaw_rate_p').value = (base.yaw_p * scale).toFixed(2);
            document.getElementById('yaw_rate_i').value = (base.yaw_i * scale).toFixed(3);

            logTerm(\`[MASTER SCALE] Điều chỉnh toàn bộ hệ số PID tỷ lệ \${scale.toFixed(2)}x\`);
        }

        function applyPdBalance(balance) {
            let label = 'Neutral';
            if (balance > 0.02) label = 'Crisp (+' + Math.round(balance * 100) + '% P)';
            else if (balance < -0.02) label = 'Smooth (+' + Math.round(-balance * 100) + '% D)';

            document.getElementById('lblPdBalance').innerText = label;
            if (document.getElementById('lblAssistantPdBalance')) {
                document.getElementById('lblAssistantPdBalance').innerText = label;
                document.getElementById('inpAssistantPdBalance').value = balance;
            }
            document.getElementById('inpPdBalance').value = balance;

            const pFactor = 1.0 + balance;
            const dFactor = 1.0 - balance;

            const base = currentBasePreset;
            const masterScale = parseFloat(document.getElementById('inpMasterScale').value) || 1.0;

            document.getElementById('roll_rate_p').value = (base.roll_p * masterScale * pFactor).toFixed(2);
            document.getElementById('roll_rate_d').value = (base.roll_d * masterScale * dFactor).toFixed(3);
            document.getElementById('pitch_rate_p').value = (base.pitch_p * masterScale * pFactor).toFixed(2);
            document.getElementById('pitch_rate_d').value = (base.pitch_d * masterScale * dFactor).toFixed(3);
        }

        function applyDiagnosticFix(fixType) {
            if (fixType === 'high_freq_shake') {
                // Giảm D 15%, giảm D-LPF xuống 70Hz
                let d = parseFloat(document.getElementById('roll_rate_d').value) * 0.85;
                document.getElementById('roll_rate_d').value = d.toFixed(3);
                document.getElementById('pitch_rate_d').value = d.toFixed(3);
                document.getElementById('fltDtermLpf').value = 70;
                logTerm("[DIAGNOSTIC] Đã giảm D-Term 15% và hạ D-LPF về 70Hz để triệt tiêu rung cao tần.");
            } else if (fixType === 'drift_yaw') {
                // Tăng I 20%, tăng P 10%
                let i = parseFloat(document.getElementById('roll_rate_i').value) * 1.20;
                let p = parseFloat(document.getElementById('roll_rate_p').value) * 1.10;
                document.getElementById('roll_rate_i').value = i.toFixed(3);
                document.getElementById('pitch_rate_i').value = i.toFixed(3);
                document.getElementById('roll_rate_p').value = p.toFixed(2);
                document.getElementById('pitch_rate_p').value = p.toFixed(2);
                logTerm("[DIAGNOSTIC] Đã tăng I-Term +20% và P-Term +10% để giữ góc ổn định và chống trôi dạt.");
            } else if (fixType === 'bounceback') {
                // Tăng D 12%
                let d = parseFloat(document.getElementById('roll_rate_d').value) * 1.12;
                document.getElementById('roll_rate_d').value = d.toFixed(3);
                document.getElementById('pitch_rate_d').value = d.toFixed(3);
                logTerm("[DIAGNOSTIC] Đã tăng D-Term +12% để dập tắt hiện tượng nảy ngược sau khi nhả cần lộn.");
            } else if (fixType === 'hot_motors') {
                // Hạ D 25% và giảm D-LPF xuống 60Hz
                let d = parseFloat(document.getElementById('roll_rate_d').value) * 0.75;
                document.getElementById('roll_rate_d').value = d.toFixed(3);
                document.getElementById('pitch_rate_d').value = d.toFixed(3);
                document.getElementById('fltDtermLpf').value = 60;
                logTerm("[DIAGNOSTIC] Đã hạ D-Term 25% và lọc D-LPF về 60Hz để giải nhiệt, bảo vệ động cơ.");
            } else if (fixType === 'sluggish_stick') {
                // Tăng Kff
                logTerm("[DIAGNOSTIC] Tăng Feedforward (kff) và SuperRate cho phản xạ bám cần lái tức thì.");
                let sr = Math.min(0.85, parseFloat(document.getElementById('inpSuperRate').value) + 0.05);
                document.getElementById('inpSuperRate').value = sr;
                updateRatesPreview();
            }
        }

        // Modal Controllers
        function openPidGuideModal() {
            document.getElementById('pidGuideModal').style.display = 'flex';
        }

        function closePidGuideModal() {
            document.getElementById('pidGuideModal').style.display = 'none';
        }

        function switchPidGuideTab(tabId) {
            const tabs = ['tabSteps', 'tabTrouble', 'tabMath', 'tabSafety'];
            const btns = document.querySelectorAll('#pidGuideModal .tab-btn');

            tabs.forEach((id, idx) => {
                const pane = document.getElementById(id);
                if (pane) {
                    if (id === tabId) {
                        pane.className = 'tab-pane active';
                        if (btns[idx]) btns[idx].className = 'tab-btn active';
                    } else {
                        pane.className = 'tab-pane';
                        if (btns[idx]) btns[idx].className = 'tab-btn';
                    }
                }
            });
        }

        function openTuningAssistantModal() {
            document.getElementById('tuningAssistantModal').style.display = 'flex';
        }

        function closeTuningAssistantModal() {
            document.getElementById('tuningAssistantModal').style.display = 'none';
        }

        // Export Config Modal
        function openConfigModal() {
            const rollP = document.getElementById('roll_rate_p').value;
            const rollI = document.getElementById('roll_rate_i').value;
            const rollD = document.getElementById('roll_rate_d').value;
            const pitchP = document.getElementById('pitch_rate_p').value;
            const pitchI = document.getElementById('pitch_rate_i').value;
            const pitchD = document.getElementById('pitch_rate_d').value;
            const yawP = document.getElementById('yaw_rate_p').value;
            const yawI = document.getElementById('yaw_rate_i').value;
            const yawD = document.getElementById('yaw_rate_d').value;

            const code = \`// === ESP32-S3 FLIGHT CONTROLLER TUNED CONFIGURATION ===
#define PID_ROLL_RATE_KP    \${rollP}f
#define PID_ROLL_RATE_KI    \${rollI}f
#define PID_ROLL_RATE_KD    \${rollD}f

#define PID_PITCH_RATE_KP   \${pitchP}f
#define PID_PITCH_RATE_KI   \${pitchI}f
#define PID_PITCH_RATE_KD   \${pitchD}f

#define PID_YAW_RATE_KP     \${yawP}f
#define PID_YAW_RATE_KI     \${yawI}f
#define PID_YAW_RATE_KD     \${yawD}f

#define TPA_RATE            \${document.getElementById('inpTpaRate').value}f
#define TPA_BREAKPOINT      \${document.getElementById('inpTpaBp').value}f
#define RC_RATE_DEFAULT     \${document.getElementById('inpRcRate').value}f
#define SUPER_RATE_DEFAULT  \${document.getElementById('inpSuperRate').value}f
#define EXPO_DEFAULT        \${document.getElementById('inpExpo').value}f
\`;
            document.getElementById('cfgCode').innerText = code;
            document.getElementById('cfgModal').style.display = 'flex';
        }

        function closeConfigModal() {
            document.getElementById('cfgModal').style.display = 'none';
        }

        function copyConfigCode() {
            const code = document.getElementById('cfgCode').innerText;
            navigator.clipboard.writeText(code);
            alert("Đã sao chép cấu hình C++ vào Clipboard!");
        }

        // === MAIN 60FPS ANIMATION LOOP ===
        function mainLoop() {
            renderAttitude();
            renderScope();
            requestAnimationFrame(mainLoop);
        }

        // Initialize Everything
        updateRatesPreview();
        requestAnimationFrame(mainLoop);
    </script>
</body>
</html>
`;

fs.writeFileSync(indexPath, generatedHtml, 'utf8');
console.log("Successfully generated tools/tuner/index.html! File size:", generatedHtml.length);
