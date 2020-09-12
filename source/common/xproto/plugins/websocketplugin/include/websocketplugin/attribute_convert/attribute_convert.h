/*!
 * -------------------------------------------
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     attrconvert.h
 * \Author   xudong.du
 * \Mail     xudong.du@horizon.ai
 * \Version  1.0.0.0
 * \Date     2020.06.29
 * -------------------------------------------
 */
#ifndef SMART_INCLUDE_HORIZON_VISION_ATTRIBUTE_CONVERT_
#define SMART_INCLUDE_HORIZON_VISION_ATTRIBUTE_CONVERT_

#include <string>

#include "hobotlog/hobotlog.hpp"
#include "json/json.h"
#include "xproto/utils/singleton.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace websocketplugin {
class AttributeConvert : public hobot::CSingleton<AttributeConvert> {
 public:
  AttributeConvert() {}
  ~AttributeConvert() {
    if (has_init_) {
      DeInit();
    }
  }

 public:
  void Init(std::string config_path) {
    if (has_init_) {
      return;
    }
    if (config_path.empty()) {
      return;
    }
    std::ifstream ifs(config_path);
    if (!ifs.is_open()) {
      LOGF << "open config file " << config_path << " failed";
      return;
    }

    Json::CharReaderBuilder builder;
    std::string err_json;
    try {
      bool ret = Json::parseFromStream(builder, ifs, &root_, &err_json);
      if (!ret) {
        LOGF << "invalid config file " << config_path;
        return;
      }
    } catch (std::exception& e) {
      LOGF << "exception while parse config file " << config_path << ", "
           << e.what();
      return;
    }
    has_init_ = true;
    return;
  }
  std::string GetAttrDes(std::string sub_tag, uint32_t attr_value) {
    auto key = std::to_string(attr_value);
    return QueryData(sub_tag, key);
  }
  std::string GetAttrDes(std::string sub_tag, int32_t attr_value) {
    auto key = std::to_string(attr_value);
    return QueryData(sub_tag, key);
  }

 private:
  std::string QueryData(std::string sub_tag, std::string attr_key) {
    std::string attr_des = "";
    if (false == has_init_) {
      return attr_des;
    }
    if (sub_tag.empty()) {
      return attr_des;
    }
    if (!root_.isMember(sub_tag)) {
      return attr_des;
    }
    auto value_js = root_[sub_tag.c_str()];
    if (value_js.isNull()) {
      return attr_des;
    }
    if (value_js.isMember(attr_key)) {
      auto result = value_js[attr_key.c_str()];
      if (result.isNull()) {
        return attr_des;
      }
      attr_des = result.asString();
      return attr_des;
    }
    return attr_des;
  }
  void DeInit() { has_init_ = false; }

 private:
  bool has_init_ = false;
  Json::Value root_;
};
}  // namespace websocketplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // SMART_INCLUDE_HORIZON_VISION_ATTRIBUTE_CONVERT_
