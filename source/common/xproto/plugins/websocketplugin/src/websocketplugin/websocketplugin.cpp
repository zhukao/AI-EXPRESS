/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     websocketplugin.cpp
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020.5.12
 * \Brief    implement of api file
 */
#include "websocketplugin/websocketplugin.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "websocketplugin/convert.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/protobuf/x3.pb.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "utils/time_helper.h"
#include "websocketplugin/attribute_convert/attribute_convert.h"

using hobot::Timer;
namespace horizon {
namespace vision {
namespace xproto {
namespace websocketplugin {
using horizon::vision::xproto::XPluginErrorCode;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::websocketplugin::AttributeConvert;
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_UWS_MESSAGE)
using std::chrono::milliseconds;
bool write_flag = true;

WebsocketPlugin::WebsocketPlugin(std::string config_file) {
  config_file_ = config_file;
  LOGI << "UwsPlugin smart config file:" << config_file_;
  smart_stop_flag_ = false;
  video_stop_flag_ = false;
  Reset();
}

WebsocketPlugin::~WebsocketPlugin() {
  config_ = nullptr;
}

int WebsocketPlugin::Init() {
  LOGI << "WebsocketPlugin::Init";
  // load config
  config_ = std::make_shared<WebsocketConfig>(config_file_);
  if (!config_ || !config_->LoadConfig()) {
    LOGE << "failed to load config file";
    return -1;
  }

  if (config_->display_mode_ != WebsocketConfig::WEB_MODE) {
    LOGE << "not support web display";
    return 0;
  }
  LOGI << "attribute_description_path: " << config_->attr_des_file_;
  AttributeConvert::Instance().Init(config_->attr_des_file_);

  RegisterMsg(TYPE_SMART_MESSAGE, std::bind(&WebsocketPlugin::FeedSmart, this,
                                            std::placeholders::_1));
  RegisterMsg(TYPE_IMAGE_MESSAGE, std::bind(&WebsocketPlugin::FeedVideo, this,
                                            std::placeholders::_1));
  // 调用父类初始化成员函数注册信息
  XPluginAsync::Init();
#ifdef X2
  jpg_encode_thread_.CreatThread(4);
#endif
#ifdef X3
  jpg_encode_thread_.CreatThread(1);
#endif
  data_send_thread_.CreatThread(1);
  return 0;
}

int WebsocketPlugin::Reset() {
  LOGI << __FUNCTION__ << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  std::unique_lock<std::mutex> lock(map_smart_mutex_);
  while (!x3_frames_.empty()) {
    x3_frames_.pop();
  }
  while (!x3_smart_msg_.empty()) {
    x3_smart_msg_.pop();
  }
  return 0;
}

int WebsocketPlugin::Start() {
  if (config_->display_mode_ != WebsocketConfig::WEB_MODE) {
    LOGE << "not support web display";
    return 0;
  }
  // start websocket server
  uws_server_ = std::make_shared<UwsServer>();
  if (uws_server_->Init()) {
    LOGE << "UwsPlugin Init uWS server failed";
    return -1;
  }
#ifdef X3_MEDIA_CODEC
  /* 1. media codec init */
  /* 1.1 get media codec manager and module init */
  MediaCodecManager &manager = MediaCodecManager::Get();
  auto rv = manager.ModuleInit();
  HOBOT_CHECK(rv == 0);
  /* 1.2 get media codec venc chn */
  chn_ = manager.GetEncodeChn();
  /* 1.3 media codec venc chn init */
  int pic_width = config_->image_width_;
  int pic_height = config_->image_height_;
  int frame_buf_depth = config_->frame_buf_depth_;
  int is_cbr = config_->is_cbr_;
  int bitrate = config_->bitrate_;

  rv = manager.EncodeChnInit(chn_, PT_JPEG, pic_width, pic_height,
          frame_buf_depth, HB_PIXEL_FORMAT_NV12, is_cbr, bitrate);
  HOBOT_CHECK(rv == 0);
  /* 1.4 set media codec venc jpg chn qfactor params */
  rv = manager.SetUserQfactorParams(chn_, config_->jpeg_quality_);
  HOBOT_CHECK(rv == 0);
  /* 1.5 set media codec venc jpg chn qfactor params */
  rv = manager.EncodeChnStart(chn_);
  HOBOT_CHECK(rv == 0);
  /* 1.6 alloc media codec vb buffer init */
  if (config_->use_vb_) {
    int vb_num = frame_buf_depth;
    int pic_stride = config_->image_width_;
    int pic_size = pic_stride * pic_height * 3 / 2;  // nv12 format
    int vb_cache_enable = 1;
    rv = manager.VbBufInit(chn_, pic_width, pic_height, pic_stride,
        pic_size, vb_num, vb_cache_enable);
    HOBOT_CHECK(rv == 0);
  }
#endif
  if (!worker_) {
    worker_ = std::make_shared<std::thread>(
        std::bind(&WebsocketPlugin::map_smart_proc, this));
  }
  return 0;
}

void WebsocketPlugin::map_smart_proc() {
  static uint64_t pre_frame_id = 0;
  while (!map_stop_) {
    std::unique_lock<std::mutex> lock(map_smart_mutex_);
    map_smart_condition_.wait(lock);
    if (x3_frames_.size() > 3 || x3_smart_msg_.size() > 3) {
      LOGW << "map_proc, frames_image size = " << x3_frames_.size()
           << ", smart_msg size = " << x3_smart_msg_.size();
    }
    if (map_stop_) {
      break;
    }
    while (!x3_smart_msg_.empty() && !x3_frames_.empty()) {
      auto msg = x3_smart_msg_.top();
      auto frame = x3_frames_.top();
      if (msg->time_stamp == frame.timestamp_()) {
        if (msg->frame_id > pre_frame_id ||
            (pre_frame_id - msg->frame_id > 300) ||
            pre_frame_id == 0) {  // frame_id maybe overflow reset to 0
          int task_num = data_send_thread_.GetTaskNum();
          if (task_num < 3) {
            data_send_thread_.PostTask(
              std::bind(&WebsocketPlugin::SendSmartMessage, this, msg, frame));
          }
          pre_frame_id = msg->frame_id;
        }
        x3_smart_msg_.pop();
        x3_frames_.pop();
      } else {
        // avoid smart or image result lost
        while (x3_smart_msg_.size() > 20) {
          auto msg_inner = x3_smart_msg_.top();
          auto frame_inner = x3_frames_.top();
          if (msg_inner->time_stamp < frame_inner.timestamp_()) {
            // 消息对应的图片一直没有过来，删除消息
            x3_smart_msg_.pop();
          } else {
            break;
          }
        }
        while (x3_frames_.size() > 20) {
          auto msg_inner = x3_smart_msg_.top();
          auto frame_inner = x3_frames_.top();
          if (frame_inner.timestamp_() < msg_inner->time_stamp) {
            // 图像对应的消息一直没有过来，删除图像
            x3_frames_.pop();
          } else {
            break;
          }
        }

        break;
      }
    }
    if (x3_smart_msg_.size() > 20) {
      LOGF << "web socket has cache smart message nun > 20";
    }
    if (x3_frames_.size() > 20) {
      LOGF << "web socket has cache image nun > 20";
    }
  }
}

int WebsocketPlugin::Stop() {
  {
    std::lock_guard<std::mutex> smart_lock(smart_mutex_);
    smart_stop_flag_ = true;
  }
  {
    std::lock_guard<std::mutex> video_lock(video_mutex_);
    video_stop_flag_ = true;
  }
  {
    if (worker_ && worker_->joinable()) {
      map_stop_ = true;
      map_smart_condition_.notify_one();
      worker_->join();
      worker_ = nullptr;
      LOGI << "WebsocketPlugin stop worker";
    }
  }
  if (config_->display_mode_ != WebsocketConfig::WEB_MODE) {
    LOGE << "not support web display";
    return 0;
  }
  LOGI << "WebsocketPlugin::Stop()";
#ifdef X3_MEDIA_CODEC
  /* 3. media codec deinit */
  /* 3.1 media codec chn stop */
  MediaCodecManager &manager = MediaCodecManager::Get();
  manager.EncodeChnStop(chn_);
  /* 3.2 media codec chn deinit */
  manager.EncodeChnDeInit(chn_);
  /* 3.3 media codec vb buf deinit */
  if (config_->use_vb_) {
    manager.VbBufDeInit(chn_);
  }
  /* 3.4 media codec module deinit */
  manager.ModuleDeInit();
#endif
  uws_server_->DeInit();
  return 0;
}

int WebsocketPlugin::FeedSmart(XProtoMessagePtr msg) {
  {
    std::lock_guard<std::mutex> smart_lock(smart_mutex_);
    if (smart_stop_flag_) {
      LOGD << "Aleardy stop, WebsocketPLugin FeedSmart return";
      return -1;
    }
  }
  std::lock_guard<std::mutex> smart_lock(map_smart_mutex_);
  auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
  if (smart_msg) {
    x3_smart_msg_.push(smart_msg);
    map_smart_condition_.notify_one();
  }

  return 0;
}

int WebsocketPlugin::SendSmartMessage(SmartMessagePtr smart_msg,
                                      x3::FrameMessage &fm) {
  std::string protocol;
  if (fm.mutable_img_()->width_() == 0) {
    // drop
    return XPluginErrorCode::ERROR_CODE_OK;
  }
  Convertor::PackSmartMsg(protocol, smart_msg.get(), config_->smart_type_,
                          origin_image_width_, origin_image_height_,
                          dst_image_width_, dst_image_height_);
  // sync video & smart frame
  x3::FrameMessage msg_send;
  msg_send.ParseFromString(protocol);
  msg_send.mutable_img_()->CopyFrom(fm.img_());
  // add system info
  std::string cpu_rate_file =
      "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq";
  std::string temp_file = "/sys/class/thermal/thermal_zone0/temp";
  std::ifstream ifs(cpu_rate_file.c_str());
  if (!ifs.is_open()) {
    LOGF << "open config file " << cpu_rate_file << " failed";
    return -1;
  }
  std::stringstream ss;
  std::string str;
  ss << ifs.rdbuf();
  ss >> str;
  ifs.close();
  auto Statistics_msg_ = msg_send.mutable_statistics_msg_();
  auto attrs = Statistics_msg_->add_attributes_();
  attrs->set_type_("cpu");
  attrs->set_value_string_(str.c_str());

  ifs.clear();
  ss.clear();
  ifs.open(temp_file.c_str());
  ss << ifs.rdbuf();
  ss >> str;

  auto temp_attrs = Statistics_msg_->add_attributes_();
  temp_attrs->set_type_("temp");
  temp_attrs->set_value_string_(str.c_str());
  std::string proto_send;
  msg_send.SerializeToString(&proto_send);
  uws_server_->Send(proto_send);
  return XPluginErrorCode::ERROR_CODE_OK;
}

int WebsocketPlugin::FeedVideo(XProtoMessagePtr msg) {
  jpg_encode_thread_.PostTask(
      std::bind(&WebsocketPlugin::EncodeJpg, this, msg));
  return 0;
}

void WebsocketPlugin::EncodeJpg(XProtoMessagePtr msg) {
  int rv;
  bool bret;
  cv::Mat yuv_img;
  std::vector<uchar> img_buf;
  x3::FrameMessage x3_frame_msg;
  std::string smart_result;
#ifdef X3_MEDIA_CODEC
  iot_venc_src_buf_t *frame_buf = nullptr;
  iot_venc_src_buf_t src_buf = { 0 };
  iot_venc_stream_buf_t *stream_buf = nullptr;
#endif
  {
    std::lock_guard<std::mutex> video_lock(video_mutex_);
    if (video_stop_flag_) {
      LOGD << "Aleardy stop, WebsocketPLugin Feedvideo return";
      return;
    }
  }

  LOGI << "WebsocketPLugin Feedvideo";
  auto frame = std::dynamic_pointer_cast<VioMessage>(msg);
  VioMessage *vio_msg = frame.get();
  auto timestamp = frame->time_stamp_;
  // get pyramid size
#ifdef X2
  auto image = vio_msg->image_[0]->image;
  img_info_t *src_img = reinterpret_cast<img_info_t *>(image.data);
  origin_image_width_ = src_img->down_scale[0].width;
  origin_image_height_ = src_img->down_scale[0].height;
  dst_image_width_ = src_img->down_scale[config_->layer_].width;
  dst_image_height_ = src_img->down_scale[config_->layer_].height;
#endif
#ifdef X3
  auto pym_image = vio_msg->image_[0];
  origin_image_width_ = pym_image->down_scale[0].width;
  origin_image_height_ = pym_image->down_scale[0].height;
  dst_image_width_ = pym_image->down_scale[config_->layer_].width;
  dst_image_height_ = pym_image->down_scale[config_->layer_].height;
#endif
#ifndef X3_MEDIA_CODEC
  rv = Convertor::GetYUV(yuv_img, vio_msg, config_->layer_);
#else
  /* 2. start encode yuv to jpeg */
  /* 2.1 get media codec vb buf for store src yuv data */
  MediaCodecManager &manager = MediaCodecManager::Get();
  if (config_->use_vb_) {
    rv = manager.GetVbBuf(chn_, &frame_buf);
    HOBOT_CHECK(rv == 0);
  } else {
    frame_buf = &src_buf;
    memset(frame_buf, 0x00, sizeof(iot_venc_src_buf_t));
  }
  frame_buf->frame_info.pts = frame->time_stamp_;
  /* 2.2 get src yuv data */
  rv = Convertor::GetYUV(frame_buf, vio_msg, config_->layer_, config_->use_vb_);
  HOBOT_CHECK(rv == 0);

#endif
  if (0 == rv) {
#ifndef X3_MEDIA_CODEC
    bret = Convertor::YUV2JPG(img_buf, yuv_img, 50);
#else
    /* 2.3. encode yuv data to jpg */
    auto ts0 = Timer::current_time_stamp();
    rv = manager.EncodeYuvToJpg(chn_, frame_buf, &stream_buf);
    if (config_->jpg_encode_time_ == 1) {
        auto ts1 = Timer::current_time_stamp();
        LOGW << "******Encode yuv to jpeg cost: " << ts1 - ts0 << "ms";
    }
    if (rv == 0) {
        bret = true;
        auto data_ptr = stream_buf->stream_info.pstPack.vir_ptr;
        auto data_size = stream_buf->stream_info.pstPack.size;
        img_buf.assign(data_ptr, data_ptr + data_size);
    } else {
        bret = false;
        LOGE << "X3 media codec jpeg encode failed!";
    }
#endif
#ifdef X3_MEDIA_CODEC
    /* 2.4 free jpg stream buf */
    if (stream_buf != nullptr) {
      rv = manager.FreeStream(chn_, stream_buf);
      HOBOT_CHECK(rv == 0);
    }
    /* 2.5 free media codec vb buf */
    if (config_->use_vb_) {
      rv = manager.FreeVbBuf(chn_, frame_buf);
      HOBOT_CHECK(rv == 0);
    }
#endif
    if (bret) {
      // todo send image to web
      auto image = x3_frame_msg.mutable_img_();
      x3_frame_msg.set_timestamp_(timestamp);
      image->set_buf_((const char *)img_buf.data(), img_buf.size());
      image->set_type_("jpeg");
      image->set_width_(dst_image_width_);
      image->set_height_(dst_image_height_);
      std::unique_lock<std::mutex> lock(map_smart_mutex_);
      x3_frames_.push(x3_frame_msg);
      map_smart_condition_.notify_one();
      if (x3_frames_.size() > cache_size_)
        LOGW << "the cache is full, maybe the encode thread is slowly";
      /* dump jpg picture */
      if (config_->dump_jpg_num_-- > 0) {
          static int frame_id = 0;
          std::string file_name = "out_stream_" +
              std::to_string(frame_id++) + ".jpg";
          std::fstream fout(file_name, std::ios::out | std::ios::binary);
          fout.write((const char *)img_buf.data(), img_buf.size());
          fout.close();
      }
    } else {
      LOGW << "encode jpg failed, push empty frame to queue";
      auto image = x3_frame_msg.mutable_img_();
      x3_frame_msg.set_timestamp_(timestamp);
      image->set_width_(0);
      image->set_height_(0);
      std::unique_lock<std::mutex> lock(map_smart_mutex_);
      x3_frames_.push(x3_frame_msg);
      map_smart_condition_.notify_one();
    }
  }
  return;
}
}  // namespace websocketplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
