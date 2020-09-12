/*
 * @Description: implement of video feedback produce
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-06-16 16:17:25
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include <unistd.h>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "utils/executor.h"

#include "hobotxstream/image_tools.h"
#include "vioplugin/viomessage.h"
#include "vioplugin/vioprocess.h"
#include "vioplugin/vioproduce.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {

VideoFeedbackProduce::VideoFeedbackProduce(const char *vio_cfg_file)
    : VioProduce() {
#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
  auto ret = hb_vio_init(vio_cfg_file);
  HOBOT_CHECK_LE(ret, 0) << "vio init failed";
  ret = hb_vio_start();
  HOBOT_CHECK_LE(ret, 0) << "vio start failed";
// #elif defined(X3_IOT_VIO)
//   auto ret = iot_vio_init(vio_cfg_file);
//   HOBOT_CHECK_LE(ret, 0) << "vio init failed";
//   ret = iot_vio_start();
//   HOBOT_CHECK_LE(ret, 0) << "vio start failed";
#endif
}

VideoFeedbackProduce::~VideoFeedbackProduce() {
#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
  hb_vio_stop();
  hb_vio_deinit();
// #elif defined(X3_IOT_VIO)
//   iot_vio_stop();
//   iot_vio_deinit();
#endif
}

#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
extern bool GetPyramidInfo(img_info_t *pvio_image, char *data, int len);
extern bool GetPyramidInfo(VioFeedbackContext *feed_back_context, char *data,
                           int len);
#endif

#ifdef X3_IOT_VIO
extern bool GetPyramidInfo(pym_buffer_t *pvio_image, char *data, int len);
#endif

int VideoFeedbackProduce::Run() {
  if (is_running_)
    return kHorizonVioErrorAlreadyStart;
  is_running_ = true;
  static uint64_t frame_id = 0;
  auto json_obj = config_->GetJson();
  int pad_width = 0;
  int pad_height = 0;
  std::string video_path = "";
  // 每帧的时间间隔 ms
  int interval_ms = 500;
  auto pad_width_cfg = json_obj["pad_width"];
  auto pad_height_cfg = json_obj["pad_height"];
  auto video_path_cfg = json_obj["video_path"];
  auto interval_ms_cfg = json_obj["interval"];
  if (!pad_width_cfg.isNull()) {
    pad_width = pad_width_cfg.asInt();
  }
  if (!pad_height_cfg.isNull()) {
    pad_height = pad_height_cfg.asInt();
  }
  if (!video_path_cfg.isNull()) {
    video_path = video_path_cfg.asString();
  }
  if (!interval_ms_cfg.isNull()) {
    interval_ms = interval_ms_cfg.asInt();
  }
  // check config
  HOBOT_CHECK(pad_width > 0 && pad_height > 0 && video_path.size() > 0 &&
              interval_ms > 0)
      << "video feedback config error";

  LOGI << "video feedback info:" << std::endl
       << "pad_width = " << pad_width << std::endl
       << "pad_height = " << pad_height << std::endl
       << "video_path = " << video_path << std::endl
       << "interval_ms = " << interval_ms << std::endl;
  cv::VideoCapture video_capture;
  video_capture.open(video_path);
  HOBOT_CHECK(video_capture.isOpened())
      << "Open video feedback file " << video_path << " failed";

  while (is_running_) {
    cv::Mat frame;
    video_capture >> frame;
    if (frame.empty()) {
      // to the file end
      video_capture.open(video_path);
      continue;
    }
    // pad BGR
    cv::Mat pad_frame = frame;
    int ori_width = frame.cols;
    int ori_height = frame.rows;
    if (ori_width != pad_width || ori_height != pad_height) {
      pad_frame = cv::Mat(pad_height, pad_width, CV_8UC3, cv::Scalar::all(0));
      if (ori_width > pad_width || ori_height > pad_height) {
        auto aspect_ratio = ori_width / ori_height;
        auto dst_ratio = static_cast<float>(pad_width) / ori_height;
        uint32_t resized_width = -1;
        uint32_t resized_height = -1;
        // 等比缩放
        if (aspect_ratio >= dst_ratio) {
          resized_width = pad_width;
          resized_height =
              static_cast<uint32_t>(ori_height * pad_width / ori_width);
        } else {
          resized_width =
              static_cast<uint32_t>(ori_width * pad_height / ori_height);
          resized_height = pad_height;
        }
        cv::resize(frame, frame, cv::Size(resized_width, resized_height));
      }
      // 复制到目标图像中间
      frame.copyTo(pad_frame(cv::Rect((pad_width - frame.cols) / 2,
                                      (pad_height - frame.rows) / 2, frame.cols,
                                      frame.rows)));
    }
    // convert BGR TO NV12
    cv::Mat yuv_frame;
    uint8_t *nv12_data = nullptr;
    int nv12_data_size = 0;
    int nv12_y_stride = 0;
    int nv12_uv_stride = 0;
    if (HobotXStreamConvertImage(pad_frame.data, pad_height * pad_width * 3,
                                 pad_width, pad_height, pad_width * 3, 0,
                                 IMAGE_TOOLS_RAW_BGR, IMAGE_TOOLS_RAW_YUV_NV12,
                                 &nv12_data, &nv12_data_size, &nv12_y_stride,
                                 &nv12_uv_stride) != 0) {
      LOGE << "Convert from BGR to NV12 failed";
      if (nv12_data) {
        HobotXStreamFreeImage(nv12_data);
        nv12_data = nullptr;
      }
      continue;
    }
    HOBOT_CHECK(nv12_data_size == pad_width * pad_height * 3 / 2)
        << "Convert from BGR to NV12 failed ";
    // 分配Buffer. 等待Buffer可用
    while (!AllocBuffer()) {
      LOGV << "NO VIO_FB_BUFFER";
      std::this_thread::sleep_for(std::chrono::microseconds(5));
    }
#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
    VioFeedbackContext *feedback_context =
        reinterpret_cast<VioFeedbackContext *>(
            std::calloc(1, sizeof(VioFeedbackContext)));
#endif
    bool ret = GetPyramidInfo(
        feedback_context, reinterpret_cast<char *>(nv12_data), nv12_data_size);
    HOBOT_CHECK(ret) << "Get Pymrid info failed";
    HobotXStreamFreeImage(nv12_data);
    nv12_data = nullptr;
    std::vector<std::shared_ptr<PymImageFrame>> pym_images;
    auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
    Convert(&(feedback_context->pym_img_info), *pym_image_frame_ptr);
    pym_image_frame_ptr->channel_id = 0;
    pym_image_frame_ptr->frame_id = frame_id;
    pym_image_frame_ptr->time_stamp = frame_id++;
    // set context to feedback_context
    pym_image_frame_ptr->context = static_cast<void *>(feedback_context);
    pym_images.push_back(pym_image_frame_ptr);
    std::shared_ptr<VioMessage> input(new ImageVioMessage(pym_images, 1),
                                      [&](ImageVioMessage *p) {
                                        if (p) {
                                          p->FreeImage(1);
                                          FreeBuffer();
                                          delete p;
                                        }
                                        p = nullptr;
                                      });
    if (push_data_cb_)
      push_data_cb_(input);
    LOGD << "Push Image message!!!";
    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
  }
  is_running_ = false;
  return kHorizonVisionSuccess;
}  // namespace vioplugin

}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
