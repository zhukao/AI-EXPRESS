/**
 * Copyright 2019 Horizon Robotics
 * @brief time record
 * @date 2020-01-15
 * @author zhuoran.rong(zhuoran.rong@horizon.ai)
 **/

#include "time_record.h"  // NOLINT
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

// 获得线程ID
#define gettid() syscall(__NR_gettid)

// 时间记录flag
volatile uint32_t __tr_record_enable = 0;

static FILE *log_file = NULL;
static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;

// 开始时间记录
int tr_start() {
  char *log_file_parh = getenv("TR_LOG_FILE");

  if (!log_file_parh) {
    fprintf(stderr, "Time Record: log file not specified!\n");
    return -1;
  }

  log_file = fopen(log_file_parh, "wb");
  if (!log_file) {
    perror("fopen");
    return -1;
  }
  __tr_record_enable = 1;
  return 1;
}

// 结束时间记录
void tr_stop() {
  __tr_record_enable = 0;

  if (!log_file) {
    return;
  }

  fflush(log_file);
  fclose(log_file);
  log_file = NULL;
}

// 记录日志
int tr_record(int start, const char *func, const char *fmt, ...) {
  va_list arg_ptr;
  struct timeval tv;

  va_start(arg_ptr, fmt);

  pthread_mutex_lock(&write_lock);
  gettimeofday(&tv, NULL);
  // 写入行首
  fprintf(log_file, "%c|%d|%s|%ld.%ld|", start > 0 ? 'S' : 'E', gettid(), func,
          tv.tv_sec, tv.tv_usec);
  // 附加内容
  vfprintf(log_file, fmt, arg_ptr);
  // 写入换行符
  fputc('\n', log_file);
  pthread_mutex_unlock(&write_lock);

  va_end(arg_ptr);

  return 0;
}
