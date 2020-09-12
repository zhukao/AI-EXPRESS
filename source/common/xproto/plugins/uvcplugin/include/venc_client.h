/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     venc_client.h
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020/5/12
 * \Brief    implement of api header
 */
#ifndef INCLUDE_UVCPLUGIN_VENC_CLIENT_H_
#define INCLUDE_UVCPLUGIN_VENC_CLIENT_H_
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include <thread>

#include "hb_comm_venc.h"
#include "hb_vdec.h"
#include "hb_venc.h"
#include "hb_vio_interface.h"
#include "hb_vps_api.h"
#include "usb_common.h"
#include "uvc_server.h"
#include "video_queue.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {

class VencClient {
 public:
  VencClient();
  ~VencClient();
  int Init(int width, int height, UvcConfig::VideoType type);
  int Start();
  int Stop();

 private:
  void VencToUvcThread(void *vencpram);
  int VencChnAttrInit(VENC_CHN_ATTR_S *pVencChnAttr, PAYLOAD_TYPE_E p_enType,
                      int p_Width, int p_Height, PIXEL_FORMAT_E pixFmt);
  int VencCommonInit();
  int VencCommonDeinit();
  int VencInit(int VeChn, int type, int width, int height, int bits);
  int VencDeinit(int VeChn);
  int VencStart(int Vechn);
  int VencStop(int Vechn);
  int VencReinit(int VeChn, int type, int width, int height, int bits);

 private:
  int g_exit;
  std::shared_ptr<std::thread> venc_pid_;
  vencParam venc_param_;
  VIDEO_STREAM_S h264_sps_frame_;
};
}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif
