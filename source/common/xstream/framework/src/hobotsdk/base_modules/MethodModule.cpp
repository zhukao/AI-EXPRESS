/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-12 00:03:04
 * @Version: v0.0.1
 * @Brief: MethodModule implementation.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 06:07:42
 */

#include "base_modules/MethodModule.h"

#include <memory>
#include <string>
#include <vector>

#include "BaseMessage.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {
MethodModule::MethodModule(const ThreadSafeMethodPtr &method,
                           const std::string &config_file_path, bool is_inited,
                           ProfilerPtr profiler, std::string unique_name,
                           bool is_use_config_file_path,
                           std::string instance_name)
    : hobot::Module(instance_name, "MethodModule"),
      method_inst_(method),
      is_inited_(is_inited),
      config_file_path_(config_file_path),
      config_mode_(ConfigMode::RAW_FILE_PATH),
      profiler_(profiler),
      unique_name_(unique_name) {
  LOGI << "raw file path";
}

MethodModule::MethodModule(const ThreadSafeMethodPtr &method,
                           const std::string &config, bool is_inited,
                           ProfilerPtr profiler, std::string unique_name,
                           std::string instance_name)
    : hobot::Module(instance_name, "MethodModule"),
      method_inst_(method),
      is_inited_(is_inited),
      config_mode_(ConfigMode::PARSED_JSON),
      profiler_(profiler),
      unique_name_(unique_name) {
  Json::CharReaderBuilder readerBuilder;
  JSONCPP_STRING errs;
  std::unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
  jsonReader->parse(
      config.c_str(), config.c_str() + config.length(), &config_, &errs);
  LOGI << "parsed json";
}

FORWARD_DEFINE(MethodModule, 0) {
  /**
   * 1.首先检查method是否是第一次执行，如果是的话需要先执行XStream::Method::InitMethod();
   *    因为类似Mxnet
   * predictor需要执行任务的线程创建，所有需要在forward中执行初始化，
   *    而不是在Module构造函数或者Init时候；这儿采用lazy
   * 执行初始化任务的方式，跟xstream 不同，xstream是Init时即完成初始化；
   * 2.创建timer，以支持任务的超时监控；超时后，设置对应输出为invalid，并调用workflow_return驱动下一个模块；
   * 3.调用method:DoProcess；
   * 4.DoProcess返回后需要检查是否已经超时，如果超时丢弃结果，不调用workflow_return;
   */
  if (!is_inited_) {
    if (config_mode_ == ConfigMode::RAW_FILE_PATH) {
      if (0 != method_inst_->Init(config_file_path_)) {
        LOGE << "method " << GetFullInstanceName() << " initial failed";
        exit(1);
      }
    } else if (config_mode_ == ConfigMode::PARSED_JSON) {
      Json::StreamWriterBuilder builder;
      std::string output = Json::writeString(builder, config_);
      if (0 != method_inst_->InitFromJsonString(output)) {
        LOGE << "method " << GetFullInstanceName() << " initial failed";
        exit(1);
      }
    } else {
      HOBOT_CHECK(false) << "invalid config mode.";
    }
    is_inited_ = true;
  }

  auto in_msg =
      std::static_pointer_cast<XStreamMethodInputMessage>((*input[0])[0]);
  if (in_msg->msg_type_ == XStreamMsgType::INIT) {
    LOGD << GetFullInstanceName() << ", inst id=" << inst_id_;
    // first instance will process init message.
    if (inst_id_ == 0) {
      auto res = std::make_shared<XStreamMethodOutputMessage>();
      std::vector<BaseDataPtr> outputs;
      for (uint32_t i = 0; i < output_slot_num_; ++i) {
        outputs.push_back(std::make_shared<BaseData>());
      }
      res->data_list_.push_back(outputs);
      res->SyncXStreamData(in_msg);
      workflow->Return(this, 0, hobot::spMessage(res), context);
    }
    return;
  }
  // TODO(SONSHAN.GONG): 创建Timer
  RUN_FPS_PROFILER_WITH_PROFILER(profiler_, unique_name_)
  // 调用method::DoProess
  auto res = method_inst_->DoProcess(in_msg->data_list_, in_msg->input_param_);

  // TODO(SONGSHAN.GONG): CHECK是否超时．
  auto out_msg = std::make_shared<XStreamMethodOutputMessage>(res);
  out_msg->SyncXStreamData(in_msg);
  workflow->Return(this, 0, hobot::spMessage(out_msg), context);
}
int MethodModule::UpdateParameter(InputParamPtr ptr) {
  auto ret = method_inst_->UpdateParameter(ptr);
  if (0 != ret) {
    LOGE << "Failed to update parameter for " << ptr->unique_name_
         << " with code " << ret;
    return -1;
  }
  return 0;
}

InputParamPtr MethodModule::GetParameter() const {
  if (!method_inst_) {
    return nullptr;
  }
  return method_inst_->GetParameter();
}

std::string MethodModule::GetVersion() const {
  if (!method_inst_) {
    return "";
  }
  return method_inst_->GetVersion();
}
}  // namespace xstream
