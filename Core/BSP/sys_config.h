#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_SCAN_FREQ_MIN_HZ 1
#define CONFIG_SCAN_FREQ_DEFAULT_HZ 30
#define CONFIG_SCAN_FREQ_MAX_HZ 60

#define CONFIG_SAMPLE_DEPTH_MIN 1
#define CONFIG_SAMPLE_DEPTH_DEFAULT 512
#define CONFIG_SAMPLE_DEPTH_MAX 1024

typedef struct SysConfig_t {
  uint8_t scan_frequency_hz; // 超声扫描频率（Hz）
  uint16_t sample_depth;     // 采样深度（点数）
} SysConfig;

void SysConfig_Init(SysConfig *config);
uint8_t SysConfig_USB_NotifyConfig(SysConfig *config);
uint8_t SysConfig_SetScanFrequency(SysConfig *config, uint8_t scan_freq_hz);
uint8_t SysConfig_SetSampleDepth(SysConfig *config, uint16_t sample_depth);
uint8_t SysConfig_SetConfig(SysConfig *config, uint8_t scan_freq_hz, uint16_t sample_depth);

#ifdef __cplusplus
}
#endif

#endif /* SYS_CONFIG_H */
