#include "main.h"
#include "FreeRTOS.h"
#include "sys_config.h"
#include "Types/UsbComTypes.h"
#include "stm32g4xx_hal_def.h"
#include <stdint.h>


SysConfig sysConfig; // 全局系统配置

void SysConfig_Init(SysConfig *config) {
  // config->scan_frequency_hz = CONFIG_SCAN_FREQ_MIN_HZ;
  // config->sample_depth = CONFIG_SAMPLE_DEPTH_MIN;
  config->scan_frequency_hz = CONFIG_SCAN_FREQ_DEFAULT_HZ;
  config->sample_depth = CONFIG_SAMPLE_DEPTH_DEFAULT;
}

uint8_t SysConfig_USB_NotifyConfig(SysConfig *config) {
  const uint16_t payload_len = 3;
  UsbFrame *frame = NULL;
  uint8_t *payload = NULL;

  if (UsbTxQueueHandle == NULL) {
    return 1;
  }

  frame = (UsbFrame *)pvPortMalloc(sizeof(UsbFrame));
  if (frame == NULL) {
    return 1;
  }
  payload = (uint8_t *)pvPortMalloc(payload_len);
  if (payload == NULL) {
    vPortFree(frame);
    return 1;
  }

  payload[0] = config->scan_frequency_hz;
  payload[1] = (uint8_t)(config->sample_depth & 0xFFU); // sample_depth 低 8 位
  payload[2] = (uint8_t)((config->sample_depth >> 8) & 0xFFU); // sample_depth 高 8 位
  frame->type = USB_PKT_TYPE_NOTIFY_CONFIG;
  frame->payload_len = payload_len;
  frame->payload = payload;

  UsbFrame *to_send = frame;
  osStatus_t qres = osMessageQueuePut(UsbTxQueueHandle, &to_send, 0, 0);
  if (qres != osOK) {
    vPortFree(payload);
    vPortFree(frame);
    return 1;
  } else {
    return 0;
  }
}

uint8_t SysConfig_SetScanFrequency(SysConfig *config, uint8_t scan_freq_hz)
{
  if (scan_freq_hz < CONFIG_SCAN_FREQ_MIN_HZ || scan_freq_hz > CONFIG_SCAN_FREQ_MAX_HZ) {
    return 1;
  }
  config->scan_frequency_hz = scan_freq_hz;
  return SysConfig_USB_NotifyConfig(config);
}

uint8_t SysConfig_SetSampleDepth(SysConfig *config, uint16_t sample_depth)
{
  if (sample_depth < CONFIG_SAMPLE_DEPTH_MIN || sample_depth > CONFIG_SAMPLE_DEPTH_MAX) {
    return 1;
  }
  config->sample_depth = sample_depth;
  return SysConfig_USB_NotifyConfig(config);
}

uint8_t SysConfig_SetConfig(SysConfig *config, uint8_t scan_freq_hz, uint16_t sample_depth)
{
  if (scan_freq_hz < CONFIG_SCAN_FREQ_MIN_HZ || scan_freq_hz > CONFIG_SCAN_FREQ_MAX_HZ) {
    return 1;
  }
  if (sample_depth < CONFIG_SAMPLE_DEPTH_MIN || sample_depth > CONFIG_SAMPLE_DEPTH_MAX) {
    return 1;
  }
  config->scan_frequency_hz = scan_freq_hz;
  config->sample_depth = sample_depth;
  return SysConfig_USB_NotifyConfig(config);
}