/*
 * @Description: implement of visualplugin
 * @Author: GYW
 * @Date: 2020-03-30
 * @Copyright 2020 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_VISUALPLUGIN_VISUALPLUGIN_H_
#define INCLUDE_VISUALPLUGIN_VISUALPLUGIN_H_

#include <memory>
#include <mutex>
#include <vector>
#include "visualplugin/visualconfig.h"
#include "visualplugin/convert.h"
#include "visualplugin/horizonserver_api.h"

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/protobuf/ioAV.pb.h"
#include <turbojpeg.h>


namespace horizon {
namespace vision {
namespace xproto {
namespace visualplugin {

using horizon::vision::xproto::XProtoMessagePtr;

class VisualPlugin : public xproto::XPluginAsync {
 public:
  VisualPlugin() = delete;
  explicit VisualPlugin(const std::string &config_file);
  ~VisualPlugin() override;
  int Init() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "visualplugin"; }

 private:
  enum InputType {
    VT_VIDEO, // VioMessage
    VT_SMART  // SmartMessage
  };

  struct VisualInput {
    InputType type;
    XProtoMessagePtr frame_msg;
  };

  int Reset();
  void PushFrame(InputType type, XProtoMessagePtr frame_msg);
  
  int EncodeJPG(tjhandle handle_tj,
    const unsigned char *yuv_buf, int width, int height,
    unsigned char **jpeg_buf, unsigned long *jpeg_size, 
    int quality = 50);

  // encode thread
  int EncodeThread();

  int FeedVideo(XProtoMessagePtr msg);
  int FeedSmart(XProtoMessagePtr msg);

 private:
  std::string config_file_;
  std::shared_ptr<VisualConfig> config_;

  const uint8_t cache_size_ = 25; // max input cache size
  bool stop_flag_;
  bool fifo_open_flag_;
  std::mutex map_mutex_;
  std::vector<VisualInput*> input_frames_;
  std::shared_ptr<std::thread> worker_;
  SERVER_PARAM_S server_param_;
  std::vector<uchar> h264_sps_frame_;
};

}  // namespace visualplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif
