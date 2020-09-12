/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: ImgInputPredictor.cpp
 * @Brief: definition of the ImgInputPredictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-16 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-16 16:23:27
 */

#include "CNNMethod/Predictor/ImgInputPredictor.h"
#include <algorithm>
#include <memory>
#include <string>
#include "CNNMethod/util/util.h"
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

int32_t ImgInputPredictor::Init(std::shared_ptr<CNNMethodConfig> config) {
  Predictor::Init(config);
  norm_params_.expand_scale = config->GetFloatValue("expand_scale", 1.0f);
  std::string s_norm_method = config->GetSTDStringValue("norm_method",
                                                        "norm_by_nothing");
  auto iter = g_norm_method_map.find(s_norm_method);
  HOBOT_CHECK(iter != g_norm_method_map.end())
      << "norm_method is unknown:" << s_norm_method;
  norm_params_.norm_type = iter->second;

  norm_params_.aspect_ratio = config->GetFloatValue("aspect_ratio", 1.0f);

  std::string s_filter_method =
      config->GetSTDStringValue("filter_method", "no_filter");
  auto filter_iter = g_filter_method_map.find(s_filter_method);
  HOBOT_CHECK(filter_iter != g_filter_method_map.end())
      << "filter_method is unknown:" << s_filter_method;
  filter_method_ = filter_iter->second;

  rotate_degree_ = config->GetIntValue("rotate_degree");
  return 0;
}

void ImgInputPredictor::UpdateParam(std::shared_ptr<CNNMethodConfig> config) {
  Predictor::UpdateParam(config);
  if (config->KeyExist("expand_scale")) {
    norm_params_.expand_scale = config->GetFloatValue("expand_scale");
  }
  if (config->KeyExist("norm_method")) {
    std::string s_norm_method = config->GetSTDStringValue("norm_method");
    auto iter = g_norm_method_map.find(s_norm_method);
    if (iter == g_norm_method_map.end()) {
      LOGD << "norm_method is unknown:" << s_norm_method;
    } else {
      norm_params_.norm_type = iter->second;
    }
  }
  if (config->KeyExist("filter_method")) {
    std::string s_filter_method = config->GetSTDStringValue("filter_method");
    auto filter_iter = g_filter_method_map.find(s_filter_method);
    if (filter_iter == g_filter_method_map.end()) {
      LOGD << "filter_method is unknown:" << s_filter_method;
    } else {
      filter_method_ = filter_iter->second;
    }
  }
  if (config->KeyExist("rotate_degree")) {
    rotate_degree_ = config->GetIntValue("rotate_degree");
  }
}

void ImgInputPredictor::Do(CNNMethodRunData *run_data) {
  int frame_size = run_data->input->size();
  run_data->mxnet_output.resize(frame_size);
  run_data->input_dim_size.resize(frame_size);
  run_data->norm_rois.resize(frame_size);
  run_data->real_nhwc = model_info_.real_nhwc_;
  run_data->elem_size = model_info_.elem_size_;
  run_data->all_shift = model_info_.all_shift_;

  for (int frame_idx = 0; frame_idx < frame_size; frame_idx++) {  // loop frame
    auto &input_data = (*(run_data->input))[frame_idx];
    auto xstream_pyramid =
        std::static_pointer_cast<XStreamData<ImageFramePtr>>(input_data[1]);
    auto pyramid = std::static_pointer_cast<PymImageFrame>(
                   xstream_pyramid->value);

    std::shared_ptr<BaseDataVector> rois;

    rois = RoisConvert(input_data);

    int box_num = rois->datas_.size();

    run_data->mxnet_output[frame_idx].resize(box_num);
    run_data->input_dim_size[frame_idx] = box_num;
    run_data->norm_rois[frame_idx].resize(box_num);

    auto &norm_rois = run_data->norm_rois[frame_idx];

    uint32_t w = pyramid->Width();
    uint32_t h = pyramid->Height();
    uint32_t dst_h = model_info_.input_nhwc_[1];
    uint32_t dst_w = model_info_.input_nhwc_[2];
    LOGI << "input h: " << model_info_.input_nhwc_[1];
    LOGI << "input w: " << model_info_.input_nhwc_[2];

    int layer_size = model_info_.output_layer_size_.size();
    uint32_t handle_num =
        max_handle_num_ < 0 ? box_num : std::min(max_handle_num_, box_num);
    for (uint32_t roi_idx = 0; roi_idx < handle_num; roi_idx++) {
      std::shared_ptr<XStreamData<BBox>> p_roi;
      p_roi = std::static_pointer_cast<XStreamData<BBox>>(
            rois->datas_[roi_idx]);

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
        // int ret = HobotXStreamCropImage(
        //     tmp_src_data, tmp_src_size, tmp_src_w, tmp_src_h, src_1_stride,
        //     src_2_stride, IMAGE_TOOLS_RAW_YUV_NV12, norm_box->x1,
        //     norm_box->y1, norm_box->x2 - 1, norm_box->y2 - 1, &tmp_dst_data,
        //     &tmp_dst_size, &tmp_dst_w, &tmp_dst_h, &dst_1_stride,
        //     &dst_2_stride);

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
        LOGI << "dst_w: " << dst_w << ", dst_h: " << dst_h;
        if (tmp_src_w != tmp_dst_w || tmp_src_h != tmp_dst_h) {
          struct HobotXStreamImageToolsResizeInfo resize_info;
          ret = HobotXStreamResizeImage(
              tmp_src_data, tmp_src_size, tmp_src_w, tmp_src_h, src_1_stride,
              src_2_stride, IMAGE_TOOLS_RAW_YUV_NV12, 0, tmp_dst_w, tmp_dst_h,
              &tmp_dst_data, &tmp_dst_size, &dst_1_stride, &dst_2_stride,
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
          ret = HobotXStreamRotateImage(tmp_src_data, tmp_src_size, tmp_src_w,
                                     tmp_src_h, src_1_stride, src_2_stride,
                                     IMAGE_TOOLS_RAW_YUV_NV12, rotate_degree_,
                                     &tmp_dst_data, &tmp_dst_size, &tmp_dst_w,
                                     &tmp_dst_h, &dst_1_stride, &dst_2_stride);
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
      {
#if 0
        static int data_idx = 0;
        DumpBinaryFile(tmp_src_data,
                       tmp_src_size,
                       std::to_string(data_idx++) + ".nv12");
#endif
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_runmodel")
        RUN_FPS_PROFILER(model_name_ + "_runmodel")
        int ret = RunModel(tmp_src_data,
                           tmp_src_size,
                           model_info_.data_type_);
        if (ret != 0) {
          HobotXStreamFreeImage(tmp_src_data);
          continue;
        }
      }
      LOGD << "RunModel From ImgInput success";
      HobotXStreamFreeImage(tmp_src_data);

      auto &one_tgt_mxnet = run_data->mxnet_output[frame_idx][roi_idx];
      one_tgt_mxnet.resize(layer_size);
      // change raw data to mxnet layout
      {
        RUN_PROCESS_TIME_PROFILER(model_name_ + "_do_hbrt")
        RUN_FPS_PROFILER(model_name_ + "_do_hbrt")
        for (int j = 0; j < layer_size; j++) {
          uint32_t feature_size = model_info_.mxnet_output_layer_size_[j];
          one_tgt_mxnet[j].resize(feature_size);
          HB_SYS_flushMemCache(&(output_tensors_[j].data),
                               HB_SYS_MEM_CACHE_INVALIDATE);

          ConvertOutputToMXNet(output_tensors_[j].data.virAddr,
                               one_tgt_mxnet[j].data(), j);
        }
        // release output
        // ReleaseOutputTensor();
      }
      LOGD << "do hbrt success";
    }
  }
}

std::shared_ptr<BaseDataVector> ImgInputPredictor::RoisConvert(
      std::vector<BaseDataPtr> input_data) {
  return std::static_pointer_cast<BaseDataVector>(input_data[0]);
}
}  // namespace xstream
