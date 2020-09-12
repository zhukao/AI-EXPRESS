/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      method_factory.cc
 * @brief     MethodFactory class implementation
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020/01/03
 */

#include "method/method_factory.h"
#include "method/bbox_filter.h"
#include "method/postbox_filter.h"


namespace xstream {
namespace method_factory {

MethodPtr CreateMethod(const std::string &method_name) {
  if ("BBoxFilter" == method_name) {
    return MethodPtr(new BBoxFilter());
  } else if ("PostBoxFilter" == method_name) {
    return MethodPtr(new PostBoxFilter());
  } else {
    return MethodPtr();
  }
}

}  // namespace method_factory
}  // namespace xstream
