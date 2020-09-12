/**
 * Copyright 2019 Horizon Robotics
 * @brief sample
 * @date 2020-01-15
 * @author zhuoran.rong(zhuoran.rong@horizon.ai)
 **/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "time_record.h"  // NOLINT

void f() {
  TR_PROLOGUE(__FUNCTION__, "");
  for (int a = 0; a < 10000; a++) {
  }
  for (int a = 0; a < 10000; a++) {
  }
  TR_EPILOGUE(__FUNCTION__);
}

void e() {
  TR_PROLOGUE(__FUNCTION__, "");
  for (int a = 0; a < 10000; a++) {
  }
  f();
  for (int a = 0; a < 10000; a++) {
  }
  TR_EPILOGUE(__FUNCTION__);
}

void d() {
  TR_PROLOGUE("d", "");
  for (int a = 0; a < 10000; a++) {
  }
  e();
  for (int a = 0; a < 10000; a++) {
  }
  TR_EPILOGUE("d");
}

void c() {
  TR_PROLOGUE("c", "");
  for (int a = 0; a < 10000; a++) {
  }
  d();
  for (int a = 0; a < 10000; a++) {
  }
  TR_EPILOGUE("c");
}

void b() {
  TR_PROLOGUE("b", "");
  for (int a = 0; a < 10000; a++) {
  }
  c();
  for (int a = 0; a < 10000; a++) {
  }
  TR_EPILOGUE("b");
}

void a() {
  TR_PROLOGUE("a", "");
  for (int a = 0; a < 10000; a++) {
  }
  b();
  for (int a = 0; a < 10000; a++) {
  }
  TR_EPILOGUE("a");
}

// 线程函数
void *thread_func(void *arg) {
  int i = (int)arg;  // NOLINT
  int j;

  TR_PROLOGUE("thread_func", "arg:%d", i);
  for (j = 0; j < 20; j++) {
    a();
  }
  TR_EPILOGUE("thread_func");
}

pthread_t threads[3];

void main(void) {
  int i;
  tr_start();
  TR_PROLOGUE("main", "%d,%s", 555666, "abcd");
  for (i = 0; i < 3; i++) {
    pthread_create(&threads[i], NULL, thread_func, (void *)i);  // NOLINT
  }
  sleep(2);
  for (i = 0; i < 3; i++) {
    pthread_join(threads[i], NULL);
  }
  TR_EPILOGUE("main");
  tr_stop();
  return;
}
