# UART Interrupt

## Overview

This lab uses UART interrupt to receive data from a computer through UART5.

When STM32 receives the string `KCC`, it sends back `Hello KCC YOUR STM32 UART5 RX ISR!` through UART.

Unlike polling, the program does not continuously wait inside `while(1)`.  
Instead, UART interrupt is triggered after the expected data length is received.

## Demo

This demo shows:

- UART wiring
- Project build and download
- Starting UART interrupt reception
- Sending `KCC` from serial terminal
- STM32 replying `Hello KCC YOUR STM32 UART5 RX ISR!`
- Sending `K`, `C`, `C` separately to show that UART interrupt waits until 3 bytes are received
- Difference between `abcKCC` and `helloKCC`

[Watch the demo video](https://youtu.be/iP01HQailqY)

<a href="https://youtu.be/iP01HQailqY">
  <img src="UART Interrupt.png" width="300">
</a>

## Board / Tool

- STM32F407G-DISC1
- MCU: STM32F407VGT6U
- IDE: Keil uVision (MDK-ARM)
- Tool: STM32CubeMX
- Debugger / Programmer: ST-LINK
- UART Tool: USB to UART module / Serial terminal

## UART Pins

| Pin | Function |
|---|---|
| PC12 | UART5_TX |
| PD2 | UART5_RX |
| GND | Common ground |

## UART Setting

| Item | Setting |
|---|---|
| UART | UART5 |
| Baud rate | 115200 |
| Word Length | 8 Bits |
| Parity | None |
| Stop Bits | 1 |
| Mode | TX / RX |
| Interrupt | Enabled |

This setting is commonly written as:

```text
115200, 8N1
```

## Wiring

| STM32F407G-DISC1 | USB to UART Module |
|---|---|
| PC12 / UART5_TX | RX |
| PD2 / UART5_RX | TX |
| GND | GND |

TX and RX need to be crossed because STM32 TX sends data to the receiver side of the USB to UART module.

## Main Concepts

- UART communication
- UART Interrupt
- `HAL_UART_Receive_IT()`
- `HAL_UART_RxCpltCallback()`
- `HAL_UART_Transmit()`
- `strcmp()`

## Behavior

```text
Program start -> STM32 sends "HAL_UART_Receive_IT has been called!"
Input "KCC"   -> STM32 replies "Hello KCC YOUR STM32 UART5 RX ISR!"
Other input   -> No response
```

If `KCC` is sent repeatedly, STM32 replies every time.

## Core Logic

```c
#include "string.h"

uint8_t uRx_Data[4] = {0};
uint8_t Test[] = "Hello KCC YOUR STM32 UART5 RX ISR!\r\n";
```

```c
if (HAL_UART_Receive_IT(&huart5, uRx_Data, 3) == HAL_OK)
{
    uint8_t msg[] = "HAL_UART_Receive_IT has been called!\r\n";
    HAL_UART_Transmit(&huart5, msg, sizeof(msg), 10);
}
```

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart5)
    {
        uRx_Data[3] = '\0';

        if(strcmp((char*)uRx_Data, "KCC") == 0)
        {
            HAL_UART_Transmit(&huart5, Test, sizeof(Test), 10);
        }

        HAL_UART_Receive_IT(&huart5, uRx_Data, 3);
    }
}
```

## Explanation

`HAL_UART_Receive_IT(&huart5, uRx_Data, 3)` starts UART5 interrupt reception.

It receives 3 bytes and stores them in `uRx_Data`.

Since the expected command is `KCC`, the receive size is set to 3 bytes.

`uRx_Data` is declared with 4 bytes because C strings need a null terminator `'\0'` at the end.

```text
'K' 'C' 'C' '\0'
```

When 3 bytes are received, `HAL_UART_RxCpltCallback()` is called automatically.

Inside the callback, `strcmp()` is used to compare the received string with `"KCC"`.

If the received string is `"KCC"`, STM32 sends back:

```text
Hello KCC YOUR STM32 UART5 RX ISR!
```

Finally, `HAL_UART_Receive_IT()` is called again inside the callback so that UART can continue receiving the next data.

## Note

`HAL_UART_Receive_IT()` only starts one interrupt reception task.

After the expected number of bytes is received, the callback is triggered and the reception task is finished.

To continue receiving data, `HAL_UART_Receive_IT()` must be called again inside the callback.

This interrupt method allows UART to notify the program when data is received, instead of continuously checking UART inside `while(1)`.
