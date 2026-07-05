# STM32F401 CAN / Temperature / FND Project

STM32F401RE(T6) firmware project built with STM32CubeIDE. It drives an MCP2515
CAN controller over SPI, reads a DS18B20 OneWire temperature sensor, and
displays data on a TM1637 4-digit 7-segment module.

## Hardware

- **MCU**: STM32F401RET6 (LQFP64)
- **CAN controller**: MCP2515 (via SPI1)
- **Temperature sensor**: DS18B20 (OneWire)
- **Display**: TM1637 4-digit 7-segment

## Pin Mapping

| Pin  | Function        | Label          |
|------|-----------------|----------------|
| PA4  | SPI1_NSS        | SPI1_NSS_MCP   |
| PA5  | GPIO Output     | GPIO_LED1      |
| PA6  | SPI1_MISO       | SPI1_MISO_MCP  |
| PA7  | SPI1_MOSI       | SPI1_MOSI_MCP  |
| PB0  | EXTI (falling)  | EXTI_MCP       |
| PB3  | SPI1_SCK        | SPI1_SCK_MCP   |
| PB9  | GPIO Output     | TEMP_DATA      |
| PC10 | GPIO Output OD  | FND_CLK        |
| PC12 | GPIO Output OD  | FND_DIO        |

SPI1 is master mode, 8-bit data, CPOL low / CPHA 1st edge, software NSS, used
exclusively to talk to the MCP2515. TIM2 is configured as a free-running
16-bit-period internal timer. EXTI0 handles the MCP2515 interrupt line
(`PB0`).

## Project Layout

```
first2/
├── Core/
│   ├── Inc/               HAL/app headers (main.h, stm32f4xx_hal_conf.h, ...)
│   ├── Src/                main.c and generated HAL init code
│   ├── Lib/
│   │   ├── Inc/, Src/      Custom drivers: MCP2515, CANSPI, ds18b20, onewire, tm1637
│   └── Startup/            Startup assembly (startup_stm32f401retx.s)
├── Drivers/                 STM32 HAL + CMSIS (ST-provided)
├── Debug/                   STM32CubeIDE Debug build output/makefiles
├── first2.ioc                STM32CubeMX device configuration
└── STM32F401RETX_FLASH.ld / _RAM.ld   Linker scripts
```

## Custom Libraries (`Core/Lib`)

- **MCP2515** – low-level SPI driver for the Microchip MCP2515 CAN
  controller: register read/write, RX/TX buffer access, mode switching, plus
  helpers `SendStandardCanMessage()` and `TryReadCanMessage()` for standard
  (11-bit ID) CAN frames.
- **CANSPI** – SPI transport helper layer used by the MCP2515 driver.
- **onewire / ds18b20** – Dallas OneWire bus bit-banging and DS18B20
  temperature conversion/read routines.
- **tm1637** – bit-banged driver for the TM1637 LED segment display
  controller (clock/data pins on GPIOC).

## Application Flow (`Core/Src/main.c`)

1. HAL init, system clock config, GPIO/SPI1 peripheral init.
2. `MCP2515_InitFull()` initializes the CAN controller; on failure, LED1
   (PA5) blinks 5 times as an error indicator.
3. MCP2515 is set to loopback mode for self-test.
4. Main loop: send a standard CAN frame (ID `0x123`, payload `"TEST"`) every
   second and attempt to read it back via `TryReadCanMessage()`.

## Building

This is an STM32CubeIDE project.

1. Open STM32CubeIDE and `File > Open Projects from File System...`, select
   the `first2` folder.
2. Build the `Debug` configuration (or `Project > Build Project`).
3. Flash/debug via ST-Link using the included `first2 Debug.launch`
   configuration.

The `.ioc` file can be reopened in STM32CubeMX to regenerate peripheral
init code if the pin configuration changes.
