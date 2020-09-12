/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: CNNPostProcessor.h
 * @Brief: declaration of the CNNPostProcessor
 * @Author: zhe.sun
 * @Date: 2020-01-10 11:26:50
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-01-10 11:26:50
 */

#ifndef INCLUDE_CNNPOSTPROCESSOR_CNNPOSTPROCESSOR_H_
#define INCLUDE_CNNPOSTPROCESSOR_CNNPOSTPROCESSOR_H_

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <mutex>
#include "hobotxstream/method.h"
#include "CNNMethodCreator.h"
#include "CNNPredictorOutputData.h"
#include "util/util.h"
#include "CNNMethodConfig.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {
namespace CnnProc {
bool CNNPostProcessQuery(const std::string& method_name);
MethodPtr CNNPostProcessCreate(const std::string& method_name);

class CNNPostProcessor : public Method {
 public:
  CNNPostProcessor() {}
  virtual ~CNNPostProcessor() {}

  virtual int Init(const std::string &cfg_path);

  virtual void Finalize() {}

  virtual std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) = 0;

  int UpdateParameter(xstream::InputParamPtr ptr);

  InputParamPtr GetParameter() const;

  std::string GetVersion() const;

  virtual void OnProfilerChanged(bool on) {}

  void ConvertOutputToMXNet(void *src_ptr, void *dest_ptr, int layer_idx);

  void getBPUResult(std::shared_ptr<CNNPredictorOutputData> run_data);

 protected:
  std::shared_ptr<CNNMethodConfig> config_;
  static std::mutex init_mutex_;

  std::string model_name_;
  int output_slot_size_ = 0;

  const ModelInfo *model_info_;
  std::vector<std::vector<int8_t>> feature_bufs_;
  std::vector<std::vector<std::vector<std::vector<int8_t>>>> mxnet_output_;
  std::vector<std::vector<bool>> targets_valid_flag;
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_CNNPOSTPROCESSOR_CNNPOSTPROCESSOR_H_
