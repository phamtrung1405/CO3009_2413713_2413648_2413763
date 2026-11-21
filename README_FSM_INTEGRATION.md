# CO3009 – Integrated Scheduler + Peripheral Drivers + FSM

Đây là bản tích hợp từ project `feat-peripheral-drivers` (đã có scheduler + button/traffic/DHT20/LCD) và phần FSM Application & Logic Engineer.

## Điều khiển nút nhấn

- BTN_1: đổi chế độ theo vòng AUTO → MANUAL → TUNING → AUTO.
- BTN_2: trong MANUAL, chuyển bước đèn; trong TUNING, chọn trường Đỏ/Xanh/Vàng.
- BTN_3: giảm thông số đang chọn 1 giây.
- BTN_4: tăng thông số đang chọn 1 giây.

## Quy tắc pha và thời gian

Ở AUTO, LCD luân phiên trang trạng thái đèn và trang nhiệt độ/độ ẩm DHT20 (mỗi trang khoảng 3 s). Mặc định Đỏ 18 s, Xanh 10 s, Vàng 3 s. Chu trình AUTO: R1 Đỏ/R2 Xanh → R1 Đỏ/R2 Vàng → cả hai Đỏ → R1 Xanh/R2 Đỏ → R1 Vàng/R2 Đỏ → cả hai Đỏ → lặp lại. Mỗi khoảng cả hai Đỏ là `(Đỏ - Xanh - Vàng) / 2`. Ràng buộc khi chỉnh: `Đỏ >= Xanh + Vàng + 2 s`, tức mỗi khoảng all-red ít nhất 1 s.

## Các điểm đã sửa để tích hợp

1. `software_timer.h/.c`: bỏ biến `TIMER_CYCLE` chưa khởi tạo; `setTimer(index, duration_ms)` quy đổi millisecond sang tick 10 ms và dùng `extern` cho timer flags/counters. Thêm `software_timer_init()`.
2. `main.c`: bỏ task LED thử nghiệm, đăng ký `button_read`, `dht20_read_task` và ba FSM cùng `fsm_mode_manager_run` vào scheduler.
3. `main.c`: đặt TIM2 PSC=6399, ARR=99. Với SYSCLK=64 MHz, APB1 prescaler=2, timer clock TIM2=64 MHz nên chu kỳ ngắt là `(6399+1)*(99+1)/64 MHz = 10 ms`, khớp `TICK_MS=10` và cấu hình `.ioc`. Giá trị cũ PSC=7999, ARR=9 không tương ứng 10 ms với clock này.

## Build trong STM32CubeIDE

1. Giải nén ZIP.
2. STM32CubeIDE → **File → Import → Existing Projects into Workspace**.
3. Chọn thư mục `CO3009_FSM_Integrated/src` (thư mục có `.project`, `.cproject`, `.ioc`).
4. Chọn project → **Project → Clean**, sau đó **Build Project**.
5. Nạp lên XNUCLEO-F103RB qua ST-LINK. Kiểm tra Board Selector là NUCLEO-F103RB và SYS → Debug = Serial Wire.

## Test trên board

**AUTO:** Sau khởi động, R1 Đỏ/R2 Xanh 10 s; R1 Đỏ/R2 Vàng 3 s; cả hai Đỏ 2.5 s; R1 Xanh/R2 Đỏ 10 s; R1 Vàng/R2 Đỏ 3 s; cả hai Đỏ 2.5 s; rồi lặp lại.

**MANUAL:** Nhấn BTN_1 vào MANUAL. Hai tuyến về Đỏ trước. BTN_2 lần lượt chuyển R1 Xanh/R2 Đỏ → R1 Vàng/R2 Đỏ → cả hai Đỏ → R1 Đỏ/R2 Xanh → R1 Đỏ/R2 Vàng → cả hai Đỏ → lặp.

**TUNING:** Nhấn BTN_1 vào TUNING. BTN_2 chọn Đỏ/Xanh/Vàng; BTN_3 giảm 1 s; BTN_4 tăng 1 s. Thử giảm Đỏ liên tục: giá trị không thấp hơn Xanh + Vàng + 2 s. Nhấn BTN_1 quay về AUTO để áp dụng chu kỳ mới.

**Chuyển chế độ:** đổi chế độ khi một tuyến đang Xanh hoặc Vàng; cả hai phải về Đỏ trước khi vào mode mới. Không được có thời điểm hai tuyến cùng Xanh.

## Lưu ý phần cứng

`button.c` đang cấu hình nhấn mức thấp (`GPIO_PIN_RESET`) và các chân nút trong `.ioc` là `GPIO_NOPULL`. Xác nhận bằng bước đo mức logic trên mạch. Nếu nút thực tế active-high, đổi `BUTTON_PRESSED_STATE`/`BUTTON_NORMAL_STATE` trong `Core/Src/button.c`.

## Giới hạn xác minh

Host test của FSM nằm trong ZIP standalone và đã chạy qua. Môi trường tạo gói không có STM32CubeIDE/`arm-none-eabi-gcc` hoặc board XNUCLEO-F103RB, nên bản firmware tích hợp chưa được build bằng toolchain STM32 hoặc chạy trên phần cứng ở đây. Cần thực hiện build/nạp/test ở máy của nhóm.
