/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: wenhao zou
 * @Mail: wenhao.zou@horizon.ai
 * @Date: 2019-12-20 01:15:22
 * @Version: v0.0.1
 * @Brief: test timer
 */


#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "timer/timer.h"
#include "hobotxstream/framework_data_shell.h"
#include "hobotlog/hobotlog.hpp"


using xstream::ExtraInfoWithinNode;
using xstream::FrameworkDataBatch;
using xstream::FrameworkDataShell;
using xstream::Timer;
using xstream::TimerManager;
using xstream::TimerTask;


namespace TimerTest {
struct ExtraStateInfoWithinNode : public ExtraInfoWithinNode {
  void *timer_token_ = nullptr;
  bool done_ = false;
};

class TimerCall{
 public:
  explicit TimerCall(uint32_t runtimes = 100) : runtimes_(runtimes) {
    LOGD << "===instace " << this << ": runtimes " << runtimes_ << "===";
    setting_timeout_duration_ms_ = 0;
    last_call_time_sec = 0;
    last_call_time_usec = 0;
  }



  int StartTimer(uint32_t duration, TimerTask::TimerType type) {
    setting_timeout_duration_ms_ = duration;
    // 创建shell user data部分
    std::function<void(void*)> call_back_f = TimerCall::OnTimerHandle;
    struct timeval tv = {0};
    (void)gettimeofday(&tv, NULL);
    LOGD << "StartTime at:" << tv.tv_sec << "." << tv.tv_usec;
    LOGD << "duration" << setting_timeout_duration_ms_;
    last_call_time_sec = tv.tv_sec;
    last_call_time_usec = tv.tv_usec;

    timer_token_ = Timer::Instance()->AddTimer(
        call_back_f, setting_timeout_duration_ms_,
        reinterpret_cast<void*>(this), type);
    return 0;
  }

  int StopTimer() {
    Timer::Instance()->RemoveTimer(timer_token_);
    return 0;
  }

  static void OnTimerHandle(void* userdata) {
    TimerCall* call = static_cast<TimerCall*>(userdata);
    call->OnTimer(userdata);
  }

  void OnTimer(void* userdata) {
    if (!IsStop()) {
      checkout();
      runtimes_--;
    } else {
      sleep(1);
    }
    LOGD << "remaining times: " << runtimes_;
  }

  bool IsStop() {
    return runtimes_ <= 0;
  }

 private:
  int checkout() {
    struct timeval tv = {0};
    uint64_t interval_us = 0;
    (void)gettimeofday(&tv, NULL);
    LOGD << "on time:" << tv.tv_sec << "." << tv.tv_usec;
    if (last_call_time_sec > 0) {
      interval_us = (tv.tv_sec-TimerCall::last_call_time_sec)*1000000
        + (tv.tv_usec-TimerCall::last_call_time_usec);
      double rate = static_cast<double>(interval_us)
        / static_cast<double>(
          setting_timeout_duration_ms_*xstream::kNumMicrosecsPerMillisec);
      LOGD << "interval fact[" << interval_us << "us]:plan["
        << setting_timeout_duration_ms_*xstream::kNumMicrosecsPerMillisec
        <<"us]=>"
        << std::setw(5)
        << std::setprecision(3)
        << std::fixed
        << rate*100
        <<"%"
        << std::endl;
      EXPECT_GE(rate, 0.85);
      EXPECT_LE(rate, 1.15);
    }
    last_call_time_sec = tv.tv_sec;
    last_call_time_usec = tv.tv_usec;
    return 0;
  }

  uint32_t setting_timeout_duration_ms_;
  void* timer_token_;

 public:
  mutable int32_t runtimes_;
  uint32_t last_call_time_sec;
  uint32_t last_call_time_usec;
};
}  // namespace  TimerTest


void TimeCallTestHighCapacity(uint32_t duration) {
  TimerTest::TimerCall timecall(100);
  timecall.StartTimer(duration, TimerTask::TimerType::CIRCLE);
  /**/
  int a = 0;
  while (!timecall.IsStop()) {
    a++;
  }
  timecall.StopTimer();
}


TEST(XStreamTimer, testOnHighCapacity) {
  // 注册timer
  uint32_t duration = 100;
  std::thread tcall(TimeCallTestHighCapacity, duration);  // pass by value
  tcall.join();
}


void TimeCallTestCommon(uint32_t duration) {
  TimerTest::TimerCall timecall(1);
  timecall.StartTimer(duration, TimerTask::TimerType::ONCE);
  /**/
  while (!timecall.IsStop()) {
    sleep(1);
  }
  timecall.StopTimer();
}


TEST(XStreamTimer, testMutilThread) {
  // 注册timer
  std::vector<std::thread*> thread_array;
  for (uint32_t duration = 100; duration <= 1000; duration += 10) {
    LOGD << "#### create std::thread";
    thread_array.push_back(new std::thread(TimeCallTestCommon, duration));
  }
  for (auto ithead = thread_array.begin();
    ithead != thread_array.end(); ithead++) {
    (*ithead)->join();
    delete (*ithead);
  }
}

void TimeCallTestDuration() {
  TimerTest::TimerCall timecall(2);
  /*  */
  for (uint32_t duration = 10; duration <= 1000; duration += 10) {
    timecall.StartTimer(duration, TimerTask::TimerType::CIRCLE);
    while (!timecall.IsStop()) {
      LOGD << "#### not Stop ";
      sleep(1);
    }
    LOGD << "#### TimeCallTestDuration ";
    timecall.StopTimer();
  }
}

TEST(XStreamTimer, testDuration) {
  // 注册timer
  TimeCallTestDuration();
}
