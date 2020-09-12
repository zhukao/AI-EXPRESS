/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     uvcplugin.h
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020/5/12
 * \Brief    implement of api header
 */
#ifndef INCLUDE_UVCPLUGIN_UVCPLUGIN_H_
#define INCLUDE_UVCPLUGIN_UVCPLUGIN_H_
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "./hid_manager.h"
#include "./usb_common.h"
#include "uvc/uvc.h"
#include "uvc/uvc_gadget.h"
#include "./uvc_server.h"
#include "./uvcplugin_config.h"
#include "./venc_client.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessagePtr;

class UvcPlugin : public xproto::XPluginAsync {
 public:
  UvcPlugin() = delete;
  explicit UvcPlugin(std::string config_path);
  ~UvcPlugin() override;
  int Init() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "uvcPlugin"; }

 private:
  int FeedVideo(XProtoMessagePtr msg);
  int FeedSmart(XProtoMessagePtr msg);
  int FeedVideoDrop(XProtoMessagePtr msg);
  void ParseConfig();
  int Reset();

 private:
  std::string config_file_;
  bool stop_flag_;
  std::shared_ptr<std::thread> worker_;
  std::mutex map_mutex_;
  const uint8_t cache_size_ = 25;  // max input cache size
  std::shared_ptr<UvcServer> uvc_server_;
  std::shared_ptr<VencClient> venc_client_;
  struct uvc_context *uvc_ctx;
  std::shared_ptr<UvcConfig> config_;

  std::shared_ptr<HidManager> hid_manager_;

  int origin_image_width_ = 1920;  // update by FeedVideo
  int origin_image_height_ = 1080;
  int dst_image_width_ = 1920;  // update by FeedVideo
  int dst_image_height_ = 1080;
  VIDEO_STREAM_S h264_sps_frame_;
};
}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_UVCPLUGIN_UVCPLUGIN_H_
