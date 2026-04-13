/**
 * @file UsbComTypes.h
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.1
 * @date 2026-04-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef USB_TYPES_H
#define USB_TYPES_H

#define USB_PKT_HEADER_HIGH 0xAA // USB 帧头高字节
#define USB_PKT_HEADER_LOW 0x55 // USB 帧头低字节
#define USB_PKT_TAIL_HIGH 0x0D // USB 帧尾高字节
#define USB_PKT_TAIL_LOW 0x0A // USB 帧尾低字节

/* 上行数据包类型 */
#define USB_PKT_TYPE_ADC 0x01 // ADC 数据包类型
#define USB_PKT_TYPE_POWER 0x02 // 电源数据包类型
#define USB_PKT_TYPE_NOTIFY_CONFIG 0x04 // 通知上位机当前配置（扫描频率、采样深度）

/* 下行数据包类型 */
#define USB_PKT_TYPE_SET_CONFIG 0x10 // 设置系统参数（扫描频率、采样深度等）
#define USB_PKT_TYPE_ACQ_CONFIG 0x11 // 请求当前系统参数

// 完整帧格式：AA55 | lenL lenH | type | payload... | checksumL checksumH | 0D0A
// 队列约定：生产者入队 UsbFrame*；USBTXTask 负责打包发送并释放 frame->payload 与 frame 本体。
typedef struct UsbFrame_t {
  uint8_t type;            // 数据类型（e.g., ADC 数据、控制命令等）
  uint16_t payload_len;    // 有效载荷长度（字节数）
  uint8_t *payload;        // 生产者动态分配的载荷指针，入队后由 USBTXTask 释放
} UsbFrame;

typedef struct UsbPacket_t {
  uint16_t len; // 完整包长度（含头尾）
  uint8_t *buf; // 动态分配的包缓冲
} UsbPacket;

#endif /* USB_TYPES_H */