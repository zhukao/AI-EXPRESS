/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: hb_vio_fb_wrapper.h
 * @Brief: declaration of the hb_vio_fb_wrapper
 * @Author: hangjun.yang
 * @Email: hangjun.yang@horizon.ai
 * @Date: 2020-05-25 14:18:28
 * @Last Modified by: hangjun.yang
 * @Last Modified time: 2020-05-25 22:00:00
 */
#ifndef INCLUDE_VIO_WRAPPER_GLOBAL_H_
#define INCLUDE_VIO_WRAPPER_GLOBAL_H_

#include <string>
#include <vector>
#include <memory>
#include "./hb_vio_common.h"
#include "./hb_vio_interface.h"
#include "./vio_debug.h"
#include "opencv2/opencv.hpp"
#ifdef X3_X2_VIO
#include "./x3_vio_patch.h"
#include "horizon/vision_type/vision_type.hpp"
#endif
#ifdef X3_IOT_VIO
// todo, add iot vio header
#endif
using hobot::vision::PymImageFrame;

class HbVioFbWrapperGlobal {
 public:
  explicit HbVioFbWrapperGlobal(std::string hb_vio_cfg);

  ~HbVioFbWrapperGlobal();

  int Init();

  int DeInit();

  std::shared_ptr<PymImageFrame> GetImgInfo(std::string rgb_file,
                                            uint32_t *effective_w,
                                            uint32_t *effective_h);

  std::shared_ptr<PymImageFrame> GetImgInfo(uint8_t *nv12, int w, int h);

  int FreeImgInfo(std::shared_ptr<PymImageFrame> pym_image);

  void TransImage(cv::Mat *src_mat, cv::Mat *dst_mat, int dst_w, int dst_h,
                  uint32_t *effective_w, uint32_t *effective_h);

 private:
  bool init_ = false;
  std::string hb_vio_cfg_;
#ifdef X3_X2_VIO
  src_img_info_t process_info_;
#endif
#ifdef X3_IOT_VIO
  hb_vio_buffer_t process_info_;
#endif
};

class HbVioMonoCameraGlobal {
 public:
  explicit HbVioMonoCameraGlobal(std::string hb_vio_cfg,
                                 std::string camera_cfg);

  ~HbVioMonoCameraGlobal();

  int Init();

  int DeInit();

  std::shared_ptr<PymImageFrame> GetImage();

  int Free(std::shared_ptr<PymImageFrame> pym_image);

 private:
  bool init_ = false;
  std::string hb_vio_cfg_;
  std::string camera_cfg_;
  int camera_idx_ = 0;
#ifdef X3_IOT_VIO
// todo, need add extra field?
#endif
};

class HbVioDualCameraGlobal {
 public:
  explicit HbVioDualCameraGlobal(std::string hb_vio_cfg,
                                 std::string camera_cfg) {}
  ~HbVioDualCameraGlobal() {}
  int Init() { return 0; }
  int DeInit() { return 0; }
  std::vector<std::shared_ptr<PymImageFrame>> GetMultiImage() {
    // todo
    std::vector<std::shared_ptr<PymImageFrame>> ret;
    return ret;
  }
  int Free(std::vector<std::shared_ptr<PymImageFrame>> &multi_pym_image) {
    // todo
    return 0;
  }
};
#endif  // INCLUDE_VIO_WRAPPER_GLOBAL_H_
