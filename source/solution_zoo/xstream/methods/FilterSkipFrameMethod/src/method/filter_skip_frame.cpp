
/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     FilterSkipFrameMethod Method
 * @author    tangji.sun
 * @email     tangji.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2019.11.06
 */

#include "FilterSkipFrameMethod/FilterSkipFrameMethod.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include <random>
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"
namespace xstream {

typedef XStreamData<hobot::vision::BBox> XStreamBBox;

FilterSkipFrameParam::FilterSkipFrameParam(const std::string &module_name)
  : InputParam(module_name) {
}

std::string FilterSkipFrameParam::Format() {
  return config_str;
}

FilterSkipFrameMethod::FilterSkipFrameMethod() {
  img_height_ = 1080;
  img_width_ = 1920;
  min_height_ = 100;
  min_width_ = 100;
  border_ = 20;
  skip_num_ = 5;
  min_score_ = 0.8;
}

int FilterSkipFrameMethod::Init(const std::string &config_file_path) {
  config_file_name_ = config_file_path;
  if (config_file_name_.empty()) {
    LOGE << "config file is empty";
    return -1;
  }
  std::ifstream config_file(config_file_name_);
  if (!config_file.is_open()) {
    LOGE << "config file open failed " << config_file_name_;
    return -1;
  }
  std::string config_str((std::istreambuf_iterator<char>(config_file)),
                         std::istreambuf_iterator<char>());
  if (UpdateConfig(config_str) != 0) {
    LOGE << "filter skip update config failed " << config_str;
    return -1;
  }
  return 0;
}

int FilterSkipFrameMethod::UpdateParameter(InputParamPtr ptr) {
  std::string config_str = ptr->Format();
  if (config_str.empty()) {
    LOGE << "config is empty";
    return -1;
  }
  if (UpdateConfig(config_str) != 0) {
    LOGE << "config is invalid";
    return -1;
  }
  std::ofstream config_file(config_file_name_);
  if (!config_file.is_open()) {
    LOGE << "save file failed";
  }
  config_file << config_str;
  config_file.close();
  return 0;
}

InputParamPtr FilterSkipFrameMethod::GetParameter() const {
  std::ifstream config_file(config_file_name_);
  if (!config_file.is_open()) {
    LOGE << "filter skip config file open failed " << config_file_name_;
    return nullptr;
  }
  std::string config_str((std::istreambuf_iterator<char>(config_file)),
                         std::istreambuf_iterator<char>());
  auto param_ptr = std::make_shared<FilterSkipFrameParam>("filter_skip");
  param_ptr->config_str = config_str;
  return param_ptr;
}

std::vector<std::vector<BaseDataPtr>> FilterSkipFrameMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); ++i) {
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];
    out_batch_i.resize(in_batch_i.size());
    LOGI << "input size: " << in_batch_i.size();
    // 只支持n个输入，输入格式是BBox的数组
    for (size_t j = 0; j < in_batch_i.size(); ++j) {
      if (in_batch_i[j]->state_ == DataState::INVALID) {
        LOGI << "input slot " << j << " is invalid";
        continue;
      }
      auto in_rects = std::static_pointer_cast<BaseDataVector>(in_batch_i[j]);
      assert("BaseDataVector" == in_rects->type_);
      auto out_rects = std::make_shared<BaseDataVector>();
      out_batch_i[j] = std::static_pointer_cast<BaseData>(out_rects);
      for (auto &in_rect : in_rects->datas_) {
        // assert("BBox" == in_rect->type_);
        auto bbox = std::static_pointer_cast<XStreamBBox>(in_rect);
        float top_left_x_ = bbox->value.x1;
        float top_left_y_ = bbox->value.y1;
        float bottom_right_x_ = bbox->value.x2;
        float bottom_right_y_ = bbox->value.y2;
        if ((top_left_x_ > border_) && (top_left_y_ > border_) &&
            (bottom_right_x_ < (img_width_ - border_)) &&
            (bottom_right_y_ < (img_height_ - border_)) &&
            ((bottom_right_y_ - top_left_y_) > min_height_) &&
            ((bottom_right_x_ - top_left_x_) > min_width_) &&
            (bbox->value.score >= min_score_)) {
          auto out_rect = std::make_shared<XStreamBBox>();
          std::unordered_map<uint32_t, int32_t>::iterator it;

          *(out_rect.get()) = *(bbox.get());

          it = trackState_.find(bbox->value.id);
          if (it == trackState_.end()) {
            trackState_[bbox->value.id] = 0;
            // out_rect->state_ = xstream::DataState::FILTERED;
            trackVec_.push_back(bbox->value.id);
            if (trackVec_.size() > 1000) {
              trackState_.erase(trackVec_[0]);
              trackVec_.pop_front();
            }
          } else if ((++(it->second)) % skip_num_ == 0) {
            it->second = 0;
            LOGI << "skip frame success";
          } else {
            out_rect->state_ = xstream::DataState::FILTERED;
          }

          out_rects->datas_.push_back(
              std::static_pointer_cast<BaseData>(out_rect));
        } else {
          auto out_rect = std::make_shared<XStreamBBox>();
          *(out_rect.get()) = *(bbox.get());
          out_rect->state_ = xstream::DataState::FILTERED;
          out_rects->datas_.push_back(
              std::static_pointer_cast<BaseData>(out_rect));
        }
      }
    }
  }

  return output;
}

void FilterSkipFrameMethod::Finalize() { LOGI << "BBoxFilter::Finalize"; }

std::string FilterSkipFrameMethod::GetVersion() const { return "0.0.1"; }

void FilterSkipFrameMethod::OnProfilerChanged(bool on) {}

int FilterSkipFrameMethod::UpdateConfig(const std::string &config_str) {
  Json::Value config_jv;
  std::stringstream config_stream(config_str);
  Json::CharReaderBuilder reader_builder;
  std::string err_str;
  if (!Json::parseFromStream(reader_builder, config_stream, &config_jv,
                             &err_str)) {
    LOGE << "json parse failed " << err_str;
    return -1;
  }
  if (config_jv.isMember("border") && config_jv["border"].isNumeric()) {
    border_ = config_jv["border"].asInt();
  }
  if (config_jv.isMember("skip_num") && config_jv["skip_num"].isNumeric()) {
    skip_num_ = config_jv["skip_num"].asInt();
  }
  if (config_jv.isMember("min_width") && config_jv["min_width"].isNumeric()) {
    min_width_ = config_jv["min_width"].asInt();
  }
  if (config_jv.isMember("min_height") && config_jv["min_height"].isNumeric()) {
    min_height_ = config_jv["min_height"].asInt();
  }
  if (config_jv.isMember("min_score") && config_jv["min_score"].isDouble()) {
    min_score_ = config_jv["min_score"].asFloat();
  }

  LOGI << "border:" << border_ << " skip_num:" << skip_num_
       << " min_width:" << min_width_ << " min_height:" << min_height_
       << " min_score_" << min_score_;

  return 0;
}

}  // namespace xstream
