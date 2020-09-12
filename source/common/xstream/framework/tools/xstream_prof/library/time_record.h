/**
 * Copyright 2019 Horizon Robotics
 * @brief time record
 * @date 2020-01-15
 * @author zhuoran.rong(zhuoran.rong@horizon.ai)
 **/

#ifndef __TIME_RECORD_H__   // NOLINT
#define __TIME_RECORD_H__   // NOLINT

#include <stdint.h>

// 放在函数调用开头
#define TR_PROLOGUE(func, fmt, ...)                                            \
  do {                                                                         \
    if (!__tr_record_enable)                                                   \
      break;                                                                   \
    tr_record(1, func, fmt, ##__VA_ARGS__);                                    \
  } while (0);

// 放在函数调用结尾
#define TR_EPILOGUE(func)                                                      \
  do {                                                                         \
    if (!__tr_record_enable)                                                   \
      break;                                                                   \
    tr_record(0, func, "");                                                    \
  } while (0);

extern volatile uint32_t __tr_record_enable;
int tr_start(void);
void tr_stop(void);
int tr_record(int start, const char *func, const char *fmt, ...);

#endif   // NOLINT
