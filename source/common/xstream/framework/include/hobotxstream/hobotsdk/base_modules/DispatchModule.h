/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-04 23:07:12
 * @Version: v0.0.1
 * @Brief: XStream Base DispatchModule declarition.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 21:14:29
 */
#ifndef HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_DISPATCHMODULE_H_
#define HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_DISPATCHMODULE_H_

#include <string>
#include <memory>
#include <vector>
#include "BaseMessage.h"
#include "hobot/hobot.h"
#include "hobotxsdk/xstream_data.h"

namespace xstream {
enum class DispatchStrategy {
  ROUND_ROBIN = 0,
  KEY_MATCHING = 1,
};

/**
 * @brief 负责实现输入的分发，同时包含FakeResult的产生（由DisableParam等产生);
 */
class DispatchModule : public hobot::Module {
 public:
  explicit DispatchModule(std::string instance_name,
                          std::string class_name = "")
      : hobot::Module(instance_name, class_name),
        unique_name_(instance_name) {}

  int Init(hobot::RunContext *context) override { return 0; }

  void Reset() override {}

  void SetStrategy(DispatchStrategy stg = DispatchStrategy::ROUND_ROBIN) {
    stgy_ = stg;
  }
  using KeyMatchingFunc = std::function<int(void*)>;
  void SetKeyMatchingFunc(KeyMatchingFunc func) {
    key_match_func_ = func;
  }

  void SetOutputSlotNum(int slot_num) { output_slot_num_ = slot_num; }

  void SetInstanceNum(int32_t inst_num) { inst_num_ = inst_num; }

 protected:
  bool IsNeedSkip(const std::vector<BaseDataPtr> &inputs,
                  const InputParamPtr &param);
  std::shared_ptr<XStreamMethodOutputMessage> FakeResult(
      const std::vector<BaseDataPtr> &inputs, const InputParamPtr &param);
  int GetSelectInstanceIdx(void *key = nullptr);
  DispatchStrategy stgy_{DispatchStrategy::ROUND_ROBIN};
  int cur_post_pos_{0};
  int32_t inst_num_;
  KeyMatchingFunc key_match_func_;
  uint32_t output_slot_num_;
  std::string unique_name_;
};
}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_DISPATCHMODULE_H_
