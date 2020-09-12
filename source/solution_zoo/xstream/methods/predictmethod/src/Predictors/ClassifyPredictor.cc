/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: ClassifyPredictor.cc
 * @Brief: definition of the ClassifyPredictor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-30 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-30 16:19:08
 */

#include "PredictMethod/Predictors/ClassifyPredictor.h"
#include <vector>
#include <memory>
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#ifdef X3
#include "./bpu_predict_x3.h"
#endif

using hobot::vision::BBox;

namespace xstream {

std::vector<std::vector<BaseDataPtr>> ClassifyPredictor::Do(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    const auto &frame_input = input[i];
    auto &frame_output = output[i];
    frame_output = RunSingleFrame(frame_input);
  }
  return output;
}

std::vector<BaseDataPtr> ClassifyPredictor::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input) {
  LOGD << "ClassifyPredictor RunSingleFrame";

  HOBOT_CHECK(frame_input.size() == 2);

  auto rois = std::static_pointer_cast<BaseDataVector>(frame_input[0]);
  auto xstream_img = std::static_pointer_cast<XStreamData<
      std::shared_ptr<hobot::vision::ImageFrame>>>(frame_input[1]);
  auto pyramid = std::static_pointer_cast<
        hobot::vision::PymImageFrame>(xstream_img->value);

  std::vector<BaseDataPtr> frame_output(1);
  auto async_data = std::make_shared<XStreamData<std::shared_ptr<
      hobot::vision::AsyncData>>>();
  frame_output[0] = async_data;
  async_data->value.reset(new hobot::vision::AsyncData());
  async_data->value->model_input_height = model_input_height_;
  async_data->value->model_input_width = model_input_width_;

  int box_num = rois->datas_.size();
  std::vector<int> valid_box(box_num, 1);
  std::vector<BPU_BBOX> boxes;
  for (int roi_idx = 0; roi_idx < box_num; roi_idx++) {
    auto &roi = rois->datas_[roi_idx];
    auto p_roi = std::static_pointer_cast<XStreamData<BBox>>(roi);
    if (p_roi->state_ != xstream::DataState::VALID) {
      valid_box[roi_idx] = 0;
    } else {
      boxes.push_back(BPU_BBOX{p_roi->value.x1,
                               p_roi->value.y1,
                               p_roi->value.x2,
                               p_roi->value.y2,
                               p_roi->value.score,
                               0,
                               true});
      LOGD << "box: " << p_roi->value;
    }
  }

  {
    RUN_PROCESS_TIME_PROFILER("Run Classify Model");
    RUN_FPS_PROFILER("Run Classify Model");

    int resizable_cnt = 0;
    if (boxes.size() <= 0) {
      LOGD << "no box to cnn";
      return frame_output;
    }

    // output_tensors
    std::shared_ptr<std::vector<BPU_TENSOR_S>> output_tensors =
        std::make_shared<std::vector<BPU_TENSOR_S>>();
    PrepareOutputTensor(output_tensors, boxes.size());

    std::shared_ptr<BPU_TASK_HANDLE> task_handle =
        std::make_shared<BPU_TASK_HANDLE>();
    BPU_RUN_CTRL_S run_ctrl{2};

#ifdef X2
    int ret = HB_BPU_runModelWithBbox(
        bpu_model_.get(),
        static_cast<BPU_CAMERA_BUFFER>(&pyramid->img),
        boxes.data(), boxes.size(), output_tensors->data(),
        bpu_model_->output_num, &run_ctrl,
        false, &resizable_cnt, task_handle.get());
#endif
#ifdef X3
    bpu_predict_x3::PyramidResult bpu_predict_pyramid;
    Convert(*pyramid, bpu_predict_pyramid);

    int ret = HB_BPU_runModelWithBbox(
        bpu_model_.get(),
        static_cast<BPU_CAMERA_BUFFER>(&bpu_predict_pyramid.result_info),
        boxes.data(), boxes.size(), output_tensors->data(),
        bpu_model_->output_num, &run_ctrl,
        true, &resizable_cnt, task_handle.get());
#endif
    if (ret != 0) {
      LOGE << "RunModelWithBBox failed, " << HB_BPU_getErrorName(ret);
      ReleaseTensor(output_tensors);
      // release BPU_TASK_HANDLE
      HB_BPU_releaseTask(task_handle.get());
      if (resizable_cnt == 0) {
        LOGI << "no box pass resizer";
      }
      return frame_output;
    }
    LOGD << "resizeable box:" << resizable_cnt;

    for (int i = 0, bpu_box_idx = 0; i < box_num; i++) {
      if (valid_box[i]) {
        valid_box[i] = boxes[bpu_box_idx].resizable;
        bpu_box_idx++;
      }
    }

#if 0
    {
      // test post-process
      int layer_size = 1;
      for (int32_t i = 0, mxnet_rlt_idx = 0; i < box_num; i++) {
        if (valid_box[i]) {
          std::vector<std::vector<int8_t>> mxnet_rlt(1);  // object i: layer
          for (int j = 0; j < layer_size; j++) {
            mxnet_rlt[j].resize(2);  // real_nhwc
            HB_SYS_flushMemCache(&(*output_tensors)[j].data,
                                 HB_SYS_MEM_CACHE_INVALIDATE);
            int output_size = 1;
            for (int dim = 0;
                 dim < bpu_model_->outputs[j].aligned_shape.ndim;
                 dim++) {
              output_size *= bpu_model_->outputs[j].aligned_shape.d[dim];
            }
            if (bpu_model_->outputs[j].data_type == BPU_TYPE_TENSOR_F32 ||
                bpu_model_->outputs[j].data_type == BPU_TYPE_TENSOR_S32 ||
                bpu_model_->outputs[j].data_type == BPU_TYPE_TENSOR_U32) {
              output_size *= 4;
            }

            int raw_idx = mxnet_rlt_idx * output_size;
            ConvertOutputToMXNet(reinterpret_cast<int8_t *>(
                                   (*output_tensors)[j].data.virAddr)+raw_idx,
                                 mxnet_rlt[j].data(), j);
            // test
            auto mxnet_out =
                reinterpret_cast<const float *>(mxnet_rlt[0].data());
            std::cout << mxnet_out[0] << std::endl;
          }
          mxnet_rlt_idx++;
        }
      }
      ReleaseTensor(output_tensors);
    }
#endif
    // output_tensors传递给async_data
    async_data->value->input_tensors = nullptr;  // resizer model无input_tensor
    async_data->value->output_tensors = output_tensors;
    async_data->value->task_handle = task_handle;
    async_data->value->bpu_model = bpu_model_;
    async_data->value->valid_box = valid_box;
    return frame_output;
  }
}

#if 0
void ClassifyPredictor::ConvertOutputToMXNet(
    void *src_ptr, void *dest_ptr, int out_index) {
  auto &real_shape = bpu_model_->outputs[out_index].shape;
  auto elem_size = 4;  // TODO(zhe.sun) 是否可判断float

  auto shift = bpu_model_->outputs[out_index].shifts;

  uint32_t dst_n_stride = 2 * elem_size;
  uint32_t dst_h_stride = 2 * elem_size;
  uint32_t dst_w_stride = 2 * elem_size;
  uint32_t src_n_stride = 8 * elem_size;
  uint32_t src_h_stride = 8 * elem_size;
  uint32_t src_w_stride = 8 * elem_size;

  float tmp_float_value;
  int32_t tmp_int32_value;

  for (int nn = 0; nn < real_shape.d[0]; nn++) {
    void *cur_n_dst = reinterpret_cast<int8_t *>(dest_ptr) + nn * dst_n_stride;
    void *cur_n_src = reinterpret_cast<int8_t *>(src_ptr) + nn * src_n_stride;
    for (int hh = 0; hh < real_shape.d[1]; hh++) {
      void *cur_h_dst =
          reinterpret_cast<int8_t *>(cur_n_dst) + hh * dst_h_stride;
      void *cur_h_src =
          reinterpret_cast<int8_t *>(cur_n_src) + hh * src_h_stride;
      for (int ww = 0; ww < real_shape.d[2]; ww++) {
        void *cur_w_dst =
            reinterpret_cast<int8_t *>(cur_h_dst) + ww * dst_w_stride;
        void *cur_w_src =
            reinterpret_cast<int8_t *>(cur_h_src) + ww * src_w_stride;
        for (int cc = 0; cc < real_shape.d[3]; cc++) {
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

float ClassifyPredictor::GetFloatByInt(int32_t value, uint32_t shift) {
  float ret_x = value;
  if (value != 0) {
    int *ix = reinterpret_cast<int *>(&ret_x);
    (*ix) -= shift * 0x00800000;
  }
  return ret_x;
}
#endif
}  // namespace xstream
