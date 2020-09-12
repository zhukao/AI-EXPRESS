/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     Postbox Filter Method
 * @author    wenhao.zou
 * @email     wenhao.zou@horizon.ai
 * @version   0.0.0.1
 * @date      2020.02.12
 */

#include "method/postbox_filter.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <random>
#if defined(HR_POSIX)
#include <pthread.h>
#endif
#include "method/bbox.h"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"
#include "hobotxstream/profiler.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int PostBoxFilter::Init(const std::string &config_file_path) {
  std::ifstream infile(config_file_path);
  if (infile.good()) {
    LOGI << "valid config file path";
    Json::Value cfg_jv;
    infile >> cfg_jv;
    has_thread_params_ = true;
    thr_policy_ = cfg_jv["thread_policy"].asString();
    thr_priority_ = cfg_jv["thread_priority"].asInt();
  }
  std::cout << "PostBoxFilter::Init " << config_file_path << std::endl;
  return 0;
}

int PostBoxFilter::UpdateParameter(InputParamPtr ptr) {
  if (ptr->is_json_format_) {
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING error;
    std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
    Json::Value out_jv;
    std::string content = ptr->Format();
    try {
      bool ok = json_reader->parse(content.c_str(),
                                   content.c_str() + content.size(),
                                   &out_jv, &error);
      if (ok && out_jv.isObject()) {
        if (out_jv.isMember("threshold") && out_jv["threshold"].isDouble()) {
          area_threshold_ = out_jv["threshold"].asDouble();
        }
        return 0;
      }
    } catch (std::exception &e) {
      return -1;
    }
  } else {
    auto real_ptr = dynamic_cast<xstream::FilterParam *>(ptr.get());
    if (real_ptr->HasThreshold()) {
      area_threshold_ = real_ptr->GetThreshold();
    }
  }
  return 0;
}

InputParamPtr PostBoxFilter::GetParameter() const {
  // method name is useless here
  auto param = std::make_shared<FilterParam>("");
  param->SetThreshold(area_threshold_);
  return param;
}

std::vector<std::vector<BaseDataPtr>> PostBoxFilter::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<InputParamPtr> &param) {

  std::cout << "PostBoxFilter::DoProcess" << std::endl;
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); ++i) {
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];
    out_batch_i.resize(in_batch_i.size());
    std::cout << "input size: " << in_batch_i.size() << std::endl;
    // 只支持n个输入，输入格式是BBox的数组
    for (size_t j = 0; j < in_batch_i.size(); ++j) {
      if (in_batch_i[j]->state_ == DataState::INVALID) {
        std::cout << "input slot " << j << " is invalid" << std::endl;
        continue;
      }
      RUN_PROCESS_TIME_PROFILER("PostBoxFilter_SingleBox")
      auto in_rects = std::static_pointer_cast<BaseDataVector>(in_batch_i[j]);
      assert("BaseDataVector" == in_rects->type_);
      auto out_rects = std::make_shared<BaseDataVector>();
      out_batch_i[j] = std::static_pointer_cast<BaseData>(out_rects);
      for (auto &in_rect : in_rects->datas_) {
        // if ("PostBoxFilter" == in_rect->type_){
          auto bbox = std::static_pointer_cast<xstream::BBox>(in_rect);
          if (bbox->value.Width() >= max_width_
            || bbox->value.Height() >= max_height_ ) {
            out_rects->datas_.push_back(in_rect);
          } else {
            std::cout << "filter out: "
                      << bbox->value.x1 << ","
                      << bbox->value.y1 << ","
                      << bbox->value.x2 << ","
                      << bbox->value.y2 << std::endl;
          }
        //}
      }
    }
  }
  return output;
}

void PostBoxFilter::Finalize() {
  std::cout << "BBoxFilter::Finalize" << std::endl;
}

std::string PostBoxFilter::GetVersion() const {
  return "test_only";
}

void PostBoxFilter::OnProfilerChanged(bool on) {
}

}  // namespace xstream
