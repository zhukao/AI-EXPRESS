/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-16 14:18:28
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-06-06 15:18:28
 */
#include "CNNMethod/Predictor/VidPredictor.h"
#include <iostream>
#include <algorithm>
#include <memory>
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type_common.h"

using hobot::vision::BBox;
using hobot::vision::ImageFrame;
using hobot::vision::PymImageFrame;
typedef std::shared_ptr<ImageFrame> ImageFramePtr;
typedef xstream::XStreamData<std::string> XRocString;
typedef std::shared_ptr<XRocString> XRocStringPtr;

namespace xstream {

int cvtTo8p4c_f(int n0, int h0, int w0, int c0, float* input, char* output,
                int shift_bits) {
  int native_offset = 0;
  int new_offset = 0;
  int W = 8;
  int C = 4;
  int dim_n = n0;
  int dim_h = h0;
  int dim_w = w0 / W;
  int dim_c = c0 / C;
  int block_sz = h0 * w0 * c0;
  int line_sz = w0 * c0;
  int sub_line_sz = C * w0;
  int8_t* outptr = reinterpret_cast<int8_t*>(output);
  for (int n = 0; n < dim_n; n++) {  // NHCW
    for (int h = 0; h < dim_h; h++) {
      for (int c = 0; c < dim_c; c++) {
        for (int w = 0; w < dim_w; w++) {
          // 8P4C
          for (int ww = 0; ww < W; ww++) {
            for (int cc = 0; cc < C; cc++) {
              native_offset =
                  n * block_sz + h * line_sz + (w * W + ww) * c0 + c * C + cc;
              new_offset = n * block_sz + h * line_sz + c * sub_line_sz +
                  w * W * C + ww * C + cc;
              int temp = floor(input[native_offset] * (1 << shift_bits));
              if (temp > 127) {
                temp = 127;
              } else if (temp < -128) {
                temp = -128;
              }
              outptr[new_offset] = (int8_t)temp;
            }
          }
        }
      }
    }
  }
  return 0;
}

VidInputPredictor::VidInputPredictor() {
  LOGD << "VidInputPredictor Constructor";
}

static std::vector<float> ddr_input(8 * 8 * 256, 0.0);

void VidInputPredictor::Do(CNNMethodRunData *run_data) {
  static int count = 0;
  LOGD << "vidInputPredictor cout = " << count++;
  int batch_size = run_data->input->size();
  run_data->mxnet_output.resize(batch_size);
  run_data->input_dim_size.resize(batch_size);
  run_data->norm_rois.resize(batch_size);
  run_data->real_nhwc = model_info_.real_nhwc_;
  run_data->elem_size = model_info_.elem_size_;
  run_data->all_shift = model_info_.all_shift_;

  int ddr_input_size =
      1 * 8 * 8 * 256;  // 8p4c layout, 1*8*1*256 align to 1*8*8*256
  HOBOT_CHECK(batch_size == 1);
  HOBOT_CHECK(run_data->input_dim_size.size() == 1);

  run_data->output.resize(batch_size);

  for (int batch_i = 0; batch_i < batch_size; batch_i++) {  // loop frame
    auto &batch_output = run_data->output[batch_i];
    auto base_data_vector = std::make_shared<BaseDataVector>();
    batch_output.push_back(base_data_vector);
    run_data->input_dim_size[0] = 1;
    auto &batch_i_input_data = (*(run_data->input))[batch_i];
    std::vector<float> frame_input;
    HOBOT_CHECK(!batch_i_input_data.empty());
    auto slot0_input_ptr =
        std::static_pointer_cast<BaseDataVector>(batch_i_input_data[0]);
    if (slot0_input_ptr->datas_.empty()) {
      std::cout << "empty slot0_input_ptr" << std::endl;
      continue;
    }
    auto first_item = slot0_input_ptr->datas_[0];
    auto input_ptr =
        std::static_pointer_cast<XStreamData<HorizonVisionFloatArray>>(
            first_item);
    for (size_t i = 0; i < input_ptr->value.num; ++i) {
      frame_input.emplace_back(*(input_ptr->value.values + i));
    }
    std::vector<float> tmp_ddr_input(ddr_input_size);
    tmp_ddr_input.resize(8 * 8 * 256);
    memcpy(tmp_ddr_input.data(), ddr_input.data() + 8 * 256,
           sizeof(float) * 7 * 8 * 256);
    memcpy(tmp_ddr_input.data() + 7 * 8 * 256, frame_input.data(),
           sizeof(float) * 256);
    ddr_input = tmp_ddr_input;
    std::vector<int8_t> cvt_output;
    cvt_output.resize(ddr_input_size);
    cvtTo8p4c_f(1, 8, 8, 256, ddr_input.data(),
                reinterpret_cast<char *>(cvt_output.data()), 7);

    {
      int ret = RunModel(reinterpret_cast<uint8_t *>(cvt_output.data()),
                         ddr_input_size,
                         model_info_.data_type_, true);
      if (ret != 0) {
        return;
      }
      LOGD << "RunModel Success";
      // change raw data to mxnet layout
      int layer_size = model_info_.output_layer_size_.size();
      run_data->mxnet_output[batch_i].resize(1);
      auto &one_tgt_mxnet = run_data->mxnet_output[batch_i][0];
      one_tgt_mxnet.resize(layer_size);
      //      std::string socket_str[] = {"stand", "run", "jump", "squat",
      //      "walk", "attack", "defence"};
      std::string socket_str[] = {"other", "stand", "run", "attack"};
      XRocStringPtr xroc_msg(new XRocString());
      HOBOT_CHECK(layer_size == 1);
      for (int j = 0; j < layer_size; j++) {
        one_tgt_mxnet[j].resize(model_info_.mxnet_output_layer_size_[j]);
        HB_SYS_flushMemCache(&(output_tensors_[j].data),
                             HB_SYS_MEM_CACHE_INVALIDATE);
        ConvertOutputToMXNet(output_tensors_[j].data.virAddr,
                             one_tgt_mxnet[j].data(), j);
        auto size = run_data->real_nhwc[j][0] * run_data->real_nhwc[j][1] *
                    run_data->real_nhwc[j][2] * run_data->real_nhwc[j][3];
        int ret_index = 0;
        float value_f, maxvalue_f = -99999;
        for (size_t index = 0; index < size; ++index) {
          value_f = *(reinterpret_cast<float *>(
              one_tgt_mxnet[j].data() + index * run_data->elem_size[j]));
          if (maxvalue_f < value_f) {
            ret_index = index;
            maxvalue_f = value_f;
          }
          std::cout << value_f << " ";
        }
        xroc_msg->value = socket_str[ret_index];
        std::cout << "result  = " << xroc_msg->value << std::endl;
        base_data_vector->datas_.emplace_back(xroc_msg);
      }
      // release output tensor
      // ReleaseOutputTensor();
    }
  }
}
}  // namespace xstream
