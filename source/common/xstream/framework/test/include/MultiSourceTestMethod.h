/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file MultiSourceTestMethod.h
 * @brief
 * @author guoqian.sun
 * @email guoqian.sun@horizon.ai
 * @date 2019/12/04
 */
#ifndef XSTREAM_FRAMEWORK_TEST_INCLUDE_MULTISOURCETESTMETHOD_H_
#define XSTREAM_FRAMEWORK_TEST_INCLUDE_MULTISOURCETESTMETHOD_H_
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/method.h"

namespace xstream {

struct MulSrcTestOut {
  size_t thread_hash_;
  size_t method_id_;
  size_t source_id_;
  size_t frame_id_;
};

typedef struct XStreamData<MulSrcTestOut> MulSrcTestOutput;
typedef std::shared_ptr<MulSrcTestOutput> MulSrcTestOutputPtr;

struct MulSrcTestIn {
  size_t source_id_;
  size_t frame_id_;
};
typedef struct XStreamData<MulSrcTestIn> MulSrcTestInput;
typedef std::shared_ptr<MulSrcTestInput> MulSrcTestInputPtr;

class MultiSourceTestMethod : public Method {
 public:
  MultiSourceTestMethod() { method_id_ = instance_count_++; }
  /// 初始化
  int Init(const std::string &config_file_path) override { return 0; }

  int UpdateParameter(InputParamPtr ptr) override { return 0; }

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<InputParamPtr> &param) override;

  /// 获取Method运行参数配置
  InputParamPtr GetParameter() const override { return InputParamPtr(); }
  /// 获取Method版本号，比如 metric_v0.4.0 或者 MD112 等
  std::string GetVersion() const override { return "MSTM0.0"; }
  /// 析构
  void Finalize() override{};
  /// 获取Method基本信息
  MethodInfo GetMethodInfo() override { return methodinfo_; }

  /// 用于告知Method整个SDK的Profiler状态更改
  void OnProfilerChanged(bool on) override {}
  static void SetMethodInfo(const MethodInfo &methodinfo);

 private:
  static std::atomic_ulong instance_count_;
  static MethodInfo methodinfo_;
  int32_t method_id_;
};

}  // namespace xstream

#endif  // XSTREAM_FRAMEWORK_TEST_INCLUDE_MULTISOURCETESTMETHOD_H_
