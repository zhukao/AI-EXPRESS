//
// Copyright 2019 Horizon Robotics.
//

#include "ssd_method/ssd_predictor.h"

#include <sys/time.h>

#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>

#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "ssd_method/ssd_post_process_module.h"
#include "bpu_predict/bpu_predict_extension.h"
#include "ssd_method/Utils.h"

#ifdef X3
#include "./bpu_predict_x3.h"
#endif

using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using hobot::vision::PymImageFrame;

namespace xstream {

const char *kPyramidImage = "PymImageFrame";
class ModelOutputBuffer {
 public:
  ModelOutputBuffer(const BPUModelInfo &output_info,
                    std::vector<BPU_Buffer_Handle> &output_buf)
      : output_buf_(output_buf) {
    // alloc output buffer.
    for (int i = 0; i < output_info.num; ++i) {
      BPU_Buffer_Handle out_handle = BPU_createEmptyBPUBuffer();
      output_buf_.push_back(out_handle);
    }
    LOGD << "create bpu buffer success.";
  }
  ~ModelOutputBuffer() {
    // release output buffer.
    for (auto &buf : output_buf_) {
      BPU_freeBPUBuffer(buf);
    }
    output_buf_.clear();
    LOGD << "release bpu buffer success.";
  }

 private:
  std::vector<BPU_Buffer_Handle> &output_buf_;
};

SSD_Predictor::~SSD_Predictor() {
  if (bpu_handle_.handle != NULL) {
    HB_BPU_releaseModel(&bpu_handle_);
  }
}

int SSD_Predictor::Init(const std::string &config_file) {
  ParseConfig(config_file);

  int ret_val = LoadModel(model_file_path_, &bpu_handle_);
  if (0 != ret_val) {
    LOGI << "load bpu model " << model_file_path_
               << " failed : " << HB_BPU_getErrorName(ret_val);
    return -1;
  }
  // PrintModelInfo(&bpu_handle_);
  out_num_ = bpu_handle_.output_num;
  GetModelInfo(model_name_);

  ssd_post_process_.reset(new xstream::SSDPostProcessModule(
      model_name_, bpu_handle_, pyramid_layer_, platform_, score_thld_,
      nms_thld_, config_file, output_layer_big_endian_, model_out_types_));
  int r = ssd_post_process_->Init();
  if (r != 0) {
    LOGF << "ssd_post_process failed" << std::endl;
    return 1;
  }
  return 0;
}

void SSD_Predictor::RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                                   std::vector<BaseDataPtr> &frame_output) {
  // only one input slot -> PyramidImage
  HOBOT_CHECK(frame_input.size() == 1);
  const auto frame_img_ = frame_input[0];

  for (size_t out_index = 0; out_index < method_outs_.size(); ++out_index) {
    frame_output.push_back(std::make_shared<xstream::BaseDataVector>());
  }
  auto xstream_img =
      std::static_pointer_cast<XStreamData<ImageFramePtr>>(frame_img_);

  std::string img_type = xstream_img->value->type;
  // BPU_TENSOR_S tensor;
  {
    RUN_PROCESS_TIME_PROFILER("SSD RunModelFromPyramid");
    RUN_FPS_PROFILER("SSD RunModelFromPyramid");
    int ret = 0;
    if (img_type == kPyramidImage) {
      auto pyramid_image =
          std::static_pointer_cast<PymImageFrame>(xstream_img->value);
      // prepare input
      std::vector<BPU_TENSOR_S> input_tensors;
      input_tensors.resize(1);
#ifdef X3
      Image2BPUTensor(*pyramid_image, pyramid_layer_, input_tensors[0]);
#endif
#ifdef X2
      Image2BPUTensor(pyramid_image->img, pyramid_layer_, input_tensors[0]);
#endif
      // prepare output
      std::vector<BPU_TENSOR_S> output_tensors;
      PrepareOutputTensors(&bpu_handle_, output_tensors);
      BPU_RUN_CTRL_S run_ctrl_s{1};
      BPU_TASK_HANDLE task_handle;
      ret = HB_BPU_runModel(
          &bpu_handle_, input_tensors.data(), 1, output_tensors.data(),
          bpu_handle_.output_num, &run_ctrl_s, true,
          &task_handle);
      if (ret != 0) {
        LOGE << "runmodule failed : " << HB_BPU_getErrorName(ret);
        return;
      }
      ssd_post_process_->GetFrameOutput(output_tensors, frame_output[0]);

      ReleaseImageInputTensors(input_tensors);
      ReleaseBPUBuffer(output_tensors);
    } else {
      HOBOT_CHECK(0) << "Not support this input type: " << img_type;
    }
  }

  RUN_PROCESS_TIME_PROFILER("SSD PostProcess");
  RUN_FPS_PROFILER("SSD PostProcess");
}
void SSD_Predictor::ParseConfig(const std::string &config_file) {
  FR_Config cfg_jv;
  std::ifstream infile(config_file.c_str());
  infile >> cfg_jv;
  config_.reset(new Config(cfg_jv));

  auto net_info = config_->GetSubConfig("net_info");
  score_thld_ = config_->GetFloatValue("score_threshold", 0.0);
  nms_thld_ = config_->GetFloatValue("nms_threshold", 0.3);
  platform_ = config_->GetSTDStringValue("input_platform", "mxnet");

  model_name_ = net_info->GetSTDStringValue("model_name");
  model_version_ = net_info->GetSTDStringValue("model_version", "1.0.0");
  model_input_width_ = net_info->GetIntValue("model_input_width", 960);
  model_input_height_ = net_info->GetIntValue("model_input_height", 540);
  pyramid_layer_ = net_info->GetIntValue("pyramid_layer", 4);
  model_out_types_ = net_info->GetIntArray("model_out_type");

  method_outs_ = config_->GetSTDStringArray("method_outs");
  LOGD << "method out type:";
  for (const auto &method_out : method_outs_) {
    LOGD << method_out;
  }

  std::string parent_path = GetParentPath(config_file);
  bpu_config_path_ =
      parent_path + config_->GetSTDStringValue("bpu_config_path");
  model_file_path_ =
      parent_path + config_->GetSTDStringValue("model_file_path");
  LOGD << "config file parent path: " << parent_path
       << " bpu_config_path: " << bpu_config_path_
       << " model_file_path: " << model_file_path_;
}

void SSD_Predictor::GetModelInfo(const std::string &model_name) {
  uint32_t output_layer_num = bpu_handle_.output_num;
  LOGD << "output_layer_num = " << output_layer_num
       << ", output_num = " << out_num_ << std::endl;
  output_layer_big_endian_.clear();
  output_layer_big_endian_.resize(output_layer_num);
  for (size_t i = 0; i < output_layer_num; ++i) {
    // get bit_endian
    output_layer_big_endian_.push_back(false);
  }
}

std::string SSD_Predictor::GetParentPath(const std::string &path) {
  auto pos = path.rfind('/');
  if (std::string::npos != pos) {
    auto parent = path.substr(0, pos);
    return parent + "/";
  } else {
    return std::string("./");
  }
}
}  // namespace xstream

