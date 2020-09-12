/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PreProcessor.cpp
 * @Brief: definition of the PreProcessor
 * @Author: zhe.sun
 * @Date: 2020-01-10 11:41:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-01-10 11:41:17
 */

#include "CNNPostProcessor/CNNPostProcessor.h"
#include <string>
#include <vector>
#include <map>
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"

namespace xstream {
namespace CnnProc {
DECLARE_MethodCreator(lmkpose_post)
DECLARE_MethodCreator(agegender_post)
DECLARE_MethodCreator(antispf_post)
DECLARE_MethodCreator(quality_post)
DECLARE_MethodCreator(feature_post)
DECLARE_MethodCreator(plate_num_post)
DECLARE_MethodCreator(vehicle_color_post)
DECLARE_MethodCreator(vehicle_type_post)
DECLARE_MethodCreator(classify_post)

static std::map<std::string, std::function<MethodPtr()>>
  s_cnnmethod_post_registry = {
  { "CNNLmkposePostMethod",  CnnProc::lmkpose_post_creator},
  { "CNNAgegenderPostMethod",  CnnProc::agegender_post_creator},
  { "CNNAntispfPostMethod",  CnnProc::antispf_post_creator},
  { "CNNQualityPostMethod",  CnnProc::quality_post_creator},
  { "CNNFeaturePostMethod",  CnnProc::feature_post_creator},
  { "CNNPlateNumPostMethod",  CnnProc::plate_num_post_creator},
  { "CNNVehicleColorPostMethod",  CnnProc::vehicle_color_post_creator},
  { "CNNVehicleTypePostMethod",  CnnProc::vehicle_type_post_creator},
  { "CNNClassifyPostMethod",  CnnProc::classify_post_creator}
};

bool CNNPostProcessQuery(const std::string& method_name) {
  if (s_cnnmethod_post_registry.count(method_name)) {
    return true;
  } else {
    return false;
  }
}

MethodPtr CNNPostProcessCreate(const std::string& method_name) {
  HOBOT_CHECK(s_cnnmethod_post_registry.count(method_name))
    << "CNNPostProcessCreate error: " << method_name << std::endl;
  return s_cnnmethod_post_registry[method_name]();
}

std::mutex CNNPostProcessor::init_mutex_;
int32_t CNNPostProcessor::Init(const std::string &cfg_path) {
  std::ifstream infile(cfg_path.c_str());
  HOBOT_CHECK(infile.good()) << "CNNMethod error config file path:" << cfg_path;

  std::stringstream buffer;
  buffer << infile.rdbuf();
  config_.reset(new CNNMethodConfig(buffer.str()));
  config_->SetSTDStringValue("parent_path", get_parent_path(cfg_path));
  std::unique_lock<std::mutex> lock(init_mutex_);

  output_slot_size_ = config_->GetIntValue("output_size");
  model_name_ = config_->GetSTDStringValue("model_name");
  HOBOT_CHECK(output_slot_size_ > 0);
  HOBOT_CHECK(model_name_.size() > 0) << "must set model_name";

  return 0;
}

int CNNPostProcessor::UpdateParameter(xstream::InputParamPtr ptr) {
  if (ptr->is_json_format_) {
    std::string content = ptr->Format();
    CNNMethodConfig cf(content);
    UpdateParams(cf.config, config_->config);
    return 0;
  } else {
    HOBOT_CHECK(0) << "only support json format config";
    return -1;
  }
}

InputParamPtr CNNPostProcessor::GetParameter() const {
  return std::static_pointer_cast<xstream::InputParam>(config_);
}

std::string CNNPostProcessor::GetVersion() const {
  return "";
}

void CNNPostProcessor::ConvertOutputToMXNet(void *src_ptr,
                                     void *dest_ptr,
                                     int layer_idx) {
  auto &aligned_nhwc = model_info_->aligned_nhwc_[layer_idx];
  auto &real_nhwc = model_info_->real_nhwc_[layer_idx];
  auto elem_size = model_info_->elem_size_[layer_idx];
  auto &shift = model_info_->all_shift_[layer_idx];

  uint32_t dst_n_stride =
      real_nhwc[1] * real_nhwc[2] * real_nhwc[3] * elem_size;
  uint32_t dst_h_stride = real_nhwc[2] * real_nhwc[3] * elem_size;
  uint32_t dst_w_stride = real_nhwc[3] * elem_size;
  uint32_t src_n_stride =
      aligned_nhwc[1] * aligned_nhwc[2] * aligned_nhwc[3] * elem_size;
  uint32_t src_h_stride = aligned_nhwc[2] * aligned_nhwc[3] * elem_size;
  uint32_t src_w_stride = aligned_nhwc[3] * elem_size;
  float tmp_float_value;
  int32_t tmp_int32_value;
  for (uint32_t nn = 0; nn < real_nhwc[0]; nn++) {
    void *cur_n_dst = reinterpret_cast<int8_t *>(dest_ptr) + nn * dst_n_stride;
    void *cur_n_src = reinterpret_cast<int8_t *>(src_ptr) + nn * src_n_stride;
    for (uint32_t hh = 0; hh < real_nhwc[1]; hh++) {
      void *cur_h_dst =
          reinterpret_cast<int8_t *>(cur_n_dst) + hh * dst_h_stride;
      void *cur_h_src =
          reinterpret_cast<int8_t *>(cur_n_src) + hh * src_h_stride;
      for (uint32_t ww = 0; ww < real_nhwc[2]; ww++) {
        void *cur_w_dst =
            reinterpret_cast<int8_t *>(cur_h_dst) + ww * dst_w_stride;
        void *cur_w_src =
            reinterpret_cast<int8_t *>(cur_h_src) + ww * src_w_stride;
        for (uint32_t cc = 0; cc < real_nhwc[3]; cc++) {
          void *cur_c_dst =
              reinterpret_cast<int8_t *>(cur_w_dst) + cc * elem_size;
          void *cur_c_src =
              reinterpret_cast<int8_t *>(cur_w_src) + cc * elem_size;
          if (elem_size == 4) {
            tmp_int32_value = *(reinterpret_cast<int32_t *>(cur_c_src));
            tmp_float_value = GetFloatByInt(tmp_int32_value, shift[cc]);
            *(reinterpret_cast<float *>(cur_c_dst)) = tmp_float_value;
          } else {
            *(reinterpret_cast<int8_t *>(cur_c_dst)) =
                *(reinterpret_cast<int8_t *>(cur_c_src));
          }
        }
      }
    }
  }
}

void CNNPostProcessor::getBPUResult(
  std::shared_ptr<CNNPredictorOutputData> run_data) {
  std::size_t frame_size = run_data->input.size();
  auto bpu_handle_ = run_data->bpu_handle_;
  model_info_ = run_data->md_info;
  model_name_ = run_data->model_name_;
  std::size_t layer_size = model_info_->output_layer_size_.size();
  mxnet_output_.resize(frame_size);

  feature_bufs_.resize(model_info_->output_layer_size_.size());
  for (std::size_t i = 0; i < model_info_->output_layer_size_.size(); i++) {
    feature_bufs_[i].resize(model_info_->output_layer_size_[i]);
  }

  targets_valid_flag.resize(frame_size);
  for (size_t frame_idx = 0; frame_idx < frame_size; frame_idx++) {
    uint32_t target_nums = run_data->target_nums[frame_idx];
    targets_valid_flag[frame_idx].resize(target_nums);
    for (size_t target_id = 0; target_id < target_nums; target_id++) {
        targets_valid_flag[frame_idx][target_id] = false;
    }
  }

  for (size_t frame_idx = 0; frame_idx < frame_size; frame_idx++) {
    uint32_t handle_num = run_data->model_handle_[frame_idx].size();
    int target_num = run_data->target_nums[frame_idx];
    mxnet_output_[frame_idx].resize(target_num);
    auto valid_targets_per_frame = run_data->valid_targets[frame_idx];
    int target_idx = 0;
    for (size_t handle_idx = 0; handle_idx < handle_num; handle_idx++) {
            auto model_handle =
              run_data->model_handle_[frame_idx][handle_idx];
            // get BPU output
            int  ret = BPU_getModelOutput(bpu_handle_, model_handle);
            if (ret != 0) {
                LOGE << "BPU_getModelOutput failed:"
                << BPU_getLastError(bpu_handle_);
                BPU_releaseModelHandle(bpu_handle_, model_handle);
            }
            if (run_data->input_type == InputType::FAKEIMAGE) {
                BPU_releaseFakeImage(run_data->bpu_fakeimage_handle_,
                &(run_data->fake_image_ptr_[frame_idx][handle_idx]));
            }
            BPU_releaseModelHandle(bpu_handle_, model_handle);
            // End bpu output
            {
            RUN_PROCESS_TIME_PROFILER(model_name_ + "_do_hbrt")
            RUN_FPS_PROFILER(model_name_ + "_do_hbrt")
            // change raw data to mxnet layout
            std::size_t bufs_size =
    run_data->out_bufs_[frame_idx][handle_idx].out_bufs_.size()/layer_size;
            for (size_t i = 0, mxnet_rlt_idx = 0; i < bufs_size; i++) {
                auto real_target_id = valid_targets_per_frame[target_idx];
                targets_valid_flag[frame_idx][real_target_id] = true;
                {
                    auto &mxnet_rlt = mxnet_output_[frame_idx][real_target_id];
                    mxnet_rlt.resize(layer_size);
                for (size_t j = 0; j < layer_size; j++) {
                    int raw_rlt_idx = mxnet_rlt_idx * layer_size + j;
                    mxnet_rlt[j].resize(
                    model_info_->mxnet_output_layer_size_[j]);
                    BPU_convertLayout(bpu_handle_, feature_bufs_[j].data(),
                                    BPU_getRawBufferPtr(
    run_data->out_bufs_[frame_idx][handle_idx].out_bufs_[raw_rlt_idx]),
                             model_name_.c_str(), BPULayoutType::LAYOUT_NHWC,
                                    j, -1, -1, -1, -1);
                    ConvertOutputToMXNet(
                        feature_bufs_[j].data(), mxnet_rlt[j].data(), j);
                }
                mxnet_rlt_idx++;
            }
            target_idx++;
            }
          }
    }
  }
}
}  // namespace CnnProc
}  // namespace xstream
