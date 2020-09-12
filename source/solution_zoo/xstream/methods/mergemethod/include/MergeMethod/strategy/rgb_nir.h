/*
 * @Description: implement of data_type
 * @Author: yutong.pan@horizon.ai
 * @Date: 2019-11-22
 * @Copyright 2015~2019 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MERGEMETHOD_STRATEGY_RGB_NIR_H_
#define INCLUDE_MERGEMETHOD_STRATEGY_RGB_NIR_H_
#ifdef RGB_NIR_MERGE
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "MergeMethod/data_type/data_type.h"
#include "MergeMethod/strategy/head_face.h"
#include "dual_camera_algo/transformer.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"
#ifdef EEPROM_ENABLED
#include "./eeprom_layout.h"
#else
#include "data_type/eeprom_header.h"
#endif

namespace xstream {

struct RGBNIRParam : MergeParam {
 public:
  RGBNIRParam() = default;
  int UpdateParameter(const JsonReaderPtr &reader) override;
  // camera type: 0 is landscape, 1 is portrait
  int camera_type = 0;
};

class RGBNIRStrategy : public HeadFaceStrategy {
 public:
  int Init(std::shared_ptr<MergeParam> config) override;

  std::vector<BaseDataPtr> ProcessFrame(const std::vector<BaseDataPtr> &in,
                                        const InputParamPtr &param) override;

  void Finalize() override;

  int UpdateParameter(const JsonReaderPtr &reader) override;

 private:
  std::shared_ptr<RGBNIRParam> GetConfig();

  // 这两个函数合并到ProcessFrame中
  void RunSingleRGBFrame(const std::vector<BaseDataPtr> &frame_input,
                         std::vector<BaseDataPtr> *frame_output);

  void ProduceOutput(const BaseDataPtr &facePtr, const BaseDataPtr &headPtr,
                     const BaseDataVectorPtr &faceResPtr,
                     const BaseDataVectorPtr &headResPtr,
                     const BaseDataPtr &rgb_landmarks,
                     const BaseDataPtr &nir_landmarks);

  void MergeRGBNIRTrackID(const BaseDataPtr &face_bbox,
                          const BaseDataPtr &head_bbox,
                          const BaseDataPtr &face_lmk,
                          const BaseDataPtr &head_lmk,
                          const std::set<int> &face_id_set,
                          const std::set<int> &head_id_set,
                          IDRelationInfo *id_merge_info);

  static std::vector<std::pair<int, int>> GetMatchedPairs(
      const BaseDataPtr &face_bbox, const BaseDataPtr &head_bbox,
      const std::vector<int> &face_pts);

 private:
  cv::Mat transfer_F_;
  char calib_buff_[CAM_CALIB_DATA_LENGTH];
  IDRelationInfo id_relation_info_;
};
}  // namespace xstream
#endif  //  RGB_NIR_MERGE
#endif  // INCLUDE_MERGEMETHOD_STRATEGY_RGB_NIR_H_
