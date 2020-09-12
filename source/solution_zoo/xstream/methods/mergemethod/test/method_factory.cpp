/*
 * @Description: implement of data_type
 * @Author: yaoyao.sun@horizon.ai
 * @Date: 2019-4-12 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:34:15
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include "hobotxstream/method_factory.h"
#include <string>
#include <iostream>
#include "MergeMethod/MergeMethod.h"

namespace xstream {
namespace method_factory {
MethodPtr CreateMethod(const std::string &method_name) {
  std::cout << "CreateMethod:" << method_name << std::endl;
  if ("MergeMethod" == method_name) {
    return MethodPtr(new MergeMethod());
  } else {
    return MethodPtr();
  }
}
}  // namespace method_factory
}  // namespace xstream
