/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: detect_const.h
 * @Brief: definition of the const var
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-25 16:59:31
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-25 17:22:45
 */

#include "PostProcessMethod/PostProcessor/DetectConst.h"
#include <map>
#include <string>

namespace xstream {

std::map<std::string, DetectBranchOutType> str2detect_out_type =
    {{"bbox", DetectBranchOutType::BBOX},
     {"apabbox", DetectBranchOutType::APABBOX},
     {"mask", DetectBranchOutType::MASK}};

}  // namespace xstream
