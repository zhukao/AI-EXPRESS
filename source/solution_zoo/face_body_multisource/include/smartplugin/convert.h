/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-02 03:42:16
 * @Version: v0.0.1
 * @Brief:  convert to xroc inputdata from input VioMessage
 * @Note:  extracted from xperson repo.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-29 02:57:24
 */
#ifndef INCLUDE_SMARTPLUGIN_CONVERT_H_
#define INCLUDE_SMARTPLUGIN_CONVERT_H_

#include "hobotxsdk/xstream_data.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace iot {
using horizon::vision::xproto::basic_msgtype::VioMessage;

class Convertor {
 public:
  /**
   * @brief convert input VioMessage to xroc inputdata.
   * @param input input VioMessage
   * @return xstream::InputDataPtr xroc input
   */
  static xstream::InputDataPtr ConvertInput(const VioMessage *input);
};

}  // namespace iot
}  // namespace horizon

#endif  //  INCLUDE_SMARTPLUGIN_CONVERT_H_
