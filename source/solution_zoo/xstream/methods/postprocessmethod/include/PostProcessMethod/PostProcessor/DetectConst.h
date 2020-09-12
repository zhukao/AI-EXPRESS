/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: DetectConst.h
 * @Brief: declaration of the const var
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-25 16:59:31
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-25 17:22:45
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSOR_DETECTCONST_H_
#define POSTPROCESSMETHOD_POSTPROCESSOR_DETECTCONST_H_

#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include "horizon/vision_type/vision_type.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace xstream {

using hobot::vision::BBox;
using hobot::vision::Landmarks;
using hobot::vision::Feature;
using hobot::vision::Segmentation;
using hobot::vision::Pose3D;
using hobot::vision::Attribute;

enum class DetectBranchOutType {
  BBOX,
  APABBOX,
  MASK,
  INVALID
};

struct DetectBranchInfo {
  DetectBranchOutType type;
  std::string name;
  std::string box_name;
  std::unordered_map<int, std::string> labels;

  uint32_t output_off;           // Size in bytes 该层相对于输出起始位置的偏移量
  uint32_t valid_output_size;    // In bytes
  uint32_t aligned_output_size;  // In bytes
  uint32_t element_type_bytes;

  std::vector<int> real_nhwc;
  std::vector<int> aligned_nhwc;
  uint32_t shift;
};

struct DetectOutMsg {
  std::map<std::string, std::vector<BBox>> boxes;
  std::map<std::string, std::vector<Landmarks>> landmarks;
  std::map<std::string, std::vector<Feature>> features;
  std::map<std::string, std::vector<Segmentation>> segmentations;
  std::map<std::string, std::vector<Pose3D>> poses;
  std::map<std::string, std::vector<Attribute<int>>> attributes;
};

extern std::map<std::string, DetectBranchOutType> str2detect_out_type;

}  // namespace xstream
#endif  // POSTPROCESSMETHOD_POSTPROCESSOR_DETECTCONST_H_
