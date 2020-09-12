/*
 * @Description: implement of reorderplugin.h
 * @Author: shiyu.fu@horizon.ai
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifndef REORDERPLUGIN_INCLUDE_REORDERPLUGIN_REORDERPLUGIN_H_
#define REORDERPLUGIN_INCLUDE_REORDERPLUGIN_REORDERPLUGIN_H_

#include <chrono>
#include <memory>
#include <string>
#include <queue>
#include <vector>

/* dependency header */
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobot_vision/blocking_queue.hpp"

#include "xproto_msgtype/hbipcplugin_data.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "xproto_msgtype/reorderplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace reorderplugin {

#define TYPE_REORDER_MESSAGE "XPLUGIN_REORDER_MESSAGE"

using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::basic_msgtype::ReorderMessage;
using horizon::vision::xproto::basic_msgtype::ReorderMsgPtr;

struct CustomReorderMessage : ReorderMessage {
  explicit CustomReorderMessage(XProtoMessagePtr msg, std::string type,
                                 uint64_t id) {
    type_ = TYPE_REORDER_MESSAGE;
    actual_msg_ = msg;
    actual_type_ = type;
    frame_id_ = id;
  }
  std::string Serialize() override;
};

struct compare{
  bool operator()(ReorderMsgPtr msg1, ReorderMsgPtr msg2){
    return msg1->frame_id_ > msg2->frame_id_;
  }
};

class ReorderPlugin : public XPluginAsync {
 public:
  ReorderPlugin() = default;
  explicit ReorderPlugin(std::string cfg_file);
  ~ReorderPlugin();

 public:
  /* xproto框架接口的封装函数 */
  // 初始化plugin
  int Init() override;
  // 反初始化plugin
  int Deinit();
  // 开启plugin服务
  int Start() override;
  // 关闭plugin服务
  int Stop() override;
  // 返回plugin的名称
  std::string desc() const { return "ReorderPlugin"; }

 private:
  int OnGetAppResult(const XProtoMessagePtr msg);
  XProtoMessagePtr Reorder(const ReorderMsgPtr msg);

 private:
  std::shared_ptr<std::thread> thread_;
  std::atomic<bool> is_stop_;
  std::priority_queue<ReorderMsgPtr,
                      std::vector<ReorderMsgPtr>, compare> msg_queue_;
  uint64_t target_frame_id_ = 0;
  int need_reorder_;
};

}  // namespace reorderplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // REORDERPLUGIN_INCLUDE_REORDERPLUGIN_REORDERPLUGIN_H_
