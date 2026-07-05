# STM32F401 CAN / 온도 / FND 프로젝트

STM32CubeIDE로 작성된 STM32F401RE(T6) 펌웨어 프로젝트입니다. SPI로 연결된
MCP2515 CAN 컨트롤러를 구동하고, DS18B20 OneWire 온도 센서 값을 읽어와
TM1637 4자리 7-세그먼트 모듈에 표시합니다.

## 하드웨어 구성

- **MCU**: STM32F401RET6 (LQFP64)
- **CAN 컨트롤러**: MCP2515 (SPI1 사용)
- **온도 센서**: DS18B20 (OneWire)
- **디스플레이**: TM1637 4자리 7-세그먼트

## 핀 매핑

| 핀   | 기능             | 라벨           |
|------|-----------------|----------------|
| PA4  | SPI1_NSS        | SPI1_NSS_MCP   |
| PA5  | GPIO 출력        | GPIO_LED1      |
| PA6  | SPI1_MISO       | SPI1_MISO_MCP  |
| PA7  | SPI1_MOSI       | SPI1_MOSI_MCP  |
| PB0  | EXTI (하강 엣지) | EXTI_MCP       |
| PB3  | SPI1_SCK        | SPI1_SCK_MCP   |
| PB9  | GPIO 출력        | TEMP_DATA      |
| PC10 | GPIO 출력 (OD)   | FND_CLK        |
| PC12 | GPIO 출력 (OD)   | FND_DIO        |

SPI1은 마스터 모드, 8비트 데이터, CPOL Low / CPHA 1st Edge, 소프트웨어 NSS로
설정되어 있으며 MCP2515와의 통신 전용으로 사용됩니다. TIM2는 16비트 자유
카운팅(내부 클럭 소스) 타이머로 구성되어 있습니다. EXTI0는 MCP2515의 인터럽트
라인(`PB0`)을 처리합니다.

## 프로젝트 구조

```
first2/
├── Core/
│   ├── Inc/               HAL/앱 헤더 (main.h, stm32f4xx_hal_conf.h 등)
│   ├── Src/                main.c 및 자동 생성된 HAL 초기화 코드
│   ├── Lib/
│   │   ├── Inc/, Src/      커스텀 드라이버: MCP2515, CANSPI, ds18b20, onewire, tm1637
│   └── Startup/            시작 어셈블리 코드 (startup_stm32f401retx.s)
├── Drivers/                 STM32 HAL + CMSIS (ST 제공)
├── Debug/                   STM32CubeIDE Debug 빌드 결과물/makefile
├── first2.ioc                STM32CubeMX 디바이스 설정 파일
└── STM32F401RETX_FLASH.ld / _RAM.ld   링커 스크립트
```

## 커스텀 라이브러리 (`Core/Lib`)

- **MCP2515** – Microchip MCP2515 CAN 컨트롤러용 저수준 SPI 드라이버.
  레지스터 읽기/쓰기, RX/TX 버퍼 접근, 모드 전환 기능을 제공하며,
  표준(11비트 ID) CAN 프레임 송수신을 위한 `SendStandardCanMessage()`,
  `TryReadCanMessage()` 헬퍼 함수를 포함합니다.
- **CANSPI** – MCP2515 드라이버가 사용하는 SPI 전송 헬퍼 계층.
- **onewire / ds18b20** – Dallas OneWire 버스 비트뱅잉 및 DS18B20 온도 변환/읽기
  루틴.
- **tm1637** – TM1637 LED 세그먼트 디스플레이 컨트롤러용 비트뱅잉 드라이버
  (GPIOC의 클럭/데이터 핀 사용).

## 애플리케이션 동작 흐름 (`Core/Src/main.c`)

1. HAL 초기화, 시스템 클럭 설정, GPIO/SPI1 주변장치 초기화.
2. `MCP2515_InitFull()`로 CAN 컨트롤러를 초기화하며, 실패 시 LED1(PA5)이
   5회 깜빡이며 오류를 표시합니다.
3. 자체 테스트를 위해 MCP2515를 루프백 모드로 설정합니다.
4. 메인 루프: 1초마다 표준 CAN 프레임(ID `0x123`, 페이로드 `"TEST"`)을
   송신하고, `TryReadCanMessage()`로 수신을 시도합니다.

## 빌드 방법

이 프로젝트는 STM32CubeIDE 프로젝트입니다.

1. STM32CubeIDE에서 `File > Open Projects from File System...`를 선택하고
   `first2` 폴더를 엽니다.
2. `Debug` 구성으로 빌드합니다 (또는 `Project > Build Project`).
3. 포함된 `first2 Debug.launch` 설정을 사용해 ST-Link로 플래시/디버그합니다.

핀 구성을 변경해야 할 경우, `.ioc` 파일을 STM32CubeMX에서 다시 열어 주변장치
초기화 코드를 재생성할 수 있습니다.
