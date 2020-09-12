/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: PostProcessMethod.h
 * @Brief: declaration of the PostProcessMethod
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-25 13:53:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-25 16:01:54
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSMETHOD_H_
#define POSTPROCESSMETHOD_POSTPROCESSMETHOD_H_

#include <vector>
#include <string>
#include <memory>
#include "json/json.h"
#include "PostProcessMethod/PostProcessor/PostProcessor.h"
#include "hobotxstream/method.h"
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"

namespace xstream {

class PostProcessMethod : public Method {
 public:
  PostProcessMethod() {}
  virtual ~PostProcessMethod() {}

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
  std::shared_ptr<PostProcessor> post_processor_;
};
}  // namespace xstream

#endif  // POSTPROCESSMETHOD_POSTPROCESSMETHOD_H_
