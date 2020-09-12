/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: method_factory.cpp
 * @Brief: definition of the method_factory
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-26 11:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-26 12:17:08
 */

#include "hobotxstream/method_factory.h"
#include <string>
#include <iostream>
#include "BehaviorMethod/BehaviorMethod.h"

namespace xstream {
namespace method_factory {
MethodPtr CreateMethod(const std::string &method_name) {
  std::cout << "CreateMethod:" << method_name << std::endl;
  if ("BehaviorMethod" == method_name) {
    return MethodPtr(new BehaviorMethod());
  } else {
    return MethodPtr();
  }
}
}  // namespace method_factory
}  // namespace xstream
