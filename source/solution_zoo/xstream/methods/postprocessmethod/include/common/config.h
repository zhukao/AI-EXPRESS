//
// Copyright (c) 2020 Horizon Robotics. All rights reserved.
//

#ifndef UTIL_CONFIG_H_
#define UTIL_CONFIG_H_
#include <string.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "json/json.h"

namespace xstream {

class Config {
 public:
  explicit Config(Json::Value config) : config_(config) {}

  int GetIntValue(std::string key, int default_value = 0) {
    auto value_js = config_[key.c_str()];
    if (value_js.isNull()) {
      return default_value;
    }
    return value_js.asInt();
  }

  bool GetBoolValue(std::string key, bool default_value = false) {
    auto value_int = GetIntValue(key, default_value);
    return value_int == 0 ? false : true;
  }

  float GetFloatValue(std::string key, float default_value = 0.0) {
    auto value_js = config_[key.c_str()];
    if (value_js.isNull()) {
      return default_value;
    }
    return value_js.asFloat();
  }

  std::string GetSTDStringValue(std::string key,
                                std::string default_value = "") {
    auto value_js = config_[key.c_str()];
    if (value_js.isNull()) {
      return default_value;
    }
    return value_js.asString();
  }

  std::vector<std::string> GetSTDStringArray(std::string key) {
    auto value_js = config_[key.c_str()];
    std::vector<std::string> ret;
    if (value_js.isNull()) {
      return ret;
    }
    ret.resize(value_js.size());
    for (Json::ArrayIndex i = 0; i < value_js.size(); ++i) {
      ret[i] = value_js[i].asString();
    }
    return ret;
  }

  std::vector<std::shared_ptr<Config>> GetSubConfigArray(std::string key) {
    auto value_js = config_[key.c_str()];
    std::vector<std::shared_ptr<Config>> ret;
    if (value_js.isNull()) {
      return ret;
    }
    ret.resize(value_js.size());
    for (Json::ArrayIndex i = 0; i < value_js.size(); ++i) {
      ret[i] = std::shared_ptr<Config>(new Config(value_js[i]));
    }
    return ret;
  }

  std::unordered_map<int, std::string> GetLabelsMap(std::string key) {
    auto value_js = config_[key.c_str()];
    std::unordered_map<int, std::string> ret;
    if (value_js.isNull()) {
      return ret;
    }
    for (Json::ArrayIndex i = 0; i < value_js.size(); ++i) {
      auto pair = value_js[i];
      auto label_idx = pair["key"].asInt();
      auto label_val = pair["value"].asString();
      ret[label_idx] = label_val;
    }
    return ret;
  }

  std::shared_ptr<Config> GetSubConfig(std::string key) {
    auto value_js = config_[key.c_str()];
    if (value_js.isNull()) {
      return nullptr;
    }
    return std::shared_ptr<Config>(new Config(value_js));
  }

 protected:
  Json::Value config_;
};
}  // namespace xstream

#endif  // UTIL_CONFIG_H_
