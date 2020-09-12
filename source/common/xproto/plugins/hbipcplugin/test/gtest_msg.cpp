/*
 * @Description: custom message test
 * @Author: hao.tian@horiozn.ai
 * @Date: 2019-08-28 19:33:08
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-14 20:48:24
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
/*
 * 添加自定义plugin分为三步：
 * 1.使用宏XPLUGIN_REGISTER_MSG_TYPE,自定义消息类型，每个plugin名字唯一；
 * 2.在生成消息的Plugin部分定义新的Message，新的Message需要继承XProtoMessage;
 * 3.继承XPluginAsync实现自定义Plugin：
 *   a.实现消息处理函数；
 *   b.覆盖Init函数，在其中完成监听消息注册及其他初始化工作，并在函数返回前调用父plugin的Init方法。
 */
#include <sys/utsname.h>

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "gtest/gtest.h"
#include "hbipcplugin/hbipcplugin.h"
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/protobuf/pack.pb.h"
#include "xproto_msgtype/protobuf/x2.pb.h"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::hbipcplugin::HbipcMessage;
using horizon::vision::xproto::hbipcplugin::HbipcPlugin;

using std::chrono::milliseconds;

struct SmartContext {
  bool exit;
  SmartContext() : exit(false) {}
};

class HbipcPluginTest : public testing::Test {
 protected:
  void SetUp() override { sc_plg = std::make_shared<HbipcPlugin>(); }
  std::shared_ptr<HbipcPlugin> sc_plg;
};

TEST_F(HbipcPluginTest, API) {
  SetLogLevel(HOBOT_LOG_DEBUG);

  struct utsname name;
  if (uname(&name)) {
    exit(-1);
  }
  LOGI << "Hello! Your remote computer's OS is " << name.sysname << " "
       << name.release;

  sc_plg->Init();

  sc_plg->Start();

  std::this_thread::sleep_for(milliseconds(60000));

  sc_plg->Stop();

  sc_plg->Deinit();

  LOGI << "Test hbipcplugin api file success";
}