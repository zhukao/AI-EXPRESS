//
// Created by shiyu.fu on 2020-04-08.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <gtest/gtest.h>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#include "yuv_utils.h"    // NOLINT
#include "FasterRCNNMethod/dump.h"
#include "horizon/vision_type/vision_type.hpp"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif
#include "opencv2/opencv.hpp"


using hobot::vision::ImageFrame;
using hobot::vision::CVImageFrame;

TEST(ImgUtilTest, BGR2NV12) {
  std::string test_img = "./test/data/1080p.jpg";
  std::ifstream ifs(test_img);
  ASSERT_TRUE(ifs.is_open());

  cv::Mat bgr_img = cv::imread(test_img);
  auto height = bgr_img.rows;
  auto width = bgr_img.cols;
  uint8_t *pbgr_img = bgr_img.ptr<uint8_t>();
  cv::Mat nv12_img;
  bgr_to_nv12(pbgr_img, height, width, nv12_img);
  ASSERT_FALSE(nv12_img.empty());
}

TEST(ImgUtilTest, NV122BGR) {
#ifdef X2
  HbVioFbWrapper fb_vio("./configs/vio_onsemi0230_fb.json");
  auto ret = fb_vio.Init();
  ASSERT_EQ(ret, 0);
  img_info_t data;
  std::string test_img = "./test/data/1080p.jpg";
  cv::Mat bgr_img = cv::imread(test_img);
  int width = bgr_img.cols;
  int height = bgr_img.rows;
  cv::Mat nv12_img;
  bgr_to_nv12(bgr_img.data, height, width, nv12_img);

  ret = fb_vio.GetImgInfo(nv12_img.data, width, height, &data);
  ASSERT_EQ(ret, 0);
  int img_height = data.src_img.height;
  int img_witdh = data.src_img.width;
  std::cout << "img height: " << img_height << "width: "
            << img_witdh << std::endl;
  int img_y_len = img_height * img_witdh;
  int img_uv_len = img_height * img_witdh / 2;
  uint8_t *img_ptr = static_cast<uint8_t*>(malloc(img_y_len + img_uv_len));
  memcpy(img_ptr, reinterpret_cast<uint8_t *>(data.src_img.y_vaddr),
          img_y_len);
  memcpy(img_ptr + img_y_len,
          reinterpret_cast<uint8_t *>(data.src_img.c_vaddr), img_uv_len);
  cv::Mat bgr_mat;
  nv12_to_bgr(img_ptr, img_height, img_witdh, bgr_mat);
  free(img_ptr);
  ASSERT_FALSE(bgr_mat.empty());
  fb_vio.FreeImgInfo(&data);
#endif
#ifdef X3
#ifdef X3_X2_VIO
  std::string fb_cfg = "./configs/vio_config/x3dev/hb_vio_x3_1080_fb.json";
#endif
#ifdef X3_IOT_VIO
  std::string fb_cfg = "./configs/vio_config/vio/x3dev/iot_vio_x3_1080_fb.json";
#endif
  HbVioFbWrapperGlobal fb_vio(fb_cfg);
  auto ret = fb_vio.Init();
  ASSERT_EQ(ret, 0);
  std::string test_img = "./test/data/1080p.jpg";
  cv::Mat bgr_img = cv::imread(test_img);
  int width = bgr_img.cols;
  int height = bgr_img.rows;
  cv::Mat nv12_img;
  bgr_to_nv12(bgr_img.data, height, width, nv12_img);

  auto pym = fb_vio.GetImgInfo(nv12_img.data, width, height);
  ASSERT_EQ(pym != nullptr, true);
  int img_height = pym->Height();
  int img_witdh = pym->Width();
  std::cout << "img height: " << img_height << "width: "
            << img_witdh << std::endl;
  int img_y_len = img_height * img_witdh;
  int img_uv_len = img_height * img_witdh / 2;
  uint8_t *img_ptr = static_cast<uint8_t*>(malloc(img_y_len + img_uv_len));
  memcpy(img_ptr, reinterpret_cast<uint8_t *>(pym->Data()),
          img_y_len);
  memcpy(img_ptr + img_y_len,
          reinterpret_cast<uint8_t *>(pym->DataUV()), img_uv_len);
  cv::Mat bgr_mat;
  nv12_to_bgr(img_ptr, img_height, img_witdh, bgr_mat);
  free(img_ptr);
  ASSERT_FALSE(bgr_mat.empty());
  fb_vio.FreeImgInfo(pym);
#endif
}
