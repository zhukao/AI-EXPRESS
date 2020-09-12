/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-06 00:41:07
 * @Version: v0.0.1
 * @Brief: Collect Result,split and send out.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-12 02:27:50
 */
#ifndef HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_COLLECTMODULE_H_
#define HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_COLLECTMODULE_H_

#include <string>
#include "hobot/hobot.h"

namespace xstream {
/**
 * @brief CollectModule输入为MethodModule与DispatchtModule，
 *        输入之间按OR表达式组织；CollectModule的作用是收集结果，
 *        并分拆打包的message驱动后面的模块。
 */
class CollectModule : public hobot::Module {
 public:
  explicit CollectModule(std::string instance_name = "")
    : hobot::Module(instance_name, "xstream::CollectModule") {}

  int Init(hobot::RunContext *context) override {
    return 0;
  }

  void Reset() override {}
  /**
   * Input Message:
   *  0: PassThrough Message, type:XStreamMethodOutputMessage;
   *  1-methods.size(): Method Output Message, type:XStreamMethodOutputMessage;
   *  Condition: OR.
   * Output Message:
   *  0- Method Output_slot_size - 1, type:XStreamInputMessage.
   */
  FORWARD_DECLARE(CollectModule, 0);
};
}  // namespace xstream
#endif  // HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_COLLECTMODULE_H_
