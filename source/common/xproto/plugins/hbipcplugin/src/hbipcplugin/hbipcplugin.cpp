/*
 * @Description: implement of hbipcplugin.cpp
 * @Author: yingmin.li@horizon.ai
 * @Date: 2019-08-24 11:14:33
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 15:06:15
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include <cstdlib>
#include <cstring>
#include <utility>
#include <fstream>

// Horizon Header
#include "hobotlog/hobotlog.hpp"

// custom header
#include "hbipcplugin/hbipcplugin.h"
#include "hbipcplugin/hbipcsession.h"
#include "utils/time_helper.h"
#include "xproto_msgtype/protobuf/x3.pb.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace hbipcplugin {

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_HBIPC_MESSAGE)

using std::chrono::milliseconds;

std::string CustomHbipcMessage::Serialize() {
  LOGI << "Serialize HbipcMessage";
  //反序列化
  pack::MessagePack proto_pack_message;
  proto_pack_message.ParseFromString(proto_);
  if (proto_pack_message.flow_() == pack::MessagePack_Flow_AP2CP) {
    if (proto_pack_message.type_() == pack::MessagePack_Type_kXConfig) {
      return proto_pack_message.content_();
    } else {
      return "";
    }
  } else {
    return "";
  }
}

HbipcPlugin::HbipcPlugin(std::string path) {
  std::ifstream ifs(path);
  HOBOT_CHECK(ifs.is_open()) << "Failed to load hbipcplugin config file";
  Json::Value cfg_jv;
  ifs >> cfg_jv;
  max_err_num_ = cfg_jv.isMember("max_err_num") ? cfg_jv["max_err_num"].asInt()
                                                : 100;
  send_err_cnt_ = 0;
  recv_err_cnt_ = 0;
  need_reorder_ =
      cfg_jv.isMember("need_reorder") ? cfg_jv["need_reorder"].asInt() : 0;
  HbipcSession::Instance().SConfig(path);
}

HbipcPlugin::~HbipcPlugin() {}

int HbipcPlugin::Init() {
  int ret = ERROR_HBIPC_OK;
  // 初始化，向总线注册需要的消息类型与回调函数
  // RegisterMsg(TYPE_HBIPC_MESSAGE, std::bind(&HbipcPlugin::OnGetHbipcResult,
  //                                              this, std::placeholders::_1));

  if (!need_reorder_) {
    // 注册智能帧结果
    RegisterMsg(TYPE_SMART_MESSAGE, std::bind(&HbipcPlugin::OnGetSmartResult,
                                              this, std::placeholders::_1));
    // 注册主动丢帧结果
    RegisterMsg(TYPE_DROP_MESSAGE,
                std::bind(&HbipcPlugin::OnGetDropResult, this,
                          std::placeholders::_1));
  } else {
    // 注册重排结果
    RegisterMsg(TYPE_REORDER_MESSAGE, std::bind(&HbipcPlugin::OnGetReorderMsg,
                                                this, std::placeholders::_1));
  }

  // 初始化系统HBIPC接口
  if ((ret = HbipcSession::Instance().SInitConnection()) != ERROR_HBIPC_OK) {
    LOGE << "[HbipcPlugin] init HbipcSession, error code = "
         << HbipcSession::Instance().SGetInitErrorCode();
    return ret;
  }
  // 调用父类初始化成员函数注册信息
  XPluginAsync::Init();
  droped_queue_.clear();
  return ret;
}

int HbipcPlugin::Deinit() {
  int ret = ERROR_HBIPC_OK;
  // 反初始化系统级的HBIPC接口
  if ((ret = HbipcSession::Instance().SDeinitConnection()) != ERROR_HBIPC_OK) {
    LOGE << "[HbipcPlugin] deinit HbipcSession, error code = "
         << HbipcSession::Instance().SGetInitErrorCode();
    return ret;
  }
  return ret;
}

void HbipcPlugin::ExecLoop() {
  int ret = ERROR_HBIPC_OK;
  do {
    LOGD << "Enter execloop " << std::endl;
    std::string proto_;
    if ((ret = HbipcSession::Instance().SRecv(&proto_)) != ERROR_HBIPC_OK) {
      LOGE << "[HbipcPlugin] hbipc recv, error code = "
           << HbipcSession::Instance().SGetRecvErrorCode();
      if (++recv_err_cnt_ > max_err_num_) {
        LOGE << "Exit, exceed max error num: " << max_err_num_;
        raise(SIGINT);
      }
    } else {
      recv_err_cnt_ = 0;
      auto msg = std::make_shared<CustomHbipcMessage>(proto_);
      PushMsg(msg);
      LOGD << "[HbipcPlugin] hbipc recv success";
    }
  } while (!is_stop_);
}

int HbipcPlugin::Start() {
  is_stop_ = false;
  thread_ =
      std::make_shared<std::thread>(std::bind(&HbipcPlugin::ExecLoop, this));
  return ERROR_HBIPC_OK;
}

int HbipcPlugin::Stop() {
  is_stop_ = true;
  thread_->join();
  return ERROR_HBIPC_OK;
}

#if 0
std::string HbipcPlugin::Serialize(std::shared_ptr<HbipcMessage> msg) {
  std::string proto_str;
  pack::MessagePack proto_pack_message;

  proto_pack_message.set_type_(type_);
  proto_pack_message.set_content_(msg->Serialize());
  proto_pack_message.clear_sequence_id_();
  proto_pack_message.clear_timestamp_();
  proto_pack_message.SerializeToString(&proto_str);
  return proto_str;
}

int HbipcPlugin::OnGetHbipcResult(XProtoMessagePtr msg) {
  int ret = ERROR_HBIPC_OK;
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

  auto hbipc_message = std::static_pointer_cast<CustomHbipcMessage>(msg);

  if ((ret = HbipcSession::Instance().SSend(Serialize(hbipc_message))) !=
      ERROR_HBIPC_OK) {
    LOGE << "[HbipcPlugin] hbipc send, error code = "
         << HbipcSession::Instance().SGetSendErrorCode();
    return ret;
  }
  return ret;
}
#endif

std::string HbipcPlugin::DropPack(std::shared_ptr<VioMessage> msg) {
  std::string proto_str;
  x3::FrameMessage proto_frame_message;
  proto_frame_message.set_timestamp_(msg->time_stamp_);
  proto_frame_message.set_sequence_id_(msg->sequence_id_);
  auto smart_msg = proto_frame_message.mutable_smart_msg_();
  smart_msg->set_timestamp_(msg->time_stamp_);
  smart_msg->set_error_code_(1);
  proto_frame_message.SerializeToString(&proto_str);
  return proto_str;
}

int HbipcPlugin::OnGetDropResult(XProtoMessagePtr msg) {
  int ret = ERROR_HBIPC_OK;
  if (is_stop_ == false) {
    auto drop_message = std::static_pointer_cast<VioMessage>(msg);
    LOGD << "[GET DROP]The drop frame seq:" << drop_message->sequence_id_
         << ", ts:" << drop_message->time_stamp_;
    droped_queue_.push(drop_message);

    if (droped_queue_.size() > 0) {
      auto cur_frame_id = last_smart_frame_id_;
      auto size = droped_queue_.size();
      for (size_t i = 0; i < size; i++) {
        auto drop_frame = droped_queue_.peek();
        // it must be in order and if droped sequence id > (cur_frame_id + 1)
        // then cur_frame_id + 1 frame must being processed by smart plugin
        if (drop_frame->sequence_id_ != cur_frame_id + 1) {
          if (drop_frame->sequence_id_ < cur_frame_id + 1) {
            LOGW << "droped frame id is less than last smart frame";
            droped_queue_.pop();
          } else {
            break;
          }
        } else {
          cur_frame_id++;
          if ((ret = HbipcSession::Instance().SSend(DropPack(drop_frame))) !=
              ERROR_HBIPC_OK) {
            LOGE << "[HbipcPlugin] hbipc send, error code = "
                 << HbipcSession::Instance().SGetSendErrorCode();
            if (++send_err_cnt_ > max_err_num_) {
              LOGE << "Exit, exceed max error num: " << max_err_num_;
              raise(SIGINT);
            }
            return ret;
          } else {
            send_err_cnt_ = 0;
          }
          droped_queue_.pop();
        }
      }
    }
  }
  return ret;
}

std::string HbipcPlugin::SmartPack(std::shared_ptr<SmartMessage> msg) {
  std::string proto_str;
  pack::MessagePack proto_pack_message;

  LOGI << "[SMART]The smart frame seq:" << msg->frame_id
       << ", ts:" << msg->time_stamp;

  proto_pack_message.set_flow_(pack::MessagePack_Flow_CP2AP);
  proto_pack_message.set_type_(pack::MessagePack_Type_kXPlugin);
  auto addition = proto_pack_message.mutable_addition_();
  auto frame = addition->mutable_frame_();
  frame->set_sequence_id_(msg->frame_id);
  frame->set_timestamp_(msg->time_stamp);
  frame->set_frame_type_(pack::Frame_FrameType_SmartFrame);
  proto_pack_message.set_content_(msg->Serialize());
  proto_pack_message.SerializeToString(&proto_str);
  return proto_str;
}

int HbipcPlugin::OnGetSmartResult(XProtoMessagePtr msg) {
  if (is_stop_ == false) {
    int ret = ERROR_HBIPC_OK;
    static uint64_t smart_time_last;
    // 实际智能帧率计算
    static int fps = 0;
    // 耗时统计，ms
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

    auto smart_message = std::static_pointer_cast<SmartMessage>(msg);
    if (smart_time_last != 0 && smart_time_last > smart_message->time_stamp) {
      LOGE << "Current timestamp is wrong!!!"
           << " last time stamp: " << smart_time_last
           << " current time stamp: " << smart_message->time_stamp;
    }

    LOGI << "[GET SMART]The smart frame seq:" << smart_message->frame_id
         << ", ts:" << smart_message->time_stamp;
    smart_time_last = smart_message->time_stamp;
    if ((ret = HbipcSession::Instance().SSend(smart_message->Serialize())) !=
        ERROR_HBIPC_OK) {
      LOGE << "[HbipcPlugin] hbipc send, error code = "
           << HbipcSession::Instance().SGetSendErrorCode();
      if (++send_err_cnt_ > max_err_num_) {
        LOGE << "Exit, exceed max error num: " << max_err_num_;
        raise(SIGINT);
      }
      return ret;
    } else {
      send_err_cnt_ = 0;
    }
    last_smart_frame_id_ = smart_message->frame_id;
  }
  return ERROR_HBIPC_OK;
}

int HbipcPlugin::OnGetReorderMsg(const XProtoMessagePtr msg) {
  int ret = ERROR_HBIPC_OK;
  if (is_stop_ == false) {
    auto reorder_msg = std::static_pointer_cast<ReorderMessage>(msg);
    auto actual_type = reorder_msg->actual_type_;
    if (actual_type == "XPLUGIN_SMART_MESSAGE") {
      static uint64_t smart_time_last;
      // 实际智能帧率计算
      static int fps = 0;
      // 耗时统计，ms
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
      auto smart_msg =
        std::static_pointer_cast<SmartMessage>(reorder_msg->actual_msg_);
      if (smart_time_last != 0 && smart_time_last > smart_msg->time_stamp) {
      LOGE << "Current timestamp is wrong!!!"
           << " last time stamp: " << smart_time_last
           << " current time stamp: " << smart_msg->time_stamp;
      }

      LOGI << "[GET SMART]The smart frame seq:" << smart_msg->frame_id
          << ", ts:" << smart_msg->time_stamp;
      smart_time_last = smart_msg->time_stamp;
      if ((ret = HbipcSession::Instance().SSend(smart_msg->Serialize())) !=
          ERROR_HBIPC_OK) {
        LOGE << "[HbipcPlugin] hbipc send, error code = "
            << HbipcSession::Instance().SGetSendErrorCode();
        if (++send_err_cnt_ > max_err_num_) {
          LOGE << "Exit, exceed max error num: " << max_err_num_;
          raise(SIGINT);
        }
        return ret;
      } else {
        send_err_cnt_ = 0;
      }
    } else if (actual_type == "XPLUGIN_DROP_MESSAGE") {
      auto drop_msg =
        std::static_pointer_cast<VioMessage>(reorder_msg->actual_msg_);
      if ((ret = HbipcSession::Instance().SSend(DropPack(drop_msg))) !=
          ERROR_HBIPC_OK) {
        LOGE << "[HbipcPlugin] hbipc send, error code = "
              << HbipcSession::Instance().SGetSendErrorCode();
        if (++send_err_cnt_ > max_err_num_) {
          LOGE << "Exit, exceed max error num: " << max_err_num_;
          raise(SIGINT);
        }
        return ret;
      } else {
        send_err_cnt_ = 0;
      }
    } else {
      LOGE << "received reorder message with type: " << actual_type;
      return -1;
    }
  }
  return ret;
}

}  // namespace hbipcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
