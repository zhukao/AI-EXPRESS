/*
 * @Description: implement of data_type
 * @Author: ruoting.ding@horizon.ai
 * @Date: 2019-11-15 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-16 19:13:49
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include "hobotxstream/method_factory.h"

#include <iostream>
#include <string>

#include "MergeMethod/MergeMethod.h"

namespace xstream {
namespace method_factory {
MethodPtr CreateMethod(const std::string &method_name) {
  LOGD << "CreateMethod:" << method_name;
  if ("MergeMethod" == method_name) {
    return MethodPtr(new MergeMethod());
  } else {
    return MethodPtr();
  }
}
}  // namespace method_factory
}  // namespace xstream
