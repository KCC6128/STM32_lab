# UART Polling

## Overview

This lab uses UART polling to receive data from a computer through UART5.

When STM32 receives the string `MCU`, it sends back `Hello!` through UART.

## Demo

This demo shows:

- UART wiring
- Project build and download
- Sending `MCU` from serial terminal
- STM32 replying `Hello!`
- Difference between `abcMCU` and `helloMCU`

[Watch the demo video](https://youtube.com/shorts/mh9_Z9emKg4?feature=share)

<a href="https://youtube.com/shorts/mh9_Z9emKg4?feature=share">
  <img src="UART Polling.png" width="300">
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
- UART Polling
- `HAL_UART_Receive()`
- `HAL_UART_Transmit()`
- `strcmp()`
- Simple delay using `HAL_Delay()`

## Behavior

```text
Input  "MCU"  -> STM32 replies "Hello!"
Other input   -> No response
```

## Core Logic

```c
#include "string.h"

unsigned char uRx_Data[4] = {0};
uint8_t Test[] = "Hello!\r\n";

while (1)
{
    if (HAL_UART_Receive(&huart5, uRx_Data, 3, 100) == HAL_OK)
    {
        if (strcmp((char*)uRx_Data, "MCU") == 0)
        {
            HAL_UART_Transmit(&huart5, Test, sizeof(Test), 10);
            HAL_Delay(1000);
        }
    }
}
```

## Explanation

`HAL_UART_Receive(&huart5, uRx_Data, 3, 100)` receives 3 bytes from UART5 and stores them in `uRx_Data`.

Since the expected command is `MCU`, the receive size is set to 3 bytes.

`uRx_Data` is declared with 4 bytes because C strings need a null terminator `'\0'` at the end.

```text
'M' 'C' 'U' '\0'
```

After receiving data, `strcmp()` is used to compare the received string with `"MCU"`.

If the received string is `"MCU"`, STM32 sends back:

```text
Hello!
```

## Note

This polling method continuously checks whether UART data has been received inside `while(1)`.

Polling is simple and easy to understand, but the program may wait inside `HAL_UART_Receive()` until data is received or timeout occurs.
