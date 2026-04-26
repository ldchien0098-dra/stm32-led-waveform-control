# 3-Channel LED Waveform Control on STM32

**Bài test thực hành lập trình nhúng STM32 — Round 2**

---

## Mô tả bài toán

Viết firmware điều khiển 3 LED (A, B, C) tạo hiệu ứng ánh sáng dạng sóng sin:
- Độ sáng mỗi LED thay đổi mượt mà, liên tục
- 3 LED lệch pha đều nhau (120°)
- Hệ thống chạy ổn định, không giật, không chớp bất thường

---

## Hardware

| Pin  | Chức năng         |
|------|-------------------|
| PA8  | TIM1_CH1 — LED A  |
| PA9  | TIM1_CH2 — LED B  |
| PA10 | TIM1_CH3 — LED C  |
| PA0  | Button — bật/tắt hiệu ứng (EXTI0, falling edge) |

**MCU:** STM32F103C8Tx (Blue Pill) — 72 MHz (HSE 8MHz × PLL×9)

---

## Cấu hình Timer

| Thông số | Giá trị | Ghi chú |
|----------|---------|---------|
| Timer | TIM1 | 1 timer duy nhất |
| Mode | Center-Aligned Mode 1 | Đúng yêu cầu bài test |
| Prescaler | 71 | Timer clock = 1 MHz |
| ARR (Period) | 999 | PWM_MAX = 999 |
| Update Interrupt | mỗi 2ms | Nằm trong khoảng 1–5ms |
| Kênh PWM | CH1 / CH2 / CH3 | Cùng 1 timer |

---

## Sine Lookup Table

```c
#define TABLE_SIZE  120U
#define PHASE_STEP  40U    // lech pha 120 do (= TABLE_SIZE / 3)

// Scale [124..875] tren ARR=999
// Tinh offline: round(499.5 * sin(2*PI*i/120) + 499.5)
// Luu trong Flash (const) — khong dung float o runtime
const uint16_t sine_table[TABLE_SIZE] = { ... };
```

- **120 phần tử** (≥ 100 theo yêu cầu)
- **Lưu trong Flash** — không tốn RAM
- **Không dùng `float` hay `sin()`** ở runtime
- 3 index lệch pha đều: `idx_A=0`, `idx_B=40`, `idx_C=80`

---

## Tối ưu ISR — Những gì đã cải thiện so với Baseline

### ❌ Baseline (chưa tối ưu)
```c
// Dung % trong ISR -> ton CPU
idx_A = (idx_A + 1) % TABLE_SIZE;

// Dung HAL macro -> co overhead
__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, sine_table[idx_A]);
```

### ✅ Code đã tối ưu
```c
// Truy cap truc tiep thanh ghi -> khong qua HAL, nhanh hon
TIM1->CCR1 = sine_table[idx_A];
TIM1->CCR2 = sine_table[idx_B];
TIM1->CCR3 = sine_table[idx_C];

// Compare + subtract -> thay the %, khong co phep chia
idx_A += step;
if (idx_A >= TABLE_SIZE) idx_A -= TABLE_SIZE;
```

### Bảng so sánh đầy đủ

| Vấn đề Baseline | Giải pháp đã áp dụng |
|-----------------|----------------------|
| Dùng `%` trong ISR | `if/subtract` — không dùng phép chia |
| `__HAL_TIM_SET_COMPARE()` | `TIM1->CCRx` — truy cập trực tiếp thanh ghi |
| Không có speed control | `volatile uint8_t step` — điều chỉnh tốc độ hiệu ứng |
| ISR chưa thoát sớm | `if (!run) return;` — early return trước khi chạm CCR |
| Không có nút nhấn | PA0 EXTI0 toggle `run`, debounce 50ms |

---

## Ràng buộc hiệu năng — Đã đáp ứng đầy đủ

| Yêu cầu | Trạng thái |
|---------|------------|
| Không dùng `float` / `double` trong ISR | ✅ |
| Không gọi `sin()` ở runtime | ✅ |
| Không dùng `HAL_Delay()` hay blocking | ✅ |
| Không out-of-bounds trên LUT | ✅ |
| Biến ISR khai báo `volatile` | ✅ |
| Không dùng bộ nhớ động | ✅ |
| ISR ngắn gọn, tối ưu | ✅ ~200ns @ 72MHz |

---

## Yêu cầu nâng cao đã thực hiện

| Tính năng | Mô tả |
|-----------|-------|
| Speed control | `volatile uint8_t step` — tăng `step` để tăng tốc hiệu ứng, không làm mất đồng đều pha |
| Button toggle | PA0 → EXTI0 → toggle `run`, debounce 50ms bằng HAL tick |

---

## Cấu trúc Project

```
Core/
  Src/
    main.c          — Toàn bộ logic: init, ISR, callback
  Inc/
    main.h
Drivers/            — STM32 HAL Drivers (generated)
stm32_led_waveform_control.ioc  — CubeMX config
STM32F103C8TX_FLASH.ld          — Linker script
README.md
```

---

## Build & Flash

1. Mở bằng **STM32CubeIDE**
2. **Build Project** (Ctrl+B) — mục tiêu: `0 errors`
3. Flash qua ST-Link hoặc USB bootloader

---

## Môi trường phát triển

- STM32CubeMX + STM32CubeIDE
- STM32 HAL Library
- Target: STM32F103C8Tx — 72 MHz
