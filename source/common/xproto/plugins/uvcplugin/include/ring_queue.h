//
// Created by Minghao Wang on 17-9-26.
// Copyright (c) 2017 Horizon Robotics. All rights reserved.
//

#ifndef INCLUDE_UVCPLUGIN_RINGQUEUE_H_
#define INCLUDE_UVCPLUGIN_RINGQUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <condition_variable>
#include <deque>
#include <mutex>

#include "xproto/utils/singleton.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {

template <typename Dtype>
class RingQueue : public hobot::CSingleton<RingQueue<Dtype>> {
 public:
  using ReleaseDataFunc = std::function<void(Dtype& elem)>;
  RingQueue() {}
  int Init(typename std::deque<Dtype>::size_type size,
           ReleaseDataFunc release_func) {
    if (!size_) {
      printf("try to construct empty ring queue");
      size_ = size;
    }
    release_func_ = release_func;
    return 0;
  }

  void Push(const Dtype& elem) {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      if (size_ <= 0) {
        return;
      }
      if (que_.size() >= size_) {
        auto e = que_.front();
        if (release_func_) {
          release_func_(e);
        }
        que_.pop_front();
      }
      que_.push_back(elem);
    }
    cv_.notify_all();
  }

  bool Pop(Dtype &video_buffer) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!que_.empty()) {
      video_buffer = que_.front();
      que_.pop_front();
    } else {
      return false;
    }
    return true;
  }

  bool Empty() {
    std::lock_guard<std::mutex> l(mtx_);
    return que_.empty();
  }

  bool IsValid() {
    std::lock_guard<std::mutex> l(mtx_);
    // may push twice at one time
    return (size_ > 0 && que_.size() < size_ - 1);
  }

  void Clear() {
    std::lock_guard<std::mutex> l(mtx_);
    while (!que_.empty()) {
      auto elem = que_.front();
      if (release_func_) {
        release_func_(elem);
      }
      que_.pop_front();
    }
  }

 private:
  typename std::deque<Dtype>::size_type size_ = 0;
  std::deque<Dtype> que_;
  std::mutex mtx_;
  std::condition_variable cv_;
  ReleaseDataFunc release_func_;
};

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_UVCPLUGIN_RINGQUEUE_H_
