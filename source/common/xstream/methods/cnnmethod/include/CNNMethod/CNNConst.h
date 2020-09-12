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

#ifndef INCLUDE_CNNMETHOD_CNNCONST_H_
#define INCLUDE_CNNMETHOD_CNNCONST_H_

#include <map>
#include <string>
#include <vector>
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

enum class InputType { NONE, RECT, LMK_IMG, IMG, PLATE_NUM_IMG, LMK_SEQ, VID };

enum class PostFun {
  FACE_ID,
  ANTI_SPF,
  LMK_POSE,
  AGE_GENDER,
  FACE_QUALITY,
  BINARYCLASSIFY,
  VEHICLE_TYPE,
  VEHICLE_COLOR,
  PLATE_NUM,
  CLASSIFY,
  ACT_DET,
  BACK_BONE,
  VID,
  COMMON_LMK
};

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

enum class LmkSeqOutputType {
  FALL,
  GESTURE
};

enum class FilterMethod { OUT_OF_RANGE, NO_FILTER };

extern const std::map<std::string, InputType> g_input_type_map;

extern const std::map<std::string, PostFun> g_post_fun_map;

extern const std::map<std::string, NormMethod> g_norm_method_map;

extern const std::map<std::string, FilterMethod> g_filter_method_map;

extern const std::map<std::string, LmkSeqOutputType> g_lmkseq_output_map;

extern const std::map<std::string, BPU_DATA_TYPE_E> g_data_type_map;

extern const std::vector<float> g_lmk_template;

extern const std::vector<int> g_age_range;

}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_CNNCONST_H_
