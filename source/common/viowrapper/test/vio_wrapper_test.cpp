/**
 * * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: vio_wrapper_test.cpp
 * @Brief: vio wrapper unit test
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-14
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-05-14
 */

#include "./vio_wrapper.h"

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#ifdef X3
#include "./vio_wrapper_global.h"
#endif
/*
TEST(VIO_WRAPPER_TEST, GetMultiImage) {
  std::string vio_cfg_file = "./config/96board/vio_onsemi_dual.json";
  std::string cam_cfg_file = "./config/hb_x2dev.json";

  mult_img_info_t data;
  int ret = 0;
  HbVioDualCamera hd_vio(vio_cfg_file, cam_cfg_file);
  ret = hd_vio.Init();
  ASSERT_TRUE(ret == 0);    // NOLINT
  ret = hd_vio.GetMultiImage(&data);
  ASSERT_TRUE(ret == 0);    // NOLINT
  hd_vio.Free(&data);
}
*/

TEST(VIO_WRAPPER_TEST, GetSingleImage_1080) {
#ifdef X2
  std::string vio_cfg_file = "./config/96board/vio_onsemi0230.json.96board";
  std::string cam_cfg_file = "/etc/cam/hb_96board.json";
#else
  std::string cam_cfg_file = "./config/x3dev/hb_camera_x3.json";
  std::string vio_cfg_file = "./config/x3dev/hb_vio_x3_1080.json";
#endif
  int ret = 0;
#ifdef X2
  img_info_t data;
  HbVioMonoCamera single_camera(vio_cfg_file, cam_cfg_file);
  ret = single_camera.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  ret = single_camera.GetImage(&data);
  ASSERT_TRUE(ret == 0);  // NOLINT
  vio_debug::print_info(data);
  vio_debug::dump_pym_nv12(&data, "single_camera");
  single_camera.Free(&data);
#else
  HbVioMonoCameraGlobal single_camera(vio_cfg_file, cam_cfg_file);
  ret = single_camera.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  std::shared_ptr<PymImageFrame> data = single_camera.GetImage();
  vio_debug::dump_pym_nv12(data, "single_camera");
  single_camera.Free(data);
#endif
}

TEST(VIO_WRAPPER_TEST, GetFbImage_1080) {
#ifdef X2
  std::string vio_cfg_file = "./config/vio_onsemi0230_fb.json";
  std::string img_list = "./data/image.list";
#else
   // must be same image resolution in image.list
  std::string vio_cfg_file = "./config/x3dev/hb_vio_x3_1080_fb.json";
  std::string img_list = "./data/image.list";
#endif
  img_info_t data;
  int ret = 0;
  std::ifstream ifs(img_list);
  ASSERT_TRUE(ifs.is_open());
  HbVioFbWrapper fb_handle(vio_cfg_file);
  ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  std::string input_image;
  std::string gt_data;
  while (getline(ifs, gt_data)) {
    std::cout << gt_data << std::endl;
    std::istringstream gt(gt_data);
    gt >> input_image;
    int pos=input_image.find_last_of('/');
    std::string file_name_bak(input_image.substr(pos+1));
    std::string file_name = file_name_bak.substr(0, file_name_bak.rfind("."));
    uint32_t effective_w, effective_h;
    ret = fb_handle.GetImgInfo(input_image, &data, &effective_w, &effective_h);
    ASSERT_TRUE(ret == 0);  // NOLINT
    vio_debug::print_info(data);
    vio_debug::dump_pym_nv12(&data, file_name);
    fb_handle.FreeImgInfo(&data);
  }
}
