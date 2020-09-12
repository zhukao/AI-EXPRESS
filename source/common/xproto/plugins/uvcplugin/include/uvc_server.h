/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     venc_client.h
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020/5/12
 * \Brief    implement of api header
 */

#ifndef INCLUDE_UVCPLUGIN_UVC_SERVER_H_
#define INCLUDE_UVCPLUGIN_UVC_SERVER_H_
#include <string>
#include <thread>
#include <mutex>
#include "hb_comm_venc.h"
#include "hb_vdec.h"
#include "hb_venc.h"
#include "hb_vio_interface.h"
#include "hb_vps_api.h"
#include "uvc/uvc_gadget.h"
#include "uvc/uvc_gadget_api.h"
#include "uvcplugin_config.h"
#include "video_queue.h"
#include "usb_common.h"
#include "media_codec/media_codec_manager.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {

typedef enum {
  VENC_INVALID = 0,
  VENC_H264,
  VENC_H265,
  VENC_MJPEG,
} venc_type;

struct media_info {
  enum uvc_fourcc_format format;
  venc_type vtype;
  int vch;
  uint32_t width;
  uint32_t height;

  /* store one encoded video stream */
  VIDEO_STREAM_S vstream;
};

struct single_buffer {
  VIDEO_STREAM_S vstream;

  int used;
  pthread_mutex_t mutex;
};

class UvcServer {
  friend class VencClient;

 public:
  UvcServer();
  ~UvcServer();
  int Init(std::string config_file);
  int Start();
  int Stop();
  int DeInit();
  /* function declare */
 public:
  static void uvc_streamon_off(struct uvc_context *ctx, int is_on,
                               void *userdata);

  static int uvc_get_frame(struct uvc_context *ctx, void **buf_to, int *buf_len,
                           void **entity, void *userdata);

  static void uvc_release_frame(struct uvc_context *ctx, void **entity,
                                void *userdata);

  int uvc_init_with_params(venc_type type, int vechn, int width, int height);

  static int InitCodecManager(vencParam *param);

  static int DeinitCodecManager(int chn);

  static bool IsUvcStreamOn() {
    std::lock_guard<std::mutex> lg(mutex_);
    if (uvc_stream_on == 0) {
      return false;
    } else {
      return true;
    }
  }

  static void SetUvcStreamOn(int on) {
    std::lock_guard<std::mutex> lg(mutex_);
    uvc_stream_on = on;
  }

  static bool IsEncoderRunning() {
    std::lock_guard<std::mutex> lg(mutex_);
    if (encoder_running_) {
      return true;
    } else {
      return false;
    }
  }

  static void SetEncoderRunning(bool running) {
    std::lock_guard<std::mutex> lg(mutex_);
    encoder_running_ = running;
  }

  static void SetNv12IsOn(bool is_on) {
    std::lock_guard<std::mutex> lg(mutex_);
    nv12_is_on_ = is_on;
  }

  static bool IsNv12On() {
    std::lock_guard<std::mutex> lg(mutex_);
    if (nv12_is_on_) {
      return true;
    } else {
      return false;
    }
  }
 public:
  static int uvc_stream_on;
  static bool encoder_running_;
  static std::mutex mutex_;
  static std::shared_ptr<UvcConfig> config_;
  static bool nv12_is_on_;

 private:
  struct uvc_params params;
  static struct uvc_context *uvc_ctx;
  char *uvc_devname; /* uvc Null, lib will find one */
  char *v4l2_devname;
  static uint64_t width;
  static uint64_t height;
  venc_type vtype_;
  std::shared_ptr<std::thread> worker_;
};
}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif /* _UVC_GADGET_API_H_ */
