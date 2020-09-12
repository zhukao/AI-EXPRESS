/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: postprocess_method.cc
 * @Brief: definition of the PostProcessMethod
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-25 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-25 16:21:08
 */

#include "PostProcessMethod/PostProcessMethod.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include "PostProcessMethod/PostProcessor/PostProcessorFactory.h"
#include "json/json.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int PostProcessMethod::Init(const std::string &cfg_path) {
  LOGD << "PostProcessMethod Init";
  // 0. parse config_
  std::ifstream infile(cfg_path.c_str());
  HOBOT_CHECK(infile.good()) << "PostProcessMethod error config file path:"
                             << cfg_path;

  std::stringstream buffer;
  buffer << infile.rdbuf();
  {
    Json::CharReaderBuilder jsonReader;
    std::string errs;
    if (!Json::parseFromStream(jsonReader, buffer, &config_, &errs)) {
      LOGE << "PostProcessMethod invalid config";
      return -1;
    }
  }

  // post_processor_
  std::string post_type = config_["post_type"].isString() ?
                          config_["post_type"].asString() : "";
  auto iter = g_postprocess_type_map.find(post_type);
  HOBOT_CHECK(iter != g_postprocess_type_map.end())
      << "postprocess_type unknown: " << post_type;
  post_processor_.reset(
      PostProcessorFactory::GetPostProcessorFactory(iter->second));
  std::string config = config_.toStyledString();
  post_processor_->Init(config);
  return 0;
}

void PostProcessMethod::Finalize() {
}

std::vector<std::vector<BaseDataPtr>> PostProcessMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  return post_processor_->Do(input, param);
}

int PostProcessMethod::UpdateParameter(xstream::InputParamPtr ptr) {
  return 0;
}

InputParamPtr PostProcessMethod::GetParameter() const {
  std::shared_ptr<InputParam> res;
  return res;
}

std::string PostProcessMethod::GetVersion() const { return ""; }

}  // namespace xstream
