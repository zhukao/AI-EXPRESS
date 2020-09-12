/*!
 * Copyright (c) 2020-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     hid_manager.cpp
 * \Author   zhe.sun
 * \Version  1.0.0.0
 * \Date     2020.6.10
 * \Brief    implement of api file
 */
#include "./hid_manager.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "smartplugin/smartplugin.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
using horizon::vision::xproto::XPluginErrorCode;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::smartplugin::CustomSmartMessage;
using horizon::vision::xproto::smartplugin::VehicleSmartMessage;

HidManager::HidManager(std::string config_path) {
  config_path_ = config_path;
  LOGI << "HidManager smart config file path:" << config_path_;
  stop_flag_ = false;
  thread_ = nullptr;
}

HidManager::~HidManager() {}

int HidManager::Init() {
  LOGI << "HidManager Init";
  // config_
  if (config_path_ != "") {
    std::ifstream ifs(config_path_);
    if (!ifs.is_open()) {
      LOGF << "open config file " << config_path_ << " failed";
      return -1;
    }
    Json::CharReaderBuilder builder;
    std::string err_json;
    try {
      bool ret = Json::parseFromStream(builder, ifs, &config_, &err_json);
      if (!ret) {
        LOGF << "invalid config file " << config_path_;
        return -1;
      }
    } catch (std::exception &e) {
      LOGF << "exception while parse config file " << config_path_ << ", "
           << e.what();
      return -1;
    }
    // smart_type_
    if (config_.isMember("smart_type")) {
      smart_type_ = static_cast<SmartType>(config_["smart_type"].asInt());
    }

    // hid_file_
    if (config_.isMember("hid_file")) {
      hid_file_ = config_["hid_file"].asString();
    }
  }

  // hid_file_handle_
  hid_file_handle_ = open(hid_file_.c_str(), O_RDWR, 0666);
  if (hid_file_handle_ < 0) {
    LOGE << "open hid device file fail: " << strerror(errno);
    return -1;
  }
  LOGD << "Hid open hid_file_handle";

  return 0;
}

void HidManager::SendThread() {
  // start send Hid 数据
  LOGD << "start HidManager";
  fd_set rset;                     // 创建文件描述符的聚合变量
  timeval timeout;                 // select timeout
  char *recv_data = new char[20];  // 接收host侧请求

  while (!stop_flag_) {
    FD_ZERO(&rset);                   // 文件描述符聚合变量清0
    FD_SET(hid_file_handle_, &rset);  // 添加文件描述符
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int retv = select(hid_file_handle_ + 1, &rset, NULL, NULL, &timeout);
    if (retv == 0) {
      LOGD << "Hid select: read request time out";
      continue;
    } else if (retv < 0) {
      LOGE << "Hid select: read request error, ret: " << retv;
      continue;
    }

    int ret = read(hid_file_handle_, recv_data, 20);
    if (ret > 0 && strncmp(recv_data, "GetSmartResult", 14) == 0) {
      LOGD << "Receive GetSmartResult";
      // 需要从pb_buffer中获取一个结果返回
      std::unique_lock<std::mutex> lck(queue_lock_);
      bool wait_ret =
          condition_.wait_for(lck, std::chrono::milliseconds(10),
                              [&]() { return pb_buffer_queue_.size() > 0; });
      // 超时，返回0 给ap
      if (wait_ret == 0) {
        LOGI << "Get smart data time out";
        int buffer_size = sizeof(int);
        char *buffer = new char[buffer_size];
        memset(buffer, 0x00, buffer_size);
        memmove(buffer, &buffer_size, buffer_size);

        // 发送前select
        FD_ZERO(&rset);                   // 文件描述符聚合变量清0
        FD_SET(hid_file_handle_, &rset);  // 添加文件描述符
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        retv = select(hid_file_handle_ + 1, &rset, NULL, NULL, &timeout);
        if (retv == 0) {
          LOGD << "Hid select: send empty data time out";
          continue;
        } else if (retv < 0) {
          LOGE << "Hid select: send empty data error, ret: " << retv;
          continue;
        }
        // 发送4字节:4，即有效长度0
        ret = write(hid_file_handle_, buffer, buffer_size);
        delete[] buffer;
        if (ret < 0 || ret != buffer_size) {
          LOGE << "Send timeout message error: " << strerror(errno);
        }
      } else {
        // send smart data
        std::string pb_string = pb_buffer_queue_.front();
        pb_buffer_queue_.pop();
        // 将pb string 发送给ap
        if (Send(pb_string)) {
          LOGE << "Hid Send error!";
        } else {
          LOGD << "Hid Send end";
        }
      }
    } else {
      LOGD << "Not receive request, hid waiting";
      sleep(3);  // 3s
    }
  }
  delete[] recv_data;
}

int HidManager::Start() {
  if (thread_ == nullptr) {
    thread_ = std::make_shared<std::thread>(&HidManager::SendThread, this);
  }
  return 0;
}

int HidManager::Stop() {
  LOGI << "HidManager Stop";
  stop_flag_ = true;
  if (thread_ != nullptr) {
    thread_->join();
  }
  close(hid_file_handle_);
  LOGI << "HidManager Stop Done";
  return 0;
}

int HidManager::FeedSmart(XProtoMessagePtr msg, int ori_image_width,
                          int ori_image_height, int dst_image_width,
                          int dst_image_height) {
  auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
  std::string protocol;
  // convert pb2string
  if (!smart_msg.get()) {
    LOGE << "msg is null";
    return -1;
  }
  switch ((SmartType)smart_type_) {
    case SmartType::SMART_FACE:
    case SmartType::SMART_BODY: {
      auto msg = dynamic_cast<CustomSmartMessage *>(smart_msg.get());
      if (msg)
        protocol = msg->Serialize(ori_image_width, ori_image_height,
                                  dst_image_width, dst_image_height);
      break;
    }
    case SmartType::SMART_VEHICLE: {
      auto msg = dynamic_cast<VehicleSmartMessage *>(smart_msg.get());
      if (msg) {
        protocol = msg->Serialize(ori_image_width, ori_image_height,
                                  dst_image_width, dst_image_height);
      }
      break;
    }
    default:
      LOGE << "not support smart_type";
      return -1;
  }

  // pb入队
  LOGD << "smart data to queue";
  std::lock_guard<std::mutex> lck(queue_lock_);
  pb_buffer_queue_.push(protocol);  // 将新的智能结果放入队列尾部
  if (pb_buffer_queue_.size() > queue_max_size_) {
    pb_buffer_queue_.pop();  // 将队列头部的丢弃
  }
  condition_.notify_one();

  return 0;
}

int HidManager::Send(const std::string &proto_str) {
  LOGD << "Start send smart data...";
  int buffer_size_src = proto_str.length();
  char *str_src = const_cast<char *>(proto_str.c_str());

  int buffer_size = buffer_size_src + sizeof(int);  // size
  char *buffer = new char[buffer_size];
  memset(buffer, 0x00, buffer_size);
  // add size
  memmove(buffer, &buffer_size, sizeof(int));
  memmove(buffer + sizeof(int), str_src, buffer_size_src);

  int ret = 0;
  if (buffer == NULL) {
    LOGE << "send error: null data!";
    return -1;
  }
  char *buffer_offset = buffer;
  int remainding_size = buffer_size;

  fd_set rset;      // 创建文件描述符的聚合变量
  timeval timeout;  // select timeout
  while (remainding_size > 0) {
    FD_ZERO(&rset);                   // 文件描述符聚合变量清0
    FD_SET(hid_file_handle_, &rset);  // 添加文件描述符
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int retv = select(hid_file_handle_ + 1, &rset, NULL, NULL, &timeout);
    if (retv == 0) {
      LOGD << "Hid select: send data time out";
      continue;
    } else if (retv < 0) {
      LOGE << "Hid select: send data error, ret: " << retv;
      continue;
    }
    if (remainding_size >= 1024) {
      LOGD << "Send 1024 bytes data...";
      ret = write(hid_file_handle_, buffer_offset, 1024);
      LOGD << "Send 1024 bytes data end";
    } else {
      LOGD << "Send " << remainding_size << " bytes data...";
      ret = write(hid_file_handle_, buffer_offset, remainding_size);
      LOGD << "Send " << remainding_size << " bytes data end";
    }
    if (ret < 0) {
      LOGF << "send package error: " << strerror(errno) << "; ret: " << ret;
      delete[] buffer;
      return -1;
    }
    remainding_size = remainding_size - ret;
    buffer_offset = buffer_offset + ret;
    if (remainding_size < 0) {
      LOGF << "send package error: " << strerror(errno) << "; ret: " << ret;
      delete[] buffer;
      return -1;
    }
  }
  delete[] buffer;
  return 0;
}

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
