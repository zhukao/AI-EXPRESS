/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CNNConst.h
 * @Brief: declaration of the const var
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-17 14:52:31
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-17 16:22:44
 */

#ifndef INCLUDE_CNNCONST_H_
#define INCLUDE_CNNCONST_H_

#include <map>
#include <string>
#include <vector>

namespace xstream {
enum class NormMethod {
  BPU_MODEL_NORM_BY_WIDTH_LENGTH,
  BPU_MODEL_NORM_BY_WIDTH_RATIO,
  BPU_MODEL_NORM_BY_HEIGHT_RATIO,
  BPU_MODEL_NORM_BY_LSIDE_RATIO,
  BPU_MODEL_NORM_BY_HEIGHT_LENGTH,
  BPU_MODEL_NORM_BY_LSIDE_LENGTH,
  BPU_MODEL_NORM_BY_LSIDE_SQUARE,
  BPU_MODEL_NORM_BY_DIAGONAL_SQUARE,
  BPU_MODEL_NORM_BY_WIDTH_SQUARE,
  BPU_MODEL_NORM_BY_HEIGHT_SQUARE,
  BPU_MODEL_NORM_BY_NOTHING
};

enum class FilterMethod {
  OUT_OF_RANGE,
  NO_FILTER
};

extern const std::map<std::string, NormMethod> g_norm_method_map;

extern const std::map<std::string, FilterMethod> g_filter_method_map;

extern const std::vector<float> g_lmk_template;

extern const std::vector<int> g_age_range;

}  // namespace xstream
#endif  // INCLUDE_CNNCONST_H_
