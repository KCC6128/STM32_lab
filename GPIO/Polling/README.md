# GPIO Polling LED

## Overview

This lab uses polling to read the PA0 button state and sequentially turn on the LEDs connected to PD12, PD13, PD14, and PD15.

## Board

- STM32F407G-DISC1
- MCU: STM32F407VGT6U
- IDE: Keil uVision (MDK-ARM)
- Tool: STM32CubeMX

## GPIO Pins

| Pin | Function |
|---|---|
| PA0 | Button input |
| PD12 | LED |
| PD13 | LED |
| PD14 | LED |
| PD15 | LED |

## Main Concepts

- GPIO Input / Output
- Polling
- `HAL_GPIO_ReadPin()`
- `HAL_GPIO_WritePin()`
- Simple delay using `HAL_Delay()`

## Behavior

```text
1st press -> PD12 ON
2nd press -> PD13 ON
3rd press -> PD14 ON
4th press -> PD15 ON
5th press -> all LEDs OFF and restart
```

## Core Logic

```c
if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
{
    if (BTN_PRESSED_COUNT < 4)
    {
        PIN_NUM = (GPIO_PIN_12 << BTN_PRESSED_COUNT);
        HAL_GPIO_WritePin(GPIOD, PIN_NUM, GPIO_PIN_SET);
        BTN_PRESSED_COUNT++;
    }
    else
    {
        BTN_PRESSED_COUNT = 0;
        HAL_GPIO_WritePin(GPIOD,
                          GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
                          GPIO_PIN_RESET);
    }

    HAL_Delay(250);
}
```

## Note

Detailed GPIO concepts and notes are documented in HackMD.
This repository mainly stores STM32 practice projects and source code.

