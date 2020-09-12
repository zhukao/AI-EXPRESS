/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PostProcessConst.h
 * @Brief: declaration of the const var
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-27 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-27 16:01:54
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSCONST_H_
#define POSTPROCESSMETHOD_POSTPROCESSCONST_H_

#include <map>
#include <string>

namespace xstream {

enum class PostProcessType {
  DETECT,
  CLASSIFY
};

extern const std::map<std::string, PostProcessType> g_postprocess_type_map;

}  // namespace xstream
#endif  // POSTPROCESSMETHOD_POSTPROCESSCONST_H_
