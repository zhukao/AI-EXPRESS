/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: Predictors.h
 * @Brief: declaration of the Predictors
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-27 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-27 18:43:54
 */

#ifndef PREDICTMETHOD_PREDICTORS_PREDICTORS_H_
#define PREDICTMETHOD_PREDICTORS_PREDICTORS_H_

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "json/json.h"
#include "hobotxstream/method.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class Predictors {
 public:
  Predictors() {}
  virtual ~Predictors() {}

  virtual int Init(const std::string &cfg);
  virtual void Finalize();

  virtual std::vector<std::vector<BaseDataPtr>> Do(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) = 0;

 protected:
  // prepare input tensor
  int PrepareInputTensorData(
      uint8_t *img_data, int data_length,
      std::shared_ptr<std::vector<BPU_TENSOR_S>> input_tensors);

  int PrepareInputTensorFromPym(
      uint8_t* y_data, uint8_t* uv_data, int y_len, int uv_len,
      std::shared_ptr<std::vector<BPU_TENSOR_S>> input_tensors,
      BPU_DATA_TYPE_E data_type);

  // pre_input tensor resize to input tensor
  int ResizeInputTensor(std::shared_ptr<std::vector<BPU_TENSOR_S>> pre_tensors,
                        std::shared_ptr<std::vector<BPU_TENSOR_S>> tensors);

  // prepare output tensor
  int PrepareOutputTensor(std::shared_ptr<std::vector<BPU_TENSOR_S>> tensors,
                          int batch_size = 1);

  // release tensor
  int ReleaseTensor(std::shared_ptr<std::vector<BPU_TENSOR_S>> tensor);

 protected:
  Json::Value config_;
  static std::mutex init_mutex_;
  std::string model_path_;

  std::shared_ptr<BPU_MODEL_S> bpu_model_ = nullptr;

  int model_input_height_;
  int model_input_width_;

  int input_h_idx_, input_w_idx_, input_c_idx_;
  int pyramid_layer_ = 0;
};
}  // namespace xstream

#endif  // PREDICTMETHOD_PREDICTORS_PREDICTORS_H_
