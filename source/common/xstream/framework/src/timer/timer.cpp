/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     timer functions
 * @author    wenhao.zou
 * @email     wenhao.zou@horizon.ai
 * @version   0.0.0.1
 * @date      2020.02.22
 */

#define _CRT_SECURE_NO_WARNINGS

#include "timer/timer.h"

#ifdef _MSC_VER
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <memory>
#include <iomanip>
#include <vector>
#include "hobotlog/hobotlog.hpp"



namespace xstream {
//////////////////////////////////////////////////////////////////////////
// TimerTask

TimerTask::TimerTask(TimerManager &manager):
  manager_(manager),
  timer_id_(0),
  clock_id_(CLOCK_MONOTONIC), /*CLOCK_MONOTONIC*/
  timer_signo_(SIGRTMIN),
  is_stop(false) {
  struct sigevent sev;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = timer_signo_;
  sev.sigev_value.sival_ptr = &timer_id_;
  HOBOT_CHECK(timer_create(clock_id_, &sev, &timer_id_) != -1)
      << "Fail to call timer_create with clockid_t " << clock_id_;
  LOGD << "create time id:" << timer_id_;
}

TimerTask::~TimerTask() {
  Stop();
  if (timer_id_ != reinterpret_cast<void*>(0)) {
    HOBOT_CHECK(timer_delete(timer_id_) != -1)
      << "Fail to delete timer " << timer_id_;
  }
}

TimerTask::TimerTaskID TimerTask::TimerID() {
  return timer_id_;
}

void TimerTask::Start(std::function<void(void*)> fun,
  unsigned interval,
  void* userdata,
  TimerType timeType) {
  struct itimerspec its;
  sigset_t mask;
  Stop();
  /*Save parameter*/
  callback_fun_ = fun;
  userdata_ = userdata;
  /* Set the timer */
  LOGD << "Timer interval: " << interval << "ms";
  its.it_value.tv_sec = interval / kNumMillisecsPerSec;
  its.it_value.tv_nsec =
    (interval-(its.it_value.tv_sec*kNumMillisecsPerSec))
    *kNumNanosecsPerMillisec;
  LOGD << "interval:" << its.it_value.tv_sec
    << "." << std::setw(5) << its.it_value.tv_nsec;
  if (timeType == TimerType::CIRCLE) {
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
  } else {
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
  }
  /* Block timer signal temporarily */
  sigemptyset(&mask);
  sigaddset(&mask, timer_signo_);
  if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
    LOGE << "Fail to block timer signal";
  }

  /* Set Timer*/
  manager_.AddTimer(std::shared_ptr<TimerTask>(this));
  HOBOT_CHECK(timer_settime(timer_id_, 0, &its, NULL) == 0)
    << "Fail to start timer "<< timer_id_;
  is_stop = false;
  /* UnBlock timer signal*/
  if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
    LOGE << "Fail to UNblock timer signal";
  }
  return;
}

void TimerTask::Stop() {
  sigset_t mask;
  if (timer_id_ != reinterpret_cast<void*>(0)
    && !is_stop) {
     /* Block timer signal temporarily */
    sigemptyset(&mask);
    sigaddset(&mask, timer_signo_);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
      LOGE << "Fail to block timer signal" << std::endl;
    }
    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval = its.it_value;
    HOBOT_CHECK(timer_settime(timer_id_, 0, &its, NULL) == 0)
      << "Fail to stop timer " << timer_id_;
    is_stop = true;
    manager_.RemoveTimer(this);
     /* UnBlock timer signal*/
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
      LOGE << "Fail to UNblock timer signal" << std::endl;
    }
  }
}

void TimerTask::OnTimer(uint64_t now) {
  callback_fun_(userdata_);
}


//////////////////////////////////////////////////////////////////////////
// TimerManager
TimerManager::TimerManager(int singno):timer_signo_(singno) {
}

void TimerManager::AddTimer(std::shared_ptr<TimerTask> timertask) {
  WriteLockGuard lock(&this->timer_tb_mutex_);
  LOGD <<  "add time_id: " << timertask->TimerID();
  timer_tb_[timertask->TimerID()] = timertask;
}

void TimerManager::RemoveTimer(TimerTask* timertask) {
  if (timertask == nullptr) {
    LOGE << "Remove TimerTask nullptr ";
    return;
  }
  std::shared_ptr<TimerTask> pTimeTask; {
    WriteLockGuard lock(&this->timer_tb_mutex_);
    auto iter = timer_tb_.find(timertask->TimerID());
    if (iter != timer_tb_.end()) {
      pTimeTask = iter->second;
      timer_tb_.erase(iter);
    } else {
      LOGD << "not found timer id " << timertask->TimerID();
    }
  }
}

int TimerManager::ProcessSignHandler(siginfo_t *si ) {
  int ret = 0;
  timer_t *tidp;

  tidp = reinterpret_cast<timer_t *>(si->si_value.sival_ptr);
  ReadLockGuard lock(&this->timer_tb_mutex_);
  auto iter = timer_tb_.find(*tidp);
  if (iter != timer_tb_.end()) {
    iter->second->OnTimer(0);
  } else {
    LOGE << "time id:" << *tidp << " not found!" << std::endl;
    ret = -1;
  }
  return ret;
}

std::mutex Timer::mutex_;
Timer *Timer::inst_ = nullptr;

Timer *Timer::Instance() {
  if (NULL == inst_) {
    std::lock_guard<std::mutex> guard(Timer::mutex_);
    if (NULL == inst_) {
      inst_ = new Timer();
    }
  }
  return inst_;
}

Timer::Timer(int singno):timer_manager_(singno), timer_signo_(singno) {
  struct sigaction sa;

  is_stop_ = false;
  thread_ptr_ = NULL;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = Timer::TimerSignHandler;
  sigemptyset(&sa.sa_mask);
  HOBOT_CHECK(sigaction(timer_signo_, &sa, NULL) != -1)
    << "Fail to sigaction at  " << timer_signo_;
}

void *Timer::AddTimer(std::function<void(void*)> callback,
      uint32_t interval,
      void* userdata,
      TimerTask::TimerType timeType) {
  if (interval < 10) {
    LOGE << "AddTimer fail for interval " << interval << "fail !";
    return NULL;
  }
  TimerTask *tmp = new TimerTask(timer_manager_);
  tmp->Start(callback, interval, userdata, timeType);
  return reinterpret_cast<void *>(tmp);
}

void Timer::RemoveTimer(void *&ptr) {
  if (NULL == ptr) {
    return;
  }
  TimerTask *timer = reinterpret_cast<TimerTask *>(ptr);
  ptr = NULL;
  timer->Stop();
  std::lock_guard<std::mutex> guard(Timer::mutex_);
}


void Timer::TimerSignHandler(int sig, siginfo_t *si, void *uc) {
  /* Note: calling printf() from a signal handler is not
  strictly correct, since printf() is not async-signal-safe;
  see signal(7) */
  Timer::Instance()->ProcessSignHandler(si);
}

void Timer::ProcessSignHandler(siginfo_t *si) {
  int ret = 0;
  ret = timer_manager_.ProcessSignHandler(si);
  if (ret != 0) {
    LOGE << "ProcessSignHandler Fail " << ret;
  }
}
}  // namespace xstream
