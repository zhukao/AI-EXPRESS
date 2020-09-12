/*
 * @Description: implement of reorderplugin.cpp
 * @Author: shiyu.fu@horizon.ai
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include <cstdlib>
#include <cstring>
#include <utility>
#include <string>
#include <memory>
#include <fstream>

// Horizon Header
#include "hobotlog/hobotlog.hpp"
#include "json/json.h"

// custom header
#include "reorderplugin/reorderplugin.h"
#include "utils/time_helper.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace reorderplugin {

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_REORDER_MESSAGE)

using std::chrono::milliseconds;

std::string CustomReorderMessage::Serialize() {
  LOGI << "Serialize ReorderMessage";
  // 反序列化
  return "reordermessage";
}

ReorderPlugin::ReorderPlugin(std::string cfg_file) {
  std::ifstream ifs(cfg_file);
  HOBOT_CHECK(ifs.is_open()) << "Failed to load reorderplugin config file";
  Json::Value cfg_jv;
  ifs >> cfg_jv;
  need_reorder_ =
    cfg_jv.isMember("need_reorder") ? cfg_jv["need_reorder"].asInt() : 1;
  LOGI << "ReorderPlugin created";
}

ReorderPlugin::~ReorderPlugin() {}

int ReorderPlugin::Init() {
  // 注册智能帧结果
  RegisterMsg(TYPE_SMART_MESSAGE, std::bind(&ReorderPlugin::OnGetAppResult,
                                            this, std::placeholders::_1));
  // 注册主动丢帧结果
  RegisterMsg(TYPE_DROP_MESSAGE, std::bind(&ReorderPlugin::OnGetAppResult,
                                           this, std::placeholders::_1));

  // 调用父类初始化成员函数注册信息
  XPluginAsync::Init();
  while (!msg_queue_.empty()) {
    msg_queue_.pop();
  }
  return 0;
}

int ReorderPlugin::Deinit() {
  return 0;
}

int ReorderPlugin::Start() {
  LOGI << "ReorderPlugin Start";
  return 0;
}

int ReorderPlugin::Stop() {
  // thread_->join();
  LOGI << "ReorderPlugin Stop";
  return 0;
}

XProtoMessagePtr ReorderPlugin::Reorder(ReorderMsgPtr msg) {
  msg_queue_.push(msg);
  auto top = msg_queue_.top();
  if (top->frame_id_ == target_frame_id_) {
    target_frame_id_++;
    msg_queue_.pop();
    return top;
  } else {
    return nullptr;
  }
}

int ReorderPlugin::OnGetAppResult(XProtoMessagePtr msg) {
  if (!need_reorder_) {
    LOGD << "do not need reorder";
  } else {
    auto msg_type = msg->type();
    uint64_t frame_id;
    if (msg_type == "XPLUGIN_SMART_MESSAGE") {
      auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
      frame_id = smart_msg->frame_id;
      LOGD << "received smart message with frame_id: " << frame_id;
    } else if (msg_type == "XPLUGIN_DROP_MESSAGE") {
      auto drop_msg = std::static_pointer_cast<VioMessage>(msg);
      frame_id = drop_msg->sequence_id_;
      LOGD << "received drop message with frame_id: " << frame_id;
    } else {
      LOGE << "received unknown message with type: " << msg_type;
      return -1;
    }
    auto reorder_msg =
      std::make_shared<CustomReorderMessage>(msg, msg_type, frame_id);
    auto msg_to_send = Reorder(reorder_msg);
    if (msg_to_send != nullptr) {
      PushMsg(msg_to_send);
    }
  }
  return 0;
}

}  // namespace reorderplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
