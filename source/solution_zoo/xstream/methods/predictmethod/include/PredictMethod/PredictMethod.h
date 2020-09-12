/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: PredictMethod.h
 * @Brief: declaration of the PredictMethod
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-19 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-19 16:01:54
 */

#ifndef PREDICTMETHOD_PREDICTMETHOD_H_
#define PREDICTMETHOD_PREDICTMETHOD_H_

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "json/json.h"
#include "PredictMethod/Predictors/Predictors.h"
#include "hobotxstream/method.h"
#include "hobotxsdk/xstream_data.h"

namespace xstream {

class PredictMethod : public Method {
 public:
  PredictMethod() {}
  virtual ~PredictMethod() {}

  virtual int Init(const std::string &cfg_path);
  virtual void Finalize();

  virtual std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param);

  virtual int UpdateParameter(xstream::InputParamPtr ptr);
  virtual InputParamPtr GetParameter() const;
  virtual std::string GetVersion() const;
  virtual void OnProfilerChanged(bool on) {}

 protected:
  Json::Value config_;
  std::shared_ptr<Predictors> predictor_;
};
}  // namespace xstream

#endif  // PREDICTMETHOD_PREDICTMETHOD_H_
