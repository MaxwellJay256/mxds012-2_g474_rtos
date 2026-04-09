#include "FreeRTOS.h"
#include "usb_com.h"
#include <stdio.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

/**
 * @brief 基于信号量的阻塞式 USB CDC 发送
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @param timeout_ms 超时时间（毫秒）
 * @return USBD_OK 成功，USBD_BUSY 超时，USBD_FAIL 失败
 * 
 * @note 使用 USBTXCpltSemHandle 信号量同步，避免轮询 TxState
 */
uint8_t CDC_Transmit_FS_Blocking(uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
  if (USBTXCpltSemHandle == NULL) {
    return USBD_FAIL;
  }
  
  uint32_t start = HAL_GetTick();
  uint8_t res;

  // 阶段 1：重试提交直到成功
  do {
    res = CDC_Transmit_FS(buf, len);
    if (res == USBD_OK) break;
    if ((HAL_GetTick() - start) >= timeout_ms) {
      return res; // 可能是 USBD_BUSY
    }
    osDelay(1);
  } while (1);

  // 阶段 2：等待 ISR 释放信号量（表示传输完成）
  uint32_t remaining = timeout_ms - (HAL_GetTick() - start);
  if (remaining == 0) remaining = 1;
  
  osStatus_t sem_res = osSemaphoreAcquire(USBTXCpltSemHandle, remaining);
  if (sem_res != osOK) {
    return USBD_BUSY; // 超时
  }

  return USBD_OK;
}
