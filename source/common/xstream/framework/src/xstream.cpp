/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework interface
 * @file      xstream.cpp
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */

#include "hobotxstream/xstream.h"

#include <future>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_error.h"
#include "hobotxstream/profiler.h"
#include "hobotxstream/xstream_config.h"
#include "hobotxstream/default_factory.hpp"

namespace xstream {

XStreamSDK *XStreamSDK::CreateSDK() { return new XStreamFlow(); }

XStreamFlow::XStreamFlow() {
  is_initial_ = false;
  profiler_ = ProfilerPtr(new Profiler());
#ifdef HOBOTSDK_ENGINE
  scheduler_ = std::make_shared<SchedulerV2>();
#else
  scheduler_ = std::make_shared<Scheduler>();
#endif
}

XStreamFlow::~XStreamFlow() {}

int XStreamFlow::Init() {
  return Init(MethodFactoryPtr(new DefaultMethodFactory()));
}

int XStreamFlow::Init(MethodFactoryPtr method_factory) {
  std::unique_lock<std::mutex> locker(mutex_);
  if (is_initial_) {
    return -2;
  }
  auto config = std::make_shared<XStreamConfig>();
  int ret;

  config->method_factory_ = method_factory;
  if (0 == (ret = config->LoadFile(config_file_))) {
    if (0 == (ret = scheduler_->Init(config, profiler_))) {
      is_initial_ = true;
      return 0;
    }
  }
  return ret;
}

int XStreamFlow::SetConfig(const std::string &key, const std::string &value) {
  std::unique_lock<std::mutex> locker(mutex_);
  if (key.compare("config_file") == 0) {
    config_file_ = value;
    param_dict_["config_file"] = config_file_;
  } else if (key.compare("profiler") == 0) {
    if (value.compare("on") == 0) {
      LOGI << "profile start!";
      profiler_->Start();
      Profiler::Get()->Start();
    } else {
      profiler_->Stop();
      Profiler::Get()->Stop();
    }
  } else if (key.compare("profiler_file") == 0) {
    if (!profiler_->SetOutputFile(value)) {
      return -1;
    }
  } else if (key.compare("profiler_name") == 0) {
    profiler_->name_ = value;
  } else if (key.compare("profiler_time_interval") == 0) {
    profiler_->SetIntervalForTimeStat(std::stoi(value));
  } else if (key.compare("profiler_fps_interval") == 0) {
    profiler_->SetIntervalForFPSStat(std::stoi(value));
  } else if (key.compare("free_framedata") == 0) {
    if (value.compare("on") == 0) {
      scheduler_->SetFreeMemery(true);
    } else {
      scheduler_->SetFreeMemery(false);
    }
  } else {
    LOGD << "config " << key << " is not supported yet";
    return -1;
  }
  return 0;
}

int XStreamFlow::SetCallback(XStreamCallback callback,
                             const std::string &name) {
  std::unique_lock<std::mutex> locker(mutex_);
  if ("" == name) {
    callback_ = callback;
    return scheduler_->SetCallback(callback_);
  } else {
    return scheduler_->SetCallback(callback, name);
  }
}

int XStreamFlow::UpdateConfig(const std::string &unique_name,
                              InputParamPtr param_ptr) {
  std::unique_lock<std::mutex> locker(mutex_);
  if (!is_initial_) {
    return -1;
  }
  return scheduler_->UpdateConfig(unique_name, param_ptr);
}

InputParamPtr XStreamFlow::GetConfig(const std::string &unique_name) const {
  //  std::unique_lock<std::mutex> locker(mutex_);
  return scheduler_->GetConfig(unique_name);
}

std::string XStreamFlow::GetVersion(const std::string &unique_name) const {
  return scheduler_->GetVersion(unique_name);
}

OutputDataPtr XStreamFlow::OnError(int64_t error_code,
                                   const std::string &error_detail) {
  auto output = std::make_shared<OutputData>();
  output->error_code_ = error_code;
  output->error_detail_ = error_detail;
  return output;
}

// 同步接口，单路输出
OutputDataPtr XStreamFlow::SyncPredict(InputDataPtr input) {
  if (!input || input->datas_.size() <= 0) {
    OnError(HOBOTXSTREAM_ERROR_INPUT_INVALID, "input error");
  }
  std::promise<std::vector<OutputDataPtr>> promise;
  auto future = promise.get_future();

  int64_t ret = scheduler_->Input(input, &promise);
  if (ret >= 0) {
    auto output = future.get();
    HOBOT_CHECK(output.size() == 1) << "Output size error";
    return output[0];
  } else {
    return OnError(ret, "exceed max running count");
  }
}

// 同步接口，多路输出
std::vector<OutputDataPtr> XStreamFlow::SyncPredict2(InputDataPtr input) {
  if (!input || input->datas_.size() <= 0) {
    OnError(HOBOTXSTREAM_ERROR_INPUT_INVALID, "input error");
  }
  std::promise<std::vector<OutputDataPtr>> promise;
  auto future = promise.get_future();

  int64_t ret = scheduler_->Input(input, &promise);
  if (ret >= 0) {
    return future.get();
  } else {
    std::vector<OutputDataPtr> err_output;
    err_output.push_back(OnError(ret, "exceed max running count"));
    return err_output;
  }
}

int64_t XStreamFlow::AsyncPredict(InputDataPtr input) {
  HOBOT_CHECK(callback_)
      << "callback error, AsyncPredict need a valid callback function.";
  return scheduler_->Input(input, nullptr);
}

}  // namespace xstream
