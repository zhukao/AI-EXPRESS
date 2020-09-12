/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: DetectPostProcessor.h
 * @Brief: declaration of the DetectPostProcessor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-28 15:17:48
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-28 18:57:24
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSOR_DETECTPOSTPROCESSOR_H_
#define POSTPROCESSMETHOD_POSTPROCESSOR_DETECTPOSTPROCESSOR_H_

#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <map>
#include <utility>
#include <memory>
#include "PostProcessMethod/PostProcessor/PostProcessor.h"
#include "PostProcessMethod/PostProcessor/DetectConst.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class DetectPostProcessor : public PostProcessor {
 public:
  DetectPostProcessor() {}
  virtual ~DetectPostProcessor() {}

  virtual int Init(const std::string &cfg);

  std::vector<std::vector<BaseDataPtr>> Do(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

 private:
  void GetModelInfo();
  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
      std::vector<BaseDataPtr> &frame_output);
  void PostProcess(DetectOutMsg &det_result);
  void GetResultMsg(
      DetectOutMsg &det_result,
      std::map<std::string, std::shared_ptr<BaseDataVector>>
        &xstream_det_result);

  void CoordinateTransOutMsg(
      DetectOutMsg &det_result,
      int src_image_width, int src_image_height,
      int model_input_width, int model_input_hight);

  void ParseDetectionBox(
      void* result, int branch_num, void* anchor,
      std::vector<BBox> &boxes);
  void LocalIOU(std::vector<BBox> &candidates,
                std::vector<BBox> &result,
                const float overlap_ratio,
                const int top_N,
                const bool addScore);
  void ParseDetectionMask(
      void* result, int branch_num, std::vector<Segmentation> &masks);
  void ParseAPADetectionBox(
      void* result, int branch_num,
      std::vector<BBox> &boxes);
 private:
  std::map<int, DetectBranchInfo> out_level2branch_info_;
  std::vector<std::string> method_outs_;

  bool is_init_ = false;

  float iou_threshold_ = 0.2;
};
}  // namespace xstream

#endif  // POSTPROCESSMETHOD_POSTPROCESSOR_DETECTPOSTPROCESSOR_H_
