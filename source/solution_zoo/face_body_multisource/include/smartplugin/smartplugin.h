/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-04 02:41:22
 * @Version: v0.0.1
 * @Brief: smartplugin declaration
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-30 00:45:01
 */

#ifndef INCLUDE_SMARTPLUGIN_SMARTPLUGIN_H_
#define INCLUDE_SMARTPLUGIN_SMARTPLUGIN_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobotxsdk/xstream_sdk.h"
#include "smartplugin/runtime_monitor.h"
#include "smartplugin/smart_config.h"

#include "xproto_msgtype/smartplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;
using horizon::vision::xproto::basic_msgtype::SmartMessage;

struct CustomSmartMessage : SmartMessage {
  explicit CustomSmartMessage(xstream::OutputDataPtr out)
      : smart_result(out) {
    type_ = TYPE_SMART_MESSAGE;
  }
  std::string Serialize() override;
  void Serialize_Print(Json::Value &root);

 private:
  xstream::OutputDataPtr smart_result;
};

class SmartPlugin : public XPluginAsync {
 public:
  SmartPlugin() = default;
  explicit SmartPlugin(const std::string &config_file);

  void SetConfig(const std::string &config_file) { config_file_ = config_file; }

  ~SmartPlugin() = default;
  int Init() override;
  int Start() override;
  int Stop() override;

 private:
  int Feed(XProtoMessagePtr msg);
  void OnCallback(xstream::OutputDataPtr out);
  void ParseConfig();
  int OverWriteSourceNum(const std::string &cfg_file, int source_num = 1);

  std::vector<std::shared_ptr<XStreamSDK>> sdk_;
  std::shared_ptr<RuntimeMonitor> monitor_;
  std::vector<Json::Value> root;
  std::string config_file_;
  // source id => [target workflow, ...]
  std::map<int, std::vector<int>> source_target_;
  // xstream instance => input source list
  std::vector<std::vector<int>> source_map_;

  std::shared_ptr<JsonConfigWrapper> config_;
  bool result_to_json{false};
};

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_SMARTPLUGIN_SMARTPLUGIN_H_
