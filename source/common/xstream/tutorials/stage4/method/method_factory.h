/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      method_factory.h
 * @brief     MethodFactory class definition
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-01-03
 */

#ifndef XSTREAM_TUTORIALS_STAGE4_METHOD_FACTORY_H_
#define XSTREAM_TUTORIALS_STAGE4_METHOD_FACTORY_H_

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

#endif  // XSTREAM_TUTORIALS_STAGE4_METHOD_FACTORY_H_
