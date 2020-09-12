/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: DetectPredictor.cc
 * @Brief: definition of the DetectPredictor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-19 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-19 16:19:08
 */

#include "PredictMethod/Predictors/DetectPredictor.h"
#include <vector>
#include <memory>
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "opencv2/imgproc.hpp"

namespace xstream {

std::vector<std::vector<BaseDataPtr>> DetectPredictor::Do(
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

std::vector<BaseDataPtr> DetectPredictor::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input) {
  LOGD << "DetectPredictor RunSingleFrame";
  HOBOT_CHECK(frame_input.size() == 1);
  auto xstream_img = std::static_pointer_cast<XStreamData<
      std::shared_ptr<hobot::vision::ImageFrame>>>(frame_input[0]);

  std::vector<BaseDataPtr> frame_output(1);
  auto async_data = std::make_shared<XStreamData<std::shared_ptr<
      hobot::vision::AsyncData>>>();
  frame_output[0] = async_data;
  async_data->value.reset(new hobot::vision::AsyncData());

  std::string img_type = xstream_img->value->type;
  {
    RUN_PROCESS_TIME_PROFILER("Run Detect Model");
    RUN_FPS_PROFILER("Run Detect Model");
    int src_img_width = 0;
    int src_img_height = 0;

    std::shared_ptr<std::vector<BPU_TENSOR_S>> input_tensors =
          std::make_shared<std::vector<BPU_TENSOR_S>>();

    if (img_type == "PymImageFrame") {
      auto pyramid_image = std::static_pointer_cast<
          hobot::vision::PymImageFrame>(xstream_img->value);
    #ifdef X2
      src_img_height = pyramid_image->img.src_img.height;
      src_img_width = pyramid_image->img.src_img.width;
    #endif
    #ifdef X3
      src_img_height = pyramid_image->down_scale[0].height;
      src_img_width = pyramid_image->down_scale[0].width;
    #endif
      pyramid_image->pym_layer = pyramid_layer_;
      uint8_t* y_data = reinterpret_cast<uint8_t*>(pyramid_image->Data());
      uint8_t* uv_data = reinterpret_cast<uint8_t*>(pyramid_image->DataUV());
      int y_len = pyramid_image->DataSize();
      int uv_len = pyramid_image->DataUVSize();
      PrepareInputTensorFromPym(y_data, uv_data, y_len, uv_len,
        input_tensors, BPU_TYPE_IMG_NV12_SEPARATE);
    } else if (img_type == "CVImageFrame") {
      auto cv_image = std::static_pointer_cast<
          hobot::vision::CVImageFrame>(xstream_img->value);
      HOBOT_CHECK(cv_image->pixel_format ==
                  HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12);
      src_img_height = cv_image->Height();
      src_img_width = cv_image->Width();
      auto img_mat = cv_image->img;

      cv::Mat resized_mat = img_mat;
      if (src_img_height != model_input_height_ ||
          src_img_width != model_input_width_) {
        cv::resize(img_mat, resized_mat,
                   cv::Size(model_input_width_, model_input_height_ * 3 / 2));
      }
      uint8_t *nv12_data = resized_mat.ptr<uint8_t>();

      int img_len = model_input_width_ * model_input_height_ * 3 / 2;
      LOGD << "nv12 img_len: " << img_len;
      PrepareInputTensorData(nv12_data, img_len, input_tensors);
    }

    async_data->value->src_image_height = src_img_height;
    async_data->value->src_image_width = src_img_width;
    async_data->value->model_input_height = model_input_height_;
    async_data->value->model_input_width = model_input_width_;

    // 4. prepare output tensor
    std::shared_ptr<std::vector<BPU_TENSOR_S>> output_tensors =
        std::make_shared<std::vector<BPU_TENSOR_S>>();
    PrepareOutputTensor(output_tensors);

    // 5. run model
    BPU_RUN_CTRL_S run_ctrl_s{2};
    std::shared_ptr<BPU_TASK_HANDLE> task_handle =
        std::make_shared<BPU_TASK_HANDLE>();
    int ret = HB_BPU_runModel(bpu_model_.get(),
                          input_tensors->data(),
                          bpu_model_->input_num,
                          output_tensors->data(),
                          bpu_model_->output_num,
                          &run_ctrl_s, false, task_handle.get());
    if (ret != 0) {
      // release input&&output tensor
      ReleaseTensor(input_tensors);
      ReleaseTensor(output_tensors);
      // release task_handle
      HB_BPU_releaseTask(task_handle.get());
      return frame_output;
    }

    // input_tensors, output_tensors传递给async_data
    async_data->value->input_tensors = input_tensors;
    async_data->value->output_tensors = output_tensors;
    async_data->value->task_handle = task_handle;
    async_data->value->bpu_model = bpu_model_;
    return frame_output;
  }
}

}  // namespace xstream
