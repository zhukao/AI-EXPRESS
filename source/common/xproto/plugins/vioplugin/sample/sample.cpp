/*
 * @Description: sample
 * @Author: songhan.gong@horizon.ai
 * @Date: 2019-09-24 15:33:49
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-14 21:06:18
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "vioplugin/vioplugin.h"

#include "hobotlog/hobotlog.hpp"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::vioplugin::VioPlugin;
using horizon::vision::xproto::basic_msgtype::VioMessage;

using std::chrono::milliseconds;

#define TYPE_IMAGE_MESSAGE "XPLUGIN_IMAGE_MESSAGE"
#define TYPE_DROP_MESSAGE "XPLUGIN_DROP_MESSAGE"

// 视频帧消费插件
class VioConsumerPlugin : public XPluginAsync {
 public:
  VioConsumerPlugin() = default;
  ~VioConsumerPlugin() = default;

  // 初始化,订阅消息
  int Init(){
    // 订阅视频帧消息
    RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&VioConsumerPlugin::OnGetImage, this, std::placeholders::_1));
    return XPluginAsync::Init();
  }

  int OnGetImage(XProtoMessagePtr msg) {
    // feed video frame to xstreamsdk.
    // 1. parse valid frame from msg
    auto valid_frame = std::static_pointer_cast<VioMessage>(msg);
    valid_frame.get();

    return 0;
  }

  std::string desc() const { return "VioConsumerPlugin"; }

  // 启动plugin
  int Start() {
    std::cout << "plugin start" << std::endl;
    return 0;
  }
  
  // 停止plugin
  int Stop() {
    std::cout << "plugin stop" << std::endl;
    return 0;
  }
};

struct SmartContext {
  volatile bool exit;
  SmartContext() : exit(false) {}
};

SmartContext g_ctx;

static void signal_handle(int param) {
  std::cout << "recv signal " << param << ", stop" << std::endl;
  if (param == SIGINT) {
    g_ctx.exit = true;
  }
}

int main() {
  SetLogLevel(HOBOT_LOG_DEBUG);

  signal(SIGINT, signal_handle);
  signal(SIGPIPE, signal_handle);
  signal(SIGSEGV, signal_handle);

  auto vio_plg = std::make_shared<VioPlugin>("./configs/vio_config.json");

  std::cout << "step 1" << std::endl;
  vio_plg->Init();

  std::cout << "step 2" << std::endl;
  vio_plg->Start();

  while (!g_ctx.exit) {
    std::this_thread::sleep_for(milliseconds(40));
  }

  std::cout << "step 3" << std::endl;
  vio_plg->Stop();

  return 0;
}
