//
// Copyright 2019 Horizon Robotics.
//

#ifndef INCLUDE_SSDMETHOD_SSD_PREDICTOR_H
#define INCLUDE_SSDMETHOD_SSD_PREDICTOR_H

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "bpu_predict/bpu_predict_extension.h"
#include "hobot_vision/bpu_model_manager.hpp"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.hpp"
#include "ssd_method/config.h"
#include "ssd_method/ssd_post_process_module.h"

namespace xstream {
class SSD_Predictor {
 public:
  SSD_Predictor() {}
  virtual ~SSD_Predictor();

  int Init(const std::string &config_file);
  void RunSingleFrame(const std::vector<xstream::BaseDataPtr> &frame_input,
                      std::vector<xstream::BaseDataPtr> &frame_output);

 private:
  void ParseConfig(const std::string &config_file);
  void GetModelInfo(const std::string &model_name);
  std::string GetParentPath(const std::string &path);

 private:
  std::shared_ptr<xstream::SSDPostProcessModule> ssd_post_process_;
  std::string model_name_;
  std::string platform_;
  int out_num_;
  std::string bpu_config_path_;
  std::string model_file_path_;
  std::string model_version_;
  float score_thld_;
  float nms_thld_;
  int pyramid_layer_;
  std::vector<int> model_out_types_;
  BPU_MODEL_S bpu_handle_;
  std::vector<BPU_Buffer_Handle> out_buf_;
  BPUModelInfo output_info_;
  std::vector<std::string> method_outs_;

  std::shared_ptr<Config> config_;
  int model_input_width_;
  int model_input_height_;
  std::vector<bool> output_layer_big_endian_;
};
}  // namespace xstream
#endif  // INCLUDE_SSDMETHOD_SSD_PREDICTOR_H
