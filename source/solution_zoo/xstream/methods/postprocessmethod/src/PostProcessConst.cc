/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PostProcessConst.cc
 * @Brief: definition of the const var
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-27 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-27 16:01:54
 */

#include "PostProcessMethod/PostProcessConst.h"
#include <map>
#include <string>

namespace xstream {

const std::map<std::string, PostProcessType> g_postprocess_type_map = {
    {"detect", PostProcessType::DETECT},
    {"classify", PostProcessType::CLASSIFY}};

}  // namespace xstream
