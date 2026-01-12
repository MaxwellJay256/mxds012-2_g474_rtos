#ifndef USB_TX_H
#define USB_TX_H

#include <stdint.h>
#include <stdbool.h>
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 基于信号量的阻塞式 USB CDC 发送
 * @param buf 数据缓冲区指针
 * @param len 数据长度（字节）
 * @param timeout_ms 超时时间（毫秒）
 * @return USBD_OK(0) 成功，USBD_BUSY(1) 超时，USBD_FAIL(2) 失败
 * 
 * @note 使用 USBTXCpltSemHandle 信号量同步，由 CDC_TransmitCplt_FS 在 ISR 中释放
 * @note 相比轮询 TxState 的方式，此函数更节省 CPU 资源
 */
uint8_t CDC_Transmit_FS_Blocking(uint8_t *buf, uint16_t len, uint32_t timeout_ms);
/*
 * 发送 5 路 ADC 结果到上位机（USB CDC）。
 * - results: 指向 5 个通道数据首地址的指针数组，顺序为 ch0..ch4
 * - length:  每个通道的样本数（必须一致）
 * - batch_lines: 批量发送的行数（例如 8、16）。函数会将多行拼接到缓冲后一次发送，最后剩余行也会发送，并在帧末追加一个空行。
 *
 * 数据格式（整帧一行，按通道分段）：
 *   ch0[0] ch0[1] ... ch0[length-1],ch1[0] ch1[1] ... ch1[length-1],...ch4[length-1]\n
 *
 * 返回：true 表示发送成功，false 表示超时或错误。
 *
 * 注意：此函数可能阻塞等待 USB 发送完成，不建议在高优先级中断中直接调用。
 */
// bool USB_ADC_Results_Transmit5(const int16_t* const results[5], uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* USB_TX_H */
