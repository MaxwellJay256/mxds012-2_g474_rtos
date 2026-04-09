#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "Types/UsbComTypes.h"
#include "main.h"
#include "usb_com.h"

static uint8_t USB_Pack_Frame(const UsbFrame *frame, UsbPacket **out_packet)
{
  uint8_t *p = NULL;
  uint16_t checksum = 0U;
  uint16_t total_len = 0U;
  UsbPacket *packet = NULL;

  if (frame == NULL || out_packet == NULL) {
    return USBD_FAIL;
  }
  if ((frame->payload_len > 0U) && (frame->payload == NULL)) {
    return USBD_FAIL;
  }

  total_len = (uint16_t)(2U + 2U + 1U + frame->payload_len + 2U + 2U);

  packet = (UsbPacket *)pvPortMalloc(sizeof(UsbPacket));
  if (packet == NULL) {
    return USBD_FAIL;
  }

  packet->buf = (uint8_t *)pvPortMalloc(total_len);
  if (packet->buf == NULL) {
    vPortFree(packet);
    return USBD_FAIL;
  }

  packet->len = total_len;
  p = packet->buf;

  *p++ = USB_PKT_HEADER_HIGH;
  *p++ = USB_PKT_HEADER_LOW;
  *p++ = (uint8_t)(frame->payload_len & 0xFFU);
  *p++ = (uint8_t)((frame->payload_len >> 8) & 0xFFU);
  *p++ = frame->type;

  if (frame->payload_len > 0U) {
    memcpy(p, frame->payload, frame->payload_len);
    p += frame->payload_len;
  }

  for (uint16_t i = 2U; i < (uint16_t)(5U + frame->payload_len); ++i) {
    checksum ^= packet->buf[i];
  }
  *p++ = (uint8_t)(checksum & 0xFFU);
  *p++ = (uint8_t)((checksum >> 8) & 0xFFU);

  *p++ = USB_PKT_TAIL_HIGH;
  *p++ = USB_PKT_TAIL_LOW;

  *out_packet = packet;
  return USBD_OK;
}

// USB 发送任务：从 UsbTxQueue 取包，分片发送到 CDC
void StartUSBTXTask(void* argument)
{
  UsbFrame *frame = NULL;
  UsbPacket *packet = NULL;
  const uint16_t chunk_size = 512; // 分片发送，避免一次过大阻塞
  // #define ENABLE_ADCSYNC_USB_TX_DEBUG

  for (;;) {
    if (osMessageQueueGet(UsbTxQueueHandle, &frame, 0, osWaitForever) == osOK && frame) {
      if (USB_Pack_Frame(frame, &packet) == USBD_OK && packet != NULL) {
        uint16_t remaining = packet->len; // 待发送数据长度
        uint8_t *p = packet->buf;

        while (remaining > 0) {
          uint16_t send_len = (remaining > chunk_size) ? chunk_size : remaining; // 最多发送 chunk_size 字节
          // 调试：打印发送块首尾 6 字节（十六进制）
          #ifdef ENABLE_ADCSYNC_USB_TX_DEBUG
          {
            uint16_t head_print = (send_len < 6) ? send_len : 6;
            printf("[usbtx] send_len=%u head:", send_len);
            for (uint16_t i = 0; i < head_print; ++i) printf(" %02X", p[i]);
            if (send_len > 6) {
              printf(" ... tail:");
              for (uint16_t i = send_len - 6; i < send_len; ++i) printf(" %02X", p[i]);
            }
            printf("\n");
          }
          #endif
          // 使用信号量同步的阻塞发送，超时可按需调整
          if (CDC_Transmit_FS_Blocking(p, send_len, 200) != USBD_OK) {
            break;
          }
          p += send_len;
          remaining = (uint16_t)(remaining - send_len);
        }

        if (packet->buf) {
          vPortFree(packet->buf);
        }
        vPortFree(packet);
      }

      if (frame->payload) {
        vPortFree(frame->payload);
      }
      vPortFree(frame);

      packet = NULL;
      frame = NULL;
    }
  }
}