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


