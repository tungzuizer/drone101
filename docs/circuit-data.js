/**
 * WIRE_CONNECTIONS_DB - Cơ sở dữ liệu 69 tuyến dây vật lý và ánh xạ chân kết nối
 * ESP32-S3 Flight Controller Quad-X Drone
 */
const WIRE_CONNECTIONS_DB = [
    // === NHÓM 1: NGUỒN 11.1V VÀ 12 DÂY PHA ĐỘNG LỰC (grp-power) ===
    {
        id: "wire-pwr-bat-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "Pin LiPo 3S (+) ➔ PDB (+)",
        color: "#dc2626",
        colorName: "Đỏ đậm (Silicon chịu nhiệt)",
        gauge: "12–14 AWG Silicon",
        signalType: "Nguồn DC 11.1V–12.6V (Dòng xả đỉnh 45A–60A)",
        filterComponent: "Tụ chính PDB 1000µF/25V Low-ESR",
        source: {
            board: "Pin LiPo 3S (2200mAh 25C)",
            pin: "Cực Dương (+)",
            pinNumber: "Jack XT60 Đực (+)",
            voltage: "11.1V (Max 12.6V)",
            location: "Cáp xả pin chính",
            coords: { x: 200, y: 110 }
        },
        dest: {
            board: "PDB (Power Distribution Board)",
            pin: "Trạm Hàn Nguồn Chính (+)",
            pinNumber: "Cọc V_BAT In (+)",
            voltage: "11.1V–12.6V",
            location: "Cọc nguồn trung tâm PDB",
            coords: { x: 290, y: 95 }
        },
        note: "Hàn chắc chắn bằng thiếc hàn chất lượng cao, bọc co nhiệt đầy đủ. Chịu toàn bộ dòng xả của 4 động cơ khi thốc ga tối đa."
    },
    {
        id: "wire-pwr-bat-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "Pin LiPo 3S (-) ➔ PDB (-)",
        color: "#475569",
        colorName: "Đen (Silicon chịu nhiệt)",
        gauge: "12–14 AWG Silicon",
        signalType: "Mass chính toàn hệ thống (GND công suất)",
        filterComponent: "Mắc song song Tụ 1000µF/25V Low-ESR",
        source: {
            board: "Pin LiPo 3S (2200mAh 25C)",
            pin: "Cực Âm (-)",
            pinNumber: "Jack XT60 Đực (-)",
            voltage: "0V (GND Mass)",
            location: "Cáp xả pin chính",
            coords: { x: 170, y: 110 }
        },
        dest: {
            board: "PDB (Power Distribution Board)",
            pin: "Trạm Hàn Nguồn Chính (-)",
            pinNumber: "Cọc GND In (-)",
            voltage: "0V (GND Mass)",
            location: "Cọc nguồn trung tâm PDB",
            coords: { x: 290, y: 120 }
        },
        note: "Tuyến mass chịu tải công suất cao nhất của máy bay. Mắc tụ hóa 1000µF/25V Low-ESR trực tiếp giữa 2 cọc (+) và (-) trên PDB."
    },
    {
        id: "wire-pwr-ubec-in-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB (+) ➔ UBEC 11.1V In (+)",
        color: "#dc2626",
        colorName: "Đỏ",
        gauge: "20 AWG",
        signalType: "Nguồn DC 11.1V nuôi UBEC",
        filterComponent: "Tụ gốm nội bộ UBEC",
        source: {
            board: "PDB",
            pin: "Trạm Phụ (+)",
            pinNumber: "Pad Aux 11.1V (+)",
            voltage: "11.1V–12.6V",
            location: "Trạm hàn PDB bên phải",
            coords: { x: 570, y: 65 }
        },
        dest: {
            board: "UBEC 5V/3A Hạ Áp Xung",
            pin: "VIN (+)",
            pinNumber: "Dây Đỏ Vào UBEC",
            voltage: "11.1V–12.6V (Chịu áp max 26V)",
            location: "Đầu vào mạch UBEC",
            coords: { x: 850, y: 65 }
        },
        note: "Cấp điện áp thô 11.1V từ Pin để UBEC buck switching hạ áp xuống 5.0V sạch cấp nguồn cho ESP32 và cảm biến."
    },
    {
        id: "wire-pwr-ubec-in-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB (-) ➔ UBEC GND In (-)",
        color: "#475569",
        colorName: "Đen",
        gauge: "20 AWG",
        signalType: "Mass nguồn vào UBEC",
        filterComponent: "GND Chung",
        source: {
            board: "PDB",
            pin: "Trạm Phụ (-)",
            pinNumber: "Pad Aux GND (-)",
            voltage: "0V (GND)",
            location: "Trạm hàn PDB bên phải",
            coords: { x: 570, y: 85 }
        },
        dest: {
            board: "UBEC 5V/3A Hạ Áp Xung",
            pin: "GND (-)",
            pinNumber: "Dây Đen Vào UBEC",
            voltage: "0V (GND)",
            location: "Đầu vào mạch UBEC",
            coords: { x: 850, y: 85 }
        },
        note: "Đảm bảo UBEC chung mass hoàn toàn với PDB và Pin LiPo."
    },
    {
        id: "wire-pwr-vbat-div-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB (+) ➔ Cầu phân áp VBAT (R1 10kΩ)",
        color: "#dc2626",
        colorName: "Đỏ mảnh",
        gauge: "26 AWG",
        signalType: "Điện áp Pin 11.1V (Dòng cực nhỏ ~1mA)",
        filterComponent: "Cầu trở 10kΩ / 2.2kΩ + Tụ gốm 104",
        source: {
            board: "PDB",
            pin: "Trạm Đo Pin (+)",
            pinNumber: "V_BAT Out",
            voltage: "11.1V–12.6V",
            location: "Pad đo áp trên PDB",
            coords: { x: 570, y: 50 }
        },
        dest: {
            board: "Cầu Phân Áp VBAT ADC",
            pin: "Đầu vào R1 (10kΩ)",
            pinNumber: "R1 10kΩ Input",
            voltage: "11.1V–12.6V",
            location: "Mạch phân áp",
            coords: { x: 620, y: 50 }
        },
        note: "Hạ áp từ 12.6V xuống tối đa 2.27V (tỷ lệ 2.2k/(10k+2.2k) = 0.1803) để an toàn tuyệt đối cho chân ADC ESP32-S3 (ngưỡng tối đa 3.1V)."
    },
    {
        id: "wire-pwr-vbat-div-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB (-) ➔ Cầu phân áp VBAT (R2 2.2kΩ GND)",
        color: "#475569",
        colorName: "Đen mảnh",
        gauge: "26 AWG",
        signalType: "Mass cầu phân áp",
        filterComponent: "Mắc song song Tụ gốm 104 (0.1µF)",
        source: {
            board: "PDB",
            pin: "Trạm Đo Pin (-)",
            pinNumber: "GND Out",
            voltage: "0V",
            location: "Pad đo áp trên PDB",
            coords: { x: 570, y: 100 }
        },
        dest: {
            board: "Cầu Phân Áp VBAT ADC",
            pin: "Đầu dưới R2 (2.2kΩ) & Tụ 104",
            pinNumber: "R2 Ground Pad",
            voltage: "0V",
            location: "Mạch phân áp",
            coords: { x: 620, y: 100 }
        },
        note: "Chân mass của trở 2.2kΩ và cực âm của tụ 104 hàn chung vào đường Mass PDB."
    },
    {
        id: "wire-pwr-esc2-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 2 Nguồn (+) [Trước-Trái M2]",
        color: "#dc2626",
        colorName: "Đỏ",
        gauge: "14–16 AWG Silicon",
        signalType: "Nguồn công suất 11.1V (Dòng 10A–25A)",
        filterComponent: "Tụ 100µF/25V tại trạm ESC",
        source: {
            board: "PDB",
            pin: "Pad ESC 2 (+)",
            pinNumber: "ESC2_VCC",
            voltage: "11.1V–12.6V",
            location: "Trạm ra ESC 2 trên PDB",
            coords: { x: 395, y: 137 }
        },
        dest: {
            board: "ESC 2 30A (Front-Left M2 CW)",
            pin: "Dây Đỏ Nguồn (+)",
            pinNumber: "VCC Input",
            voltage: "11.1V–12.6V",
            location: "Cáp nguồn vào ESC 2",
            coords: { x: 80, y: 955 }
        },
        note: "Cấp nguồn công suất cho ESC 2. Hàn dây càng ngắn càng tốt để giảm tổn hao và phát xạ EMI."
    },
    {
        id: "wire-pwr-esc2-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 2 Nguồn (-) [Trước-Trái M2]",
        color: "#475569",
        colorName: "Đen",
        gauge: "14–16 AWG Silicon",
        signalType: "Mass công suất ESC 2",
        filterComponent: "GND PDB",
        source: {
            board: "PDB",
            pin: "Pad ESC 2 (-)",
            pinNumber: "ESC2_GND",
            voltage: "0V",
            location: "Trạm ra ESC 2 trên PDB",
            coords: { x: 413, y: 137 }
        },
        dest: {
            board: "ESC 2 30A (Front-Left M2 CW)",
            pin: "Dây Đen Nguồn (-)",
            pinNumber: "GND Input",
            voltage: "0V",
            location: "Cáp nguồn vào ESC 2",
            coords: { x: 105, y: 955 }
        },
        note: "Đường mass công suất chịu tải xung băm PWM của ESC 2."
    },
    {
        id: "wire-pwr-esc1-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 1 Nguồn (+) [Trước-Phải M1]",
        color: "#dc2626",
        colorName: "Đỏ",
        gauge: "14–16 AWG Silicon",
        signalType: "Nguồn công suất 11.1V (Dòng 10A–25A)",
        filterComponent: "Tụ 100µF/25V tại trạm ESC",
        source: {
            board: "PDB",
            pin: "Pad ESC 1 (+)",
            pinNumber: "ESC1_VCC",
            voltage: "11.1V–12.6V",
            location: "Trạm ra ESC 1 trên PDB",
            coords: { x: 340, y: 137 }
        },
        dest: {
            board: "ESC 1 30A (Front-Right M1 CCW)",
            pin: "Dây Đỏ Nguồn (+)",
            pinNumber: "VCC Input",
            voltage: "11.1V–12.6V",
            location: "Cáp nguồn vào ESC 1",
            coords: { x: 510, y: 955 }
        },
        note: "Cấp nguồn công suất cho ESC 1 điều khiển động cơ Trước-Phải M1."
    },
    {
        id: "wire-pwr-esc1-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 1 Nguồn (-) [Trước-Phải M1]",
        color: "#475569",
        colorName: "Đen",
        gauge: "14–16 AWG Silicon",
        signalType: "Mass công suất ESC 1",
        filterComponent: "GND PDB",
        source: {
            board: "PDB",
            pin: "Pad ESC 1 (-)",
            pinNumber: "ESC1_GND",
            voltage: "0V",
            location: "Trạm ra ESC 1 trên PDB",
            coords: { x: 358, y: 137 }
        },
        dest: {
            board: "ESC 1 30A (Front-Right M1 CCW)",
            pin: "Dây Đen Nguồn (-)",
            pinNumber: "GND Input",
            voltage: "0V",
            location: "Cáp nguồn vào ESC 1",
            coords: { x: 535, y: 955 }
        },
        note: "Mass công suất ESC 1."
    },
    {
        id: "wire-pwr-esc4-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 4 Nguồn (+) [Sau-Trái M4]",
        color: "#dc2626",
        colorName: "Đỏ",
        gauge: "14–16 AWG Silicon",
        signalType: "Nguồn công suất 11.1V (Dòng 10A–25A)",
        filterComponent: "Tụ 100µF/25V tại trạm ESC",
        source: {
            board: "PDB",
            pin: "Pad ESC 4 (+)",
            pinNumber: "ESC4_VCC",
            voltage: "11.1V–12.6V",
            location: "Trạm ra ESC 4 trên PDB",
            coords: { x: 505, y: 137 }
        },
        dest: {
            board: "ESC 4 30A (Rear-Left M4 CCW)",
            pin: "Dây Đỏ Nguồn (+)",
            pinNumber: "VCC Input",
            voltage: "11.1V–12.6V",
            location: "Cáp nguồn vào ESC 4",
            coords: { x: 940, y: 955 }
        },
        note: "Cấp nguồn công suất cho ESC 4 điều khiển động cơ Sau-Trái M4."
    },
    {
        id: "wire-pwr-esc4-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 4 Nguồn (-) [Sau-Trái M4]",
        color: "#475569",
        colorName: "Đen",
        gauge: "14–16 AWG Silicon",
        signalType: "Mass công suất ESC 4",
        filterComponent: "GND PDB",
        source: {
            board: "PDB",
            pin: "Pad ESC 4 (-)",
            pinNumber: "ESC4_GND",
            voltage: "0V",
            location: "Trạm ra ESC 4 trên PDB",
            coords: { x: 523, y: 137 }
        },
        dest: {
            board: "ESC 4 30A (Rear-Left M4 CCW)",
            pin: "Dây Đen Nguồn (-)",
            pinNumber: "GND Input",
            voltage: "0V",
            location: "Cáp nguồn vào ESC 4",
            coords: { x: 965, y: 955 }
        },
        note: "Mass công suất ESC 4."
    },
    {
        id: "wire-pwr-esc3-pos",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 3 Nguồn (+) [Sau-Phải M3]",
        color: "#dc2626",
        colorName: "Đỏ",
        gauge: "14–16 AWG Silicon",
        signalType: "Nguồn công suất 11.1V (Dòng 10A–25A)",
        filterComponent: "Tụ 100µF/25V tại trạm ESC",
        source: {
            board: "PDB",
            pin: "Pad ESC 3 (+)",
            pinNumber: "ESC3_VCC",
            voltage: "11.1V–12.6V",
            location: "Trạm ra ESC 3 trên PDB",
            coords: { x: 450, y: 137 }
        },
        dest: {
            board: "ESC 3 30A (Rear-Right M3 CW)",
            pin: "Dây Đỏ Nguồn (+)",
            pinNumber: "VCC Input",
            voltage: "11.1V–12.6V",
            location: "Cáp nguồn vào ESC 3",
            coords: { x: 1370, y: 955 }
        },
        note: "Cấp nguồn công suất cho ESC 3 điều khiển động cơ Sau-Phải M3."
    },
    {
        id: "wire-pwr-esc3-neg",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "PDB ➔ ESC 3 Nguồn (-) [Sau-Phải M3]",
        color: "#475569",
        colorName: "Đen",
        gauge: "14–16 AWG Silicon",
        signalType: "Mass công suất ESC 3",
        filterComponent: "GND PDB",
        source: {
            board: "PDB",
            pin: "Pad ESC 3 (-)",
            pinNumber: "ESC3_GND",
            voltage: "0V",
            location: "Trạm ra ESC 3 trên PDB",
            coords: { x: 468, y: 137 }
        },
        dest: {
            board: "ESC 3 30A (Rear-Right M3 CW)",
            pin: "Dây Đen Nguồn (-)",
            pinNumber: "GND Input",
            voltage: "0V",
            location: "Cáp nguồn vào ESC 3",
            coords: { x: 1395, y: 955 }
        },
        note: "Mass công suất ESC 3."
    },

    // 12 DÂY PHA MOTOR (ESC ➔ A2212)
    // Motor 2 (CW - Đảo pha A-B)
    {
        id: "wire-phase-m2-u",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 2 Pha U ➔ Motor 2 Pha B (⚡ Đảo pha CW)",
        color: "#38bdf8",
        colorName: "Xanh dương (Dây silicon mềm)",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC băm xung",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 2 30A",
            pin: "Cọc Pha U (A)",
            pinNumber: "Output Phase U",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên trái ESC 2",
            coords: { x: 120, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 2 (A2212 1000KV)",
            pin: "Cọc Pha B (Đảo chiều)",
            pinNumber: "Motor Wire 2",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Trước-Trái (M2 CW)",
            coords: { x: 220, y: 1020 }
        },
        note: "⚡ ĐẢO PHA A-B: Nối Pha U của ESC 2 sang Cọc Pha B của động cơ để ép rotor quay theo chiều kim đồng hồ (CW) với cánh thuận 1045R."
    },
    {
        id: "wire-phase-m2-v",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 2 Pha V ➔ Motor 2 Pha A (⚡ Đảo pha CW)",
        color: "#fbbf24",
        colorName: "Vàng",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 2 30A",
            pin: "Cọc Pha V (B)",
            pinNumber: "Output Phase V",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ ở giữa ESC 2",
            coords: { x: 220, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 2 (A2212 1000KV)",
            pin: "Cọc Pha A (Đảo chiều)",
            pinNumber: "Motor Wire 1",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Trước-Trái (M2 CW)",
            coords: { x: 120, y: 1020 }
        },
        note: "⚡ ĐẢO PHA A-B: Nối Pha V của ESC 2 sang Cọc Pha A của động cơ M2."
    },
    {
        id: "wire-phase-m2-w",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 2 Pha W ➔ Motor 2 Pha C",
        color: "#34d399",
        colorName: "Xanh lá / Đen",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 2 30A",
            pin: "Cọc Pha W (C)",
            pinNumber: "Output Phase W",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên phải ESC 2",
            coords: { x: 320, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 2 (A2212 1000KV)",
            pin: "Cọc Pha C",
            pinNumber: "Motor Wire 3",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Trước-Trái (M2 CW)",
            coords: { x: 320, y: 1020 }
        },
        note: "Nối thẳng cọc Pha W sang Cọc C của Motor 2."
    },

    // Motor 1 (CCW - Nối thẳng)
    {
        id: "wire-phase-m1-u",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 1 Pha U ➔ Motor 1 Pha A (Nối thẳng CCW)",
        color: "#38bdf8",
        colorName: "Xanh dương",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 1 30A",
            pin: "Cọc Pha U",
            pinNumber: "Output Phase U",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên trái ESC 1",
            coords: { x: 550, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 1 (A2212 1000KV)",
            pin: "Cọc Pha A",
            pinNumber: "Motor Wire 1",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Trước-Phải (M1 CCW)",
            coords: { x: 550, y: 1020 }
        },
        note: "Nối chuẩn U-A, V-B, W-C giúp rotor quay ngược chiều kim đồng hồ (CCW) với cánh chuẩn 1045."
    },
    {
        id: "wire-phase-m1-v",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 1 Pha V ➔ Motor 1 Pha B (Nối thẳng CCW)",
        color: "#fbbf24",
        colorName: "Vàng",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 1 30A",
            pin: "Cọc Pha V",
            pinNumber: "Output Phase V",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ ở giữa ESC 1",
            coords: { x: 650, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 1 (A2212 1000KV)",
            pin: "Cọc Pha B",
            pinNumber: "Motor Wire 2",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Trước-Phải (M1 CCW)",
            coords: { x: 650, y: 1020 }
        },
        note: "Nối thẳng Pha V sang Pha B của Motor 1."
    },
    {
        id: "wire-phase-m1-w",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 1 Pha W ➔ Motor 1 Pha C (Nối thẳng CCW)",
        color: "#34d399",
        colorName: "Xanh lá",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 1 30A",
            pin: "Cọc Pha W",
            pinNumber: "Output Phase W",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên phải ESC 1",
            coords: { x: 750, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 1 (A2212 1000KV)",
            pin: "Cọc Pha C",
            pinNumber: "Motor Wire 3",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Trước-Phải (M1 CCW)",
            coords: { x: 750, y: 1020 }
        },
        note: "Nối thẳng Pha W sang Pha C của Motor 1."
    },

    // Motor 4 (CCW - Nối thẳng)
    {
        id: "wire-phase-m4-u",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 4 Pha U ➔ Motor 4 Pha A (Nối thẳng CCW)",
        color: "#38bdf8",
        colorName: "Xanh dương",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 4 30A",
            pin: "Cọc Pha U",
            pinNumber: "Output Phase U",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên trái ESC 4",
            coords: { x: 980, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 4 (A2212 1000KV)",
            pin: "Cọc Pha A",
            pinNumber: "Motor Wire 1",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Sau-Trái (M4 CCW)",
            coords: { x: 980, y: 1020 }
        },
        note: "Nối chuẩn U-A giúp Motor 4 quay ngược chiều kim đồng hồ (CCW)."
    },
    {
        id: "wire-phase-m4-v",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 4 Pha V ➔ Motor 4 Pha B (Nối thẳng CCW)",
        color: "#fbbf24",
        colorName: "Vàng",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 4 30A",
            pin: "Cọc Pha V",
            pinNumber: "Output Phase V",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ ở giữa ESC 4",
            coords: { x: 1080, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 4 (A2212 1000KV)",
            pin: "Cọc Pha B",
            pinNumber: "Motor Wire 2",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Sau-Trái (M4 CCW)",
            coords: { x: 1080, y: 1020 }
        },
        note: "Nối thẳng Pha V sang Pha B của Motor 4."
    },
    {
        id: "wire-phase-m4-w",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 4 Pha W ➔ Motor 4 Pha C (Nối thẳng CCW)",
        color: "#34d399",
        colorName: "Xanh lá",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 4 30A",
            pin: "Cọc Pha W",
            pinNumber: "Output Phase W",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên phải ESC 4",
            coords: { x: 1180, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 4 (A2212 1000KV)",
            pin: "Cọc Pha C",
            pinNumber: "Motor Wire 3",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Sau-Trái (M4 CCW)",
            coords: { x: 1180, y: 1020 }
        },
        note: "Nối thẳng Pha W sang Pha C của Motor 4."
    },

    // Motor 3 (CW - Đảo pha A-B)
    {
        id: "wire-phase-m3-u",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 3 Pha U ➔ Motor 3 Pha B (⚡ Đảo pha CW)",
        color: "#38bdf8",
        colorName: "Xanh dương",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 3 30A",
            pin: "Cọc Pha U",
            pinNumber: "Output Phase U",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên trái ESC 3",
            coords: { x: 1410, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 3 (A2212 1000KV)",
            pin: "Cọc Pha B (Đảo chiều)",
            pinNumber: "Motor Wire 2",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Sau-Phải (M3 CW)",
            coords: { x: 1510, y: 1020 }
        },
        note: "⚡ ĐẢO PHA A-B: Nối Pha U của ESC 3 sang Cọc Pha B của Motor 3 để ép quay CW."
    },
    {
        id: "wire-phase-m3-v",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 3 Pha V ➔ Motor 3 Pha A (⚡ Đảo pha CW)",
        color: "#fbbf24",
        colorName: "Vàng",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 3 30A",
            pin: "Cọc Pha V",
            pinNumber: "Output Phase V",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ ở giữa ESC 3",
            coords: { x: 1510, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 3 (A2212 1000KV)",
            pin: "Cọc Pha A (Đảo chiều)",
            pinNumber: "Motor Wire 1",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Sau-Phải (M3 CW)",
            coords: { x: 1410, y: 1020 }
        },
        note: "⚡ ĐẢO PHA A-B: Nối Pha V của ESC 3 sang Cọc Pha A của Motor 3."
    },
    {
        id: "wire-phase-m3-w",
        groupId: "grp-power",
        groupName: "Nguồn 11.1V & Động lực",
        name: "ESC 3 Pha W ➔ Motor 3 Pha C",
        color: "#34d399",
        colorName: "Xanh lá",
        gauge: "18 AWG Silicon",
        signalType: "Xoay chiều 3 pha BLDC",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESC 3 30A",
            pin: "Cọc Pha W",
            pinNumber: "Output Phase W",
            voltage: "Xung AC 11.1V",
            location: "Cọc ra động cơ bên phải ESC 3",
            coords: { x: 1610, y: 970 }
        },
        dest: {
            board: "Động Cơ Motor 3 (A2212 1000KV)",
            pin: "Cọc Pha C",
            pinNumber: "Motor Wire 3",
            voltage: "Xung AC 11.1V",
            location: "Động cơ Sau-Phải (M3 CW)",
            coords: { x: 1610, y: 1020 }
        },
        note: "Nối thẳng cọc Pha W sang Cọc C của Motor 3."
    },

    // === NHÓM 2: NGUỒN 5.0V UBEC, 3.3V LDO & COMMON GROUND (grp-logic-power) ===
    {
        id: "wire-5v-esp32",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "UBEC 5V (+) ➔ ESP32-S3 Chân 5V/VIN",
        color: "#ef4444",
        colorName: "Đỏ tươi",
        gauge: "22 AWG",
        signalType: "Nguồn DC 5.0V chính nuôi vi điều khiển (Dòng ~500mA)",
        filterComponent: "1x Tụ hóa 100µF/16V + 1x Tụ gốm 104",
        source: {
            board: "UBEC 5V/3A",
            pin: "5V Out (+)",
            pinNumber: "UBEC 5V (+)",
            voltage: "5.0V Sạch (Dòng max 3A)",
            location: "Đầu ra mạch UBEC",
            coords: { x: 895, y: 130 }
        },
        dest: {
            board: "ESP32-S3 DevKitC-1 (44 Pins)",
            pin: "5V / VIN",
            pinNumber: "Header Trái - Pin 22",
            voltage: "5.0V Định mức (Chịu áp 4.5V–6.0V)",
            location: "Header bên trái ESP32 (Chân 22)",
            coords: { x: 690, y: 345 }
        },
        note: "Nguồn cấp điện chính nuôi vi điều khiển. Hàn kèm bộ lọc kép gồm tụ hóa 100µF + tụ gốm 104 sát chân 5V-GND để triệt tiêu hoàn toàn nguy cơ Brownout Reset khi thốc ga."
    },
    {
        id: "wire-5v-pca",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "UBEC 5V (+) ➔ PCA9685 Cọc V+ (Nguồn động lực PWM)",
        color: "#ef4444",
        colorName: "Đỏ",
        gauge: "22 AWG",
        signalType: "Nguồn 5.0V công suất cho chân V+ PCA9685",
        filterComponent: "Tụ phân cực trên PCA",
        source: {
            board: "UBEC 5V/3A",
            pin: "5V Out (+)",
            pinNumber: "UBEC 5V (+)",
            voltage: "5.0V",
            location: "Rail 5V chính",
            coords: { x: 895, y: 180 }
        },
        dest: {
            board: "PCA9685 16-Channel PWM Driver",
            pin: "Cọc V+ (Nguồn Servo/ESC)",
            pinNumber: "Power Terminal V+",
            voltage: "5.0V",
            location: "Cọc vặn ốc màu xanh trên PCA9685",
            coords: { x: 1240, y: 625 }
        },
        note: "Cấp nguồn 5V cho đường dây giữa của các cổng PWM 3-pin trên PCA9685."
    },
    {
        id: "wire-5v-rc",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "UBEC 5V (+) ➔ Bộ Thu Tay Cầm RC (5V In)",
        color: "#ef4444",
        colorName: "Đỏ",
        gauge: "24 AWG",
        signalType: "Nguồn DC 5.0V nuôi bộ thu ELRS / SBUS (Dòng ~100mA)",
        filterComponent: "Tụ lọc trên mạch thu",
        source: {
            board: "UBEC 5V/3A",
            pin: "5V Rail",
            pinNumber: "UBEC 5V (+)",
            voltage: "5.0V",
            location: "Rail 5V",
            coords: { x: 1200, y: 180 }
        },
        dest: {
            board: "Bộ Thu Tay Cầm RC (ELRS / SBUS)",
            pin: "VCC / 5V In",
            pinNumber: "Pin 5V Mạch Thu",
            voltage: "4.5V–5.5V",
            location: "Cổng 4-pin của Receiver",
            coords: { x: 1220, y: 275 }
        },
        note: "Cấp nguồn nuôi tay thu sóng điều khiển từ xa. Đảm bảo nguồn ổn định để không bị ngắt kết nối sóng (Failsafe)."
    },
    {
        id: "wire-5v-buzzer",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "UBEC 5V (+) ➔ Active Buzzer 5V (Cực Dương +)",
        color: "#ef4444",
        colorName: "Đỏ",
        gauge: "24 AWG",
        signalType: "Nguồn 5.0V cho cuộn hút còi chíp",
        filterComponent: "Transistor NPN S8050 đệm dòng",
        source: {
            board: "UBEC 5V/3A",
            pin: "5V Rail",
            pinNumber: "UBEC 5V (+)",
            voltage: "5.0V",
            location: "Rail 5V",
            coords: { x: 1200, y: 180 }
        },
        dest: {
            board: "Active Buzzer 5V & C1815/S8050 Driver",
            pin: "Cực Dương Còi (+)",
            pinNumber: "Buzzer Anode (+)",
            voltage: "5.0V",
            location: "Mạch còi báo động",
            coords: { x: 1220, y: 420 }
        },
        note: "Còi lấy nguồn trực tiếp từ 5V UBEC, đóng cắt mass qua Transistor NPN được kích từ chân GPIO 10 của ESP32."
    },
    {
        id: "wire-gnd-esp32",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "UBEC GND (-) ➔ ESP32-S3 GND (Mass Chung)",
        color: "#475569",
        colorName: "Đen / Xám",
        gauge: "22 AWG",
        signalType: "Common Ground Reference (Mass hệ thống)",
        filterComponent: "Nối chung toàn bộ Mass",
        source: {
            board: "UBEC 5V/3A",
            pin: "GND Out (-)",
            pinNumber: "UBEC GND (-)",
            voltage: "0V",
            location: "Đầu ra mạch UBEC",
            coords: { x: 965, y: 130 }
        },
        dest: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GND (Mass)",
            pinNumber: "Header Trái - Pin 23",
            voltage: "0V",
            location: "Header bên trái ESP32 (Chân 23)",
            coords: { x: 690, y: 375 }
        },
        note: "Tuyến Mass chuẩn của vi điều khiển. Tất cả tín hiệu I2C, UART, PWM, ADC đều lấy mốc 0V từ tuyến này."
    },
    {
        id: "wire-gnd-mpu",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Common GND ➔ MPU6050 GND",
        color: "#475569",
        colorName: "Đen",
        gauge: "26 AWG",
        signalType: "Mass cảm biến IMU",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Common Ground Rail",
            pin: "GND Rail",
            pinNumber: "GND Bus",
            voltage: "0V",
            location: "Bus Mass cảm biến",
            coords: { x: 320, y: 312 }
        },
        dest: {
            board: "MPU6050 (Gyroscope + Accelerometer)",
            pin: "GND",
            pinNumber: "Pin 2 trên module",
            voltage: "0V",
            location: "Header MPU6050",
            coords: { x: 98, y: 312 }
        },
        note: "Nối mass cho MPU6050. Hàn tụ gốm 104 trực tiếp giữa chân 1 (VCC) và chân 2 (GND)."
    },
    {
        id: "wire-gnd-hmc",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Common GND ➔ HMC/QMC5883L GND",
        color: "#475569",
        colorName: "Đen",
        gauge: "26 AWG",
        signalType: "Mass từ kế la bàn",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Common Ground Rail",
            pin: "GND Rail",
            pinNumber: "GND Bus",
            voltage: "0V",
            location: "Bus Mass cảm biến",
            coords: { x: 320, y: 487 }
        },
        dest: {
            board: "HMC5883L / QMC5883L (Magnetometer)",
            pin: "GND",
            pinNumber: "Pin 2 trên module",
            voltage: "0V",
            location: "Header La Bàn",
            coords: { x: 115, y: 487 }
        },
        note: "Mass cho từ kế la bàn. Giữ dây mass sạch để tránh méo từ trường đo được."
    },
    {
        id: "wire-gnd-bmp",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Common GND ➔ BMP280 GND",
        color: "#475569",
        colorName: "Đen",
        gauge: "26 AWG",
        signalType: "Mass khí áp kế",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Common Ground Rail",
            pin: "GND Rail",
            pinNumber: "GND Bus",
            voltage: "0V",
            location: "Bus Mass cảm biến",
            coords: { x: 320, y: 662 }
        },
        dest: {
            board: "BMP280 (Barometer Khí Áp Kế)",
            pin: "GND",
            pinNumber: "Pin 2 trên module",
            voltage: "0V",
            location: "Header BMP280",
            coords: { x: 90, y: 662 }
        },
        note: "Mass cho BMP280. Lọc sạch nguồn giúp đo độ cao chính xác sai số dưới 10cm."
    },
    {
        id: "wire-gnd-gps",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Common GND ➔ GPS ATGM336H GND",
        color: "#475569",
        colorName: "Đen",
        gauge: "26 AWG",
        signalType: "Mass module GPS",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Common Ground Rail",
            pin: "GND Rail",
            pinNumber: "GND Bus Phải",
            voltage: "0V",
            location: "Bus Mass bên phải",
            coords: { x: 1160, y: 132 }
        },
        dest: {
            board: "GPS ATGM336H",
            pin: "GND",
            pinNumber: "Pin 2 trên module GPS",
            voltage: "0V",
            location: "Header GPS",
            coords: { x: 1275, y: 132 }
        },
        note: "Mass cho module định vị GPS vệ tinh."
    },
    {
        id: "wire-gnd-rc",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Common GND ➔ RC Receiver GND",
        color: "#475569",
        colorName: "Đen",
        gauge: "26 AWG",
        signalType: "Mass tay thu RC",
        filterComponent: "GND Chung",
        source: {
            board: "Common Ground Rail",
            pin: "GND Rail",
            pinNumber: "GND Bus Phải",
            voltage: "0V",
            location: "Bus Mass bên phải",
            coords: { x: 1160, y: 275 }
        },
        dest: {
            board: "Bộ Thu Tay Cầm RC (ELRS / SBUS)",
            pin: "GND (Mass)",
            pinNumber: "Pin GND Mạch Thu",
            voltage: "0V",
            location: "Header Mạch Thu RC",
            coords: { x: 1275, y: 275 }
        },
        note: "Mass cho bộ thu tín hiệu điều khiển RC."
    },
    {
        id: "wire-gnd-pca",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Common GND ➔ PCA9685 GND",
        color: "#475569",
        colorName: "Đen",
        gauge: "24 AWG",
        signalType: "Mass mạch logic PCA9685",
        filterComponent: "GND Chung",
        source: {
            board: "Common Ground Rail",
            pin: "GND Rail",
            pinNumber: "GND Bus Phải",
            voltage: "0V",
            location: "Bus Mass bên phải",
            coords: { x: 1160, y: 590 }
        },
        dest: {
            board: "PCA9685 PWM Driver",
            pin: "GND Logic",
            pinNumber: "Pin 2 Header I2C PCA",
            voltage: "0V",
            location: "Cổng 6-pin Header PCA",
            coords: { x: 1260, y: 590 }
        },
        note: "Mass logic cho chip PCA9685."
    },
    {
        id: "wire-3v3-rail",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "ESP32-S3 3V3 VOUT ➔ Rail Nguồn 3.3V Cảm Biến",
        color: "#f59e0b",
        colorName: "Vàng cam (LDO 3.3V Sạch)",
        gauge: "24 AWG",
        signalType: "Nguồn DC 3.3V nội sạch từ LDO ESP32 (Max 600mA)",
        filterComponent: "1x Tụ hóa 100µF + 1x Tụ gốm 104",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "3V3 (VOUT)",
            pinNumber: "Header Trái - Pin 1",
            voltage: "3.3V Sạch (±1%)",
            location: "Header bên trái ESP32 (Chân 1)",
            coords: { x: 690, y: 315 }
        },
        dest: {
            board: "Rail Nguồn 3.3V Cảm Biến & Trở Kéo I2C",
            pin: "3.3V Distribution Rail",
            pinNumber: "VCC 3.3V Bus",
            voltage: "3.3V",
            location: "Thanh nguồn 3.3V cảm biến",
            coords: { x: 305, y: 210 }
        },
        note: "Nguồn 3.3V tạo ra bởi chip LDO trên ESP32 cực phẳng và sạch nhiễu. Dùng để nuôi toàn bộ MPU6050, BMP280, HMC5883L, GPS và cấp áp kéo cho 2 trở 4.7kΩ I2C."
    },
    {
        id: "wire-3v3-mpu",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Rail 3.3V ➔ MPU6050 VCC",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "26 AWG",
        signalType: "Nguồn 3.3V nuôi chip Gyro/Accel",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Rail Nguồn 3.3V Cảm Biến",
            pin: "3.3V Bus",
            pinNumber: "VCC 3.3V",
            voltage: "3.3V",
            location: "Rail 3.3V",
            coords: { x: 305, y: 312 }
        },
        dest: {
            board: "MPU6050",
            pin: "VCC",
            pinNumber: "Pin 1 trên module",
            voltage: "3.3V (Khuyến nghị 3.3V)",
            location: "Header MPU6050",
            coords: { x: 55, y: 312 }
        },
        note: "Cấp nguồn 3.3V trực tiếp cho MPU6050."
    },
    {
        id: "wire-3v3-hmc",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Rail 3.3V ➔ HMC/QMC5883L VCC",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "26 AWG",
        signalType: "Nguồn 3.3V nuôi La Bàn",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Rail Nguồn 3.3V Cảm Biến",
            pin: "3.3V Bus",
            pinNumber: "VCC 3.3V",
            voltage: "3.3V",
            location: "Rail 3.3V",
            coords: { x: 305, y: 487 }
        },
        dest: {
            board: "HMC5883L / QMC5883L",
            pin: "VCC",
            pinNumber: "Pin 1 trên module",
            voltage: "3.3V",
            location: "Header La Bàn",
            coords: { x: 62, y: 487 }
        },
        note: "Cấp nguồn 3.3V cho cảm biến từ trường Trái Đất."
    },
    {
        id: "wire-3v3-bmp",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Rail 3.3V ➔ BMP280 VCC",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "26 AWG",
        signalType: "Nguồn 3.3V nuôi Khí Áp Kế",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "Rail Nguồn 3.3V Cảm Biến",
            pin: "3.3V Bus",
            pinNumber: "VCC 3.3V",
            voltage: "3.3V",
            location: "Rail 3.3V",
            coords: { x: 305, y: 662 }
        },
        dest: {
            board: "BMP280",
            pin: "VCC",
            pinNumber: "Pin 1 trên module",
            voltage: "3.3V (Tuyệt đối không cấp 5V)",
            location: "Header BMP280",
            coords: { x: 50, y: 662 }
        },
        note: "BMP280 chỉ chịu tối đa 3.6V. Cấp 3.3V sạch từ LDO ESP32."
    },
    {
        id: "wire-3v3-pullups",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Rail 3.3V ➔ Cấp nguồn 2 Trở kéo 4.7kΩ I2C",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "26 AWG",
        signalType: "Điện áp tham chiếu kéo I2C (Pull-up VCC)",
        filterComponent: "2x Điện trở 4.7kΩ (1/4W)",
        source: {
            board: "Rail Nguồn 3.3V Cảm Biến",
            pin: "3.3V Bus",
            pinNumber: "Pull-up VCC",
            voltage: "3.3V",
            location: "Rail 3.3V",
            coords: { x: 415, y: 210 }
        },
        dest: {
            board: "Cặp Trở Kéo 4.7kΩ I2C (R1 & R2)",
            pin: "Đầu trên R1 & R2",
            pinNumber: "Resistor Top Lead",
            voltage: "3.3V",
            location: "Mạch trở kéo I2C",
            coords: { x: 415, y: 420 }
        },
        note: "Cung cấp nguồn 3.3V cho 2 điện trở kéo 4.7kΩ nối vào tuyến SDA và SCL."
    },
    {
        id: "wire-3v3-gps",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Rail 3.3V ➔ GPS ATGM336H VCC",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "26 AWG",
        signalType: "Nguồn 3.3V nuôi module GPS",
        filterComponent: "Tụ gốm 104 sát chân VCC-GND",
        source: {
            board: "ESP32 3.3V Rail",
            pin: "3.3V Bus Phải",
            pinNumber: "3V3 Out",
            voltage: "3.3V",
            location: "Rail 3.3V bên phải",
            coords: { x: 1140, y: 210 }
        },
        dest: {
            board: "GPS ATGM336H",
            pin: "VCC",
            pinNumber: "Pin 1 Header GPS",
            voltage: "3.3V (Hỗ trợ 3.3V/5V)",
            location: "Header GPS",
            coords: { x: 1220, y: 132 }
        },
        note: "Cấp nguồn 3.3V cho module định vị GPS."
    },
    {
        id: "wire-3v3-pca",
        groupId: "grp-logic-power",
        groupName: "Nguồn 5V, 3.3V & GND",
        name: "Rail 3.3V ➔ PCA9685 VCC (Nguồn Logic I2C)",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "26 AWG",
        signalType: "Nguồn 3.3V nuôi chip logic I2C PCA9685",
        filterComponent: "Tụ trên mạch PCA",
        source: {
            board: "ESP32 3.3V Rail",
            pin: "3.3V Bus Phải",
            pinNumber: "3V3 Out",
            voltage: "3.3V",
            location: "Rail 3.3V bên phải",
            coords: { x: 1140, y: 590 }
        },
        dest: {
            board: "PCA9685 PWM Driver",
            pin: "VCC (Logic Power)",
            pinNumber: "Pin 1 Header I2C PCA",
            voltage: "3.3V Logic (Tránh 5V làm hỏng I2C ESP32)",
            location: "Cổng 6-pin Header PCA",
            coords: { x: 1215, y: 590 }
        },
        note: "⚠️ BẮT BUỘC CẤP 3.3V vào chân VCC logic của PCA9685 để mức logic SDA/SCL là 3.3V tương thích với ESP32-S3."
    },

    // === NHÓM 3: BUS I2C CẢM BIẾN (SDA, SCL + TRỞ KÉO 4.7kΩ) (grp-i2c) ===
    {
        id: "wire-i2c-sda-mpu",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 8 (SDA) ➔ MPU6050 SDA",
        color: "#10b981",
        colorName: "Xanh lá (Emerald)",
        gauge: "26 AWG",
        signalType: "I2C Serial Data (Fast-mode 400kHz)",
        filterComponent: "Trở kéo R1 4.7kΩ lên 3.3V",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 8 (I2C SDA)",
            pinNumber: "Header Trái - Pin 10",
            voltage: "3.3V Logic Level",
            location: "Header bên trái ESP32 (Chân 10)",
            coords: { x: 690, y: 540 }
        },
        dest: {
            board: "MPU6050 (Addr 0x68)",
            pin: "SDA",
            pinNumber: "Pin 4 trên module",
            voltage: "3.3V Max",
            location: "Header MPU6050",
            coords: { x: 141, y: 312 }
        },
        note: "Đường dữ liệu I2C đọc Gyroscope/Accelerometer 500Hz trong vòng lặp điều khiển bay PID."
    },
    {
        id: "wire-i2c-sda-hmc",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 8 (SDA) ➔ HMC/QMC5883L SDA",
        color: "#10b981",
        colorName: "Xanh lá",
        gauge: "26 AWG",
        signalType: "I2C Serial Data (400kHz)",
        filterComponent: "Trở kéo R1 4.7kΩ",
        source: {
            board: "Tuyến I2C SDA Bus",
            pin: "SDA Bus",
            pinNumber: "GPIO 8",
            voltage: "3.3V",
            location: "Tuyến SDA",
            coords: { x: 590, y: 487 }
        },
        dest: {
            board: "HMC5883L / QMC5883L (Addr 0x1E / 0x0D)",
            pin: "SDA",
            pinNumber: "Pin 4 trên module",
            voltage: "3.3V",
            location: "Header La Bàn",
            coords: { x: 170, y: 487 }
        },
        note: "Truyền dữ liệu từ trường 3 trục X-Y-Z để tính góc Heading la bàn."
    },
    {
        id: "wire-i2c-sda-bmp",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 8 (SDA) ➔ BMP280 SDA",
        color: "#10b981",
        colorName: "Xanh lá",
        gauge: "26 AWG",
        signalType: "I2C Serial Data (400kHz)",
        filterComponent: "Trở kéo R1 4.7kΩ",
        source: {
            board: "Tuyến I2C SDA Bus",
            pin: "SDA Bus",
            pinNumber: "GPIO 8",
            voltage: "3.3V",
            location: "Tuyến SDA",
            coords: { x: 590, y: 662 }
        },
        dest: {
            board: "BMP280 (Addr 0x76)",
            pin: "SDA",
            pinNumber: "Pin 4 trên module",
            voltage: "3.3V",
            location: "Header BMP280",
            coords: { x: 128, y: 662 }
        },
        note: "Truyền dữ liệu áp suất khí quyển và nhiệt độ để ước lượng độ cao tuyệt đối."
    },
    {
        id: "wire-i2c-sda-pullup",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "Tuyến SDA (GPIO 8) ➔ Trở Kéo R1 (4.7kΩ)",
        color: "#10b981",
        colorName: "Xanh lá",
        gauge: "26 AWG",
        signalType: "Điện áp kéo sườn lên SDA",
        filterComponent: "Điện trở 4.7kΩ 1/4W",
        source: {
            board: "Tuyến I2C SDA",
            pin: "SDA Bus",
            pinNumber: "GPIO 8",
            voltage: "3.3V",
            location: "Tuyến SDA",
            coords: { x: 455, y: 540 }
        },
        dest: {
            board: "Trở Kéo R1 4.7kΩ",
            pin: "Chân Dưới R1",
            pinNumber: "R1 Bottom Lead",
            voltage: "3.3V Kéo",
            location: "Mạch trở kéo I2C",
            coords: { x: 455, y: 515 }
        },
        note: "Trở kéo 4.7kΩ giúp sườn lên của tín hiệu SDA vuông vắn và triệt tiêu méo sóng do điện dung ký sinh trên đường dây dài."
    },
    {
        id: "wire-i2c-sda-pca",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 8 (SDA) ➔ PCA9685 SDA",
        color: "#10b981",
        colorName: "Xanh lá",
        gauge: "26 AWG",
        signalType: "I2C Serial Data (400kHz)",
        filterComponent: "Trở kéo R1 4.7kΩ",
        source: {
            board: "ESP32-S3",
            pin: "GPIO 8",
            pinNumber: "Header Trái - Pin 10",
            voltage: "3.3V",
            location: "ESP32 SDA",
            coords: { x: 690, y: 540 }
        },
        dest: {
            board: "PCA9685 PWM Driver (Addr 0x40)",
            pin: "SDA",
            pinNumber: "Pin 4 Header I2C PCA",
            voltage: "3.3V",
            location: "Header PCA9685",
            coords: { x: 1305, y: 590 }
        },
        note: "Truyền lệnh thanh ghi điều khiển xung PWM 16 kênh của PCA9685."
    },
    {
        id: "wire-i2c-scl-mpu",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 9 (SCL) ➔ MPU6050 SCL",
        color: "#06b6d4",
        colorName: "Xanh ngọc (Cyan)",
        gauge: "26 AWG",
        signalType: "I2C Serial Clock (400kHz)",
        filterComponent: "Trở kéo R2 4.7kΩ lên 3.3V",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 9 (I2C SCL)",
            pinNumber: "Header Trái - Pin 11",
            voltage: "3.3V Logic Level",
            location: "Header bên trái ESP32 (Chân 11)",
            coords: { x: 690, y: 570 }
        },
        dest: {
            board: "MPU6050 (Addr 0x68)",
            pin: "SCL",
            pinNumber: "Pin 3 trên module",
            voltage: "3.3V Max",
            location: "Header MPU6050",
            coords: { x: 184, y: 312 }
        },
        note: "Tuyến xung nhịp đồng bộ 400kHz cho giao tiếp I2C MPU6050."
    },
    {
        id: "wire-i2c-scl-hmc",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 9 (SCL) ➔ HMC/QMC5883L SCL",
        color: "#06b6d4",
        colorName: "Xanh ngọc",
        gauge: "26 AWG",
        signalType: "I2C Serial Clock (400kHz)",
        filterComponent: "Trở kéo R2 4.7kΩ",
        source: {
            board: "Tuyến I2C SCL Bus",
            pin: "SCL Bus",
            pinNumber: "GPIO 9",
            voltage: "3.3V",
            location: "Tuyến SCL",
            coords: { x: 575, y: 487 }
        },
        dest: {
            board: "HMC5883L / QMC5883L",
            pin: "SCL",
            pinNumber: "Pin 3 trên module",
            voltage: "3.3V",
            location: "Header La Bàn",
            coords: { x: 225, y: 487 }
        },
        note: "Xung đồng hồ I2C cho la bàn số."
    },
    {
        id: "wire-i2c-scl-bmp",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 9 (SCL) ➔ BMP280 SCL",
        color: "#06b6d4",
        colorName: "Xanh ngọc",
        gauge: "26 AWG",
        signalType: "I2C Serial Clock (400kHz)",
        filterComponent: "Trở kéo R2 4.7kΩ",
        source: {
            board: "Tuyến I2C SCL Bus",
            pin: "SCL Bus",
            pinNumber: "GPIO 9",
            voltage: "3.3V",
            location: "Tuyến SCL",
            coords: { x: 575, y: 662 }
        },
        dest: {
            board: "BMP280",
            pin: "SCL",
            pinNumber: "Pin 3 trên module",
            voltage: "3.3V",
            location: "Header BMP280",
            coords: { x: 166, y: 662 }
        },
        note: "Xung đồng hồ I2C cho khí áp kế BMP280."
    },
    {
        id: "wire-i2c-scl-pullup",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "Tuyến SCL (GPIO 9) ➔ Trở Kéo R2 (4.7kΩ)",
        color: "#06b6d4",
        colorName: "Xanh ngọc",
        gauge: "26 AWG",
        signalType: "Điện áp kéo sườn lên SCL",
        filterComponent: "Điện trở 4.7kΩ 1/4W",
        source: {
            board: "Tuyến I2C SCL",
            pin: "SCL Bus",
            pinNumber: "GPIO 9",
            voltage: "3.3V",
            location: "Tuyến SCL",
            coords: { x: 475, y: 570 }
        },
        dest: {
            board: "Trở Kéo R2 4.7kΩ",
            pin: "Chân Dưới R2",
            pinNumber: "R2 Bottom Lead",
            voltage: "3.3V Kéo",
            location: "Mạch trở kéo I2C",
            coords: { x: 475, y: 515 }
        },
        note: "Trở kéo 4.7kΩ giúp giữ xung nhịp SCL sắc nét, chống sụt áp ở tần số 400kHz."
    },
    {
        id: "wire-i2c-scl-pca",
        groupId: "grp-i2c",
        groupName: "Bus I2C Cảm Biến",
        name: "ESP32-S3 GPIO 9 (SCL) ➔ PCA9685 SCL",
        color: "#06b6d4",
        colorName: "Xanh ngọc",
        gauge: "26 AWG",
        signalType: "I2C Serial Clock (400kHz)",
        filterComponent: "Trở kéo R2 4.7kΩ",
        source: {
            board: "ESP32-S3",
            pin: "GPIO 9",
            pinNumber: "Header Trái - Pin 11",
            voltage: "3.3V",
            location: "ESP32 SCL",
            coords: { x: 690, y: 570 }
        },
        dest: {
            board: "PCA9685 PWM Driver (Addr 0x40)",
            pin: "SCL",
            pinNumber: "Pin 3 Header I2C PCA",
            voltage: "3.3V",
            location: "Header PCA9685",
            coords: { x: 1350, y: 590 }
        },
        note: "Xung nhịp đồng bộ I2C cho mạch điều khiển PWM PCA9685."
    },

    // === NHÓM 4: TUYẾN XUNG PWM ĐIỀU KHIỂN 4 ESC ĐỘNG CƠ (grp-motor-ctrl) ===
    {
        id: "wire-pwm-esc1",
        groupId: "grp-motor-ctrl",
        groupName: "Xung PWM Động Cơ",
        name: "ESP32 GPIO 4 ➔ ESC 1 Signal [Trước-Phải M1 CCW]",
        color: "#c084fc",
        colorName: "Tím nhạt (Lavender)",
        gauge: "26 AWG (Dây Trắng/Cam Servo)",
        signalType: "Xung PWM 50Hz–400Hz (Độ rộng 1000µs–2000µs)",
        filterComponent: "CẮT DÂY ĐỎ 5V BEC",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 4 (LEDC_CH0 / PWM 1)",
            pinNumber: "Header Trái - Pin 3",
            voltage: "3.3V Logic PWM",
            location: "Header bên trái ESP32 (Chân 3)",
            coords: { x: 690, y: 410 }
        },
        dest: {
            board: "ESC 1 30A (Front-Right M1 CCW)",
            pin: "Dây Trắng/Cam Tín Hiệu (PWM In)",
            pinNumber: "Servo Signal Pin",
            voltage: "3.3V–5.0V Logic",
            location: "Dây 3-pin tín hiệu ESC 1",
            coords: { x: 780, y: 955 }
        },
        note: "⚠️ CẮT DÂY ĐỎ 5V BEC: Chỉ nối dây Tín hiệu (Trắng/Cam) và dây Mass (Đen/Nâu) vào ESP32. Tuyệt đối không cắm dây đỏ 5V của ESC để tránh xung đột điện áp làm cháy UBEC 5V/3A!"
    },
    {
        id: "wire-pwm-esc2",
        groupId: "grp-motor-ctrl",
        groupName: "Xung PWM Động Cơ",
        name: "ESP32 GPIO 5 ➔ ESC 2 Signal [Trước-Trái M2 CW]",
        color: "#c084fc",
        colorName: "Tím nhạt",
        gauge: "26 AWG",
        signalType: "Xung PWM 50Hz–400Hz (1000µs–2000µs)",
        filterComponent: "CẮT DÂY ĐỎ 5V BEC",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 5 (LEDC_CH1 / PWM 2)",
            pinNumber: "Header Trái - Pin 4",
            voltage: "3.3V Logic PWM",
            location: "Header bên trái ESP32 (Chân 4)",
            coords: { x: 690, y: 440 }
        },
        dest: {
            board: "ESC 2 30A (Front-Left M2 CW)",
            pin: "Dây Trắng/Cam Tín Hiệu (PWM In)",
            pinNumber: "Servo Signal Pin",
            voltage: "3.3V–5.0V Logic",
            location: "Dây 3-pin tín hiệu ESC 2",
            coords: { x: 350, y: 955 }
        },
        note: "Điều khiển tốc độ Motor 2 (Trước-Trái CW). Cắt dây đỏ 5V của ESC."
    },
    {
        id: "wire-pwm-esc3",
        groupId: "grp-motor-ctrl",
        groupName: "Xung PWM Động Cơ",
        name: "ESP32 GPIO 6 ➔ ESC 3 Signal [Sau-Phải M3 CW]",
        color: "#c084fc",
        colorName: "Tím nhạt",
        gauge: "26 AWG",
        signalType: "Xung PWM 50Hz–400Hz (1000µs–2000µs)",
        filterComponent: "CẮT DÂY ĐỎ 5V BEC",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 6 (LEDC_CH2 / PWM 3)",
            pinNumber: "Header Trái - Pin 5",
            voltage: "3.3V Logic PWM",
            location: "Header bên trái ESP32 (Chân 5)",
            coords: { x: 690, y: 470 }
        },
        dest: {
            board: "ESC 3 30A (Rear-Right M3 CW)",
            pin: "Dây Trắng/Cam Tín Hiệu (PWM In)",
            pinNumber: "Servo Signal Pin",
            voltage: "3.3V–5.0V Logic",
            location: "Dây 3-pin tín hiệu ESC 3",
            coords: { x: 1640, y: 955 }
        },
        note: "Điều khiển tốc độ Motor 3 (Sau-Phải CW). Cắt dây đỏ 5V của ESC."
    },
    {
        id: "wire-pwm-esc4",
        groupId: "grp-motor-ctrl",
        groupName: "Xung PWM Động Cơ",
        name: "ESP32 GPIO 7 ➔ ESC 4 Signal [Sau-Trái M4 CCW]",
        color: "#c084fc",
        colorName: "Tím nhạt",
        gauge: "26 AWG",
        signalType: "Xung PWM 50Hz–400Hz (1000µs–2000µs)",
        filterComponent: "CẮT DÂY ĐỎ 5V BEC",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 7 (LEDC_CH3 / PWM 4)",
            pinNumber: "Header Trái - Pin 6",
            voltage: "3.3V Logic PWM",
            location: "Header bên trái ESP32 (Chân 6)",
            coords: { x: 690, y: 500 }
        },
        dest: {
            board: "ESC 4 30A (Rear-Left M4 CCW)",
            pin: "Dây Trắng/Cam Tín Hiệu (PWM In)",
            pinNumber: "Servo Signal Pin",
            voltage: "3.3V–5.0V Logic",
            location: "Dây 3-pin tín hiệu ESC 4",
            coords: { x: 1210, y: 955 }
        },
        note: "Điều khiển tốc độ Motor 4 (Sau-Trái CCW). Cắt dây đỏ 5V của ESC."
    },

    // === NHÓM 5: GIÁM SÁT PIN ADC, BUZZER & ĐÈN ARM LED (grp-adc-buzzer) ===
    {
        id: "wire-adc-vbat",
        groupId: "grp-adc-buzzer",
        groupName: "ADC, Còi & Đèn Báo",
        name: "Cầu phân áp VBAT Out ➔ ESP32 GPIO 1 (ADC1_CH0)",
        color: "#a855f7",
        colorName: "Tím đậm",
        gauge: "26 AWG",
        signalType: "Tín hiệu Analog 0–2.27V (Điện áp đo Pin LiPo 3S)",
        filterComponent: "Tụ gốm 104 (0.1µF) song song R2 2.2kΩ",
        source: {
            board: "Cầu Phân Áp VBAT (10k / 2.2k)",
            pin: "Điểm Giữa Cầu Trở (V_ADC Out)",
            pinNumber: "Divider Output Pad",
            voltage: "0V–2.27V (Tương ứng Pin 0–12.6V)",
            location: "Mạch phân áp",
            coords: { x: 710, y: 137 }
        },
        dest: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 1 (ADC1_CH0)",
            pinNumber: "Header Trái - Pin 7",
            voltage: "0–3.1V Max ADC",
            location: "Header bên trái ESP32 (Chân 7)",
            coords: { x: 690, y: 610 }
        },
        note: "Đọc điện áp Pin LiPo 3S theo công thức V_Pin = V_ADC * (10k+2.2k)/2.2k = V_ADC * 5.545. Tụ gốm 104 lọc mượt nhiễu gai điện áp khi động cơ hoạt động."
    },
    {
        id: "wire-buzzer-sig",
        groupId: "grp-adc-buzzer",
        groupName: "ADC, Còi & Đèn Báo",
        name: "ESP32 GPIO 10 ➔ Cực B Transistor NPN (Active Buzzer 5V)",
        color: "#fb923c",
        colorName: "Cam đậm",
        gauge: "26 AWG",
        signalType: "Tín hiệu kích đóng ngắt Digital 3.3V (0/1)",
        filterComponent: "Trở hạn dòng Base 1kΩ + Transistor S8050/C1815",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 10 (Digital Output)",
            pinNumber: "Header Trái - Pin 12",
            voltage: "3.3V Logic High",
            location: "Header bên trái ESP32 (Chân 12)",
            coords: { x: 690, y: 710 }
        },
        dest: {
            board: "Active Buzzer 5V Driver",
            pin: "Cực B (Base) Transistor NPN",
            pinNumber: "Base Pin (Qua trở 1kΩ)",
            voltage: "0.7V V_BE",
            location: "Mạch kích còi báo",
            coords: { x: 1220, y: 435 }
        },
        note: "Khi GPIO 10 lên HIGH (3.3V), transistor mở dẫn dòng từ 5V UBEC qua cuộn hút còi xuống Mass làm còi kêu bíp. Báo trạng thái Arm, Pin yếu, mất sóng RC."
    },
    {
        id: "wire-led-arm",
        groupId: "grp-adc-buzzer",
        groupName: "ADC, Còi & Đèn Báo",
        name: "ESP32 GPIO 3 ➔ Anode (+) Đèn Arm LED Đỏ",
        color: "#ef4444",
        colorName: "Đỏ mỏng",
        gauge: "28 AWG",
        signalType: "Digital Output 3.3V (Dòng ~5mA)",
        filterComponent: "Điện trở hạn dòng 330Ω",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 3 (Arm LED Signal)",
            pinNumber: "Header Trái - Pin 9",
            voltage: "3.3V Logic",
            location: "Header bên trái ESP32 (Chân 9)",
            coords: { x: 690, y: 675 }
        },
        dest: {
            board: "Đèn Arming LED Đỏ",
            pin: "Anode (+) LED Đỏ (Qua trở 330Ω)",
            pinNumber: "LED Anode Pin",
            voltage: "1.8V–2.2V Forward Drop",
            location: "Đèn LED trạng thái máy bay",
            coords: { x: 1200, y: 300 }
        },
        note: "Đèn sáng liên tục khi drone đã ARM sẵn sàng bay. Đèn chớp tắt khi chưa hoàn thành calib Gyro hoặc đang trong trạng thái Disarm an toàn."
    },

    // === NHÓM 6: GPS UART1 & RC RECEIVER UART2 (grp-comms) ===
    {
        id: "wire-gps-tx-rx",
        groupId: "grp-comms",
        groupName: "GPS & RC UART",
        name: "GPS TXD ➔ ESP32 GPIO 17 (U1RXD)",
        color: "#ec4899",
        colorName: "Hồng cánh sen (Pink)",
        gauge: "26 AWG",
        signalType: "UART Serial Data (9600–115200 baud)",
        filterComponent: "Tụ 104 lọc nguồn GPS",
        source: {
            board: "GPS ATGM336H",
            pin: "TXD (Transmit)",
            pinNumber: "Pin 3 Header GPS",
            voltage: "3.3V Logic UART",
            location: "Header module GPS",
            coords: { x: 1330, y: 132 }
        },
        dest: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 17 (U1RXD)",
            pinNumber: "Header Phải - Pin 17",
            voltage: "3.3V Logic",
            location: "Header bên phải ESP32 (Chân 17)",
            coords: { x: 1045, y: 315 }
        },
        note: "ESP32 đọc các câu bản tin định vị vệ tinh NMEA ($GNGGA, $GNRMC, $GNVTG) với tần số 5Hz–10Hz để lấy tọa độ kinh độ, vĩ độ, độ cao và số lượng vệ tinh 3D Fix."
    },
    {
        id: "wire-gps-rx-tx",
        groupId: "grp-comms",
        groupName: "GPS & RC UART",
        name: "ESP32 GPIO 18 (U1TXD) ➔ GPS RXD",
        color: "#3b82f6",
        colorName: "Xanh dương (Blue)",
        gauge: "26 AWG",
        signalType: "UART Serial Config Commands",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 18 (U1TXD)",
            pinNumber: "Header Phải - Pin 18",
            voltage: "3.3V Logic",
            location: "Header bên phải ESP32 (Chân 18)",
            coords: { x: 1045, y: 345 }
        },
        dest: {
            board: "GPS ATGM336H",
            pin: "RXD (Receive)",
            pinNumber: "Pin 4 Header GPS",
            voltage: "3.3V Logic",
            location: "Header module GPS",
            coords: { x: 1385, y: 132 }
        },
        note: "ESP32 gửi chuỗi cấu hình CASIC lên GPS khi khởi động để đổi baudrate lên 115200 và nâng tần số quét vệ tinh từ 1Hz lên 5Hz/10Hz."
    },
    {
        id: "wire-rc-tx-rx",
        groupId: "grp-comms",
        groupName: "GPS & RC UART",
        name: "RC Receiver TX/Signal ➔ ESP32 GPIO 43 (U2RXD)",
        color: "#ec4899",
        colorName: "Hồng / Trắng",
        gauge: "26 AWG",
        signalType: "CRSF / SBUS / IBUS Serial Protocol (115200 / 420000 baud)",
        filterComponent: "Không qua tụ",
        source: {
            board: "Bộ Thu Tay Cầm RC (ELRS / SBUS)",
            pin: "TXD / Signal (CRSF TX)",
            pinNumber: "Pin 3 Header Tay Thu",
            voltage: "3.3V Logic",
            location: "Header mạch thu RC",
            coords: { x: 1330, y: 275 }
        },
        dest: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 43 (U2RXD)",
            pinNumber: "Header Phải - Pin 19",
            voltage: "3.3V Logic",
            location: "Header bên phải ESP32 (Chân 19)",
            coords: { x: 1045, y: 385 }
        },
        note: "Nhận 16 kênh điều khiển (Roll, Pitch, Yaw, Throttle, Arm Switch, Flight Mode, Return-to-Home) với độ trễ siêu thấp (<5ms với ExpressLRS)."
    },
    {
        id: "wire-rc-rx-tx",
        groupId: "grp-comms",
        groupName: "GPS & RC UART",
        name: "ESP32 GPIO 44 (U2TXD) ➔ RC Receiver RX (Telemetry)",
        color: "#3b82f6",
        colorName: "Xanh dương",
        gauge: "26 AWG",
        signalType: "CRSF Telemetry Backlink",
        filterComponent: "Không qua tụ",
        source: {
            board: "ESP32-S3 DevKitC-1",
            pin: "GPIO 44 (U2TXD)",
            pinNumber: "Header Phải - Pin 20",
            voltage: "3.3V Logic",
            location: "Header bên phải ESP32 (Chân 20)",
            coords: { x: 1045, y: 415 }
        },
        dest: {
            board: "Bộ Thu Tay Cầm RC (ELRS / SBUS)",
            pin: "RXD (CRSF RX / Telemetry)",
            pinNumber: "Pin 4 Header Tay Thu",
            voltage: "3.3V Logic",
            location: "Header mạch thu RC",
            coords: { x: 1385, y: 275 }
        },
        note: "Gửi ngược dữ liệu điện áp Pin LiPo, trạng thái Arm, góc nghiêng Roll/Pitch và tọa độ GPS về màn hình tay cầm điều khiển từ xa."
    },

    // === NHÓM 7: TỤ LỌC NHIỄU VÀ CHÂN CẤU HÌNH CỨNG (grp-cfg-noise) ===
    {
        id: "wire-cfg-mpu-ad0",
        groupId: "grp-cfg-noise",
        groupName: "Cấu hình cứng & Tụ lọc",
        name: "MPU6050 AD0 ➔ GND (Khóa địa chỉ I2C 0x68)",
        color: "#9333ea",
        colorName: "Tím / Đen",
        gauge: "Dây câu ngắn",
        signalType: "Mức logic tĩnh GND (0V)",
        filterComponent: "Khóa chân cấu hình",
        source: {
            board: "MPU6050",
            pin: "AD0 (Address Pin)",
            pinNumber: "Pin 5 trên module",
            voltage: "0V",
            location: "Header MPU6050",
            coords: { x: 235, y: 302 }
        },
        dest: {
            board: "MPU6050 GND Pad",
            pin: "GND (Mass)",
            pinNumber: "Pin 2 trên module",
            voltage: "0V",
            location: "Header MPU6050",
            coords: { x: 98, y: 302 }
        },
        note: "Kéo chân AD0 xuống GND để cố định địa chỉ I2C của MPU6050 là 0x68 (Nếu nối 3.3V sẽ thành 0x69)."
    },
    {
        id: "wire-cfg-bmp-sdo",
        groupId: "grp-cfg-noise",
        groupName: "Cấu hình cứng & Tụ lọc",
        name: "BMP280 SDO ➔ GND (Khóa địa chỉ I2C 0x76)",
        color: "#9333ea",
        colorName: "Tím / Đen",
        gauge: "Dây câu ngắn",
        signalType: "Mức logic tĩnh GND (0V)",
        filterComponent: "Khóa chân cấu hình",
        source: {
            board: "BMP280",
            pin: "SDO (Address Select)",
            pinNumber: "Pin 5 trên module",
            voltage: "0V",
            location: "Header BMP280",
            coords: { x: 202, y: 652 }
        },
        dest: {
            board: "BMP280 GND Pad",
            pin: "GND",
            pinNumber: "Pin 2 trên module",
            voltage: "0V",
            location: "Header BMP280",
            coords: { x: 90, y: 652 }
        },
        note: "Nối SDO xuống GND để cố định địa chỉ I2C của BMP280 là 0x76 (Nếu nối VCC sẽ thành 0x77)."
    },
    {
        id: "wire-cfg-bmp-csb",
        groupId: "grp-cfg-noise",
        groupName: "Cấu hình cứng & Tụ lọc",
        name: "BMP280 CSB ➔ 3.3V (Khóa chế độ I2C Mode)",
        color: "#f59e0b",
        colorName: "Vàng",
        gauge: "Dây câu ngắn",
        signalType: "Mức logic tĩnh 3.3V",
        filterComponent: "Khóa chân giao tiếp",
        source: {
            board: "BMP280",
            pin: "CSB (Chip Select Bar)",
            pinNumber: "Pin 6 trên module",
            voltage: "3.3V",
            location: "Header BMP280",
            coords: { x: 244, y: 652 }
        },
        dest: {
            board: "BMP280 VCC Pad",
            pin: "VCC (3.3V)",
            pinNumber: "Pin 1 trên module",
            voltage: "3.3V",
            location: "Header BMP280",
            coords: { x: 50, y: 652 }
        },
        note: "⚠️ Kéo CSB lên 3.3V để ép BMP280 chạy ở chế độ giao tiếp I2C thay vì SPI."
    },
    {
        id: "wire-cfg-pca-oe",
        groupId: "grp-cfg-noise",
        groupName: "Cấu hình cứng & Tụ lọc",
        name: "PCA9685 OE ➔ GND (Luôn Enable ngõ ra PWM)",
        color: "#9333ea",
        colorName: "Tím / Đen",
        gauge: "Dây câu ngắn",
        signalType: "Mức logic tĩnh GND (0V)",
        filterComponent: "Khóa chân điều khiển",
        source: {
            board: "PCA9685 PWM Driver",
            pin: "OE (Output Enable Bar)",
            pinNumber: "Pin 6 Header I2C PCA",
            voltage: "0V Active Low",
            location: "Header PCA9685",
            coords: { x: 1415, y: 580 }
        },
        dest: {
            board: "PCA9685 GND Pad",
            pin: "GND Logic",
            pinNumber: "Pin 2 Header PCA",
            voltage: "0V",
            location: "Header PCA9685",
            coords: { x: 1260, y: 580 }
        },
        note: "Chân OE (Active Low) nối xuống GND để ngõ ra xung PWM của PCA9685 luôn ở trạng thái kích hoạt sẵn sàng."
    }
];

if (typeof window !== "undefined") {
    window.WIRE_CONNECTIONS_DB = WIRE_CONNECTIONS_DB;
}
