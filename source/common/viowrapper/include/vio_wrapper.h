/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: hb_vio_fb_wrapper.h
 * @Brief: declaration of the hb_vio_fb_wrapper
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-05-23 14:18:28
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-23 16:16:58
 */

#ifndef INCLUDE_VIO_WRAPPER_H_
#define INCLUDE_VIO_WRAPPER_H_

#include <string>

#include "./hb_vio_common.h"
#include "./hb_vio_interface.h"
#include "./vio_debug.h"
#include "opencv2/opencv.hpp"

class HbVioWrapper {
 public:
  explicit HbVioWrapper(std::string hb_vio_cfg) : hb_vio_cfg_(hb_vio_cfg) {}

  virtual int Init() = 0;
  virtual int DeInit() = 0;

 protected:
  std::string hb_vio_cfg_;
  bool init_ = false;
};

class HbVioFbWrapper : public HbVioWrapper {
 public:
  explicit HbVioFbWrapper(std::string hb_vio_cfg) : HbVioWrapper(hb_vio_cfg) {}
  virtual ~HbVioFbWrapper() {
    if (init_) DeInit();
  }
  virtual int Init();
  virtual int DeInit();
  int FreeImgInfo(img_info_t *fb_img);

  int GetImgInfo(std::string rgb_file, img_info_t *fb_img,
                 uint32_t *effective_w, uint32_t *effective_h);
  int GetImgInfo(uint8_t *nv12, int w, int h, img_info_t *fb_img);
  void TransImage(cv::Mat *src_mat, cv::Mat *dst_mat, int dst_w, int dst_h,
                  uint32_t *effective_w, uint32_t *effective_h);

 private:
  src_img_info_t process_info_;
};

class HbVioMonoCamera : public HbVioWrapper {
 public:
  explicit HbVioMonoCamera(std::string hb_vio_cfg, std::string camera_cfg)
      : HbVioWrapper(hb_vio_cfg), camera_cfg_(camera_cfg) {}
  virtual ~HbVioMonoCamera() {
    if (init_) DeInit();
  }
  virtual int Init();
  virtual int DeInit();

  int GetImage(img_info_t *pyd_img);

  int Free(img_info_t *pyd_img);

 private:
  std::string camera_cfg_;
  int camera_idx_ = 0;
};

class HbVioDualCamera : public HbVioWrapper {
 public:
  explicit HbVioDualCamera(std::string hb_vio_cfg, std::string camera_cfg)
      : HbVioWrapper(hb_vio_cfg), camera_cfg_(camera_cfg) {}
  virtual ~HbVioDualCamera() {
    if (init_) DeInit();
  }
  virtual int Init();
  virtual int DeInit();

  int GetMultiImage(mult_img_info_t *pyd_img);
  int Free(mult_img_info_t *pyd_img);

 private:
  std::string camera_cfg_;
  int camera_idx_ = 2;
};
#endif  // INCLUDE_VIO_WRAPPER_H_

