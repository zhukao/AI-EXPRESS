/*!
 * -------------------------------------------
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     profile.h
 * \Author   hangjun.yang
 * \Mail     hangjun.yang@horizon.ai
 * \Version  1.0.0.0
 * \Date     2020.08.03
 * \Brief    implement profile of message
 * -------------------------------------------
 */

#ifndef XPROTO_UTILS_PROFILE_H_
#define XPROTO_UTILS_PROFILE_H_

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"

namespace horizon {
namespace vision {
namespace xproto {

class MsgScopeProfile {
 public:
  explicit MsgScopeProfile(std::string msg_name) {
    message_name_ = msg_name;
    start_time_ = std::chrono::system_clock::now();
  }

  ~MsgScopeProfile() {
    std::lock_guard<std::mutex> lg(mutex_);
    auto end_time = std::chrono::system_clock::now();
    auto duration_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time_);
    LOGW << "[profile] message(" << message_name_ << ") exist "
         << duration_time.count() << " ms";

    for (auto itr = plugin_start_time_.begin(); itr != plugin_start_time_.end();
         ++itr) {
      std::string plugin_name = itr->first;
      if (plugin_stop_time_.count(plugin_name) > 0) {
        auto plugin_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                plugin_stop_time_[plugin_name] - itr->second);
        LOGW << "[profile] plugin(" << plugin_name << ") handle message("
             << message_name_ << ") cost " << plugin_time.count() << " ms";
      }
    }
  }

  void UpdatePluginStartTime(std::string plugin_name) {
    std::lock_guard<std::mutex> lg(mutex_);
    plugin_start_time_[plugin_name] = std::chrono::system_clock::now();
  }

  void UpdatePluginStopTime(std::string plugin_name) {
    std::lock_guard<std::mutex> lg(mutex_);
    plugin_stop_time_[plugin_name] = std::chrono::system_clock::now();
  }

 private:
  std::string message_name_;
  std::mutex mutex_;
  std::chrono::system_clock::time_point start_time_;
  std::map<std::string, std::chrono::system_clock::time_point>
      plugin_start_time_;
  std::map<std::string, std::chrono::system_clock::time_point>
      plugin_stop_time_;
};

}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // XPROTO_UTILS_PROFILE_H_
