/*
 * @Description: implement of  can data header
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-09-10 18:35:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef XPROTO_MSGTYPE_CAN_BUS_DATA_H_
#define XPROTO_MSGTYPE_CAN_BUS_DATA_H_
#include <string>
#include <memory>
#include "xproto/message/pluginflow/flowmsg.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace basic_msgtype {

// 从MCU读取到的消息
#define TYPE_CAN_BUS_FROM_MCU_MESSAGE "XPLUGIN_CAN_BUS_FROM_MCU_MESSAGE"

// 向MCU写入的消息
#define TYPE_CAN_BUS_TO_MCU_MESSAGE "XPLUGIN_CAN_BUS_TO_MCU_MESSAGE"

struct CanBusFromMcuMessage : public XProtoMessage {
 public:
  CanBusFromMcuMessage() {
    type_ = TYPE_CAN_BUS_FROM_MCU_MESSAGE;
  }
  virtual ~CanBusFromMcuMessage() = default;
  std::string Serialize() override { return ""; };

  // 接收或者发送消息的系统时间，与CAN消息实体中包含的时间戳不一样，实际使用哪个，看具体需求
  uint64_t time_stamp_ = 0;
  uint8_t can_data_[1024];  // can消息直接透传输，具体解析参考can的示例代码
  int can_data_len_ = 0;  // can消息的实际长度
};

struct CanBusToMcuMessage : public CanBusFromMcuMessage {
 public:
  CanBusToMcuMessage() {
    type_ = TYPE_CAN_BUS_TO_MCU_MESSAGE;
  }
  virtual ~CanBusToMcuMessage() = default;
};

using CanBusFromMcuMessagePtr = std::shared_ptr<CanBusFromMcuMessage>;
using CanBusToMcuMessagePtr = std::shared_ptr<CanBusToMcuMessage>;

}  // namespace basic_msgtype
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // XPROTO_MSGTYPE_CAN_BUS_DATA_H_
