/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     method factory
 * @file      method_fatory.h
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */
#ifndef HOBOTXSTREAM_METHOD_FACTORY_H_
#define HOBOTXSTREAM_METHOD_FACTORY_H_

#include <string>
#include <memory>

#include "hobotxstream/method.h"
#include "hobotxsdk/xstream_data.h"

namespace xstream {

class MethodFactory {
 public:
  /// Method 工厂方法
  virtual MethodPtr CreateMethod(const std::string &method_name) = 0;
};

typedef std::shared_ptr<MethodFactory> MethodFactoryPtr;

}  // namespace xstream

#endif  // HOBOTXSTREAM_METHOD_FACTORY_H_
