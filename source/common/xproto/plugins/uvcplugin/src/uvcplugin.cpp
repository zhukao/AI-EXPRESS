/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     uvcplugin.cpp
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020.5.12
 * \Brief    implement of api file
 */

#include "uvcplugin/uvcplugin.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "./ring_queue.h"
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
using horizon::vision::xproto::XPluginErrorCode;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::basic_msgtype::VioMessage;
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_UVC_MESSAGE)
using std::chrono::milliseconds;
bool write_flag = true;

UvcPlugin::UvcPlugin(std::string config_file) {
  config_file_ = config_file;
  LOGI << "UwsPlugin smart config file:" << config_file_;
  stop_flag_ = false;
  Reset();
}

UvcPlugin::~UvcPlugin() {
  //  config_ = nullptr;
}

int UvcPlugin::Init() {
  LOGI << "UvcPlugin Init";
  RingQueue<VIDEO_STREAM_S>::Instance().Init(8, [](VIDEO_STREAM_S &elem) {
    if (elem.pstPack.vir_ptr) {
      free(elem.pstPack.vir_ptr);
      elem.pstPack.vir_ptr = nullptr;
    }
  });
  memset(&h264_sps_frame_, 0, sizeof(VIDEO_STREAM_S));

  uvc_server_ = std::make_shared<UvcServer>();
  if (uvc_server_->Init(config_file_)) {
    LOGE << "UwsPlugin Init uWS server failed";
    return -1;
  }

  hid_manager_ = std::make_shared<HidManager>(config_file_);
  if (hid_manager_->Init()) {
    LOGE << "UvcPlugin Init HidManager failed";
    return -1;
  }

  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&UvcPlugin::FeedVideo, this, std::placeholders::_1));
  RegisterMsg(TYPE_DROP_IMAGE_MESSAGE, std::bind(&UvcPlugin::FeedVideoDrop,
                                                 this, std::placeholders::_1));
  RegisterMsg(TYPE_SMART_MESSAGE,
              std::bind(&UvcPlugin::FeedSmart, this, std::placeholders::_1));
  // 调用父类初始化成员函数注册信息
  XPluginAsync::Init();
  return 0;
}

int UvcPlugin::Reset() {
  LOGI << __FUNCTION__ << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  return 0;
}

int UvcPlugin::Start() {
  uvc_server_->Start();
  hid_manager_->Start();
  return 0;
}

int UvcPlugin::Stop() {
  LOGI << "UvcPlugin::Stop()";
  stop_flag_ = true;
  uvc_server_->DeInit();
  hid_manager_->Stop();
  return 0;
}

int UvcPlugin::FeedSmart(XProtoMessagePtr msg) {
  return hid_manager_->FeedSmart(msg, origin_image_width_, origin_image_height_,
                                 dst_image_width_, dst_image_height_);
}

int UvcPlugin::FeedVideo(XProtoMessagePtr msg) {
  if (UvcServer::IsUvcStreamOn() == 0) {
    return 0;
  }
  UvcServer::SetEncoderRunning(true);
  LOGD << "UvcPlugin Feedvideo";
  VIDEO_FRAME_S pstFrame;
  memset(&pstFrame, 0, sizeof(VIDEO_FRAME_S));
  auto vio_msg = std::dynamic_pointer_cast<VioMessage>(msg);
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStartTime(desc());
  }
  int level = UvcServer::config_->layer_;
#ifdef X2
  auto image = vio_msg->image_[0]->image;
  img_info_t *src_img = reinterpret_cast<img_info_t *>(image.data);
  auto height = src_img->down_scale[level].height;
  auto width = src_img->down_scale[level].width;
  auto y_addr = src_img->down_scale[level].y_vaddr;
  auto uv_addr = src_img->down_scale[level].c_vaddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * src_img->down_scale[level].step;
  auto img_uv_size = img_y_size / 2;

  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.width = src_img->down_scale[0].width;
  pstFrame.stVFrame.height = src_img->down_scale[0].height;
  pstFrame.stVFrame.size =
      src_img->down_scale[0].width * src_img->down_scale[0].height * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = src_img->down_scale[0].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = src_img->down_scale[0].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = src_img->down_scale[0].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = src_img->down_scale[0].c_vaddr;

  origin_image_width_ = src_img->down_scale[0].width;
  origin_image_height_ = src_img->down_scale[0].height;
  dst_image_width_ = src_img->down_scale[level].width;
  dst_image_height_ = src_img->down_scale[level].height;
#endif

#ifdef X3
  auto pym_image = vio_msg->image_[0];
  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.height = pym_image->down_scale[level].height;
  pstFrame.stVFrame.width = pym_image->down_scale[level].width;
  pstFrame.stVFrame.size = pym_image->down_scale[level].height *
                           pym_image->down_scale[level].width * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = (uint32_t)pym_image->down_scale[level].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = (uint32_t)pym_image->down_scale[level].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = (char *)pym_image->down_scale[level].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = (char *)pym_image->down_scale[level].c_vaddr;
  pstFrame.stVFrame.pts = vio_msg->time_stamp_;

  origin_image_width_ = pym_image->down_scale[0].width;
  origin_image_height_ = pym_image->down_scale[0].height;
  dst_image_width_ = pym_image->down_scale[level].width;
  dst_image_height_ = pym_image->down_scale[level].height;
#endif

  if (false == RingQueue<VIDEO_STREAM_S>::Instance().IsValid()) {
    UvcServer::SetEncoderRunning(false);
    return 0;
  }
  
  if (UvcServer::IsNv12On()) {
    VIDEO_STREAM_S vstream;
    memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
    auto buffer_size = pstFrame.stVFrame.size;
    vstream.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
    vstream.pstPack.size = buffer_size;
    memcpy(vstream.pstPack.vir_ptr, pstFrame.stVFrame.vir_ptr[0],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width);
    memcpy(vstream.pstPack.vir_ptr + pstFrame.stVFrame.height * pstFrame.stVFrame.width,
                pstFrame.stVFrame.vir_ptr[1],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width/2);
    RingQueue<VIDEO_STREAM_S>::Instance().Push(vstream);
    return 0;
  }

  int ret = HB_VENC_SendFrame(0, &pstFrame, 0);
  if (ret < 0) {
    LOGE << "HB_VENC_SendFrame 0 error!!!ret " << ret;
    UvcServer::SetEncoderRunning(false);
    return 0;
  }

  VIDEO_STREAM_S vstream;
  memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
  ret = HB_VENC_GetStream(0, &vstream, 2000);
  if (ret < 0) {
    LOGE << "HB_VENC_GetStream timeout: " << ret;
  } else {
    auto video_buffer = vstream;
    auto buffer_size = video_buffer.pstPack.size;
    HOBOT_CHECK(buffer_size > 5) << "encode bitstream too small";
    int nal_type = -1;
    if ((0 == static_cast<int>(vstream.pstPack.vir_ptr[0])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[1])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[2])) &&
        (1 == static_cast<int>(vstream.pstPack.vir_ptr[3]))) {
      nal_type = static_cast<int>(vstream.pstPack.vir_ptr[4] & 0x1F);
    }
    LOGD << "nal type is " << nal_type;
    if (nal_type == H264_NALU_SPS) {
      if (h264_sps_frame_.pstPack.vir_ptr) {
        free(h264_sps_frame_.pstPack.vir_ptr);
        h264_sps_frame_.pstPack.vir_ptr = nullptr;
      }
      h264_sps_frame_ = vstream;
      h264_sps_frame_.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      memcpy(h264_sps_frame_.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                buffer_size);
      video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      if (video_buffer.pstPack.vir_ptr) {
        memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
              buffer_size);
        RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
      }
      HB_VENC_ReleaseStream(0, &vstream);
      // need to Get Stream one more, to Get IDR frame
      // 注意： xj3 26号已经合入修改，SPS与I帧一起给出。后续这块务必记得修改
      ret = HB_VENC_GetStream(0, &vstream, 2000);
      if (ret < 0) {
        LOGE << "HB_VENC_GetStream first IDR Frame timeout: " << ret;
      } else {
        video_buffer = vstream;
        buffer_size = video_buffer.pstPack.size;
        video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
        if (video_buffer.pstPack.vir_ptr) {
          memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                  buffer_size);
          RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
        }
        HB_VENC_ReleaseStream(0, &vstream);
      }
    } else if (nal_type == H264_NALU_IDR) {
      if (h264_sps_frame_.pstPack.vir_ptr) {
        VIDEO_STREAM_S video_buffer;
        video_buffer = h264_sps_frame_;
        auto sps_size = h264_sps_frame_.pstPack.size;
        video_buffer.pstPack.vir_ptr = (char *)calloc(1, sps_size);
        if (video_buffer.pstPack.vir_ptr) {
          memcpy(video_buffer.pstPack.vir_ptr,
                h264_sps_frame_.pstPack.vir_ptr, sps_size);
          RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
        }
      }
      video_buffer = vstream;
      video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      if (video_buffer.pstPack.vir_ptr) {
        memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                  buffer_size);
        RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
      }
      HB_VENC_ReleaseStream(0, &vstream);
    } else {
      video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      if (video_buffer.pstPack.vir_ptr) {
        memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
              buffer_size);
        RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
      }
      HB_VENC_ReleaseStream(0, &vstream);
    }
    if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
      vio_msg->profile_->UpdatePluginStopTime(desc());
    }
  }
  UvcServer::SetEncoderRunning(false);
  return 0;
}

int UvcPlugin::FeedVideoDrop(XProtoMessagePtr msg) {
  if (UvcServer::IsUvcStreamOn() == 0) {
    return 0;
  }
  UvcServer::SetEncoderRunning(true);
  VIDEO_FRAME_S pstFrame;
  memset(&pstFrame, 0, sizeof(VIDEO_FRAME_S));
  auto vio_msg = std::dynamic_pointer_cast<VioMessage>(msg);
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStartTime(desc());
  }
  int level = UvcServer::config_->layer_;
#ifdef X2
  auto image = vio_msg->image_[0]->image;
  img_info_t *src_img = reinterpret_cast<img_info_t *>(image.data);
  auto height = src_img->down_scale[level].height;
  auto width = src_img->down_scale[level].width;
  auto y_addr = src_img->down_scale[level].y_vaddr;
  auto uv_addr = src_img->down_scale[level].c_vaddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * src_img->down_scale[level].step;
  auto img_uv_size = img_y_size / 2;

  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.width = src_img->down_scale[0].width;
  pstFrame.stVFrame.height = src_img->down_scale[0].height;
  pstFrame.stVFrame.size =
      src_img->down_scale[0].width * src_img->down_scale[0].height * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = src_img->down_scale[0].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = src_img->down_scale[0].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = src_img->down_scale[0].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = src_img->down_scale[0].c_vaddr;

  origin_image_width_ = src_img->down_scale[0].width;
  origin_image_height_ = src_img->down_scale[0].height;
  dst_image_width_ = src_img->down_scale[level].width;
  dst_image_height_ = src_img->down_scale[level].height;

#endif
  
#ifdef X3
  auto pym_image = vio_msg->image_[0];
  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.height = pym_image->down_scale[level].height;
  pstFrame.stVFrame.width = pym_image->down_scale[level].width;
  pstFrame.stVFrame.size = pym_image->down_scale[level].height *
                           pym_image->down_scale[level].width * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = (uint32_t)pym_image->down_scale[level].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = (uint32_t)pym_image->down_scale[level].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = (char *)pym_image->down_scale[level].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = (char *)pym_image->down_scale[level].c_vaddr;
  pstFrame.stVFrame.pts = vio_msg->time_stamp_;

  origin_image_width_ = pym_image->down_scale[0].width;
  origin_image_height_ = pym_image->down_scale[0].height;
  dst_image_width_ = pym_image->down_scale[level].width;
  dst_image_height_ = pym_image->down_scale[level].height;
#endif

  if (false == RingQueue<VIDEO_STREAM_S>::Instance().IsValid()) {
    UvcServer::SetEncoderRunning(false);
    return 0;
  }

  if (UvcServer::IsNv12On()) {
    VIDEO_STREAM_S vstream;
    memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
    auto buffer_size = pstFrame.stVFrame.size;
    vstream.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
    vstream.pstPack.size = buffer_size;
    memcpy(vstream.pstPack.vir_ptr, pstFrame.stVFrame.vir_ptr[0],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width);
    memcpy(vstream.pstPack.vir_ptr + pstFrame.stVFrame.height * pstFrame.stVFrame.width,
                pstFrame.stVFrame.vir_ptr[1],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width/2);  
    RingQueue<VIDEO_STREAM_S>::Instance().Push(vstream);
    return 0;
  }


  int ret = HB_VENC_SendFrame(0, &pstFrame, 0);
  if (ret < 0) {
    LOGD << "HB_VENC_SendFrame 0 error!!!ret " << ret;
    UvcServer::SetEncoderRunning(false);
    return 0;
  }

  VIDEO_STREAM_S vstream;
  memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
  ret = HB_VENC_GetStream(0, &vstream, 2000);
  if (ret < 0) {
    LOGE << "HB_VENC_GetStream timeout: " << ret;
  } else {
    auto video_buffer = vstream;
    auto buffer_size = video_buffer.pstPack.size;
    HOBOT_CHECK(buffer_size > 5) << "encode bitstream too small";
    int nal_type = -1;
    if ((0 == static_cast<int>(vstream.pstPack.vir_ptr[0])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[1])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[2])) &&
        (1 == static_cast<int>(vstream.pstPack.vir_ptr[3]))) {
      nal_type = static_cast<int>(vstream.pstPack.vir_ptr[4] & 0x1F);
    }
    LOGD << "nal type is " << nal_type;
    if (nal_type == H264_NALU_SPS) {
      if (h264_sps_frame_.pstPack.vir_ptr) {
        free(h264_sps_frame_.pstPack.vir_ptr);
        h264_sps_frame_.pstPack.vir_ptr = nullptr;
      }
      h264_sps_frame_ = vstream;
      h264_sps_frame_.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      memcpy(h264_sps_frame_.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
              buffer_size);
      video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      if (video_buffer.pstPack.vir_ptr) {
        memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                buffer_size);
        RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
      }
      HB_VENC_ReleaseStream(0, &vstream);
      // need to Get Stream one more, to Get IDR frame
      ret = HB_VENC_GetStream(0, &vstream, 2000);
      if (ret < 0) {
        LOGE << "HB_VENC_GetStream first IDR Frame timeout: " << ret;
      } else {
        video_buffer = vstream;
        buffer_size = video_buffer.pstPack.size;
        video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
        if (video_buffer.pstPack.vir_ptr) {
          memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                  buffer_size);
          RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
        }
        HB_VENC_ReleaseStream(0, &vstream);
      }
    } else if (nal_type == H264_NALU_IDR) {
      if (h264_sps_frame_.pstPack.vir_ptr) {
        VIDEO_STREAM_S video_buffer;
        video_buffer = h264_sps_frame_;
        auto sps_size = h264_sps_frame_.pstPack.size;
        video_buffer.pstPack.vir_ptr = (char *)calloc(1, sps_size);
        if (video_buffer.pstPack.vir_ptr) {
          memcpy(video_buffer.pstPack.vir_ptr,
                  h264_sps_frame_.pstPack.vir_ptr, sps_size);
          RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
        }
      }
      video_buffer = vstream;
      video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      if (video_buffer.pstPack.vir_ptr) {
        memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                buffer_size);
        RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
      }
      HB_VENC_ReleaseStream(0, &vstream);
    } else {
      video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
      if (video_buffer.pstPack.vir_ptr) {
        memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
                buffer_size);
        RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
      }
      HB_VENC_ReleaseStream(0, &vstream);
    }
  }
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStopTime(desc());
  }
  UvcServer::SetEncoderRunning(false);
  return 0;
}
}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon