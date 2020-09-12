/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PredictConst.h
 * @Brief: declaration of the const var
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-27 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-27 16:01:54
 */

#ifndef PREDICTMETHOD_PREDICTCONST_H_
#define PREDICTMETHOD_PREDICTCONST_H_

#include <map>
#include <string>

namespace xstream {

enum class PredictType {
  DETECT,
  CLASSIFY
};

extern const std::map<std::string, PredictType> g_predict_type_map;

}  // namespace xstream
#endif  // PREDICTMETHOD_PREDICTCONST_H_
