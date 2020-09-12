/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-24 21:06:46
 * @Version: v0.0.1
 * @Brief: Default Dispatch Module, dispatch by RoundRobin.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 21:13:18
 */

#ifndef HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_DEFAULTDISPATCHMODULE_H_
#define HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_DEFAULTDISPATCHMODULE_H_
#include <string>
#include "base_modules/DispatchModule.h"

namespace xstream {
class DefaultDispatchModule : public DispatchModule {
 public:
  explicit DefaultDispatchModule(std::string instance_name = "")
      : DispatchModule(instance_name, "xstream::DefaultDispatchModule") {}
  /**
   * Input Message:
   *  0: InputParam, type: XStreamInputParamMessage
   *  1- : Node inputs, type:XStreamInputMessage
   * Output Message:
   *  0: PassThrough Message,type:XStreamMethodOutputMessage
   *  1- : MethodModule.size() * Method Input Message,
   * type:XStreamMethodInputMessage
   */
  FORWARD_DECLARE(DefaultDispatchModule, 0);
};
}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_DEFAULTDISPATCHMODULE_H_
