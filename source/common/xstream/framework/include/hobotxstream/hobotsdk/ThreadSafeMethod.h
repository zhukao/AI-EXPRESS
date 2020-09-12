/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-24 04:53:41
 * @Version: v0.0.1
 * @Brief: method wrapper to make it thread-safe.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 06:23:34
 */
#ifndef HOBOTXSTREAM_HOBOTSDK_THREADSAFEMETHOD_H_
#define HOBOTXSTREAM_HOBOTSDK_THREADSAFEMETHOD_H_
#include <memory>
#include <string>
#include <vector>

#include "common/rw_mutex.h"
#include "hobotxstream/method.h"

namespace xstream {
class ThreadSafeMethod {
  MethodPtr method_;
  RWLock lock_;

 public:
  explicit ThreadSafeMethod(MethodPtr method) : method_(method) {}

  ~ThreadSafeMethod() { Finalize(); }
  /// 初始化
  virtual int Init(const std::string &config_file_path) {
    return method_->Init(config_file_path);
  }

  virtual int InitFromJsonString(const std::string &config) {
    return method_->InitFromJsonString(config);
  }

  /// 动态改变Method运行参数配置
  virtual int UpdateParameter(InputParamPtr ptr) {
    WriteLockGuard guard(&lock_);
    auto ret = method_->UpdateParameter(ptr);
    if (0 != ret) {
      LOGE << "Failed to update parameter for " << ptr->unique_name_
           << " with code " << ret;
      return -1;
    }
    return 0;
  }
  // 数据处理函数，第一个参数是输入数据（双重vector，外层vector表示batch是多帧的输入
  // 内层的vector表示单帧的数据列表），
  // 内层vector对应workflow的"inputs"输入列表
  virtual std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<InputParamPtr> &param) {
    ReadLockGuard guard(&lock_);
    return method_->DoProcess(input, param);
  }
  /// 获取Method运行参数配置
  virtual InputParamPtr GetParameter() const {
    if (!method_) {
      return nullptr;
    }
    return method_->GetParameter();
  }
  /// 获取Method版本号，比如 metric_v0.4.0 或者 MD112 等
  virtual std::string GetVersion() const {
    if (!method_) {
      return "";
    }
    return method_->GetVersion();
  }
  /// 析构
  virtual void Finalize() { method_->Finalize(); }
  /// 获取Method基本信息
  virtual MethodInfo GetMethodInfo() { return method_->GetMethodInfo(); }
  /// 用于告知Method整个SDK的Profiler状态更改
  virtual void OnProfilerChanged(bool on) { method_->OnProfilerChanged(on); }
};
using ThreadSafeMethodPtr = std::shared_ptr<ThreadSafeMethod>;
}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_THREADSAFEMETHOD_H_
