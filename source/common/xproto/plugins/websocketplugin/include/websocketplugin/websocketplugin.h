/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     websocketplugin.h
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020/5/12
 * \Brief    implement of api header
 */
#ifndef INCLUDE_WEBSOCKETPLUGIN_WEBSOCKETPLUGIN_H_
#define INCLUDE_WEBSOCKETPLUGIN_WEBSOCKETPLUGIN_H_
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "websocketplugin/server/uws_server.h"
#include "websocketplugin/websocketconfig.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto/threads/threadpool.h"
#include "xproto_msgtype/protobuf/x3.pb.h"
#include "xproto_msgtype/smartplugin_data.h"
#ifdef X3_MEDIA_CODEC
#include "media_codec/media_codec_manager.h"
#endif

namespace horizon {
namespace vision {
namespace xproto {
namespace websocketplugin {
using horizon::vision::xproto::server::UwsServer;
using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::basic_msgtype::SmartMessagePtr;
struct compare_frame {
  bool operator()(const x3::FrameMessage &f1, const x3::FrameMessage &f2) {
    return (f1.timestamp_() > f2.timestamp_());
  }
};
struct compare_msg {
  bool operator()(const SmartMessagePtr m1, const SmartMessagePtr m2) {
    return (m1->time_stamp > m2->time_stamp);
  }
};
class WebsocketPlugin : public xproto::XPluginAsync {
 public:
  WebsocketPlugin() = delete;
  explicit WebsocketPlugin(std::string config_path);
  ~WebsocketPlugin() override;
  int Init() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "WebsocketPlugin"; }

 private:
  int FeedVideo(XProtoMessagePtr msg);
  int FeedSmart(XProtoMessagePtr msg);
  int SendSmartMessage(SmartMessagePtr msg, x3::FrameMessage &fm);

  void EncodeJpg(XProtoMessagePtr msg);
  void ParseConfig();
  int Reset();
  void map_smart_proc();

 private:
  std::shared_ptr<UwsServer> uws_server_;
  std::string config_file_;
  std::shared_ptr<WebsocketConfig> config_;
  std::shared_ptr<std::thread> worker_;
  std::mutex map_smart_mutex_;
  bool map_stop_ = false;
  std::condition_variable map_smart_condition_;
  const uint8_t cache_size_ = 25;  // max input cache size
  std::priority_queue<x3::FrameMessage, std::vector<x3::FrameMessage>,
                      compare_frame>
      x3_frames_;
  std::priority_queue<SmartMessagePtr, std::vector<SmartMessagePtr>,
                      compare_msg>
      x3_smart_msg_;
  int origin_image_width_ = 1920;  // update by FeedVideo
  int origin_image_height_ = 1080;
  int dst_image_width_ = 1920;  // update by FeedVideo
  int dst_image_height_ = 1080;
  std::mutex smart_mutex_;
  bool smart_stop_flag_;
  std::mutex video_mutex_;
  bool video_stop_flag_;
#ifdef X3_MEDIA_CODEC
  int chn_;
#endif
  hobot::CThreadPool jpg_encode_thread_;
  hobot::CThreadPool data_send_thread_;
};

}  // namespace websocketplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_WEBSOCKETPLUGIN_WEBSOCKETPLUGIN_H_
