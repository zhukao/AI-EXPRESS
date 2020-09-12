/*
 * @Description: implement of hbipcplugin.h
 * @Author: yingmin.li@horizon.ai
 * @Date: 2019-08-24 11:29:24
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 14:42:16
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifndef HBIPCPLUGIN_INCLUDE_HBIPCPLUGIN_HBIPCPLUGIN_H_
#define HBIPCPLUGIN_INCLUDE_HBIPCPLUGIN_HBIPCPLUGIN_H_

#include <chrono>
#include <memory>
#include <string>

/* dependency header */
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "xproto_msgtype/protobuf/pack.pb.h"
#include "xproto_msgtype/protobuf/x2.pb.h"

#include "hbipc_cp/hbipc_cp.h"
#include "hbipc_cp/hbipc_errno.h"

#include "hobot_vision/blocking_queue.hpp"

#include "xproto_msgtype/hbipcplugin_data.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "xproto_msgtype/reorderplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace hbipcplugin {

#define TYPE_HBIPC_MESSAGE "XPLUGIN_HBIPC_MESSAGE"

using horizon::vision::xproto::basic_msgtype::HbipcMessage;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::basic_msgtype::ReorderMessage;

struct CustomHbipcMessage : HbipcMessage {
  explicit CustomHbipcMessage(std::string proto) : proto_(proto) {
    type_ = TYPE_HBIPC_MESSAGE;
  }
  std::string Serialize() override;

 private:
  std::string proto_;
};

class HbipcPlugin : public XPluginAsync {
 public:
  HbipcPlugin() = default;
  explicit HbipcPlugin(std::string path);
  ~HbipcPlugin();

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
  std::string desc() const { return "HbipcPlugin"; }

 private:
  int OnGetSmartResult(const XProtoMessagePtr msg);
  int OnGetDropResult(const XProtoMessagePtr msg);
  std::string SmartPack(std::shared_ptr<SmartMessage> msg);
  std::string DropPack(std::shared_ptr<VioMessage> msg);
  int OnGetReorderMsg(const XProtoMessagePtr msg);

 private:
  void ExecLoop();

 private:
  std::shared_ptr<std::thread> thread_;
  std::atomic<bool> is_stop_;
  hobot::vision::BlockingQueue<std::shared_ptr<VioMessage>> droped_queue_;
  uint64_t last_smart_frame_id_ = 0;
  int max_err_num_;
  int send_err_cnt_;
  int recv_err_cnt_;
  int need_reorder_;
};

}  // namespace hbipcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // HBIPCPLUGIN_INCLUDE_HBIPCPLUGIN_HBIPCPLUGIN_H_
