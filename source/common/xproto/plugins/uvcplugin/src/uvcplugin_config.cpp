/*
 * @Copyright 2020 Horizon Robotics, Inc.
 */
#include "./uvcplugin_config.h"

#include <fstream>
#include <iostream>

#include "hobotlog/hobotlog.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {

// uvc config
UvcConfig::UvcConfig(const std::string &path) : path_(path) {
  auth_mode_ = 0;
  display_mode_ = UVC_MODE;
  layer_ = 4;
  jpeg_quality_ = 50;
  user_ = "";
  password_ = "";
  smart_type_ = SMART_BODY;
  video_type_ = VIDEO_MJPEG;
  image_width_ = 1920;
  image_height_ = 1080;
  data_buf_size_ = 3110400;
  packet_size_ = 102400;
  res_2160p_layer_ = 0;
  res_1080p_layer_ = 4;
  res_720p_layer_ = 5;
  is_cbr_ = 1;
  bitrate_ = 2000;
}

bool UvcConfig::LoadConfig() {
  std::ifstream ifs(path_);
  if (!ifs.is_open()) {
    LOGF << "open config file " << path_ << " failed";
    return false;
  }

  Json::CharReaderBuilder builder;
  std::string err_json;
  try {
    bool ret = Json::parseFromStream(builder, ifs, &json_, &err_json);
    if (!ret) {
      LOGF << "invalid config file " << path_;
      return false;
    }
  } catch (std::exception &e) {
    LOGF << "exception while parse config file " << path_ << ", " << e.what();
    return false;
  }

  return CheckConfig();
}

std::string UvcConfig::GetValue(const std::string &key) {
  std::lock_guard<std::mutex> lk(mutex_);
  if (json_[key].empty()) {
    LOGW << "Can not find key: " << key;
    return "";
  }

  return json_[key].asString();
}

Json::Value UvcConfig::GetJson() const { return this->json_; }

bool UvcConfig::CheckConfig() {
  if (json_.isMember("auto_mode")) {
    auth_mode_ = json_["auto_mode"].asInt();
  }

  if (auth_mode_ != 0) {
    auth_mode_ = 1;
    if (json_.isMember("user")) {
      user_ = json_["user"].asInt();
    }
    if (json_.isMember("password")) {
      password_ = json_["password"].asInt();
    }
  }
  if (json_.isMember("layer")) {
    layer_ = json_["layer"].asInt();
  }

  if (json_.isMember("2160p_layer")) {
    res_2160p_layer_ = json_["2160p_layer"].asInt();
  }

  if (json_.isMember("1080p_layer")) {
    res_1080p_layer_ = json_["1080p_layer"].asInt();
  }

  if (json_.isMember("720p_layer")) {
    res_720p_layer_ = json_["720p_layer"].asInt();
  }


  if (json_.isMember("jpeg_quality")) {
    jpeg_quality_ = json_["jpeg_quality"].asUInt();
    if (jpeg_quality_ > 100) jpeg_quality_ = 100;
  }

  if (json_.isMember("display_mode")) {
    display_mode_ = static_cast<DisplayType>(json_["display_mode"].asUInt());
  }

  if (json_.isMember("smart_type")) {
    smart_type_ = static_cast<SmartType>(json_["smart_type"].asInt());
  }
  if (json_.isMember("video_type")) {
    video_type_ = static_cast<VideoType>(json_["video_type"].asInt());
  }
  if (json_.isMember("image_width")) {
    image_width_ = json_["image_width"].asUInt();
  }
  if (json_.isMember("image_height")) {
    image_height_ = json_["image_height"].asUInt();
  }
  if (json_.isMember("attribute_description_path")) {
    attr_des_file_ = json_["attribute_description_path"].asString();
  }
  if (json_.isMember("data_buf_size")) {
    data_buf_size_ = json_["data_buf_size"].asUInt();
  }
  if (json_.isMember("packet_size")) {
    packet_size_ = json_["packet_size"].asUInt();
  }

  if (json_.isMember("is_cbr")) {
    is_cbr_ = json_["is_cbr"].asInt();
  }

  if (json_.isMember("bitrate")) {
    bitrate_ = json_["bitrate"].asInt();
  }

  // check the value
  // to do ..
  return true;
}

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
