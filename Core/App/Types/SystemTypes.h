#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

typedef enum SystemState_t {
  SystemState_Stop = 0,
  SystemState_Running = 1,
} SystemState;

typedef struct SysControlMessage_t {
  SystemState state;
} SysControlMessage;

// Event group bits
#define SYS_EVENT_RUNNING_BIT (1UL << 0) // 系统运行状态位

#endif /* SYSTEM_TYPES_H */