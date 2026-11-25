# Kịch bản Quay & Chạy Demo — BTL Vi xử lý – Vi điều khiển (CO3009)

**Đề tài:** Hệ thống Điều khiển Đèn Giao Thông 2 Tuyến Đường & Giám sát Môi trường Thời gian thực trên STM32F103RB  
**Board:** XNUCLEO-F103RB (`64 MHz`, `TIM2` tick `10 ms`, `I2C1` `100 kHz`)  
**Thời lượng quay mục tiêu:** 2 phút 30 giây – 3 phút (khuyến nghị quay 1 take liền mạch)

---

## 1. Quy ước nhanh trước khi quay

| Phần tử | Chân STM32 / Địa chỉ | Chức năng thực tế trong firmware (`MERGE`) |
| :--- | :--- | :--- |
| **`BTN_1`** (D7) | `PA8` | Đổi chế độ: `AUTO → MANUAL → TUNING → AUTO` |
| **`BTN_2`** (D8) | `PA9` | `MANUAL`: sang bước tiếp theo \| `TUNING`: chọn trường `RED / GREEN / YELLOW` |
| **`BTN_3`** (D9) | `PC7` | `TUNING`: giảm thông số đang chọn `-1 s` |
| **`BTN_4`** (D10) | `PB6` | `TUNING`: tăng thông số đang chọn `+1 s` |
| **Đèn `R1` / `R2`** | `PB3,PB5` / `PB4,PB10` | Mã hóa 2 dây: `(0,0)=OFF`, `(1,1)=RED`, `(0,1)=AMBER`, `(1,0)=GREEN` |
| **`DHT20` / `LCD`** | `PB8/PB9` (`0x38` / `0x27`) | Chung bus `I2C1`; DHT20 đọc mỗi `~2.1 s`, LCD đảo trang mỗi `3.0 s` ở `AUTO` |

- **Thông số mặc định:** `Red = 18 s`, `Green = 10 s`, `Yellow = 3 s` $\Rightarrow$ `All-Red = (18 - 10 - 3) / 2 = 2.5 s`.
- **Ràng buộc `TUNING`:** `3 <= Green <= 90`, `1 <= Yellow <= 10`, `Red <= 180` và `Red >= Green + Yellow + 2 s`.

---

## 2. Checklist chuẩn bị bàn quay & phần cứng (60 giây trước khi bấm REC)

### Bố trí khung hình
- [ ] Cố định camera trên giá đỡ, góc nghiêng **35°–45° từ trên xuống** để không lóa mặt kính LCD và tay bấm không che đèn.
- [ ] Đặt gọn trong cùng 1 khung hình:
  - **Hàng trên:** Cụm đèn `R1` (trái) — Màn hình `LCD 16x2` (giữa) — Cụm đèn `R2` (phải).
  - **Hàng dưới:** Board `XNUCLEO-F103RB` — Cảm biến `DHT20` — 4 nút nhấn (dán nhãn giấy `1:MODE`, `2:ACT`, `3:-`, `4:+`).
- [ ] Chỉnh biến trở tương phản mặt sau PCF8574 để chữ trên LCD nét rõ qua ống kính camera.

### Kiểm tra mạch chạy thử
- [ ] Xác nhận đang build & nạp đúng code nhánh **`MERGE`**.
- [ ] Kiểm tra bus I2C thấy đủ 2 thiết bị: `0x38` (`DHT20`) và `0x27` (`LCD`).
- [ ] Vào `MANUAL` bấm `BTN_2` đủ 1 vòng xác nhận đúng thứ tự màu `ĐỎ → XANH → VÀNG → ĐỎ` cho cả `R1` và `R2`.
- [ ] Để yên mạch 10 giây không chạm nút, đảm bảo không tự nhảy chế độ.
- [ ] Chờ 3 giây ở `AUTO`, xác nhận LCD hiển thị đúng nhiệt độ/độ ẩm (không bị `"DHT20: WAITING"`).

### 4 bẫy thao tác cần tránh khi đang quay
1. **Không chạm nhầm `BTN_2..4` lúc đang ở `AUTO`:** Cờ nút nhấn chưa được xóa khi đổi mode; nếu lỡ bấm `BTN_2` trước đó, khi sang `MANUAL` đèn sẽ nhảy thẳng sang Step 1 (`R1:GRN R2:RED`), bỏ qua Step 0 (`All-Red`).
2. **Bấm – nhả dứt khoát từng nhịp ở `TUNING`:** `fsm_tuning` dùng `isButtonPressed()`, giữ lì nút sẽ không tăng/giảm liên tục.
3. **Chờ đủ nhịp cập nhật `DHT20` trên LCD:** Sau khi hà hơi/sưởi ấm cảm biến, cần chờ khoảng **5–6 giây** (1 chu kỳ đo ~2.1 s + nhịp đảo trang LCD 3 s) mới thấy thông số tăng rõ trên màn hình.
4. **Kiểm tra dây nguồn/mass cụm nút nhấn:** Do 4 chân nút đang cấu hình `GPIO_NOPULL` (Active-Low), dây lỏng sẽ gây floating làm loạn chế độ.

---

## 3. Kịch bản Thao tác Tay & Lời thoại từng cảnh

| Cảnh | Thời gian | Thao tác phần cứng | Hiện tượng kỳ vọng (Đèn + LCD) | Lời thoại thuyết minh (15–20 giây/cảnh) |
| :--- | :---: | :--- | :--- | :--- |
| **Cảnh 1**<br>Khởi động & Tổng quan | `0:00 – 0:20` | Nhấn nút **Reset (đen)** trên board XNUCLEO-F103RB. Giữ yên tay. | - **Đèn:** `R1` ĐỎ, `R2` XANH.<br>- **LCD (0–3 s):**<br>`MODE: AUTO`<br>`R1:RED  R2:GRN`<br>- **LCD (3–6 s)** tự lật trang:<br>`TEMP (C):xx.xC`<br>`HUMIDITY:xx.x%` | *"Đây là hệ thống điều khiển đèn giao thông 2 tuyến đường tích hợp giám sát môi trường trên STM32F103RB chạy 64 MHz. Toàn bộ tác vụ chạy trên Cooperative Scheduler $O(1)$ theo nhịp ngắt cứng TIM2 mỗi 10 ms, không sử dụng `HAL_Delay()` trong vòng lặp chính."* |
| **Cảnh 2**<br>Chu trình `AUTO` & Pha đệm All-Red | `0:20 – 0:45` | Giữ yên máy quay, theo dõi nhịp chuyển pha từ giây thứ 10 đến giây thứ 16. | - **10 s:** `R1` ĐỎ, `R2` VÀNG (`3 s`). LCD: `R1:RED  R2:YEL`.<br>- **13 s:** **Cả R1 và R2 cùng ĐỎ** (`2.5 s`). LCD: `R1:RED  R2:RED`.<br>- **15.5 s:** `R1` XANH, `R2` ĐỎ (`10 s`). LCD: `R1:GRN  R2:RED`. | *"Ở chế độ AUTO mặc định Đỏ 18 s, Xanh 10 s, Vàng 3 s, hệ thống chạy chu trình 6 pha đối xứng. Giữa hai chiều luôn có pha đệm All-Red 2.5 giây tính theo công thức $(T_{\text{red}} - T_{\text{green}} - T_{\text{yellow}})/2$, giúp dọn sạch giao lộ và ngăn tuyệt đối 2 tuyến cùng xanh."* |
| **Cảnh 3**<br>Giám sát môi trường `DHT20` | `0:45 – 1:10` | Hà hơi nhẹ hoặc đặt ngón tay ấm cách cảm biến **DHT20** ~1 cm trong 3 giây, chờ LCD lật sang trang môi trường tiếp theo. | - Đèn giao thông vẫn chuyển pha mượt, **không bị khựng**.<br>- Khi LCD lật trang: `HUMIDITY` tăng rõ (+5% đến +15%), `TEMP (C)` nhích nhẹ. | *"Cảm biến DHT20 dùng chung bus I2C1 100 kHz với LCD. Driver tách thành máy trạng thái non-blocking 2 pha để tránh chờ 80 ms gây treo CPU, kiểm tra toàn vẹn bằng CRC-8 trước khi hiển thị luân phiên lên LCD mỗi 3 giây."* |
| **Cảnh 4**<br>Chế độ `MANUAL` (7 bước an toàn) | `1:10 – 1:40` | Nhấn **`BTN_1`** đúng 1 lần khi một tuyến đang Xanh/Vàng.<br>Sau đó nhấn **`BTN_2`** từng nhịp (cách nhau ~1.5 s) đi qua 4–5 bước. | - Sau `BTN_1`: **Cả 2 tuyến lập tức về ĐỎ** (Step 0). LCD: `MODE: MANUAL` / `R1:RED  R2:RED`.<br>- Sau mỗi lần `BTN_2`:<br>1: `R1:GRN  R2:RED`<br>2: `R1:YEL  R2:RED`<br>3: `R1:RED  R2:RED`<br>4: `R1:RED  R2:GRN`. | *"Khi nhấn BTN\_1 chuyển sang MANUAL, `fsm_mode_manager` lập tức ép cả 2 tuyến về All-Red an toàn. Các nút nhấn đều qua bộ lọc chống dội 30 ms bằng 3 thanh ghi dịch. Bấm BTN\_2 sẽ đi qua bảng 7 bước, luôn ghi Đỏ tuyến đối diện trước khi bật Xanh."* |
| **Cảnh 5**<br>Chế độ `TUNING` & Ràng buộc an toàn | `1:40 – 2:15` | 1. Nhấn **`BTN_1`** vào `TUNING`.<br>2. Bấm **`BTN_3` (`-`)** liên tiếp 4–5 lần ở `RED` để chứng minh bị chặn ở `15s`.<br>3. Bấm **`BTN_2`** sang `GREEN`, bấm **`BTN_3`** giảm về `6s`.<br>4. Bấm **`BTN_2`** sang `YELLOW`, bấm **`BTN_3`** giảm về `2s`. | - Đèn giữ cố định **All-Red**.<br>- LCD dòng 0: `MODE: TUNING`.<br>- `RED` giảm từ `18s` xuống **`15s`** rồi đứng yên (không giảm thêm dù bấm `BTN_3`).<br>- `GREEN TIME:  6s`.<br>- `YELLOW TIME:  2s`. | *"Ở chế độ TUNING, BTN\_2 chọn Đỏ/Xanh/Vàng, BTN\_3 giảm và BTN\_4 tăng 1 giây. Hệ thống khóa chặt ràng buộc $T_{\text{red}} \ge T_{\text{green}} + T_{\text{yellow}} + 2$ giây nên không thể giảm Đỏ dưới 15 s khi Xanh 10 s, Vàng 3 s — đảm bảo pha All-Red luôn tối thiểu 1 giây."* |
| **Cảnh 6**<br>Áp dụng thông số mới & Kết thúc | `2:15 – 2:45` | Nhấn **`BTN_1`** quay lại `AUTO`. Quay thêm ~10 giây để chứng minh thời gian mới có hiệu lực ngay. | - Hệ thống về `MODE: AUTO`: `R1:RED R2:GRN` sáng đúng **6 s**, `R2:YEL` sáng **2 s**, `All-Red` sáng **3.5 s** (`(15-6-2)/2`), rồi đảo sang `R1:GRN`. | *"Nhấn BTN\_1 quay lại AUTO, bộ tham số mới Xanh 6 s – Vàng 2 s được áp dụng tức thì mà không cần reset mạch. Toàn bộ hệ thống vận hành ổn định, đáp ứng đầy đủ yêu cầu thời gian thực và an toàn giao thông."* |

---

## 4. Xử lý sự cố nhanh & Câu hỏi phản biện lúc Demo

### 4.1. Troubleshooting trong 30 giây
- **LCD sáng đèn nền nhưng không có chữ:** Xoay nhẹ biến trở xanh mặt sau PCF8574; nếu vẫn trống, đặt breakpoint tại `main.c:112` kiểm tra `found_i2c[]` xem địa chỉ là `0x27` hay `0x3F` (cập nhật `LCD_DEFAULT_ADDR_7BIT` trong `lcd.h`).
- **Đèn sai màu hoặc không sáng ở `PB3`/`PB4`:** Kiểm tra bảng chân trị `TL_PIN_MAP` trong `traffic_light.c`; xác nhận `.ioc` đã bật `SYS → Serial Wire` (`__HAL_AFIO_REMAP_SWJ_NOJTAG()`).
- **LCD báo `DHT20: WAITING / NO VALID SAMPLE`:** Kiểm tra lại dây `PB8 (SCL)` / `PB9 (SDA)` và nguồn cảm biến, nhấn Reset board.
- **Chế độ tự nhảy loạn khi không bấm nút:** Kiểm tra mass/nguồn module nút nhấn; nếu dùng nút nhấn rời nối GND không có điện trở kéo lên ngoài, đổi cấu hình `BTN_1..4` sang `GPIO_PULLUP`.

### 4.2. Câu hỏi vấn đáp nhanh (Trả lời ngắn gọn 1–2 câu)
1. **Tại sao `TIM2` có `PSC = 6399`, `ARR = 99` lại ra `10 ms` khi APB1 là `32 MHz`?**  
   Vì `APB1 Prescaler = /2` ($\ne 1$) nên xung cấp cho timer APB1 trên STM32F103 tự nhân đôi thành `64 MHz` $\Rightarrow T = \frac{6400 \times 100}{64 \times 10^6} = 10\text{ ms}$.
2. **`SCH_Update()` đạt $O(1)$ bằng cách nào?**  
   Mảng `SCH_tasks_G[]` dùng hàng đợi độ trễ tương đối (Delta Queue); trong ngắt `TIM2`, `SCH_Update()` chỉ trừ `SCH_tasks_G[0].Delay` mà không duyệt vòng lặp.
3. **Tại sao không dùng `HAL_Delay(80)` khi đọc `DHT20`?**  
   `HAL_Delay(80)` làm đóng băng CPU 8 tick 10 ms, mất nhịp chống dội nút nhấn. `dht20_read_task()` chạy mỗi 100 ms với 2 trạng thái `IDLE` (gửi `0xAC`) và `WAIT_DATA` (100 ms sau đọc 7 byte + CRC-8).
4. **Hướng tối ưu nếu muốn tránh I2C LCD kéo dài quá 1 tick 10 ms?**  
   `lcd_print_line()` đã gộp 16 ký tự (64 byte) trong 1 gói I2C (~6.8 ms); để vẽ 2 dòng không vượt 10 ms, có thể tách vẽ dòng 0 và dòng 1 ra 2 tick khác nhau hoặc dùng `HAL_I2C_Master_Transmit_DMA/IT`.
