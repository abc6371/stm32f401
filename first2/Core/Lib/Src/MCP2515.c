/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "MCP2515.h"
//#include "gpio.h"

/* SPI related variables */
extern SPI_HandleTypeDef        hspi1;
#define SPI_CAN                 &hspi1
#define SPI_TIMEOUT             10
#define MCP2515_CS_HIGH()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define MCP2515_CS_LOW()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)

/* Prototypes */
static void SPI_Tx(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length);
static uint8_t SPI_Rx(void);
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length);

/* initialize MCP2515 */
bool MCP2515_Initialize(void)
{
  MCP2515_CS_HIGH();    
  
  uint8_t loop = 10;
  
  do {
    /* check SPI Ready */
    if(HAL_SPI_GetState(SPI_CAN) == HAL_SPI_STATE_READY)
      return true;
    
    loop--;
  } while(loop > 0); 
      
  return false;
}

/* change mode as configuration mode */
bool MCP2515_SetConfigMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x80);
  
  uint8_t loop = 10;
  
  do {    
    /* confirm mode configuration */
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x80)
      return true;
    
    loop--;
  } while(loop > 0); 
  
  return false;
}

/* change mode as normal mode */
bool MCP2515_SetNormalMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x00);
  
  uint8_t loop = 10;
  
  do {    
    /* confirm mode configuration */
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x00)
      return true;
    
    loop--;
  } while(loop > 0);
  
  return false;
}

/* Entering sleep mode */
bool MCP2515_SetSleepMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x20);
  
  uint8_t loop = 10;
  
  do {    
    /* confirm mode configuration */
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x20)
      return true;
    
    loop--;
  } while(loop > 0);
  
  return false;
}

/* MCP2515 SPI-Reset */
void MCP2515_Reset(void)
{    
  MCP2515_CS_LOW();
      
  SPI_Tx(MCP2515_RESET);
      
  MCP2515_CS_HIGH();
}

/* read single byte */
uint8_t MCP2515_ReadByte (uint8_t address)
{
  uint8_t retVal;
  
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_READ);
  SPI_Tx(address);
  retVal = SPI_Rx();
      
  MCP2515_CS_HIGH();
  
  return retVal;
}

/* read buffer */
void MCP2515_ReadRxSequence(uint8_t instruction, uint8_t *data, uint8_t length)
{
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);        
  SPI_RxBuffer(data, length);
    
  MCP2515_CS_HIGH();
}

/* write single byte */
void MCP2515_WriteByte(uint8_t address, uint8_t data)
{    
  MCP2515_CS_LOW();  
  
  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(address);
  SPI_Tx(data);  
    
  MCP2515_CS_HIGH();
}

/* write buffer */
void MCP2515_WriteByteSequence(uint8_t startAddress, uint8_t endAddress, uint8_t *data)
{    
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(startAddress);
  SPI_TxBuffer(data, (endAddress - startAddress + 1));
  
  MCP2515_CS_HIGH();
}

/* write to TxBuffer */
void MCP2515_LoadTxSequence(uint8_t instruction, uint8_t *idReg, uint8_t dlc, uint8_t *data)
{    
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);
  SPI_TxBuffer(idReg, 4);
  SPI_Tx(dlc);
  SPI_TxBuffer(data, dlc);
       
  MCP2515_CS_HIGH();
}

/* write to TxBuffer(1 byte) */
void MCP2515_LoadTxBuffer(uint8_t instruction, uint8_t data)
{
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);
  SPI_Tx(data);
        
  MCP2515_CS_HIGH();
}

/* request to send */
void MCP2515_RequestToSend(uint8_t instruction)
{
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);
      
  MCP2515_CS_HIGH();
}

/* read status */
uint8_t MCP2515_ReadStatus(void)
{
  uint8_t retVal;
  
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_READ_STATUS);
  retVal = SPI_Rx();
        
  MCP2515_CS_HIGH();
  
  return retVal;
}

/* read RX STATUS register */
uint8_t MCP2515_GetRxStatus(void)
{
  uint8_t retVal;
  
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_RX_STATUS);
  retVal = SPI_Rx();
        
  MCP2515_CS_HIGH();
  
  return retVal;
}

/* Use when changing register value */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{    
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_BIT_MOD);
  SPI_Tx(address);
  SPI_Tx(mask);
  SPI_Tx(data);
        
  MCP2515_CS_HIGH();
}

/* SPI Tx wrapper function  */
static void SPI_Tx(uint8_t data)
{
  HAL_SPI_Transmit(SPI_CAN, &data, 1, SPI_TIMEOUT);    
}

/* SPI Tx wrapper function */
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
{
  HAL_SPI_Transmit(SPI_CAN, buffer, length, SPI_TIMEOUT);    
}

/* SPI Rx wrapper function */
static uint8_t SPI_Rx(void)
{
  uint8_t retVal;
  HAL_SPI_Receive(SPI_CAN, &retVal, 1, SPI_TIMEOUT);
  return retVal;
}

/* SPI Rx wrapper function */
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
{
  HAL_SPI_Receive(SPI_CAN, buffer, length, SPI_TIMEOUT);
}


bool MCP2515_InitFull(void)
{
  // 1. 하드웨어 리셋
  MCP2515_Reset();
  HAL_Delay(20);

  // 2. SPI 통신 확인
  if (!MCP2515_Initialize()) {
    return false;
  }

  // 3. 설정 모드 진입
  if (!MCP2515_SetConfigMode()) {
    return false;
  }

  // 4. CAN 비트 타이밍 (500kbps @8MHz 크리스탈 예시)
  MCP2515_WriteByte(MCP2515_CNF1, 0x01);  // SJW=1, BRP=1
  MCP2515_WriteByte(MCP2515_CNF2, 0x90);  // PRSEG=1, PHSEG1=2
  MCP2515_WriteByte(MCP2515_CNF3, 0x02);  // PHSEG2=3

  // 5. RX 버퍼 설정 (모든 메시지 수용)
  MCP2515_WriteByte(MCP2515_RXB0CTRL, 0x64);  // RXM=00, BUKT=1
  MCP2515_WriteByte(MCP2515_RXB1CTRL, 0x60);

  // 6. 인터럽트 설정 (필요 시, RX 인터럽트)
  MCP2515_WriteByte(MCP2515_CANINTE, 0x03);  // RX0IE + RX1IE

  // 7. Normal 모드 진입
  if (!MCP2515_SetNormalMode()) {
    return false;
  }

  return true;
}

////////////////
// 표준 CAN 메시지 송신 함수
bool SendStandardCanMessage(uint16_t can_id, uint8_t len, uint8_t *data)
{
  uint8_t id_reg[4];
  id_reg[0] = (can_id >> 3) & 0xFF;  // SIDH
  id_reg[1] = (can_id << 5) & 0xE0;  // SIDL (EXIDE=0)
  id_reg[2] = 0x00;  // EID8
  id_reg[3] = 0x00;  // EID0

  uint8_t dlc = len & 0x0F;
  if (dlc > 8) dlc = 8;

  // TXB0에 로드
  MCP2515_LoadTxSequence(MCP2515_LOAD_TXB0SIDH, id_reg, dlc, data);  // MCP_LOAD_TXB0_ID는 MCP2515.h에 정의 (0x40)

  // 전송 요청
  MCP2515_RequestToSend(MCP2515_RTS_TX0);

  // 완료 대기 (폴링)
  uint8_t timeout = 100;
  while (timeout--) {
    if (!(MCP2515_ReadStatus() & 0x04)) {  // TX0REQ 비트 확인
      return true;
    }
    HAL_Delay(1);
  }
  return false;
}

// CAN 메시지 수신 함수 (폴링 방식)
bool TryReadCanMessage(uint32_t *id, uint8_t *dlc, uint8_t *data, bool *is_extended)
{
  uint8_t status = MCP2515_ReadStatus();

  if (!(status & 0x03)) return false;  // RX 버퍼 없음

  uint8_t rx_buf = (status & 0x01) ? 0 : 1;  // RXB0 우선
  uint8_t instr = (rx_buf == 0) ? MCP2515_READ_RXB0SIDH : MCP2515_READ_RXB1SIDH;

  uint8_t raw[13];
  MCP2515_ReadRxSequence(instr, raw, 13);

  // ID 추출
  *is_extended = (raw[1] & 0x08) != 0;
  if (*is_extended) {
    *id = ((uint32_t)raw[0] << 21) | ((uint32_t)(raw[1] & 0xE0) << 13) | ((uint32_t)(raw[1] & 0x03) << 16) | ((uint32_t)raw[2] << 8) | raw[3];
  } else {
    *id = ((uint32_t)raw[0] << 3) | (raw[1] >> 5);
  }

  *dlc = raw[4] & 0x0F;
  if (*dlc > 8) *dlc = 8;
  memcpy(data, &raw[5], *dlc);

  // 플래그 클리어
  MCP2515_BitModify(MCP2515_CANINTF, (rx_buf == 0) ? 0x01 : 0x02, 0x00);

  return true;
}
