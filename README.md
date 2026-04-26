# stm32-led-waveform-control
STM32 Embedded Test - 3 Channel LED Waveform Control using Single Timer PWM + Interrupt
# STM32 PWM 3-Channel Waveform Project

## Overview
This project demonstrates 3-channel PWM waveform generation using TIM1 on STM32F103C8T6.

## Features
- 3 PWM channels (TIM1 CH1/CH2/CH3)
- Center-aligned PWM mode
- Sine wave LUT control
- Phase-shifted waveform generation
- Optimized ISR (no floating point, no modulo)

## Hardware
- STM32F103C8T6 (Blue Pill)

## Clock
- HSI 8MHz (or HSE 72MHz optional)

## PWM Settings
- Prescaler: 71
- ARR: 999
- Center-aligned mode

## Algorithm
- Lookup table (LUT) sine wave
- Phase shifted 3-phase output
- Direct CCR register update in interrupt

## How to Build
Open project in STM32CubeIDE and click Build.

## Author
Embedded Engineer Candidate
