/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      method_factory.cc
 * @brief     MethodFactory class implementation
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */

#include "xstream/tutorials/stage3/method_factory.h"
#include "xstream/tutorials/stage3/method/passthrough_method.h"
#include "xstream/tutorials/stage3/method/thread_safety_method.h"

namespace xstream {
namespace method_factory {

MethodPtr CreateMethod(const std::string &method_name) {
  if ("ThreadSafetyMethod" == method_name) {
    return MethodPtr(new ThreadSafetyMethod());
  } else if ("PassthroughMethod" == method_name) {
    return MethodPtr(new PassthroughMethod());
  } else {
    return MethodPtr();
  }
}

}  // namespace method_factory
}  // namespace xstream
