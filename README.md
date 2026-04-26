# 3-Channel LED Waveform Control on STM32

Bài test thực hành lập trình nhúng STM32 — Round 2

---

## Giới thiệu

Project điều khiển 3 LED tạo hiệu ứng sóng sin lệch pha đều nhau trên vi điều khiển STM32F103C8Tx.  
Em thực hiện bài test này nhằm đáp ứng các yêu cầu kỹ thuật về Timer PWM, Interrupt và tối ưu ISR.

---

## Phần cứng sử dụng

| Pin  | Chức năng |
|------|-----------|
| PA8  | TIM1_CH1 — LED A |
| PA9  | TIM1_CH2 — LED B |
| PA10 | TIM1_CH3 — LED C |
| PA0  | Nút nhấn — bật/tắt hiệu ứng |

- **Board:** STM32F103C8Tx (Blue Pill)
- **Clock:** 72 MHz (HSE 8MHz × PLL×9)
- **IDE:** STM32CubeIDE + STM32CubeMX

---

## Cấu hình Timer

Em sử dụng **duy nhất 1 Timer (TIM1)** để vừa xuất 3 kênh PWM vừa tạo ngắt cập nhật.

| Thông số | Giá trị |
|----------|---------|
| Timer | TIM1 |
| Chế độ | Center-Aligned Mode 1 |
| Prescaler | 71 |
| Period (ARR) | 999 |
| Chu kỳ ngắt | ~2ms (nằm trong khoảng 1–5ms yêu cầu) |
| Kênh PWM | CH1, CH2, CH3 |

---

## Thuật toán điều khiển độ sáng

Em dùng **Lookup Table (LUT)** gồm 120 giá trị mô phỏng sóng sin, tính sẵn và lưu trong Flash:

```c
#define TABLE_SIZE  120   // 120 phan tu (> 100 theo yeu cau)
#define PHASE_STEP   40   // lech pha 120 do giua moi LED

// 3 LED bat dau o vi tri khac nhau trong bang
idx_A = 0;    // LED A
idx_B = 40;   // LED B lech 120 do
idx_C = 80;   // LED C lech 240 do
```

Mỗi lần ngắt xảy ra, 3 chỉ số được tăng lên đồng thời → 3 LED luôn giữ khoảng cách đều nhau.

---

## Tối ưu ISR

Đây là phần em tập trung cải thiện so với code baseline được cung cấp:

**Baseline (chưa tối ưu):**
```c
// Dung % -> co phep chia, ton CPU trong ISR
idx_A = (idx_A + 1) % TABLE_SIZE;

// Dung HAL macro -> co overhead
__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, sine_table[idx_A]);
```

**Code em đã cải thiện:**
```c
// Truy cap truc tiep thanh ghi -> nhanh hon, khong qua HAL
TIM1->CCR1 = sine_table[idx_A];

// Thay % bang compare+subtract -> khong co phep chia
idx_A += step;
if (idx_A >= TABLE_SIZE) idx_A -= TABLE_SIZE;
```

---

## Các yêu cầu kỹ thuật đã đáp ứng

| Yêu cầu | Kết quả |
|---------|---------|
| Chỉ dùng 1 Timer cho cả PWM lẫn Interrupt | ✅ |
| PWM Center-Aligned | ✅ |
| LUT tối thiểu 100 phần tử | ✅ 120 phần tử |
| 3 LED lệch pha đều nhau | ✅ 120° mỗi LED |
| Cập nhật PWM trong ISR, chu kỳ 1–5ms | ✅ ~2ms |
| Không dùng `float` hay `sin()` ở runtime | ✅ |
| Không dùng `HAL_Delay()` hay blocking | ✅ |
| Không dùng `%` trong ISR | ✅ |
| Biến dùng trong ISR khai báo `volatile` | ✅ |
| ISR ngắn gọn, không out-of-bounds | ✅ |

---

## Tính năng nâng cao đã thực hiện

- **Speed control:** Biến `step` cho phép thay đổi tốc độ hiệu ứng mà không làm mất đồng đều pha giữa 3 LED
- **Nút nhấn:** PA0 toggle bật/tắt hiệu ứng, có debounce 50ms

---

## Cấu trúc thư mục

```
Core/
  Src/main.c       — Code chính: khởi tạo, ISR, callback nút nhấn
  Inc/main.h
Drivers/           — HAL Library (do CubeMX tạo)
*.ioc              — File cấu hình CubeMX
*.ld               — Linker script
README.md
```

---

## Cách build project

1. Mở **STM32CubeIDE**
2. **File → Open Projects from File System** → chọn thư mục này
3. Bấm **Build** (Ctrl+B)
4. Kết quả mong đợi: `Build Finished. 0 errors, 0 warnings`
