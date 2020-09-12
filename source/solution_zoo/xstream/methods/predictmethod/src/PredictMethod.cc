/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: prediction_method.cc
 * @Brief: definition of the PredictMethod
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-22 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-22 16:19:08
 */

#include "PredictMethod/PredictMethod.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include "PredictMethod/PredictConst.h"
#include "PredictMethod/Predictors/PredictorFactory.h"
#include "json/json.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int PredictMethod::Init(const std::string &cfg_path) {
  LOGD << "PredictMethod Init";
  // 0. parse config_
  std::ifstream infile(cfg_path);
  HOBOT_CHECK(infile.good()) << "PredictMethod error config file path:"
                             << cfg_path;
  infile >> config_;

  auto get_parent_path = [](const std::string path) -> std::string {
    auto pos = path.rfind('/');
    if (std::string::npos != pos) {
      auto parent = path.substr(0, pos);
      return parent + "/";
    } else {
      return std::string("./");
    }
  };

  // 1. model_path_
  std::string model_path =
      config_["model_file_path"].isString() ?
      config_["model_file_path"].asString() : "";
  HOBOT_CHECK(model_path.size() > 0) << "must set model_file_path";
  std::string parent_path = get_parent_path(cfg_path);
  config_["model_file_path"] = parent_path + model_path;

  // 2. predictor_
  std::string predictor_type =
      config_["predict_type"].isString() ?
      config_["predict_type"].asString() : "";
  auto iter = g_predict_type_map.find(predictor_type);
  HOBOT_CHECK(iter != g_predict_type_map.end())
      << "predict_type unknown: " << predictor_type;
  predictor_.reset(PredictorFactory::GetPredictor(iter->second));
  std::string config = config_.toStyledString();
  predictor_->Init(config);
  return 0;
}

void PredictMethod::Finalize() {
  // predictor_ Finalize
  predictor_->Finalize();
}

std::vector<std::vector<BaseDataPtr>> PredictMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  return predictor_->Do(input, param);
}

int PredictMethod::UpdateParameter(xstream::InputParamPtr ptr) {
  return 0;
}

InputParamPtr PredictMethod::GetParameter() const {
  std::shared_ptr<InputParam> res;
  return res;
}

std::string PredictMethod::GetVersion() const { return ""; }

}  // namespace xstream
