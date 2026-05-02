# GPIO Polling LED Lab

## Board

- STM32F407G-DISC1
- MCU: STM32F407VGT6U

## Goal

Use polling to read the PA0 button state and sequentially turn on LEDs connected to PD12, PD13, PD14, and PD15.

## GPIO Pins

| Pin | Function |
|---|---|
| PA0 | Button input |
| PD12 | LED |
| PD13 | LED |
| PD14 | LED |
| PD15 | LED |

## Main Concepts

- GPIO Input
- GPIO Output
- Polling
- `HAL_GPIO_ReadPin()`
- `HAL_GPIO_WritePin()`
- Simple delay using `HAL_Delay()`

## Behavior

Each button press turns on the next LED:

```text
1st press -> PD12 ON
2nd press -> PD13 ON
3rd press -> PD14 ON
4th press -> PD15 ON
5th press -> all LEDs OFF and restart
