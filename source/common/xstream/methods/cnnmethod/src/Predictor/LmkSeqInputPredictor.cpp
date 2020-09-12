/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: LmkSeqInputPredictor.cpp
 * @Brief: definition of the LmkSeqInputPredictor
 * @Author: shiyu.fu
 * @Email: shiyu.fu@horizon.ai
 * @Date: 2020-05-25
 */

#include <algorithm>
#include <memory>
#include <vector>
#include <chrono>
#include "CNNMethod/Predictor/LmkSeqInputPredictor.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"

using hobot::vision::BBox;
using hobot::vision::Landmarks;
using hobot::vision::Points;
using hobot::vision::Attribute;
using hobot::vision::ImageFrame;
using hobot::vision::PymImageFrame;
typedef std::shared_ptr<ImageFrame> ImageFramePtr;

namespace xstream {

void InputFloat2Int(int8_t* feat_buffer1, int input_quanti_factor,
                    float* BPU_input_data, int N, int H, int W, int C,
                    int frame_input_size) {
  int32_t tmp = 0;
  int index = 0;
  for (int n = 0; n < N; ++n) {
    for (int h = 0; h < H; ++h) {
      for (int w = 0; w < W; ++w) {
        for (int c = 0; c < C; ++c) {
          int offset = n * H * W * C + h * W * C + w * C + c;
          tmp = floor(BPU_input_data[offset] * input_quanti_factor);
          if (tmp > 127) {
            tmp = 127;
          } else if (tmp < -128) {
            tmp = -128;
          }
          feat_buffer1[index] = static_cast<int8_t>(tmp);
          index++;
        }
      }
    }
  }
  HOBOT_CHECK(index == frame_input_size);
}

int32_t LmkSeqInputPredictor::Init(std::shared_ptr<CNNMethodConfig> config) {
  // TODO(shiyu.fu): do not need to create fake image handle
  Predictor::Init(config);

  seq_len_ = config->GetIntValue("seq_len");
  kps_len_ = config->GetIntValue("kps_len");
  buf_len_ = config->GetIntValue("buf_len");
  input_shift_ = config->GetIntValue("input_shift");
  stride_ = config->GetFloatValue("stride");
  max_gap_ = config->GetFloatValue("max_gap");
  kps_norm_scale_ = config->GetFloatValue("kps_norm_scale");
  norm_kps_conf_ = config->GetBoolValue("norm_kps_conf");
  std::string s_out_type = config->GetSTDStringValue("output_type");
  auto out_type = g_lmkseq_output_map.find(s_out_type);
  HOBOT_CHECK(out_type != g_lmkseq_output_map.end())
      << "output type " << s_out_type << " not support";
  output_type_ = out_type->second;

  data_processor_.Instance().Init(kps_len_, seq_len_, stride_, max_gap_,
                                  kps_norm_scale_, norm_kps_conf_, buf_len_);
  return 0;
}

void LmkSeqInputPredictor::UpdateParam(
                           std::shared_ptr<CNNMethodConfig> config) {
  Predictor::UpdateParam(config);
  if (config->KeyExist("expand_scale")) {
    norm_params_.expand_scale = config->GetFloatValue("expand_scale");
  }
  if (config->KeyExist("seq_len")) {
    seq_len_ = config->GetIntValue("seq_len");
  }
  if (config->KeyExist("kps_len")) {
    kps_len_ = config->GetIntValue("kps_len");
  }
  if (config->KeyExist("stride")) {
    stride_ = config->GetFloatValue("stride");
  }
  if (config->KeyExist("max_gap")) {
    max_gap_ = config->GetFloatValue("max_gap");
  }
  if (config->KeyExist("buf_len")) {
    buf_len_ = config->GetIntValue("buf_len");
  }
  if (config->KeyExist("kps_norm_scale")) {
    kps_norm_scale_ = config->GetFloatValue("buf_len");
  }
}

void LmkSeqInputPredictor::Do(CNNMethodRunData *run_data) {
  static float timestamp_;

  int frame_size = run_data->input->size();
  run_data->mxnet_output.resize(frame_size);
  run_data->input_dim_size.resize(frame_size);
  run_data->real_nhwc = model_info_.real_nhwc_;
  run_data->elem_size = model_info_.elem_size_;
  run_data->all_shift = model_info_.all_shift_;

  for (int frame_idx = 0; frame_idx < frame_size; frame_idx++) {  // loop frame
    auto &input_data = (*(run_data->input))[frame_idx];
    // timestamp_ += stride_;
    auto cur_microsec = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
    float cur_sec = cur_microsec / 1000000.0;
    timestamp_ = cur_sec;
    LOGD << "cur_microsec: " << cur_microsec << ", cur_sec: " << cur_sec;
    float *timestamp_context = new float(timestamp_);
    run_data->context = timestamp_context;

    auto rois = std::static_pointer_cast<BaseDataVector>(input_data[0]);
    auto kpses = std::static_pointer_cast<BaseDataVector>(input_data[1]);
    auto disappeared_track_ids =
        std::static_pointer_cast<BaseDataVector>(input_data[2]);

    int box_num = rois->datas_.size();
    run_data->mxnet_output[frame_idx].resize(box_num);
    run_data->input_dim_size[frame_idx] = box_num;

    int input_n = model_info_.input_nhwc_[0];
    int input_h = model_info_.input_nhwc_[1];
    int input_w = model_info_.input_nhwc_[2];
    int input_c = model_info_.input_nhwc_[3];
    int input_size = input_n * input_h * input_w * input_c;

    int handle_num =
        max_handle_num_ < 0 ? box_num : std::min(max_handle_num_, box_num);
    int enable_dump = 0;
    auto dump_input_feat_env = getenv("dump_input_feature");
    if (dump_input_feat_env) {
      enable_dump = std::stoi(dump_input_feat_env);
    }
    if (enable_dump) {
      std::fstream outfile;
      outfile.open("lmkseq_input_feature.txt", std::ios_base::app);
      outfile << "[";
    }
    for (int roi_idx = 0; roi_idx < box_num; ++roi_idx) {
      auto p_roi =
        std::static_pointer_cast<XStreamData<BBox>>(rois->datas_[roi_idx]);
      auto p_kps =
        std::static_pointer_cast<XStreamData<Landmarks>>(
            kpses->datas_[roi_idx]);
      if (p_roi->state_ != xstream::DataState::VALID ||
          p_kps->value.values.size() != static_cast<uint32_t>(kps_len_) ||
          roi_idx >= handle_num) {
        continue;
      } else {
        // pre-process
        int8_t* feature_buf_ = nullptr;
        {
          RUN_PROCESS_TIME_PROFILER(model_name_ + "_preprocess")
          RUN_FPS_PROFILER(model_name_ + "_preprocess")
          data_processor_.Instance().Update(p_roi, p_kps, timestamp_);
          auto cached_kpses = std::make_shared<BaseDataVector>();
          data_processor_.Instance().GetClipKps(cached_kpses, p_roi->value.id,
                                                timestamp_, output_type_);
          // concatenate
          if (cached_kpses->datas_.size() < static_cast<size_t>(seq_len_)) {
            if (enable_dump) {
              std::fstream outfile;
              outfile.open("lmkseq_input_feature.txt", std::ios_base::app);
              std::stringstream sstream;
              sstream << "[]";
              if (roi_idx != box_num - 1 || roi_idx != handle_num - 1) {
                sstream << ", ";
              }
              outfile << sstream.str().c_str();
            }
            continue;
          } else {
            Tensor tensor(input_n, input_h, input_w, input_c);
            for (int nn = 0; nn < input_n; ++nn) {
              for (int hh = 0; hh < input_h; ++hh) {
                for (int ww = 0; ww < input_w; ++ww) {
                  int cached_data_idx = input_w == seq_len_ ? ww : hh;
                  int kps_idx = input_h == kps_len_ ? hh : ww;
                  auto cur_kps =
                      std::static_pointer_cast<XStreamData<Landmarks>>(
                          cached_kpses->datas_[cached_data_idx]);
                  tensor.Set(nn, hh, ww, 0, cur_kps->value.values[kps_idx].x);
                  tensor.Set(nn, hh, ww, 1, cur_kps->value.values[kps_idx].y);
                  tensor.Set(nn, hh, ww, 2,
                             cur_kps->value.values[kps_idx].score);
                }
              }
            }
            if (enable_dump) {
              tensor.Display(roi_idx, box_num, handle_num);
            }
            float *BPU_input_data = tensor.data.data();
            feature_buf_ = static_cast<int8_t*>(malloc(input_size));
            int input_quanti_factor = 1 << input_shift_;
            InputFloat2Int(feature_buf_, input_quanti_factor, BPU_input_data,
                           input_n, input_h, input_w, input_c, input_size);
          }
        }
        // run model
        {
          RUN_PROCESS_TIME_PROFILER(model_name_ + "_runmodel")
          RUN_FPS_PROFILER(model_name_ + "_runmodel")
          int ret = RunModel(reinterpret_cast<uint8_t *>(feature_buf_),
                             input_size,
                             model_info_.data_type_);
          if (ret != 0) {
            LOGE << "Failed to RunModelFromDDR";
            free(feature_buf_);
            continue;
          }
        }
        LOGD << "RunModelFromDDR success";
        free(feature_buf_);
        // convert to mxnet out
        uint32_t layer_size = model_info_.output_layer_size_.size();
        auto &one_tgt_mxnet = run_data->mxnet_output[frame_idx][roi_idx];
        one_tgt_mxnet.resize(layer_size);
        {
          RUN_PROCESS_TIME_PROFILER(model_name_ + "_do_hbrt")
          RUN_FPS_PROFILER(model_name_ + "_do_hbrt")
          for (uint32_t j = 0; j < layer_size; j++) {
            uint32_t feature_size = model_info_.mxnet_output_layer_size_[j];
            one_tgt_mxnet[j].resize(feature_size);
            HB_SYS_flushMemCache(&(output_tensors_[j].data),
                                 HB_SYS_MEM_CACHE_INVALIDATE);
            ConvertOutputToMXNet(output_tensors_[j].data.virAddr,
                                 one_tgt_mxnet[j].data(), j);
          }
          // release output tensor
          // ReleaseOutputTensor();
        }
        LOGD << "do hbrt success";
      }
    }
    if (enable_dump) {
      std::fstream outfile;
      outfile.open("lmkseq_input_feature.txt", std::ios_base::app);
      outfile << "]\n";
    }

    data_processor_.Instance().Clean(disappeared_track_ids);
  }
}

}  // namespace xstream
