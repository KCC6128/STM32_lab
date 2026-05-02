# GPIO Interrupt LED

## Overview

This lab uses GPIO external interrupt to detect the PA0 button event and control LEDs connected to PD12, PD13, PD14, and PD15.

Unlike polling, this lab does not continuously check the button state in `while(1)`.  
Instead, the button event triggers an EXTI interrupt, and the LED control logic is handled in `HAL_GPIO_EXTI_Callback()`.

## Demo

<img src="demo.gif" width="300"/>

## Board / Tool

- STM32F407G-DISC1
- MCU: STM32F407VGT6U
- IDE: Keil uVision (MDK-ARM)
- Tool: STM32CubeMX

## GPIO Pins

| Pin | Function |
|---|---|
| PA0 | Button external interrupt input |
| PD12 | LED |
| PD13 | LED |
| PD14 | LED |
| PD15 | LED |

## Main Concepts

- GPIO Input / Output
- External interrupt
- EXTI
- `HAL_GPIO_EXTI_Callback()`
- `HAL_GPIO_WritePin()`
- Software debounce using `HAL_GetTick()`

## Behavior

```text
1st valid press -> PD12 and PD14 ON
2nd valid press -> PD13 and PD15 ON
3rd valid press -> all LEDs OFF and restart
```

## Core Logic

```c
volatile int BTN_PRESSED_COUNT = 0;
uint16_t PIN_NUM1 = 0;
uint16_t PIN_NUM2 = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_tick = 0;
    uint32_t now_tick = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_0)
    {
        if (now_tick - last_tick < 200)
        {
            return;
        }
        last_tick = now_tick;

        if (BTN_PRESSED_COUNT < 2)
        {
            PIN_NUM1 = (GPIO_PIN_12 << BTN_PRESSED_COUNT);
            PIN_NUM2 = (GPIO_PIN_14 << BTN_PRESSED_COUNT);

            HAL_GPIO_WritePin(GPIOD, PIN_NUM1 | PIN_NUM2, GPIO_PIN_SET);
            BTN_PRESSED_COUNT++;
        }
        else
        {
            BTN_PRESSED_COUNT = 0;
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
        }
    }
}
```
