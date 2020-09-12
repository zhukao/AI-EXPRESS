/*
 * @Description: implement of uvcconfig
 * @Author: ronghui.zhang
 * @Date: 2020-05-14
 * @Copyright 2020 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_UVCPLUGIN_VISUALCONFIG_H_
#define INCLUDE_UVCPLUGIN_VISUALCONFIG_H_

#include <memory>
#include <mutex>
#include <string>

#include "json/json.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
class UvcPlugin;

class UvcConfig {
  friend class UvcPlugin;
  friend class UvcServer;

 public:
  /* type of smart frame */
  enum SmartType { SMART_FACE, SMART_BODY, SMART_VEHICLE };

  /* type of video frame */
  enum VideoType { VIDEO_YUV, VIDEO_H264, VIDEO_JPG, VIDEO_MJPEG };

  /* type of display mode */
  enum DisplayType { QT_MODE, WEB_MODE, UVC_MODE };

 public:
  UvcConfig() = delete;
  explicit UvcConfig(const std::string &path);
  bool LoadConfig();
  std::string GetValue(const std::string &key);
  Json::Value GetJson() const;

 private:
  bool CheckConfig();
  std::string path_;
  Json::Value json_;
  std::mutex mutex_;

  uint8_t auth_mode_;
  DisplayType display_mode_;
  int layer_;
  uint8_t jpeg_quality_;
  std::string user_, password_;
  SmartType smart_type_;
  VideoType video_type_;
  uint32_t image_width_, image_height_;
  uint32_t data_buf_size_, packet_size_;
  std::string attr_des_file_;
  int res_720p_layer_;
  int res_1080p_layer_;
  int res_2160p_layer_;
  int is_cbr_ = 1;
  int bitrate_ = 2000;
};

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif
