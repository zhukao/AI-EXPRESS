/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: RectInputPredictor.cpp
 * @Brief: definition of the RectInputPredictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-16 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-16 16:23:27
 */

#include "CNNMethod/Predictor/RectInputPredictor.h"
#include <algorithm>
#include <memory>
#include <vector>
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"

using hobot::vision::BBox;
using hobot::vision::ImageFrame;
using hobot::vision::PymImageFrame;
typedef std::shared_ptr<ImageFrame> ImageFramePtr;

namespace xstream {

void RectInputPredictor::Do(CNNMethodRunData *run_data) {
  int frame_size = run_data->input->size();
  run_data->mxnet_output.resize(frame_size);
  run_data->input_dim_size.resize(frame_size);
  run_data->norm_rois.resize(frame_size);
  run_data->real_nhwc = model_info_.real_nhwc_;
  run_data->elem_size = model_info_.elem_size_;
  run_data->all_shift = model_info_.all_shift_;

  for (int frame_idx = 0; frame_idx < frame_size; frame_idx++) {  // loop frame
    auto &input_data = (*(run_data->input))[frame_idx];

    int ret = 0;
    auto rois = std::static_pointer_cast<BaseDataVector>(input_data[0]);
    auto xstream_pyramid =
        std::static_pointer_cast<XStreamData<ImageFramePtr>>(input_data[1]);
    auto pyramid = std::static_pointer_cast<PymImageFrame>(
                   xstream_pyramid->value);

    int32_t box_num = rois->datas_.size();
    run_data->input_dim_size[frame_idx] = box_num;
    std::vector<int> valid_box(box_num, 1);
    run_data->mxnet_output[frame_idx].resize(box_num);
    run_data->norm_rois[frame_idx].resize(box_num);

    auto &norm_rois = run_data->norm_rois[frame_idx];

    std::vector<BPU_BBOX> boxes;
    int32_t handle_num =
        max_handle_num_ < 0 ? box_num : std::min(max_handle_num_, box_num);
    for (int32_t roi_idx = 0; roi_idx < box_num; roi_idx++) {
      auto &roi = rois->datas_[roi_idx];
      auto p_roi = std::static_pointer_cast<XStreamData<BBox>>(roi);
      auto p_norm_roi = std::make_shared<XStreamData<BBox>>();
      norm_rois[roi_idx] = std::static_pointer_cast<BaseData>(p_norm_roi);
      p_norm_roi->value = p_roi->value;
      if (p_roi->state_ != xstream::DataState::VALID ||
          roi_idx >= handle_num) {
        valid_box[roi_idx] = 0;
      } else {
        boxes.push_back(BPU_BBOX{p_roi->value.x1,
                                p_roi->value.y1,
                                p_roi->value.x2,
                                p_roi->value.y2,
                                p_roi->value.score,
                                0,
                                true});
        LOGD << "box {" << p_roi->value.x1 << "," << p_roi->value.y1 << ","
             << p_roi->value.x2 << "," << p_roi->value.y2 << "}";
      }
    }
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_runmodel")
      RUN_FPS_PROFILER(model_name_ + "_runmodel")
      int resizable_cnt = 0;
      if (boxes.size() <= 0) {
        LOGD << "no box to cnn";
        return;
      }
      ret = RunModelWithBBox(*pyramid, boxes.data(),
                             boxes.size(), &resizable_cnt);
      if (ret == -1) {
        return;
      }
      for (int32_t i = 0, bpu_box_idx = 0; i < box_num; i++) {
        if (valid_box[i]) {
          valid_box[i] = boxes[bpu_box_idx].resizable;
          if (valid_box[i]) {
            auto p_norm_roi =
                std::static_pointer_cast<XStreamData<BBox>>(norm_rois[i]);
            p_norm_roi->value.x1 = boxes[bpu_box_idx].x1;
            p_norm_roi->value.y1 = boxes[bpu_box_idx].y1;
            p_norm_roi->value.x2 = boxes[bpu_box_idx].x2;
            p_norm_roi->value.y2 = boxes[bpu_box_idx].y2;
          }
          bpu_box_idx++;
        }
      }
    }

    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_do_hbrt")
      RUN_FPS_PROFILER(model_name_ + "_do_hbrt")
      // change raw data to mxnet layout
      int layer_size = model_info_.output_layer_size_.size();
      for (int32_t i = 0, mxnet_rlt_idx = 0; i < box_num; i++) {
        if (valid_box[i]) {
          auto &mxnet_rlt = run_data->mxnet_output[frame_idx][i];
          mxnet_rlt.resize(layer_size);
          for (int j = 0; j < layer_size; j++) {
            mxnet_rlt[j].resize(model_info_.mxnet_output_layer_size_[j]);
            HB_SYS_flushMemCache(&(output_tensors_[j].data),
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
                                   output_tensors_[j].data.virAddr)+raw_idx,
                                 mxnet_rlt[j].data(), j);
          }
          mxnet_rlt_idx++;
        }
      }
      // ReleaseOutputTensor();
    }
  }
}
}  // namespace xstream
