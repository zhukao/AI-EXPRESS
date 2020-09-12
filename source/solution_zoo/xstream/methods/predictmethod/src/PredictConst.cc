/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PredictConst.cc
 * @Brief: definition of the const var
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-27 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-27 16:01:54
 */

#include "PredictMethod/PredictConst.h"
#include <map>
#include <string>

namespace xstream {

const std::map<std::string, PredictType> g_predict_type_map = {
    {"detect", PredictType::DETECT},
    {"classify", PredictType::CLASSIFY}};

}  // namespace xstream
