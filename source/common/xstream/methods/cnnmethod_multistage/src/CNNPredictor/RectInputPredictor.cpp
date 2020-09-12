/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: RectInputPredictor.cpp
 * @Brief: declaration of the RectInputPredictor
 * @Author: zhe.sun
 * @Date: 2020-01-20 11:26:50
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-01-20 11:26:50
 */

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "CNNPredictor/RectInputPredictor.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"

using hobot::vision::BBox;
using hobot::vision::ImageFrame;
using hobot::vision::PymImageFrame;
typedef std::shared_ptr<ImageFrame> ImageFramePtr;

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(rect_pre, RectInputPredictor())

std::vector<std::vector<BaseDataPtr>>
RectInputPredictor::DoProcess(
    const std::vector<std::vector<BaseDataPtr> > &input,
    const std::vector<xstream::InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> result(1);
  std::shared_ptr<CNNPredictorOutputData> run_data =
      std::make_shared<CNNPredictorOutputData>();
  result[0].push_back(run_data);

  run_data->md_info = &model_info_;
  run_data->input = input;
  run_data->input_type = InputType::PYRAMID;

  int frame_size = input.size();
  run_data->target_nums.resize(frame_size);
  run_data->targets_data.resize(frame_size);

  run_data->model_handle_.resize(frame_size);
  run_data->out_bufs_.resize(frame_size);
  run_data->valid_targets.resize(frame_size);

  run_data->model_name_ = model_name_;
  run_data->bpu_handle_ = bpu_handle_;
  run_data->bpu_fakeimage_handle_ = fake_img_handle_;

  for (int frame_idx = 0; frame_idx < frame_size; frame_idx++) {
    auto &input_data = input[frame_idx];

    int ret = 0;
    auto rois = std::static_pointer_cast<BaseDataVector>(input_data[0]);
    auto xstream_pyramid =
        std::static_pointer_cast<XStreamData<ImageFramePtr>>(input_data[1]);
    auto pyramid =
    std::static_pointer_cast<PymImageFrame>(xstream_pyramid->value);

    uint32_t box_num = rois->datas_.size();
    run_data->target_nums[frame_idx] = box_num;
    run_data->targets_data[frame_idx].resize(box_num);
    run_data->out_bufs_[frame_idx].resize(1);
    run_data->model_handle_[frame_idx].resize(1);
    auto &norm_rois = run_data->targets_data[frame_idx];

    std::vector<BPUBBox> boxes;
    uint32_t handle_num =
        max_handle_num_ < 0
          ? box_num :
          std::min(max_handle_num_, static_cast<int32_t>(box_num));
    for (uint32_t roi_idx = 0; roi_idx < box_num; roi_idx++) {
      auto &roi = rois->datas_[roi_idx];
      auto p_roi = std::static_pointer_cast<XStreamData<BBox>>(roi);
      auto p_norm_roi = std::make_shared<XStreamData<BBox>>();
      norm_rois[roi_idx] = std::static_pointer_cast<BaseData>(p_norm_roi);
      p_norm_roi->value = p_roi->value;
      if (p_roi->state_ != xstream::DataState::VALID
          || roi_idx >= handle_num) {
      } else {
        run_data->valid_targets[frame_idx].push_back(roi_idx);
        boxes.push_back(BPUBBox{p_roi->value.x1,
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
    ModelOutputBuffer bufs(model_info_, boxes.size());
    {
      RUN_PROCESS_TIME_PROFILER(model_name_ + "_runmodel")
      RUN_FPS_PROFILER(model_name_ + "_runmodel")
      int resizable_cnt = 0;
      BPUModelHandle model_handle;
      ret = RunModelFromResizer(
          *pyramid,
          boxes.data(),
          boxes.size(),
          &resizable_cnt,
          bufs.out_bufs_.data(),
          boxes.size() * model_info_.output_layer_size_.size(),
          &model_handle);
      if (ret == -1) {
        return result;
      }
      run_data->model_handle_[frame_idx][0] = model_handle;
      for (uint32_t i = 0, bpu_box_idx = 0; i < box_num; i++) {
      std::vector<int>::iterator it =
      find(run_data->valid_targets[frame_idx].begin(),
          run_data->valid_targets[frame_idx].end(), i);
          if (it != run_data->valid_targets[frame_idx].end()) {
          if (boxes[bpu_box_idx].resizable) {
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
    run_data->out_bufs_[frame_idx][0] = bufs;
  }
  return result;
}
}  // namespace CnnProc
}  // namespace xstream
