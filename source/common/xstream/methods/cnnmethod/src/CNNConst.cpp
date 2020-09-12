/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CNNConst.h
 * @Brief: definition of the const var
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-17 14:52:31
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-17 16:22:44
 */

#include "CNNMethod/CNNConst.h"
#include <map>
#include <string>
#include <vector>

namespace xstream {

const std::map<std::string, InputType> g_input_type_map = {
    {"rect", InputType::RECT},
    {"lmk", InputType::LMK_IMG},
    {"img", InputType::IMG},
    {"plate_num_img", InputType::PLATE_NUM_IMG},
    {"lmk_seq", InputType::LMK_SEQ},
    {"vid", InputType::VID}};

const std::map<std::string, PostFun> g_post_fun_map = {
    {"face_feature", PostFun::FACE_ID},
    {"antispoofing", PostFun::ANTI_SPF},
    {"face_quality", PostFun::FACE_QUALITY},
    {"binary_classify", PostFun::BINARYCLASSIFY},
    {"lmk_pose", PostFun::LMK_POSE},
    {"age_gender", PostFun::AGE_GENDER},
    {"vehicle_type", PostFun::VEHICLE_TYPE},
    {"vehicle_color", PostFun::VEHICLE_COLOR},
    {"plate_num", PostFun::PLATE_NUM},
    {"classify", PostFun::CLASSIFY},
    {"act_det", PostFun::ACT_DET},
    {"back_bone", PostFun::BACK_BONE},
    {"vid", PostFun::VID},
    {"common_lmk", PostFun::COMMON_LMK}};

const std::map<std::string, NormMethod> g_norm_method_map = {
    {"norm_by_width_length", NormMethod::BPU_MODEL_NORM_BY_WIDTH_LENGTH},
    {"norm_by_width_ratio", NormMethod::BPU_MODEL_NORM_BY_WIDTH_RATIO},
    {"norm_by_height_rario", NormMethod::BPU_MODEL_NORM_BY_HEIGHT_RATIO},
    {"norm_by_lside_ratio", NormMethod::BPU_MODEL_NORM_BY_LSIDE_RATIO},
    {"norm_by_height_length", NormMethod::BPU_MODEL_NORM_BY_HEIGHT_LENGTH},
    {"norm_by_lside_length", NormMethod::BPU_MODEL_NORM_BY_LSIDE_LENGTH},
    {"norm_by_lside_square", NormMethod::BPU_MODEL_NORM_BY_LSIDE_SQUARE},
    {"norm_by_diagonal_square", NormMethod::BPU_MODEL_NORM_BY_DIAGONAL_SQUARE},
    {"norm_by_width_square", NormMethod::BPU_MODEL_NORM_BY_WIDTH_SQUARE},
    {"norm_by_height_square", NormMethod::BPU_MODEL_NORM_BY_HEIGHT_SQUARE},
    {"norm_by_nothing", NormMethod::BPU_MODEL_NORM_BY_NOTHING}};

const std::map<std::string, FilterMethod> g_filter_method_map = {
    {"out_of_range", FilterMethod::OUT_OF_RANGE},
    {"no_filter", FilterMethod::NO_FILTER}};

const std::map<std::string, LmkSeqOutputType> g_lmkseq_output_map = {
    {"fall", LmkSeqOutputType::FALL},
    {"gesture", LmkSeqOutputType::GESTURE}};

const std::map<std::string, BPU_DATA_TYPE_E> g_data_type_map = {
    {"img_y", BPU_DATA_TYPE_E::BPU_TYPE_IMG_Y},
    {"img_yuv_nv12", BPU_DATA_TYPE_E::BPU_TYPE_IMG_YUV_NV12},
    {"img_yuv444", BPU_DATA_TYPE_E::BPU_TYPE_IMG_YUV444},
    {"img_bgr", BPU_DATA_TYPE_E::BPU_TYPE_IMG_BGR},
    {"img_bgrp", BPU_DATA_TYPE_E::BPU_TYPE_IMG_BGRP},
    {"img_rgb", BPU_DATA_TYPE_E::BPU_TYPE_IMG_RGB},
    {"img_rgbp", BPU_DATA_TYPE_E::BPU_TYPE_IMG_RGBP},
    {"img_nv12_separate", BPU_DATA_TYPE_E::BPU_TYPE_IMG_NV12_SEPARATE},
    {"tensor_u8", BPU_DATA_TYPE_E::BPU_TYPE_TENSOR_U8},
    {"tensor_s8", BPU_DATA_TYPE_E::BPU_TYPE_TENSOR_S8},
    {"tensor_f32", BPU_DATA_TYPE_E::BPU_TYPE_TENSOR_F32},
    {"tensor_s32", BPU_DATA_TYPE_E::BPU_TYPE_TENSOR_S32},
    {"tensor_u32", BPU_DATA_TYPE_E::BPU_TYPE_TENSOR_U32},
    {"max", BPU_DATA_TYPE_E::BPU_TYPE_MAX}
};

const std::vector<float> g_lmk_template = {
    38.2946f, 51.6963f, 73.5318f, 51.5014f, 56.0252f,
    71.7366f, 41.5493f, 92.3655f, 70.7299f, 92.2041f};

const std::vector<int> g_age_range = {1,  6,  7,  12, 13, 18, 19, 28,
                                      29, 35, 36, 45, 46, 55, 56, 100};

}  // namespace xstream
