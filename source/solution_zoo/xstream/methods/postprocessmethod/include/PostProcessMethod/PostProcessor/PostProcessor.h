/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PostProcessor.h
 * @Brief: declaration of the PostProcessor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-28 00:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-28 00:43:54
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSOR_POSTPROCESSOR_H_
#define POSTPROCESSMETHOD_POSTPROCESSOR_POSTPROCESSOR_H_

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "json/json.h"
#include "common/config.h"
#include "hobotxstream/method.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class PostProcessor {
 public:
  PostProcessor() {}
  virtual ~PostProcessor() {}

  virtual int Init(const std::string &cfg);
  virtual void Finalize();

  virtual std::vector<std::vector<BaseDataPtr>> Do(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) = 0;

 protected:
  // release tensor
  int ReleaseTensor(std::shared_ptr<std::vector<BPU_TENSOR_S>> tensor);

 protected:
  std::shared_ptr<Config> config_;
  std::shared_ptr<std::vector<BPU_TENSOR_S>> input_tensors_ = nullptr;
  std::shared_ptr<std::vector<BPU_TENSOR_S>> output_tensors_ = nullptr;
  std::shared_ptr<BPU_TASK_HANDLE> task_handle_ = nullptr;
  std::shared_ptr<BPU_MODEL_S> bpu_model_ = nullptr;

  int src_image_width_;
  int src_image_height_;
  int model_input_height_;
  int model_input_width_;
};
}  // namespace xstream

#endif  // POSTPROCESSMETHOD_POSTPROCESSOR_POSTPROCESSOR_H_
