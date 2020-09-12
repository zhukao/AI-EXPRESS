/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ImgInputPredictor.cpp
 * @Brief: definition of the ImgInputPredictor
 * @Author: ronghui.zhang
 * @Date: 2020-01-20 14:27:05
 * @Last Modified by: ronghui.zhang
 * @Last Modified time: 2020-01-20 16:23:27
 */

#include "CNNPredictor/ImgInputPredictor.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/image_tools.h"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "opencv2/opencv.hpp"

using hobot::vision::BBox;
using hobot::vision::ImageFrame;
using hobot::vision::PymImageFrame;
typedef std::shared_ptr<ImageFrame> ImageFramePtr;

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(img_pre, ImgInputPredictor())

int ImgInputPredictor::Init(const std::string &cfg_path) {
  CNNPredictor::Init(cfg_path);
  norm_params_.expand_scale = config_->GetFloatValue("expand_scale", 1.0f);
  std::string s_norm_method = config_->GetSTDStringValue("norm_method");
  auto iter = g_norm_method_map.find(s_norm_method);
  HOBOT_CHECK(iter != g_norm_method_map.end())
      << "norm_method is unknown:" << s_norm_method;
  norm_params_.norm_type = iter->second;

  norm_params_.aspect_ratio = config_->GetFloatValue("aspect_ratio", 1.0f);

  std::string s_filter_method =
      config_->GetSTDStringValue("filter_method", "no_filter");
  auto filter_iter = g_filter_method_map.find(s_filter_method);
  HOBOT_CHECK(filter_iter != g_filter_method_map.end())
      << "filter_method is unknown:" << s_filter_method;
  filter_method_ = filter_iter->second;

  rotate_degree_ = config_->GetIntValue("rotate_degree");
  return 0;
}

std::vector<std::vector<BaseDataPtr>>
ImgInputPredictor::DoProcess(
    const std::vector<std::vector<BaseDataPtr> > &input,
    const std::vector<xstream::InputParamPtr> &param) {
  LOGE <<" ImgInputPredictor::DoProcess ";
  std::vector<std::vector<BaseDataPtr>> result(1);
  std::shared_ptr<CNNPredictorOutputData> run_data_ =
      std::make_shared<CNNPredictorOutputData>();
  result[0].push_back(run_data_);
  run_data_->input = input;
  run_data_->md_info = &model_info_;
  run_data_->input_type = InputType::FAKEIMAGE;

  std::size_t frame_size = run_data_->input.size();
  run_data_->target_nums.resize(frame_size);
  run_data_->targets_data.resize(frame_size);

  run_data_->model_handle_.resize(frame_size);
  run_data_->out_bufs_.resize(frame_size);
  run_data_->valid_targets.resize(frame_size);

  run_data_->model_name_ = model_name_;
  run_data_->bpu_handle_ = bpu_handle_;
  run_data_->bpu_fakeimage_handle_ = fake_img_handle_;
  run_data_->fake_image_ptr_.resize(frame_size);

  for (size_t frame_idx = 0; frame_idx < frame_size; frame_idx++) {
    auto &input_data = ((run_data_->input))[frame_idx];
    auto rois = RoisConvert(input_data);
    auto xstream_pyramid =
        std::static_pointer_cast<XStreamData<ImageFramePtr>>(input_data[1]);
    auto pyramid =
    std::static_pointer_cast<PymImageFrame>(xstream_pyramid->value);

    int box_num = rois->datas_.size();
    run_data_->target_nums[frame_idx] = box_num;
    run_data_->targets_data[frame_idx].resize(box_num);

    run_data_->model_handle_[frame_idx].resize(box_num);
    run_data_->out_bufs_[frame_idx].resize(box_num);
    run_data_->fake_image_ptr_[frame_idx].resize(box_num);
    auto &norm_rois = run_data_->targets_data[frame_idx];

    uint32_t w = pyramid->Width();
    uint32_t h = pyramid->Height();
    uint32_t dst_h = model_info_.input_nhwc_[1];
    uint32_t dst_w = model_info_.input_nhwc_[2];

    int layer_size = model_info_.output_layer_size_.size();
    uint32_t handle_num =
        max_handle_num_ < 0 ? box_num : std::min(max_handle_num_, box_num);
    for (size_t roi_idx = 0; roi_idx < handle_num; roi_idx++) {
      auto p_roi =
          std::static_pointer_cast<XStreamData<BBox>>(rois->datas_[roi_idx]);
      auto p_norm_roi = std::make_shared<XStreamData<BBox>>();
      norm_rois[roi_idx] = std::static_pointer_cast<BaseData>(p_norm_roi);
      if (p_roi->state_ != xstream::DataState::VALID) {
        p_norm_roi->value = p_roi->value;
        continue;
      }

      BBox *norm_box = &(p_norm_roi->value);
      if (NormalizeRoi(&p_roi->value,
                       norm_box,
                       norm_params_,
                       w,
                       h,
                       filter_method_)) {
        LOGD << "norm roi error, box: [" << p_roi->value.x1 << ", "
             << p_roi->value.y1 << ", " << p_roi->value.x2 << ", "
             << p_roi->value.y2 << "]";
        continue;
      }

      int ret = 0;
      uint8_t *tmp_src_data = nullptr, *tmp_dst_data = nullptr;
      uint8_t *tmp_src_uv_data = nullptr;
      int tmp_src_size = 0, tmp_dst_size = 0;
      int tmp_src_uv_size = 0;
      int src_1_stride = 0, src_2_stride = 0, dst_1_stride = 0,
          dst_2_stride = 0;
      int tmp_src_w = 0, tmp_src_h = 0, tmp_dst_w = 0, tmp_dst_h = 0;
      {
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_crop")
        RUN_FPS_PROFILER(model_name_ + "_crop")

        // crop
        tmp_src_data = reinterpret_cast<uint8_t *>(pyramid->Data());
        tmp_src_uv_data = reinterpret_cast<uint8_t *>(pyramid->DataUV());
        tmp_src_size = pyramid->Stride() * pyramid->Height();
        tmp_src_uv_size = pyramid->StrideUV() * pyramid->Height() / 2;
        tmp_src_w = w;
        tmp_src_h = h;
        src_1_stride = pyramid->Stride();
        src_2_stride = pyramid->StrideUV();
        // int ret = HobotXStreamCropImage(tmp_src_data,
        //                              tmp_src_size,
        //                              tmp_src_w,
        //                              tmp_src_h,
        //                              src_1_stride,
        //                              src_2_stride,
        //                              IMAGE_TOOLS_RAW_YUV_NV12,
        //                              norm_box->x1,
        //                              norm_box->y1,
        //                              norm_box->x2 - 1,
        //                              norm_box->y2 - 1,
        //                              &tmp_dst_data,
        //                              &tmp_dst_size,
        //                              &tmp_dst_w,
        //                              &tmp_dst_h,
        //                              &dst_1_stride,
        //                              &dst_2_stride);

        // y and uv may not continue in pyramid result
        const uint8_t *input_yuv_data[3] = {tmp_src_data, tmp_src_uv_data,
                                            nullptr};
        const int input_yuv_size[3] = {tmp_src_size, tmp_src_uv_size, 0};
        int ret = HobotXStreamCropYUVImage(
            input_yuv_data, input_yuv_size, tmp_src_w, tmp_src_h, src_1_stride,
            src_2_stride, IMAGE_TOOLS_RAW_YUV_NV12, norm_box->x1, norm_box->y1,
            norm_box->x2 - 1, norm_box->y2 - 1, &tmp_dst_data, &tmp_dst_size,
            &tmp_dst_w, &tmp_dst_h, &dst_1_stride, &dst_2_stride);
        HOBOT_CHECK(ret == 0)
            << "crop img failed"
            << ", src_size:" << tmp_src_size << ", src_w:" << tmp_src_w
            << ", src_h:" << tmp_src_h << ", src_1_stride:" << src_1_stride
            << ", src_2_stride:" << src_2_stride;

        tmp_src_data = tmp_dst_data;
        tmp_src_size = tmp_dst_size;
        tmp_src_uv_data = nullptr;
        tmp_src_uv_size = 0;
        tmp_src_w = tmp_dst_w;
        tmp_src_h = tmp_dst_h;
        src_1_stride = dst_1_stride;
        src_2_stride = dst_2_stride;
      }

      {
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_resize")
        RUN_FPS_PROFILER(model_name_ + "_resize")
        // resize
        tmp_dst_w = (rotate_degree_ && rotate_degree_ != 180) ? dst_h : dst_w;
        tmp_dst_h = (rotate_degree_ && rotate_degree_ != 180) ? dst_w : dst_h;
        if (tmp_src_w != tmp_dst_w || tmp_src_h != tmp_dst_h) {
          struct HobotXStreamImageToolsResizeInfo resize_info;
          ret = HobotXStreamResizeImage(tmp_src_data,
                                     tmp_src_size,
                                     tmp_src_w,
                                     tmp_src_h,
                                     src_1_stride,
                                     src_2_stride,
                                     IMAGE_TOOLS_RAW_YUV_NV12,
                                     0,
                                     tmp_dst_w,
                                     tmp_dst_h,
                                     &tmp_dst_data,
                                     &tmp_dst_size,
                                     &dst_1_stride,
                                     &dst_2_stride,
                                     &resize_info);
          HOBOT_CHECK(ret == 0)
              << "resize img failed"
              << ", src_size:" << tmp_src_size << ", src_w:" << tmp_src_w
              << ", src_h:" << tmp_src_h << ", src_1_stride:" << src_1_stride
              << ", src_2_stride:" << src_2_stride;
          HobotXStreamFreeImage(tmp_src_data);
          tmp_src_data = tmp_dst_data;
          tmp_src_size = tmp_dst_size;
          tmp_src_w = tmp_dst_w;
          tmp_src_h = tmp_dst_h;
          src_1_stride = dst_1_stride;
          src_2_stride = dst_2_stride;
        }
      }

      {
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_rotate")
        RUN_FPS_PROFILER(model_name_ + "_rotate")

        // rotate
        if (rotate_degree_) {
          ret = HobotXStreamRotateImage(tmp_src_data,
                                     tmp_src_size,
                                     tmp_src_w,
                                     tmp_src_h,
                                     src_1_stride,
                                     src_2_stride,
                                     IMAGE_TOOLS_RAW_YUV_NV12,
                                     rotate_degree_,
                                     &tmp_dst_data,
                                     &tmp_dst_size,
                                     &tmp_dst_w,
                                     &tmp_dst_h,
                                     &dst_1_stride,
                                     &dst_2_stride);
          HOBOT_CHECK(ret == 0)
              << "rotate img failed"
              << ", src_size:" << tmp_src_size << ", src_w:" << tmp_src_w
              << ", src_h:" << tmp_src_h << ", src_1_stride:" << src_1_stride
              << ", src_2_stride:" << src_2_stride;
          HobotXStreamFreeImage(tmp_src_data);
          tmp_src_data = tmp_dst_data;
          tmp_src_size = tmp_dst_size;
          tmp_src_w = tmp_dst_w;
          tmp_src_h = tmp_dst_h;
          src_1_stride = dst_1_stride;
          src_2_stride = dst_2_stride;
        }
      }
      ModelOutputBuffer bufs(model_info_, 1);
      {
#if 0
        static int data_idx = 0;
        DumpBinaryFile(tmp_src_data,
                       tmp_src_size,
                       std::to_string(data_idx++) + ".nv12");
#endif
        BPUModelHandle model_handle;
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_runmodel")
        RUN_FPS_PROFILER(model_name_ + "_runmodel")
        int ret = RunModelFromImage(
            tmp_src_data, tmp_src_size, bufs.out_bufs_.data(), layer_size,
            &model_handle,
            &(run_data_->fake_image_ptr_[frame_idx][roi_idx]));
        if (ret == -1) {
          HobotXStreamFreeImage(tmp_src_data);
          continue;
        }
        run_data_->model_handle_[frame_idx][roi_idx] = model_handle;
      }
      run_data_->valid_targets[frame_idx].push_back(roi_idx);
      run_data_->out_bufs_[frame_idx][roi_idx] = bufs;
      LOGD << "RunModelFromImage success";
      HobotXStreamFreeImage(tmp_src_data);
    }
    }
  return result;
}

int ImgInputPredictor::UpdateParameter(xstream::InputParamPtr ptr) {
  CNNPredictor::UpdateParameter(ptr);
  if (config_->KeyExist("expand_scale")) {
    norm_params_.expand_scale = config_->GetFloatValue("expand_scale");
  }
  if (config_->KeyExist("norm_method")) {
    std::string s_norm_method = config_->GetSTDStringValue("norm_method");
    auto iter = g_norm_method_map.find(s_norm_method);
    if (iter == g_norm_method_map.end()) {
      LOGD << "norm_method is unknown:" << s_norm_method;
    } else {
      norm_params_.norm_type = iter->second;
    }
  }
  if (config_->KeyExist("filter_method")) {
    std::string s_filter_method = config_->GetSTDStringValue("filter_method");
    auto filter_iter = g_filter_method_map.find(s_filter_method);
    if (filter_iter == g_filter_method_map.end()) {
      LOGD << "filter_method is unknown:" << s_filter_method;
    } else {
      filter_method_ = filter_iter->second;
    }
  }
  if (config_->KeyExist("rotate_degree")) {
    rotate_degree_ = config_->GetIntValue("rotate_degree");
  }
  return 0;
}

std::shared_ptr<BaseDataVector> ImgInputPredictor::RoisConvert(
      std::vector<BaseDataPtr> input_data) {
  return std::static_pointer_cast<BaseDataVector>(input_data[0]);
}
}  // namespace CnnProc
}  // namespace xstream
