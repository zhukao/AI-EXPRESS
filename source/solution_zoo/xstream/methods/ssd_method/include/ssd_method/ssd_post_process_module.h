//
// Copyright 2019 Horizon Robotics.
//

#ifndef INCLUDE_SSDMETHOD_SSD_POST_PROCESS_MODULES_H
#define INCLUDE_SSDMETHOD_SSD_POST_PROCESS_MODULES_H

#include <iostream>
#include <string>
#include <vector>

#include "bpu_predict/bpu_predict_extension.h"
#include "hobotxstream/method.h"
#include "ssd_method/ssd_result_post_process.h"
using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
namespace xstream {
class SSDPostProcessModule {
 public:
  SSDPostProcessModule(const std::string &modelname, BPU_MODEL_S bpu_handle,
                       int pyr_level, std::string input_platform,
                       float score_threshold, float nms_threshold,
                       const std::string ssd_params_file,
                       const std::vector<bool> output_layer_big_endian,
                       const std::vector<int> model_out_type) {
    bpu_handle_ = bpu_handle;
    pyr_level_ = pyr_level;
    score_threshold_ = (score_threshold == 0) ? 0.3 : score_threshold;
    model_name_ = modelname;
    input_platform_ = input_platform;
    nms_threshold_ = (nms_threshold == 0) ? 0.45 : nms_threshold;
    is_tf_ = false;
    ssd_params_file_ = ssd_params_file;
    output_layer_big_endian_.assign(output_layer_big_endian.begin(),
                                    output_layer_big_endian.end());
    model_out_type_.assign(model_out_type.begin(), model_out_type.end());
  }
  int Init();

  void Reset();

 private:
  std::string model_name_;
  BPU_MODEL_S bpu_handle_;
  std::string input_platform_;
  std::vector<std::vector<Anchor>> anchors_table_;
  SsdConfig ssd_conf_;
  uint32_t num_layer_;
  float score_threshold_;
  float nms_threshold_;
  int pyr_level_;
  bool is_tf_;
  std::string ssd_params_file_;
  std::vector<bool> output_layer_big_endian_;
  std::vector<int> model_out_type_;
  // PERF_MEMBER

 public:
  void GetFrameOutput(const std::vector<BPU_TENSOR_S> &input_buf,
                      BaseDataPtr frame_output);
};
}  // namespace xstream
#endif  // INCLUDE_SSDMETHOD_SSD_POST_PROCESS_MODULES_H
