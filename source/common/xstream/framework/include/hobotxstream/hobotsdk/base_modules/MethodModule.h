/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-05 23:49:27
 * @Version: v0.0.1
 * @Brief: Method instantiation Declaration.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 22:14:25
 */
#ifndef HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_METHODMODULE_H_
#define HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_METHODMODULE_H_

#include <memory>
#include <string>
#include <vector>

#include "ThreadSafeMethod.h"
#include "hobot/hobot.h"
#include "hobotxstream/method.h"
#include "hobotxstream/profiler.h"
#include "json/json.h"

namespace xstream {
/**
 * @brief 封装Method实例，同时完成method　task的超时监控．
 */
class MethodModule : public hobot::Module {
 public:
  MethodModule() = default;
  explicit MethodModule(const ThreadSafeMethodPtr &method,
                        const std::string &config_file_path, bool is_inited,
                        ProfilerPtr profiler, std::string unique_name,
                        bool is_use_config_file_path,
                        std::string instance_name = nullptr);
  explicit MethodModule(const ThreadSafeMethodPtr &method,
                        const std::string &config, bool is_inited,
                        ProfilerPtr profiler, std::string unique_name,
                        std::string instance_name = nullptr);
  int Init(hobot::RunContext *context) override { return 0; }

  void Reset() override {}
  int UpdateParameter(InputParamPtr ptr);
  InputParamPtr GetParameter() const;
  std::string GetVersion() const;
  void Finalize() {}
  /// 用于告知Method整个SDK的Profiler状态更改
  void OnProfilerChanged(bool on) {}

  /**
   * Input Message:
   *  0: type-XStreamMethodInputMessage
   * Output Message:
   *  0: type-XStreamMethodOutputMessage
   *
   */
  FORWARD_DECLARE(MethodModule, 0);

  void SetOutputSlotNum(int num) { output_slot_num_ = num; }

  void SetInstId(int id) { inst_id_ = id; }

 private:
  ThreadSafeMethodPtr method_inst_;
  bool is_inited_{false};
  uint32_t output_slot_num_;
  int inst_id_;
  enum class ConfigMode { RAW_FILE_PATH, PARSED_JSON };
  std::string config_file_path_;
  Json::Value config_;
  ConfigMode config_mode_;
  ProfilerPtr profiler_;
  std::string unique_name_;
};
}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_METHODMODULE_H_
