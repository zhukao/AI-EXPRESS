/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file profiler.cpp
 * @brief
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 * @date 2019/4/15
 */

#include "hobotxstream/profiler.h"

#include <errno.h>
#include <string.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#include "hobotlog/hobotlog.hpp"
#include "json/json.h"
namespace hobotprofiler {
static std::vector<std::string> split(const std::string &str, char delim) {
  std::vector<std::string> elems;
  std::istringstream iss(str);
  for (std::string item; getline(iss, item, delim);)
    if (item.empty()) {
      continue;
    } else {
      item.erase(0, item.find_first_not_of(" "));
      item.erase(item.find_last_not_of(" ") + 1);
      elems.push_back(item);
    }
  return elems;
}
}  // namespace hobotprofiler

int TimeStatisticInfo::cycle_ms = 3000;

int FpsStatisticInfo::cycle_ms = 200 * 1000;

ProfilerPtr Profiler::instance_;

/// used to guarantee the Profiler::instance_ is created only once
std::once_flag create_profiler_flag;

ProfilerPtr Profiler::Get() {
  if (!instance_) {
    std::call_once(create_profiler_flag, []() {
      instance_ = ProfilerPtr(new Profiler());
      instance_->SetOutputFile("PROFILER_METHOD.txt");
    });
  }
  return instance_;
}

// Profiler::~Profiler() {
//   std::lock_guard<std::mutex> lock(lock_);
//   if (foi_.is_open()) {
//     foi_.close();
//   }
// }

Profiler::~Profiler() {
  LOGD << "~Profiler() Start";
  std::lock_guard<std::mutex> lock(lock_);

  Json::Value array_config;

  if (!foi_.is_open()) {
    LOGD << "~Profiler() End";
    return;
  }
  std::string line;
  foi_.seekg(std::ios::beg);  // 从头开始读取

  while (getline(foi_, line)) {  // line中不包括每行的换行符
    std::vector<std::string> elements = hobotprofiler::split(line, '|');
    if (elements.size() != 5) {
      std::cout << "Record format invailed!" << std::endl;
      continue;
    }

    Json::Value item;
    item["pid"] = "0";
    item["name"] = elements[0];
    item["tid"] = elements[1];
    item["ph"] = elements[2];
    item["ts"] = elements[3];
    // 判断是哪种类型
    if (elements[2] == "X") {
      item["dur"] = elements[4];
      array_config.append(item);
    } else if (elements[2] == "C") {
      Json::Value args;
      args["frame cnt"] = elements[4];
      item["args"] = args;
      array_config.append(item);
    }
  }

  foi_.clear();               // 从头开始写入
  foi_.seekg(std::ios::beg);  // 从头开始写入
  foi_ << array_config.toStyledString();

  if (foi_.is_open()) {
    foi_.close();
  }
  LOGD << "~Profiler() End";
}

void Profiler::Log(const std::stringstream &ss) {
  // TODO(SONGSHAN.GONG): fstream or iostream may not thread-safe,
  // need replace as an thread-safe logging system.
  std::lock_guard<std::mutex> lock(lock_);
  if (foi_.is_open()) {
    foi_ << ss.str();
  } else {
    std::cout << ss.str() << std::endl;
  }
}

void Profiler::SetState(Profiler::State state) {
  std::lock_guard<std::mutex> lock(lock_);
  if (state != state_) {
    state_ = state;
    //
    for (auto &stat : fps_stats_) {
      stat.second->cnt = 0;
    }
    for (auto &stat : time_stats_) {
      stat.second->pre_time = 0;
    }
  }
}

bool Profiler::SetOutputFile(const std::string &file) {
  std::lock_guard<std::mutex> lock(lock_);
  if (foi_.is_open()) {
    foi_.close();
  }
  foi_.open(file, std::fstream::out | std::fstream::in | std::fstream::binary |
                      std::fstream::trunc);
  if (foi_.fail()) {
    LOGE << "Failed to open " << file << ", error msg is " << strerror(errno);
    return false;
  }
  foi_.seekg(0, std::ios::beg);  // 从头开始写入
  return true;
}

void Profiler::SetIntervalForTimeStat(int cycle_ms) {
  HOBOT_CHECK_GE(cycle_ms, 1);
  TimeStatisticInfo::cycle_ms = cycle_ms;
  LOGI << "SetFrameIntervalForTimeStat to " << cycle_ms;
}

void Profiler::SetIntervalForFPSStat(int cycle_ms) {
  HOBOT_CHECK_GE(cycle_ms, 1);
  FpsStatisticInfo::cycle_ms = cycle_ms;
  LOGI << "SetTimeIntervalForFPSStat to " << cycle_ms;
}

std::unique_ptr<ProfilerScope> Profiler::CreateScope(const std::string &name,
                                                     const std::string &tag) {
  std::lock_guard<std::mutex> lock(lock_);

  auto id = std::this_thread::get_id();
  std::stringstream ss;
  ss << id;
  std::string thread_id = name + "_" + ss.str();

  if (tag == "FPS") {
    std::shared_ptr<FpsStatisticInfo> curr_stat;
    if (fps_stats_.find(thread_id) == fps_stats_.end()) {
      fps_stats_[thread_id] = std::make_shared<FpsStatisticInfo>();
    }
    curr_stat = fps_stats_[thread_id];
    std::unique_ptr<ProfilerScope> scope(
        new ScopeFps(shared_from_this(), name, tag, curr_stat));
    return scope;
  } else if (tag == "TIME") {
    std::shared_ptr<TimeStatisticInfo> curr_stat;
    if (time_stats_.find(thread_id) == time_stats_.end()) {
      time_stats_[thread_id] = std::make_shared<TimeStatisticInfo>();
    }
    curr_stat = time_stats_[thread_id];
    std::unique_ptr<ProfilerScope> scope(
        new ScopeTime(shared_from_this(), name, tag, curr_stat));
    return scope;
  }
  HOBOT_CHECK(false) << "Not support scope: " << tag;
  return nullptr;
}
