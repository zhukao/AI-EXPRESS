/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: BehaviorMethod.cpp
 * @Brief: definition of the BehaviorMethod
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 14:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 16:17:08
 */

#include "BehaviorMethod/BehaviorMethod.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include "BehaviorMethod/BehaviorFactory.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int32_t BehaviorMethod::Init(const std::string &cfg_path) {
  LOGI << "BehaviorMethod Init";
  std::ifstream infile(cfg_path.c_str());
  HOBOT_CHECK(infile.good()) << "BehaviorMethod error config file path:"
                             << cfg_path;

  std::stringstream buffer;
  buffer << infile.rdbuf();
  {
    Json::CharReaderBuilder jsonReader;
    std::string errs;
    if (!Json::parseFromStream(jsonReader, buffer, &config_, &errs)) {
      LOGE << "BehaviorMethod invalid config";
      return -1;
    }
  }

  // 获取行为类型
  auto value_js = config_["behavior_type"];
  std::string behavior_type = value_js.isNull() ?
                              "" : value_js.asString();
  auto iter = g_behavior_map.find(behavior_type);
  HOBOT_CHECK(iter != g_behavior_map.end())
      << "behavior_type unknown: " << behavior_type;
  behavior_event_.reset(BehaviorFactory::GetPredictor(iter->second));
  behavior_event_->Init(config_);
  return 0;
}

void BehaviorMethod::Finalize() {}

std::vector<std::vector<BaseDataPtr>> BehaviorMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  return behavior_event_->Process(input, param);
}

int BehaviorMethod::UpdateParameter(xstream::InputParamPtr ptr) {
  return 0;
}

InputParamPtr BehaviorMethod::GetParameter() const {
  std::shared_ptr<InputParam> res;
  return res;
}

std::string BehaviorMethod::GetVersion() const { return ""; }

}  // namespace xstream
