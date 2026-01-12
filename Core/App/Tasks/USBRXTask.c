#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "main.h"
#include "adc_sync.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"
#include <stdint.h>

void StartUSBRXTask(void* argument)
{
  uint8_t receive;
  uint8_t usbRXData[32];
  uint8_t dataIndex = 0; // 接收数据索引
  uint8_t dataLength = 0; // 实际接收数据长度

  for (;;) {
    osMessageQueueGet(USBRXQueueHandle, &receive, 0, osWaitForever);

    if (dataIndex == 0) { // 如果命令索引为 0，则寻找包头
      if (receive == 0xAA) { // 找到包头
        usbRXData[dataIndex++] = receive; // 存储起始字节
      }
    } else if (dataIndex == 1) { // 如果命令索引为 1，则 receive 为命令长度
      if (receive < 4 || receive > sizeof(usbRXData)) {
        // 无效长度，重置索引，继续寻找包头
        dataIndex = 0;
        continue;
      }
      dataLength = receive;
      usbRXData[dataIndex++] = receive; // 存储长度字节
    } else {
      usbRXData[dataIndex++] = receive;
      if (dataIndex == dataLength) { // 如果索引等于数据长度，则表示接收完成
        uint8_t checksum = 0; // 奇偶校验和
        for (uint8_t i = 0; i < dataLength - 1; i++) {
          checksum += usbRXData[i];
        }
        if (checksum == usbRXData[dataLength - 1]) { // 校验和等于最后一个字节，表示数据有效
          // TODO: 处理接收到的有效数据包
          printf("Received valid USB RX packet: ");
          for (uint8_t i = 0; i < dataLength; i++) {
            printf("%02X ", usbRXData[i]);
          }
          printf("\n");
          
        }
        dataIndex = 0; // 重置索引，准备接收下一个包
        dataLength = 0; // 重置数据长度
      }
    }
  }
}