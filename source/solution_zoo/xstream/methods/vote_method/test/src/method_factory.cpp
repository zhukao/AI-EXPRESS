
/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief: vote method factory
 * @author : tangji.sun
 * @email : tangji.sun@horizon.ai
 * @date: 2019-11-04
 */

#include "hobotxstream/method_factory.h"

#include <iostream>
#include <string>
#include "vote_method/vote_method.h"
namespace xstream {
namespace method_factory {
MethodPtr CreateMethod(const std::string &method_name) {
  std::cout << "CreateMethod:" << method_name << std::endl;
  if ("VoteMethod" == method_name) {
    return MethodPtr(new VoteMethod());
  } else {
    return MethodPtr();
  }
}
}  // namespace method_factory
}  // namespace xstream
