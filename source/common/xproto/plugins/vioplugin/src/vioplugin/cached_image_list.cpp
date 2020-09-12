/*
 * @Description: implement of vioplugin
 * @Author: zhuoran.rong@horizon.ai
 * @Date: 2020-04-08 16:17:25
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
#include "opencv2/opencv.hpp"
#include "utils/executor.h"

#include "vioplugin/viomessage.h"
#include "vioplugin/vioprocess.h"
#include "vioplugin/vioproduce.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {

CachedImageList::CachedImageList(const char *vio_cfg_file) : VioProduce() {
  auto ret = hb_vio_init(vio_cfg_file);
  HOBOT_CHECK_LE(ret, 0) << "vio init failed";
  ret = hb_vio_start();
  HOBOT_CHECK_LE(ret, 0) << "vio start failed";
}

CachedImageList::~CachedImageList() {
  hb_vio_stop();
  hb_vio_deinit();
}

int CachedImageList::Run() {
  static uint64_t frame_id = 0;
  // 每帧的时间间隔 ms
  int interval_ms = 500;

  if (is_running_)
    return kHorizonVioErrorAlreadyStart;

  // 获得文件名数组
  auto json = config_->GetJson();
  // 图像文件列表
  auto cache_of_img_list = json["image_list"];

  if (cache_of_img_list.isNull()) {
    cache_of_img_list = Json::Value("");
  }

  if (cache_of_img_list.isString()) {
    auto file_list_obj = Json::Value();
    file_list_obj.resize(1);
    file_list_obj[0] = cache_of_img_list.asString();

    cache_of_img_list = file_list_obj;
  }

  auto interval_cfg = json["interval"];
  if (!interval_cfg.isNull()) {
    interval_ms = interval_cfg.asInt();
    HOBOT_CHECK(interval_ms >= 0) << "interval must great or equal than 0";
  }

  // 图像数据列表
  std::vector<img_info_t *> image_data_cache;
  std::vector<mult_img_info_t *> image_data_cache_multi;

  if (cam_type_ == "mono") {
    image_data_cache.resize(cache_of_img_list.size());
    // 加载图像到内存
    for (unsigned int i = 0; i < cache_of_img_list.size(); ++i) {
      auto *pvio_image =
          reinterpret_cast<img_info_t *>(std::calloc(1, sizeof(img_info_t)));
      HOBOT_CHECK(pvio_image != nullptr) << "failed to alloc memory";

      auto ret =
          FillVIOImageByImagePath(pvio_image, cache_of_img_list[i].asString());
      HOBOT_CHECK(ret) << "fill vio from image failed";

      // 加入到cache中
      image_data_cache[i] = pvio_image;
    }
  } else if (cam_type_ == "dual") {
    image_data_cache_multi.resize(cache_of_img_list.size());
    // 加载图像到内存
    for (unsigned int i = 0; i < cache_of_img_list.size(); ++i) {
      auto *pvio_image = reinterpret_cast<mult_img_info_t *>(
          std::calloc(1, sizeof(mult_img_info_t)));
      HOBOT_CHECK(pvio_image != nullptr) << "failed to alloc memory";

      auto ret =
          FillVIOImageByImagePath(pvio_image, cache_of_img_list[i].asString());
      HOBOT_CHECK(ret) << "fill vio from image failed";

      // 加入到cache中
      image_data_cache_multi[i] = pvio_image;
    }
  } else {
    LOGE << "Unsupport camera type : " << cam_type_;
    return kHorizonVisionErrorParam;
  }

  LOGI << "Finish importing images";

  is_running_ = true;

  while (1) {
    // 循环这些list, 循环读出一个 sid => source id
    for (unsigned int sid = 0; sid < cache_of_img_list.size(); sid++) {
      if (!is_running_) {
        goto exit_loop;
      }
      if (cam_type_ == "mono") {
        if (AllocBuffer()) {
          auto **images = new HorizonVisionImageFrame *[1];

          HorizonVisionImageFrame *image_frame = nullptr;
          HorizonVisionAllocImageFrame(&image_frame);
          HOBOT_CHECK(image_frame != nullptr);

          // 装填数据
          image_frame->channel_id = sid;
          image_frame->frame_id = frame_id++;
          // XXX 时间戳简单的自增,只要保证每次不一样
          image_frame->time_stamp = image_data_cache[sid]->timestamp++;
          image_frame->image.height = image_data_cache[sid]->src_img.height;
          image_frame->image.width = image_data_cache[sid]->src_img.width;
        #ifdef X2
          image_frame->image.pixel_format = kHorizonVisionPixelFormatX2PYM;
        #endif
          image_frame->image.stride = image_data_cache[sid]->src_img.width;
          image_frame->image.stride_uv = image_data_cache[sid]->src_img.width;
          image_frame->image.data =
              reinterpret_cast<uint8_t *>(image_data_cache[sid]);
          image_frame->image.data_size = sizeof(img_info_t);

          images[0] = image_frame;

          std::shared_ptr<VioMessage> input(new ImageVioMessage(images, 1),
                                            [&](ImageVioMessage *p) {
                                              if (p) {
                                                // p->FreeImage();
                                                // 释放image 和image frame
                                                if (p->image_) {
                                                  // XXX
                                                  // 这里直接释放是否会导致内存泄露
                                                  std::free(p->image_[0]);
                                                  delete[] p->image_;
                                                  p->image_ = nullptr;
                                                }
                                                FreeBuffer();
                                                delete p;
                                              }
                                              p = nullptr;
                                            });
          if (push_data_cb_)
            push_data_cb_(input);
          LOGD << "Push Image message!!!";
        } else {
          // 产生Drop帧
          LOGV << "NO VIO BUFFER";
          auto input = std::make_shared<DropVioMessage>(
              static_cast<uint64_t>(image_data_cache[sid]->timestamp++),
              frame_id++);
          if (push_data_cb_)
            push_data_cb_(input);
          LOGI << "Push Drop message!!!";
        }
      } else if (cam_type_ == "dual") {
        if (AllocBuffer()) {
          auto **images = new HorizonVisionImageFrame *[2];

          for (int i = 0; i < image_data_cache_multi[sid]->img_num; ++i) {
            HorizonVisionImageFrame *image_frame = nullptr;
            HorizonVisionAllocImageFrame(&image_frame);

            auto &pvio = image_data_cache_multi[sid]->img_info[i];
            image_frame->channel_id = sid;
            image_frame->frame_id = frame_id++;
            image_frame->time_stamp = pvio.timestamp++;
            image_frame->image.height = pvio.src_img.height;
            image_frame->image.width = pvio.src_img.width;
          #ifdef X2
            image_frame->image.pixel_format = kHorizonVisionPixelFormatX2PYM;
          #endif
            image_frame->image.stride = pvio.src_img.width;
            image_frame->image.stride_uv = pvio.src_img.width;
            image_frame->image.data = reinterpret_cast<uint8_t *>(&pvio);
            image_frame->image.data_size = sizeof(mult_img_info_t);
            images[i] = image_frame;
          }

          std::shared_ptr<VioMessage> input(
              new ImageVioMessage(images, image_data_cache_multi[sid]->img_num,
                                  true, image_data_cache_multi[sid]),
              [&](ImageVioMessage *p) {
                if (p) {
                  // p->FreeImage();
                  if (p->image_) {
                    for (uint32_t i = 0; i < p->num_; ++i) {
                      std::free(p->image_[i]);
                    }
                    delete[] p->image_;
                    p->image_ = nullptr;
                  }
                  FreeBuffer();
                  delete p;
                }
                p = nullptr;
              });
          if (push_data_cb_)
            push_data_cb_(input);
          LOGD << "Push Image message!!!";
        } else {
          int64_t timestamp = 0;
          // 产生Drop帧
          LOGV << "NO VIO BUFFER";
          // XXX 保持两个时间戳同步增长
          for (int i = 0; i < image_data_cache_multi[sid]->img_num; ++i) {
            auto &pvio = image_data_cache_multi[sid]->img_info[i];
            timestamp = pvio.timestamp++;
          }
          auto input = std::make_shared<DropVioMessage>(
              static_cast<uint64_t>(timestamp), frame_id++);
          if (push_data_cb_)
            push_data_cb_(input);
          LOGI << "Push Drop message!!!";
        }
      }
      // 暂停
      std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
  }

exit_loop:
  // 释放资源
  if (cam_type_ == "mono") {
    for (unsigned int i = 0; i < cache_of_img_list.size(); ++i) {
      hb_vio_free(image_data_cache[i]);
      std::free(image_data_cache[i]);
      image_data_cache[i] = nullptr;
    }
  } else if (cam_type_ == "dual") {
    for (unsigned int i = 0; i < cache_of_img_list.size(); ++i) {
      hb_vio_mult_free(image_data_cache_multi[i]);
      std::free(image_data_cache_multi[i]);
      image_data_cache_multi[i] = nullptr;
    }
  }

  return kHorizonVisionSuccess;
}

// 此函数用于通过路径将jpeg格式回灌
extern bool GetPyramidInfo(mult_img_info_t *pvio_image, char *data, int len);
extern bool GetPyramidInfo(img_info_t *pvio_image, char *data, int len);
template <typename T>
bool CachedImageList::FillVIOImageByImagePath(T *pvio_image,
                                              const std::string &img_name) {
  auto image = GetImageFrame(img_name);
  auto pad_width = std::stoi(config_->GetValue("pad_width"));
  auto pad_height = std::stoi(config_->GetValue("pad_height"));
  auto res = PadImage(&image->image, pad_width, pad_height);
  if (res != 0) {
    LOGF << "Failed to pad image " << img_name << ", error code is " << res;
    return false;
  }
  HorizonVisionImage *nv12;
  HorizonVisionAllocImage(&nv12);
  HorizonConvertImage(&image->image, nv12, kHorizonVisionPixelFormatRawNV12);
  auto ret = GetPyramidInfo(pvio_image, reinterpret_cast<char *>(nv12->data),
                            nv12->height * nv12->width * 3 / 2);
  HorizonVisionFreeImage(nv12);
  HorizonVisionFreeImageFrame(image);
  return ret;
}

}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
