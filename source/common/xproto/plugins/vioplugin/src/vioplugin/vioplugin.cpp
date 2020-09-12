/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 15:35:08
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include <fstream>
#include <iostream>

#include "hobotlog/hobotlog.hpp"
#include "vioplugin/vioplugin.h"
#include "vioplugin/vioproduce.h"

#include "utils/time_helper.h"

#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/hbipcplugin_data.h"

#include "xproto_msgtype/protobuf/pack.pb.h"
#include "xproto_msgtype/protobuf/x2.pb.h"

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_IMAGE_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_DROP_MESSAGE)

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {

using horizon::vision::xproto::basic_msgtype::HbipcMessage;

int VioPlugin::OnGetHbipcResult(XProtoMessagePtr msg) {
  int ret = 0;
  std::string proto_str;
  static int fps = 0;
  // 耗时统计
  static auto lastTime = hobot::Timer::tic();
  static int frameCount = 0;

  ++frameCount;

  auto curTime = hobot::Timer::toc(lastTime);
  // 统计数据发送帧率
  if (curTime > 1000) {
    fps = frameCount;
    frameCount = 0;
    lastTime = hobot::Timer::tic();
    LOGE << "[HbipcPlugin] fps = " << fps;
  }

  auto hbipc_message = std::static_pointer_cast<HbipcMessage>(msg);
  x2::InfoMessage proto_info_message;
  proto_info_message.ParseFromString(hbipc_message->Serialize());
  if (proto_info_message.config__size() <= 0) {
    LOGE << "PB don't have config param";
    return false;
  }
  for (auto i = 0; i < proto_info_message.config__size(); ++i) {
    auto &params = proto_info_message.config_(i);
    if (params.type_() == "Detect") {
      LOGI << "Set CP detect params";
      for (auto j = 0; j < params.shield__size(); ++j) {
        box_t box;
        auto &shield = params.shield_(j);
        if (shield.type_() == "valid_zone" ||
            shield.type_() == "invalid_zone") {
          auto &point1 = shield.top_left_();
          auto &point2 = shield.bottom_right_();
          box.x1 = point1.x_();
          box.y1 = point1.y_();
          box.x2 = point2.x_();
          box.y2 = point2.y_();
          LOGI << shield.type_() << ":" << box.x1 << " " << box.y1 << " "
               << box.x2 << " " << box.y2;
          Shields_.emplace_back(box);
        } else {
          LOGE << "invalid zone type: " << shield.type_();
        }
      }
    }
  }
  return ret;
}

VioPlugin::VioPlugin(const std::string &path) {
  config_ = GetConfigFromFile(path);
  config_->SetConfig(config_);
  HOBOT_CHECK(config_);
}

int VioPlugin::Init() {
  ClearAllQueue();
  auto data_source_ = config_->GetValue("data_source");
  VioProduceHandle_ = VioProduce::CreateVioProduce(data_source_);
  HOBOT_CHECK(VioProduceHandle_);
  VioProduceHandle_->SetConfig(config_);

  if (!is_sync_mode_) {
#ifndef PYAPI
    // 注册智能帧结果
    RegisterMsg(TYPE_HBIPC_MESSAGE,
                std::bind(&VioPlugin::OnGetHbipcResult, this,
                          std::placeholders::_1));
#else
    for (auto const& it : message_cb_) {
      RegisterMsg(it.first, it.second);
    }
#endif  // end PYAPI
    // 调用父类初始化成员函数注册信息
    XPluginAsync::Init();
  } else {
    LOGI << "Sync mode";
    // XPluginAsync::Init();
  }
  is_inited_ = true;
  return 0;
}

VioPlugin::~VioPlugin() { delete config_; }

int VioPlugin::Start() {
  int ret;

  auto send_frame = [&](const std::shared_ptr<VioMessage> input) {
    if (!input) {
      LOGE << "VioMessage is NULL, return";
      return -1;
    }

    if (!is_sync_mode_) {
      PushMsg(input);
    } else {
      if (input->type_ == TYPE_IMAGE_MESSAGE) {
        img_msg_queue_.push(input);
      } else if (input->type_ == TYPE_DROP_MESSAGE) {
        drop_msg_queue_.push(input);
      } else {
        LOGE << "received message with unknown type";
      }
    }
    return 0;
  };

  VioProduceHandle_->SetListener(send_frame);
  ret = VioProduceHandle_->Start();
  if (ret < 0) {
    LOGF << "VioPlugin start failed, err: " << ret << std::endl;
    return -1;
  }

  return 0;
}

int VioPlugin::Stop() {
  ClearAllQueue();
  VioProduceHandle_->Stop();
  return 0;
}

VioConfig *VioPlugin::GetConfigFromFile(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    LOGF << "Open config file " << path << " failed";
    return nullptr;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  ifs.close();
  std::string content = ss.str();
  Json::Value value;
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING error;
  std::shared_ptr<Json::CharReader> reader(builder.newCharReader());
  try {
    bool ret = reader->parse(content.c_str(), content.c_str() + content.size(),
                             &value, &error);
    if (ret) {
      auto *config = new VioConfig(path, value);
      return config;
    } else {
      return nullptr;
    }
  } catch (std::exception &e) {
    return nullptr;
  }
}

XProtoMessagePtr VioPlugin::GetImage() {
  // compare img_msg_queue.head.frame_id with drop_msg_queue.head.frame_id
  // if former is greater than latter, pop from drop_msg_queue
  if (img_msg_queue_.size() == 0) {
    LOGI << "image message queue is empty";
    return nullptr;
  }
  auto img_msg = img_msg_queue_.peek();
  auto img_vio_msg = dynamic_cast<VioMessage *>(img_msg.get());
  auto img_msg_seq_id = img_vio_msg->sequence_id_;
  img_msg_queue_.pop();
  for (size_t i = 0; i < drop_msg_queue_.size(); ++i) {
    auto drop_msg = drop_msg_queue_.peek();
    auto drop_vio_msg = dynamic_cast<VioMessage *>(drop_msg.get());
    auto drop_msg_seq_id = drop_vio_msg->sequence_id_;
    if (drop_msg_seq_id != ++(img_msg_seq_id)) {
      if (drop_msg_seq_id < img_msg_seq_id) {
        LOGI << "Dropped msg id less than image msg id";
        drop_msg_queue_.pop();
      } else {
        LOGI << "Current image msg id: " << img_msg_seq_id
             << "dropped msg id: " << drop_msg_seq_id;
      }
    } else {
      // TODO(shiyu.fu): send drop message
      drop_msg_queue_.pop();
    }
  }
  return img_msg;
}

void VioPlugin::ClearAllQueue() {
  img_msg_queue_.clear();
  drop_msg_queue_.clear();
}

#ifdef PYAPI
int VioPlugin::AddMsgCB(const std::string msg_type, pybind11::function cb) {
  // wrap python callback into XProtoMessageFunc
  auto callback = [=](XProtoMessagePtr msg) -> int {
    // TODO(shiyu.fu): handle specific message
    pybind11::object py_input = pybind11::cast(msg);
    cb(py_input);
    return 0;
  };
  message_cb_[msg_type] = callback;
  return 0;
}
#endif

}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
