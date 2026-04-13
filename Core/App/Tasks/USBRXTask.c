#include "cmsis_os2.h"
#include "main.h"
#include "Types/UsbComTypes.h"
#include "sys_config.h"
#include <stdint.h>
#include <stdio.h>

#define USBRX_MAX_PAYLOAD 32U
// #define ENABLE_USBRX_STATE_DEBUG

typedef enum {
  RX_WAIT_HEADER_H = 0,
  RX_WAIT_HEADER_L,
  RX_WAIT_LEN_L,
  RX_WAIT_LEN_H,
  RX_WAIT_TYPE,
  RX_WAIT_PAYLOAD,
  RX_WAIT_CSUM_L,
  RX_WAIT_CSUM_H,
  RX_WAIT_TAIL_H,
  RX_WAIT_TAIL_L
} UsbRxState;

#ifdef ENABLE_USBRX_STATE_DEBUG
static const char *USBRX_StateName(UsbRxState state)
{
  switch (state) {
    case RX_WAIT_HEADER_H: return "WAIT_HEADER_H";
    case RX_WAIT_HEADER_L: return "WAIT_HEADER_L";
    case RX_WAIT_LEN_L: return "WAIT_LEN_L";
    case RX_WAIT_LEN_H: return "WAIT_LEN_H";
    case RX_WAIT_TYPE: return "WAIT_TYPE";
    case RX_WAIT_PAYLOAD: return "WAIT_PAYLOAD";
    case RX_WAIT_CSUM_L: return "WAIT_CSUM_L";
    case RX_WAIT_CSUM_H: return "WAIT_CSUM_H";
    case RX_WAIT_TAIL_H: return "WAIT_TAIL_H";
    case RX_WAIT_TAIL_L: return "WAIT_TAIL_L";
    default: return "UNKNOWN";
  }
}

#define USBRX_LOG(...) printf(__VA_ARGS__)
#else
#define USBRX_LOG(...)
#endif

static void USBRX_Reset(UsbRxState *state,
                        uint16_t *payload_len,
                        uint16_t *payload_idx,
                        uint16_t *checksum_calc,
                        uint8_t *type)
{
  *state = RX_WAIT_HEADER_H;
  *payload_len = 0U;
  *payload_idx = 0U;
  *checksum_calc = 0U;
  *type = 0U;
}

static void USBRX_HandleSetConfig(const uint8_t *payload, uint16_t payload_len)
{
  if (payload_len != 3U) {
    USBRX_LOG("[USBRX] SET_CONFIG invalid payload_len=%u\n", (unsigned)payload_len);
    return;
  }

  uint8_t scan_frequency_hz = payload[0];
  uint16_t sample_depth = (uint16_t)payload[1] | (uint16_t)((uint16_t)payload[2] << 8);
#ifdef ENABLE_USBRX_STATE_DEBUG
  uint8_t res = SysConfig_SetConfig(&sysConfig, scan_frequency_hz, sample_depth);
  USBRX_LOG("[USBRX] SET_CONFIG freq=%u depth=%u res=%u\n",
            (unsigned)scan_frequency_hz,
            (unsigned)sample_depth,
            (unsigned)res);
#else
  (void)SysConfig_SetConfig(&sysConfig, scan_frequency_hz, sample_depth);
#endif
}

static void USBRX_HandleAcqConfig(uint16_t payload_len)
{
  if (payload_len != 0U) {
    USBRX_LOG("[USBRX] ACQ_CONFIG invalid payload_len=%u\n", (unsigned)payload_len);
    return;
  }

#ifdef ENABLE_USBRX_STATE_DEBUG
  uint8_t res = SysConfig_USB_NotifyConfig(&sysConfig);
  USBRX_LOG("[USBRX] ACQ_CONFIG res=%u\n", (unsigned)res);
#else
  (void)SysConfig_USB_NotifyConfig(&sysConfig);
#endif
}

static void USBRX_DispatchFrame(uint8_t type, const uint8_t *payload, uint16_t payload_len)
{
  switch (type) {
    case USB_PKT_TYPE_SET_CONFIG:
      USBRX_HandleSetConfig(payload, payload_len);
      break;

    case USB_PKT_TYPE_ACQ_CONFIG:
      USBRX_HandleAcqConfig(payload_len);
      break;

    default:
      USBRX_LOG("[USBRX] discard frame type=0x%02X payload_len=%u\n",
                type,
                (unsigned)payload_len);
      break;
  }
}

void StartUSBRXTask(void *argument)
{
  (void)argument;

  UsbRxState state = RX_WAIT_HEADER_H;
  uint8_t byte_in = 0U;
  uint8_t payload[USBRX_MAX_PAYLOAD];
  uint16_t payload_len = 0U;
  uint16_t payload_idx = 0U;
  uint16_t checksum_calc = 0U;
  uint16_t checksum_recv = 0U;
  uint8_t type = 0U;

  USBRX_LOG("[USBRX] Initialized.\n");

  for (;;) {
    if (osMessageQueueGet(USBRXQueueHandle, &byte_in, 0, osWaitForever) != osOK) {
      continue;
    }

    USBRX_LOG("[USBRX] state=%s byte=0x%02X\n", USBRX_StateName(state), byte_in);

    switch (state) {
      case RX_WAIT_HEADER_H:
        if (byte_in == USB_PKT_HEADER_HIGH) {
          state = RX_WAIT_HEADER_L;
          // USBRX_LOG("[USBRX] transition: WAIT_HEADER_H -> WAIT_HEADER_L\n");
        }
        break;

      case RX_WAIT_HEADER_L:
        if (byte_in == USB_PKT_HEADER_LOW) {
          state = RX_WAIT_LEN_L;
          // USBRX_LOG("[USBRX] transition: WAIT_HEADER_L -> WAIT_LEN_L\n");
        } else if (byte_in == USB_PKT_HEADER_HIGH) {
          state = RX_WAIT_HEADER_L;
          // USBRX_LOG("[USBRX] keep WAIT_HEADER_L (re-sync high header)\n");
        } else {
          state = RX_WAIT_HEADER_H;
          USBRX_LOG("[USBRX] invalid low header, reset to WAIT_HEADER_H\n");
        }
        break;

      case RX_WAIT_LEN_L:
        payload_len = (uint16_t)byte_in;
        checksum_calc = (uint16_t)byte_in;
        state = RX_WAIT_LEN_H;
        break;

      case RX_WAIT_LEN_H:
        payload_len |= (uint16_t)((uint16_t)byte_in << 8);
        checksum_calc ^= (uint16_t)byte_in;
        USBRX_LOG("[USBRX] payload_len=%u\n", (unsigned)payload_len);

        if (payload_len > USBRX_MAX_PAYLOAD) {
          USBRX_LOG("[USBRX] payload too large, reset\n");
          USBRX_Reset(&state, &payload_len, &payload_idx, &checksum_calc, &type);
        } else {
          payload_idx = 0U;
          state = RX_WAIT_TYPE;
          // USBRX_LOG("[USBRX] transition: WAIT_LEN_H -> WAIT_TYPE\n");
        }
        break;

      case RX_WAIT_TYPE:
        type = byte_in;
        checksum_calc ^= (uint16_t)type;
        USBRX_LOG("[USBRX] type=0x%02X\n", type);

        if (payload_len == 0U) {
          state = RX_WAIT_CSUM_L;
          // USBRX_LOG("[USBRX] transition: WAIT_TYPE -> WAIT_CSUM_L\n");
        } else {
          state = RX_WAIT_PAYLOAD;
          // USBRX_LOG("[USBRX] transition: WAIT_TYPE -> WAIT_PAYLOAD\n");
        }
        break;

      case RX_WAIT_PAYLOAD:
        payload[payload_idx++] = byte_in;
        checksum_calc ^= (uint16_t)byte_in;
        USBRX_LOG("[USBRX] payload_idx=%u/%u\n", (unsigned)payload_idx, (unsigned)payload_len);

        if (payload_idx >= payload_len) {
          state = RX_WAIT_CSUM_L;
          // USBRX_LOG("[USBRX] transition: WAIT_PAYLOAD -> WAIT_CSUM_L\n");
        }
        break;

      case RX_WAIT_CSUM_L:
        checksum_recv = (uint16_t)byte_in;
        state = RX_WAIT_CSUM_H;
        break;

      case RX_WAIT_CSUM_H:
        checksum_recv |= (uint16_t)((uint16_t)byte_in << 8);
        if (checksum_recv == checksum_calc) {
          state = RX_WAIT_TAIL_H;
          USBRX_LOG("[USBRX] checksum OK: recv=0x%04X calc=0x%04X\n", checksum_recv, checksum_calc);
        } else {
          USBRX_LOG("[USBRX] checksum FAIL: recv=0x%04X calc=0x%04X\n", checksum_recv, checksum_calc);
          USBRX_Reset(&state, &payload_len, &payload_idx, &checksum_calc, &type);
        }
        break;

      case RX_WAIT_TAIL_H:
        if (byte_in == USB_PKT_TAIL_HIGH) {
          state = RX_WAIT_TAIL_L;
          // USBRX_LOG("[USBRX] transition: WAIT_TAIL_H -> WAIT_TAIL_L\n");
        } else {
          USBRX_LOG("[USBRX] invalid tail high, reset\n");
          USBRX_Reset(&state, &payload_len, &payload_idx, &checksum_calc, &type);
        }
        break;

      case RX_WAIT_TAIL_L:
        if (byte_in == USB_PKT_TAIL_LOW) {
          USBRX_DispatchFrame(type, payload, payload_len);
        } else {
          USBRX_LOG("[USBRX] invalid tail low, reset\n");
        }
        USBRX_Reset(&state, &payload_len, &payload_idx, &checksum_calc, &type);
        break;

      default:
        USBRX_Reset(&state, &payload_len, &payload_idx, &checksum_calc, &type);
        break;
    }
  }
}