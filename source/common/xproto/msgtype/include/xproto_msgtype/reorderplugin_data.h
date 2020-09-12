/*
 * @Description: implement of reorderplugin_data.h
 * @Author: shiyu.fu@horizon.ai
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifndef XPROTO_MSGTYPE_REORDERPLUGIN_DATA_H_
#define XPROTO_MSGTYPE_REORDERPLUGIN_DATA_H_

#include <string>
#include <memory>
#include <vector>

#include "xproto/message/pluginflow/flowmsg.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace basic_msgtype {

#define TYPE_REORDER_MESSAGE "XPLUGIN_REORDER_MESSAGE"

struct ReorderMessage : XProtoMessage {
  ReorderMessage() { type_ = TYPE_REORDER_MESSAGE; }
  std::string Serialize() override { return "Default reorder message"; }
  virtual ~ReorderMessage() = default;

  XProtoMessagePtr actual_msg_;
  std::string actual_type_;
  uint64_t frame_id_;
};

using ReorderMsgPtr = std::shared_ptr<ReorderMessage>;

}  // namespace basic_msgtype
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // XPROTO_MSGTYPE_REORDERPLUGIN_DATA_H_
