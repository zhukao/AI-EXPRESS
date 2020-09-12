/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     MOT Method
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.15
 */

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "json/json.h"
#include "hobotxsdk/xstream_data.h"
#include "MOTMethod/error_code.h"
#include "hobotlog/hobotlog.hpp"
#include "MOTMethod/MOTMethod.h"
#include "MOTMethod/MOT/IOU.h"
#include "MOTMethod/data_type/mot_data_type.hpp"
// #include "MOTMethod/MOT/ReID.h"
#ifdef ENABLE_IOU2
#include "MOTMethod/MOT/IOU2.h"
#endif



namespace xstream {

int MOTMethod::Init(const std::string& config_file_path) {
  LOGI << "MOTMethod::Init " << config_file_path << std::endl;

  std::ifstream config_if(config_file_path);
  if (!config_if.good()) {
    LOGI << "MOTParam: no config, using default parameters" << std::endl;
    mot_ = std::make_shared<IOU>();
    mot_->MotInit(config_file_path);
  } else {
    Json::Value config_jv;
    config_if >> config_jv;
    std::string tracker_type;
    if (config_jv.isMember("tracker_type") &&
        config_jv["tracker_type"].isString())
      tracker_type = config_jv["tracker_type"].asString();
#ifdef ENABLE_IOU2
    LOGI << "tracker_type " << tracker_type << std::endl;
#endif
    if (tracker_type == "IOU") {
      mot_ = std::make_shared<IOU>();
      mot_->MotInit(config_file_path);
#ifdef ENABLE_IOU2
    } else if (tracker_type == "IOU_2.0") {
      mot_ = std::make_shared<IOU2>();
      mot_->MotInit(config_file_path);
#endif
    } else if (tracker_type == "ReID") {
      LOGE << "not support yet";
      return XSTREAM_MOT_ERR_PARAM;
    } else {
      LOGE << "config param error";
      return XSTREAM_MOT_ERR_PARAM;
    }
  }
  return XSTREAM_MOT_OK;
}

std::vector<std::vector<BaseDataPtr>> MOTMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>>& input,
    const std::vector<InputParamPtr>& param) {

  LOGI << "MOTMethod::DoProcess" << std::endl;
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];
    auto &param_batch_i = param[i];
    if (param_batch_i && param_batch_i->Format() == "pass-through") {
      LOGI << "pass-through";
      PassThrough(in_batch_i, out_batch_i);
    } else {
      LOGI << "mot";
      mot_->Track(in_batch_i, out_batch_i);
    }
  }
  return output;
}

void MOTMethod::Finalize() {
  mot_->MotFinalize();
  LOGI << "MOTMethod::Finalize" << std::endl;
}

static int CopyBaseDataVector(BaseDataVector *in, BaseDataVector *out) {
  if (!in || !out) {
    return XSTREAM_MOT_ERR_PARAM;
  }
  out->datas_ = in->datas_;
  out->state_ = in->state_;
  out->type_ = in->type_;
  out->name_ = in->name_;
  out->c_data_ = in->c_data_;
  out->error_code_ = in->error_code_;
  out->error_detail_ = in->error_detail_;
  return XSTREAM_MOT_OK;
}

int MOTMethod::PassThrough(const std::vector<BaseDataPtr> &in,
                     std::vector<BaseDataPtr> &out) {
  assert(!in.empty());
  auto in_rects = std::static_pointer_cast<BaseDataVector>(in[0]);
  assert("BaseDataVector" == in_rects->type_);

  auto out_rects = std::make_shared<BaseDataVector>();
  int ret = CopyBaseDataVector(in_rects.get(), out_rects.get());
  if (XSTREAM_MOT_OK != ret) {
    return ret;
  }
  auto out_disappeared_ids = std::make_shared<BaseDataVector>();
  out.push_back(std::static_pointer_cast<BaseData>(out_rects));
  out.push_back(std::static_pointer_cast<BaseData>(out_disappeared_ids));

  return XSTREAM_MOT_OK;
}

InputParamPtr MOTMethod::GetParameter() const {
  return mot_->GetParameter();
}

int MOTMethod::UpdateParameter(InputParamPtr ptr) {
  if (ptr->is_json_format_) {
    std::string content = ptr->Format();
    int ret = mot_->UpdateParameter(content);
    if (XSTREAM_MOT_OK != ret) {
      return ret;
    }
  } else {
     if (ptr && ptr->Format() == "Reset") {
      LOGI << "Reset mot";
      mot_->Reset();
      return XSTREAM_MOT_OK;
    }
    HOBOT_CHECK(0) << "only support json format config and reset config";
    return XSTREAM_MOT_ERR_PARAM;
  }
  return 0;
}

InputParamPtr Mot::GetParameter() {
  return config_param_;
}
}  // namespace xstream
