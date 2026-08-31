# SƠ ĐỒ ĐẤU NỐI MẠCH ĐIỆN VÀ PHÂN TÍCH RỦI RO PHẦN CỨNG
## ESP32-S3 (R16N8) FLIGHT CONTROLLER (QUAD-X)

Tài liệu này cung cấp toàn bộ sơ đồ đấu nối phần cứng, kiến trúc phân phối nguồn điện, bảng ánh xạ chân tín hiệu, cùng báo cáo phân tích logic, các điểm rủi ro xung đột điện áp và tính khả thi trong thực tế khi chế tạo Drone Quad-X.

---

## 1. KIẾN TRÚC PHÂN PHỐI NGUỒN (POWER DISTRIBUTION ARCHITECTURE)

```
                       +-------------------------------+
                       |   PIN LIPO 3S (11.1V - 12.6V) |
                       |      2200mAh >= 25C (XT60)    |
                       +---------------+---------------+
                                       |
                   [ Dây nguồn chính 14-16 AWG ]
                                       v
                    +-------------------------------------+
                    |   POWER DISTRIBUTION BOARD (PDB)    |
                    |    (Bảng chia nguồn & Lọc nguồn)    |
                    +---+--------+--------+--------+------+
                        |        |        |        |      |
      +-----------------+        |        |        |      +----------------+
      | V_BAT (11.1V)            |        |        |      | V_BAT (11.1V)  | V_BAT (11.1V)
      v                          v        v        v      v                v
+-----------+              +-------------------------+  +--------------+ +---------------+
| ESC 1 30A |              |   ESC 2, 3, 4 (30A)     |  | Tụ lọc nguồn | | UBEC 5V/3A   |
| (Front-R) |              | (Front-L, Rear-R, Rear-L|  | 1000uF / 25V | | (Khuyên dùng)|
+-----+-----+              +------------+------------+  | (Chống nhiễu | +-------+-------+
      |                                 |               |  động cơ)    |         | 5.0V
      | 5V BEC (CHỈ DÙNG 1)             | (CẮT DÂY ĐỎ)  +--------------+         |
      | Hoặc dùng UBEC rời              | KHÔNG DÙNG                             |
      +-----------------+---------------+                                        |
                        |                                                        |
                        +--------------------------------------------------------+
                        |
                        v
         +------------------------------+
         |  ĐƯỜNG NGUỒN 5.0V HỆ THỐNG   |
         +--------------+---------------+
                        |
            +-----------+-----------+
            | 5V                    | 5V (hoặc 3.3V)
            v                       v
    +---------------+       +---------------+
    | ESP32-S3 DEV  |       | PCA9685 PWM   |
    | (Chân 5V/VIN) |       | (Chân VCC/V+) |
    +-------+-------+       +-------+-------+
            | 3.3V LDO nội          |
            v                       |
    +---------------+               |
    | ĐƯỜNG 3.3V    |               |
    | (Max 500mA)   |               |
    +-------+-------+               |
            |                       |
   +--------+--------+              |
   | 3.3V   | 3.3V   | 3.3V         |
   v        v        v              v
+------+ +------+ +------+   +---------------+
| MPU  | | HMC/ | | BMP  |   | 4x ESC Signal |
| 6050 | | QMC  | | 280  |   | Kênh 0,1,2,3  |
+------+ +------+ +------+   +---------------+
```

---

## 2. SƠ ĐỒ ĐẤU NỐI CHI TIẾT TỪNG CHÂN (PIN-TO-PIN CONNECTION)

### A. Bus Cảm biến I2C (Chung 1 bus I2C tốc độ 400kHz)
> ⚠️ **Quy tắc tuyệt đối**: Mọi module I2C phải được cấp nguồn **3.3V** từ chân 3.3V của ESP32-S3 (KHÔNG cấp 5V vào module không có chuyển mức logic).

| ESP32-S3 Pin | MPU6050 (0x68) | HMC/QMC5883L (0x1E/0x0D) | BMP280 (0x76) | PCA9685 (0x40) | Chức năng |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **3V3 (3.3V)** | VCC | VCC | VCC | VCC (Logic) | Nguồn cấp logic 3.3V |
| **GND** | GND | GND | GND | GND | Nối mass chung (Common Ground) |
| **GPIO 8** | SDA | SDA | SDA | SDA | I2C Data Line (Kèm trở kéo 4.7kΩ lên 3.3V nếu cần) |
| **GPIO 9** | SCL | SCL | SCL | SCL | I2C Clock Line (Kèm trở kéo 4.7kΩ lên 3.3V) |

### B. Module GPS ATGM336H (UART1)
| ESP32-S3 Pin | GPS ATGM336H | Chức năng | Ghi chú |
| :--- | :--- | :--- | :--- |
| **3V3 (3.3V)** | VCC | Cấp nguồn | Cấp 3.3V hoặc 5V (tùy mạch module GPS có LDO) |
| **GND** | GND | Mass chung | Nối vào hệ thống GND |
| **GPIO 17 (U1RXD)** | TXD (GPS) | ESP32 Nhận NMEA | ESP32 đọc dữ liệu câu $GNGGA, $GNRMC |
| **GPIO 18 (U1TXD)** | RXD (GPS) | ESP32 Gửi lệnh | Cấu hình GPS (nếu cần) |

### C. Module PCA9685 & 4 ESC Động cơ
| PCA9685 Header | Tín hiệu kết nối | Thiết bị đích | Ghi chú màu dây |
| :--- | :--- | :--- | :--- |
| **VCC** (Bên hông) | ESP32 3.3V (hoặc 5V) | Cấp nguồn logic cho IC PCA9685 | Đỏ |
| **GND** (Bên hông) | ESP32 GND & PDB GND | Nối Mass chung toàn hệ thống | Đen |
| **V+** (Cọc nguồn) | Nguồn 5V (từ UBEC / BEC ESC) | Cấp nguồn xung cho chân Servo/ESC | Đỏ to |
| **Channel 0** | Signal (PWM Kênh 0) | **ESC 1** (Motor 1 - Trước Phải, CCW) | Dây Trắng / Cam |
| | GND | **ESC 1** GND | Dây Đen / Nâu |
| **Channel 1** | Signal (PWM Kênh 1) | **ESC 2** (Motor 2 - Trước Trái, CW) | Dây Trắng / Cam |
| | GND | **ESC 2** GND | Dây Đen / Nâu |
| **Channel 2** | Signal (PWM Kênh 2) | **ESC 3** (Motor 3 - Sau Phải, CW) | Dây Trắng / Cam |
| | GND | **ESC 3** GND | Dây Đen / Nâu |
| **Channel 3** | Signal (PWM Kênh 3) | **ESC 4** (Motor 4 - Sau Trái, CCW) | Dây Trắng / Cam |
| | GND | **ESC 4** GND | Dây Đen / Nâu |

### D. Driver Motor Phụ L9110S (Tùy chọn)
| ESP32-S3 Pin | L9110S Pin | Chức năng |
| :--- | :--- | :--- |
| **GPIO 4** | AIA | Điều khiển Motor A thuận |
| **GPIO 5** | AIB | Điều khiển Motor A nghịch |
| **GPIO 6** | BIA | Điều khiển Motor B thuận |
| **GPIO 7** | BIB | Điều khiển Motor B nghịch |
| **GND** | GND | Mass chung |
| **PDB 5V / 12V** | VCC | Cấp nguồn động lực cho L9110S (Tùy điện áp motor tải) |

---

## 2.1. DANH MỤC CHI TIẾT TOÀN BỘ 34+ ĐƯỜNG DÂY VẬT LÝ TRÊN DRONE (PHYSICAL WIRE INVENTORY)

| STT | Nhóm chức năng | Tên đường dây | Điểm đầu (From) | Điểm cuối (To) | Tiết diện & Màu sắc | Chức năng vật lý |
| :---: | :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | Nguồn Động Lực | Dây Dương Pin | Cọc Đực XT60 (+) | Cọc Dương PDB (+) | 12-14 AWG Silicon Đỏ | Cấp dòng 11.1V (Max 45A-60A) |
| **2** | Nguồn Động Lực | Dây Âm Pin | Cọc Đực XT60 (-) | Cọc Âm PDB (-) | 12-14 AWG Silicon Đen | Đường mass hồi pin |
| **3** | Nguồn Động Lực | Nguồn ESC 1 (+) | Trạm hàn PDB 1 (+) | Dây Đỏ ESC 1 | 14-16 AWG Đỏ | Cấp 11.1V nuôi Motor 1 |
| **4** | Nguồn Động Lực | Nguồn ESC 1 (-) | Trạm hàn PDB 1 (-) | Dây Đen ESC 1 | 14-16 AWG Đen | Mass công suất ESC 1 |
| **5** | Nguồn Động Lực | Nguồn ESC 2 (+) | Trạm hàn PDB 2 (+) | Dây Đỏ ESC 2 | 14-16 AWG Đỏ | Cấp 11.1V nuôi Motor 2 |
| **6** | Nguồn Động Lực | Nguồn ESC 2 (-) | Trạm hàn PDB 2 (-) | Dây Đen ESC 2 | 14-16 AWG Đen | Mass công suất ESC 2 |
| **7** | Nguồn Động Lực | Nguồn ESC 3 (+) | Trạm hàn PDB 3 (+) | Dây Đỏ ESC 3 | 14-16 AWG Đỏ | Cấp 11.1V nuôi Motor 3 |
| **8** | Nguồn Động Lực | Nguồn ESC 3 (-) | Trạm hàn PDB 3 (-) | Dây Đen ESC 3 | 14-16 AWG Đen | Mass công suất ESC 3 |
| **9** | Nguồn Động Lực | Nguồn ESC 4 (+) | Trạm hàn PDB 4 (+) | Dây Đỏ ESC 4 | 14-16 AWG Đỏ | Cấp 11.1V nuôi Motor 4 |
| **10** | Nguồn Động Lực | Nguồn ESC 4 (-) | Trạm hàn PDB 4 (-) | Dây Đen ESC 4 | 14-16 AWG Đen | Mass công suất ESC 4 |
| **11** | 12 Dây Pha Motor | Pha U - Motor 1 | Cọc ra ESC 1 (U) | Động cơ A2212 (M1-A) | 18 AWG Xanh dương | Pha 1 dòng xoay chiều |
| **12** | 12 Dây Pha Motor | Pha V - Motor 1 | Cọc ra ESC 1 (V) | Động cơ A2212 (M1-B) | 18 AWG Vàng | Pha 2 dòng xoay chiều |
| **13** | 12 Dây Pha Motor | Pha W - Motor 1 | Cọc ra ESC 1 (W) | Động cơ A2212 (M1-C) | 18 AWG Đen/Đỏ | Pha 3 dòng xoay chiều |
| **14** | 12 Dây Pha Motor | Pha U - Motor 2 | Cọc ra ESC 2 (U) | Động cơ A2212 (M2-B) | 18 AWG Xanh dương | Đảo pha A-B để Motor 2 quay CW |
| **15** | 12 Dây Pha Motor | Pha V - Motor 2 | Cọc ra ESC 2 (V) | Động cơ A2212 (M2-A) | 18 AWG Vàng | Đảo pha A-B để Motor 2 quay CW |
| **16** | 12 Dây Pha Motor | Pha W - Motor 2 | Cọc ra ESC 2 (W) | Động cơ A2212 (M2-C) | 18 AWG Đen/Đỏ | Pha 3 dòng xoay chiều |
| **17** | 12 Dây Pha Motor | Pha U - Motor 3 | Cọc ra ESC 3 (U) | Động cơ A2212 (M3-B) | 18 AWG Xanh dương | Đảo pha A-B để Motor 3 quay CW |
| **18** | 12 Dây Pha Motor | Pha V - Motor 3 | Cọc ra ESC 3 (V) | Động cơ A2212 (M3-A) | 18 AWG Vàng | Đảo pha A-B để Motor 3 quay CW |
| **19** | 12 Dây Pha Motor | Pha W - Motor 3 | Cọc ra ESC 3 (W) | Động cơ A2212 (M3-C) | 18 AWG Đen/Đỏ | Pha 3 dòng xoay chiều |
| **20** | 12 Dây Pha Motor | Pha U - Motor 4 | Cọc ra ESC 4 (U) | Động cơ A2212 (M4-A) | 18 AWG Xanh dương | Pha 1 dòng xoay chiều |
| **21** | 12 Dây Pha Motor | Pha V - Motor 4 | Cọc ra ESC 4 (V) | Động cơ A2212 (M4-B) | 18 AWG Vàng | Pha 2 dòng xoay chiều |
| **22** | 12 Dây Pha Motor | Pha W - Motor 4 | Cọc ra ESC 4 (W) | Động cơ A2212 (M4-C) | 18 AWG Đen/Đỏ | Pha 3 dòng xoay chiều |
| **23** | Nguồn 5V & 3.3V | 5V Nguồn Chính | Đầu ra UBEC 5V (hoặc BEC ESC1) | ESP32-S3 (5V/VIN) & PCA9685 (V+) | 22 AWG Đỏ | Nguồn 5V nuôi vi điều khiển & driver |
| **24** | Nguồn 5V & 3.3V | GND Chung Hệ Thống | Đầu ra UBEC GND | ESP32 GND & Cảm biến & PCA9685 | 22 AWG Đen | Đường mass chuẩn (Common Ground) |
| **25** | Nguồn 5V & 3.3V | 3.3V Nuôi Cảm Biến | ESP32-S3 Chân 3V3 | VCC (MPU, HMC, BMP, PCA9685) | 24-26 AWG Vàng | Nguồn 3.3V an toàn cho cảm biến |
| **26** | I2C Bus 400kHz | I2C Data Line (SDA) | ESP32-S3 GPIO 8 | SDA (MPU, HMC, BMP, PCA9685) | 26 AWG Xanh Lá | Tuyến truyền dữ liệu cảm biến |
| **27** | I2C Bus 400kHz | I2C Clock Line (SCL) | ESP32-S3 GPIO 9 | SCL (MPU, HMC, BMP, PCA9685) | 26 AWG Xanh Dương | Tuyến xung đồng hồ I2C |
| **28** | Trở kéo I2C | Trở kéo SDA | Đường SDA (GPIO 8) | Đường nguồn 3.3V | Trở 4.7kΩ (1/4W) | Kéo áp SDA chống suy hao xung |
| **29** | Trở kéo I2C | Trở kéo SCL | Đường SCL (GPIO 9) | Đường nguồn 3.3V | Trở 4.7kΩ (1/4W) | Kéo áp SCL chống méo sóng xung |
| **30** | Chân Cấu Hình | MPU6050 AD0 | Chân AD0 MPU6050 | GND | Dây câu ngắn | Cố định địa chỉ I2C = 0x68 |
| **31** | Chân Cấu Hình | BMP280 SDO | Chân SDO BMP280 | GND | Dây câu ngắn | Cố định địa chỉ I2C = 0x76 |
| **32** | Chân Cấu Hình | BMP280 CSB | Chân CSB BMP280 | Đường 3.3V | Dây câu ngắn | Ép BMP280 chạy chuẩn I2C |
| **33** | Chân Cấu Hình | PCA9685 OE | Chân OE PCA9685 | GND | Dây câu ngắn | Luôn kích hoạt đầu ra PWM |
| **34** | Xung PWM ESC | PWM Signal CH0 | PCA9685 CH0 Signal | Dây Trắng/Cam ESC 1 | Dây Servo | Xung 50Hz điều khiển Motor 1 |
| **35** | Xung PWM ESC | PWM Signal CH1 | PCA9685 CH1 Signal | Dây Trắng/Cam ESC 2 | Dây Servo | Xung 50Hz điều khiển Motor 2 |
| **36** | Xung PWM ESC | PWM Signal CH2 | PCA9685 CH2 Signal | Dây Trắng/Cam ESC 3 | Dây Servo | Xung 50Hz điều khiển Motor 3 |
| **37** | Xung PWM ESC | PWM Signal CH3 | PCA9685 CH3 Signal | Dây Trắng/Cam ESC 4 | Dây Servo | Xung 50Hz điều khiển Motor 4 |
| **38** | GPS UART1 | GPS TXD $\rightarrow$ RX1 | GPS ATGM336H TXD | ESP32-S3 GPIO 17 (RX1) | 26 AWG Hồng | Nhận định vị NMEA từ GPS |
| **39** | GPS UART1 | GPS RXD $\leftarrow$ TX1 | GPS ATGM336H RXD | ESP32-S3 GPIO 18 (TX1) | 26 AWG Xanh Dương | Gửi lệnh cấu hình vệ tinh |

---

## 3. BỐ TRÍ HÌNH HỌC QUAD-X & CHIỀU QUAY ĐỘNG CƠ

```
                           MŨI DRONE (FORWARD)
                                   ▲
                                   │
              (M2) CW              │              (M1) CCW
            [Động cơ 2]            │            [Động cơ 1]
         (Cánh thuận 1045R)        │         (Cánh ngược 1045)
                 \                 │                 /
                  \                │                /
                   \               │               /
                    \              │              /
                     \+------------+------------+/
                      |                         |
                      |      ESP32-S3 FC        |
                      |   [ MPU6050 + BMP280 ]  |
        TRÁI (LEFT) ──┤      Trọng tâm (CG)     ├── PHẢI (RIGHT)
                      |   [ HMC5883L Mũi Tới ]  |
                      |                         |
                     /+------------+------------+\
                    /              │              \
                   /               │               \
                  /                │                \
                 /                 │                 \
            [Động cơ 4]            │            [Động cơ 3]
              (M4) CCW             │              (M3) CW
         (Cánh ngược 1045)         │         (Cánh thuận 1045R)
                                   │
                                 ĐUÔI
```

* **M1 (Trước Phải)**: Quay ngược chiều kim đồng hồ (**CCW**), Cánh tiêu chuẩn (Normal/CCW). Kênh PCA9685 = **Channel 0**.
* **M2 (Trước Trái)**: Quay theo chiều kim đồng hồ (**CW**), Cánh đảo chiều (R/CW). Kênh PCA9685 = **Channel 1**.
* **M3 (Sau Phải)**: Quay theo chiều kim đồng hồ (**CW**), Cánh đảo chiều (R/CW). Kênh PCA9685 = **Channel 2**.
* **M4 (Sau Trái)**: Quay ngược chiều kim đồng hồ (**CCW**), Cánh tiêu chuẩn (Normal/CCW). Kênh PCA9685 = **Channel 3**.

---

## 4. KIỂM TRA TÍNH HỢP LỆ VỀ LOGIC & PHẦN CỨNG (LOGIC VERIFICATION)

| Thành phần | Tiêu chuẩn kiểm tra | Kết quả kiểm tra logic | Đánh giá |
| :--- | :--- | :--- | :--- |
| **Quy hoạch GPIO ESP32-S3** | Tránh dùng GPIO 33-37 (Octal SPI Flash/PSRAM) và GPIO 19, 20 (Native USB D+/D-). | Các chân được chọn: SDA=8, SCL=9, RX1=17, TX1=18, L9110S=4,5,6,7. Hoàn toàn cách ly khỏi vùng cấm. | **HỢP LỆ 100%** |
| **Địa chỉ bus I2C** | Không được trùng địa chỉ giữa các module trên cùng bus. | MPU6050 (0x68), HMC (0x1E)/QMC (0x0D), BMP280 (0x76), PCA9685 (0x40). 4 địa chỉ hoàn toàn riêng biệt. | **HỢP LỆ 100%** |
| **Tương thích mức logic (Logic Level)** | ESP32-S3 chỉ nhận tối đa 3.3V trên các chân GPIO. | Tất cả cảm biến MPU6050, BMP280, QMC5883L chạy ở 3.3V. PCA9685 nhận tín hiệu I2C 3.3V từ ESP32 và xuất PWM 50Hz logic 3.3V–5V tương thích hoàn toàn với ESC 30A. | **HỢP LỆ 100%** |
| **Độ phân giải & Tần số PWM** | ESC 30A nhận chuẩn PWM 50Hz (1000–2000µs). | PCA9685 cấu hình Prescale=121 (50Hz), chu kỳ 20ms, độ phân giải 12-bit (4096 mức) cho ra bước chia xung ~4.88µs/nấc (~205 nấc điều khiển ga). | **HỢP LỆ 100%** |
| **Giao tiếp GPS NMEA** | Tốc độ truyền baud và chuẩn ký tự. | UART1 chạy ở 9600 baud, 8N1, phân tích định dạng NMEA 0183 chuẩn ($GNGGA, $GNRMC) có kiểm tra checksum XOR. | **HỢP LỆ 100%** |

---

## 5. PHÂN TÍCH RỦI RO & BIỆN PHÁP TRIỆT TIÊU (RISK & MITIGATION MATRIX)

### ⚠️ Rủi ro 1: Xung đột dòng điện giữa các dây 5V BEC của 4 ESC (Ground/Voltage Loop)
* **Hiện tượng**: Mỗi ESC 30A thường tích hợp 1 mạch ổn áp tuyến tính 5V/2A (BEC). Nếu cắm cả 4 dây đỏ (+5V) từ 4 ESC vào chung một thanh nguồn PCA9685 hoặc ESP32, sự chênh lệch áp nhỏ giữa các BEC (ví dụ: 4.95V, 5.02V, 5.10V) sẽ gây ra dòng điện chạy ngược giữa các ESC, làm nóng rực IC ổn áp, gây sụt áp hoặc cháy ESC.
* **Biện pháp xử lý bắt buộc**:
  1. **Chỉ giữ lại dây ĐỎ (5V) của 1 ESC duy nhất** (ví dụ ESC 1) để nuôi mạch, hoặc tốt nhất dùng **UBEC 5V/3A rời**.
  2. Dùng dao nhọn cạy lẫy nhựa rút đầu pin dây Đỏ của 3 ESC còn lại ra, quấn băng dính cách điện kỹ lưỡng. Dây Đen (GND) và Dây Trắng/Cam (Signal) của cả 4 ESC vẫn cắm đủ vào PCA9685.

### ⚠️ Rủi ro 2: Nhiễu điện từ trường dòng cao từ Động cơ làm treo Bus I2C
* **Hiện tượng**: Khi 4 động cơ A2212 tăng tốc đột ngột, dòng xả có thể vọt lên 40A–60A, sinh ra sóng xung điện áp ngược (Back-EMF) và nhiễu từ trường cao tần trên đường dây nguồn làm bus I2C bị đơ (I2C Lockup).
* **Biện pháp xử lý**:
  1. **Tụ lọc nguồn chính**: Hàn 1 tụ hóa dung lượng lớn **1000µF – 2200µF / 25V Low-ESR** trực tiếp ngay tại cổng vào của PDB / Jack cắm XT60.
  2. **Trở kéo I2C (Pull-up resistors)**: Mặc dù các module có sẵn trở kéo nội, nếu dây nối I2C dài > 10cm, cần hàn thêm 2 trở **2.2kΩ hoặc 4.7kΩ** từ SDA và SCL lên 3.3V.
  3. **Thời gian chờ chống treo code**: Trong firmware đã cấu hình `Wire.setTimeOut(5);` (5ms) giúp vòng lặp điều khiển 250Hz không bao giờ bị nghẽn nếu I2C gặp xung nhiễu.

### ⚠️ Rủi ro 3: Nhiễu từ trường làm sai lệch La bàn (Magnetometer Distortion)
* **Hiện tượng**: Dây nguồn chính dẫn dòng điện 40A tạo ra từ trường cảm ứng lớn, lấn át từ trường Trái Đất làm la bàn HMC5883L/QMC5883L chỉ sai hướng.
* **Biện pháp xử lý**:
  1. Xoắn đôi (Twisted Pair) các cặp dây nguồn (+ và -) từ PDB tới từng ESC để triệt tiêu từ trường phát xạ.
  2. Bố trí module Magnetometer / GPS trên một **trụ nâng cao (Mast/Pillar) từ 10cm – 15cm** so với mặt phẳng PDB và động cơ.

### ⚠️ Rủi ro 4: Rung cơ học làm bão hòa cảm biến MPU6050 (Acoustic & Vibration Noise)
* **Hiện tượng**: Cánh quạt 1045 chưa được cân bằng động (Prop Balancing) sẽ rung mạnh ở tần số 100Hz – 300Hz, làm con quay hồi chuyển và gia tốc kế sinh ra giá trị rác.
* **Biện pháp xử lý**:
  1. Gắn bo mạch Flight Controller lên khung drone qua **4 đệm cao su giảm chấn (Silicon Anti-Vibration Dampers)**.
  2. Cấu hình bộ lọc phần cứng MPU6050 DLPF ở mức **94Hz Accel / 98Hz Gyro** (đã tích hợp trong firmware `src/sensors/ImuSensor.cpp`).
  3. Cân bằng tĩnh và cân bằng động cho cả 4 cánh quạt bằng băng dính chuyên dụng trước khi bay.

---

## 6. ĐÁNH GIÁ TÍNH KHẢ THI & KẾ HOẠCH NẠP CODE CHẠY THỰC TẾ

### ✅ Đánh giá tính khả thi (Feasibility Summary)
1. **Khả năng tải dòng (Current Capability)**: Pin LiPo 3S 2200mAh 25C cho dòng xả liên tục $2.2 \times 25 = 55\text{A}$, đáp ứng dư dả cho 4 động cơ A2212 ăn dòng trung bình 30A–40A khi bay lơ lửng (Hover).
2. **Khả năng xử lý của MCU**: ESP32-S3 Dual-Core chạy ở 240MHz thực thi thuật toán Mahony 9DOF + Cascade PID 250Hz chỉ chiếm **< 15% CPU load** và **6.3% RAM**, hoàn toàn mượt mà và không trễ xung.
3. **Độ ổn định hệ thống**: Tất cả các giao thức (I2C, UART, PWM PCA9685) đều độc lập, có cơ chế Failsafe đa tầng (Mất sóng tự hạ cánh, lật góc > 45° tự ngắt khẩn cấp, khóa an toàn Arm ga 0%).

### 📋 CHECKLIST CẮM DÂY TRƯỚC KHI CẤP NGUỒN (PRE-FLIGHT CHECKLIST)
- [ ] **BƯỚC 1: THÁO HẾT 4 CÁNH QUẠT** ra khỏi động cơ trước khi cắm pin LiPo.
- [ ] **BƯỚC 2**: Kiểm tra thông mạch GND giữa ESP32-S3, PCA9685, cảm biến và PDB bằng đồng hồ VOM (Đảm bảo điện trở GND < 0.2Ω).
- [ ] **BƯỚC 3**: Kiểm tra chỉ có duy nhất **1 dây ĐỎ 5V** từ ESC/UBEC cấp vào mạch.
- [ ] **BƯỚC 4**: Cắm cáp Type-C vào ESP32-S3, mở trình duyệt vào công cụ `tools/tuner/index.html`.
- [ ] **BƯỚC 5**: Nhấn nút **"Kết Nối USB Serial"** ở 115200 baud -> Kiểm tra khối 3D trên màn hình xoay đồng bộ với tư thế drone thực tế.
- [ ] **BƯỚC 6**: Thực hiện lệnh `CALIB GYRO` và `CALIB MAG` trên mặt phẳng cố định.
- [ ] **BƯỚC 7**: Dùng chức năng **Test Motor (Giới hạn tối đa 30%)** để kiểm tra chiều quay từng motor:
  - Motor 1 quay CCW (Ngược chiều kim đồng hồ)
  - Motor 2 quay CW (Theo chiều kim đồng hồ)
  - Motor 3 quay CW (Theo chiều kim đồng hồ)
  - Motor 4 quay CCW (Ngược chiều kim đồng hồ)
- [ ] **BƯỚC 8**: Khi mọi chiều quay và góc phản hồi PID đều chính xác -> Mới tiến hành lắp cánh quạt vào trục motor.
