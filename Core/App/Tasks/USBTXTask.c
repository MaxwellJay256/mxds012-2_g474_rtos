#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "adc_sync.h"
#include "main.h"
#include "usb_tx.h"

// USB 发送任务：从 ADCQueue 取包，分片发送到 CDC
void StartUSBTXTask(void* argument)
{
  ADCUsbPacket *packet = NULL;
  const uint16_t chunk_size = 256; // 分片发送，避免一次过大阻塞

  for (;;) {
    if (osMessageQueueGet(ADCQueueHandle, &packet, 0, osWaitForever) == osOK && packet) {
      uint16_t remaining = packet->len; // 待发送数据长度
      uint8_t *p = packet->buf;

      while (remaining > 0) {
        uint16_t send_len = (remaining > chunk_size) ? chunk_size : remaining; // 最多发送 chunk_size 字节
        // 使用信号量同步的阻塞发送，超时可按需调整
        CDC_Transmit_FS_Blocking(p, send_len, 200);
        p += send_len;
        remaining = (uint16_t)(remaining - send_len);
      }

      if (packet->buf) {
        vPortFree(packet->buf);
      }
      vPortFree(packet);
    }
  }
}