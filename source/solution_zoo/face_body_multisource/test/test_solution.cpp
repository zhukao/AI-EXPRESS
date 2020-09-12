/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     test_solution.cpp
 * \Author   Zhuoran Rong
 * \Mail     zhuoran.rong@horizon.ai
 * \Version  1.0.0.0
 * \Date     2020/04/18
 * \Brief    implement of test_solution.cpp
 */
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision/util.h"
#include "smartplugin/smartplugin.h"
#include "gtest/gtest.h"
#include <fstream>
#include <string>
#include <sys/utsname.h>


// declear actual main
int solution_main(int argc, const char **argv);

namespace {

// normal run
TEST(MultiSourceSolution, normal) {
  int ret = 0;
  const char *argv[] = {
      "face_body_multisource_test",                              // argv0
      "./configs/vio_config.json.96board.hg",                    // argv1
      "./face_body_multisource/configs/face_body_solution.json",  // argv2
      "-i",                                                      // argv3
      "ut",                                                      // normal
  };
  ret = solution_main(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(ret, 0);

  argv[3] = "-g";  // unknow parameter
  ret = solution_main(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ(ret, 0);
}

// ImageConvert Test
TEST(MultiSourceSolution, ut_test_1) {
  HorizonVisionImage test_img;

  std::ifstream file("face_body_multisource/configs/images/image_test.jpg",
                     std::ios::in | std::ios::binary | std::ios::ate);
  EXPECT_TRUE(file.is_open());

  std::streampos size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::string buffer(size, ' ');
  file.read(&buffer[0], size);
  file.close();

  uint8_t *tmp_img_data = new uint8_t[size];
  memcpy((void *)tmp_img_data, buffer.c_str(), size);  // NOLINT
  // set pseudo image
  test_img.pixel_format = kHorizonVisionPixelFormatNone;
  test_img.data = tmp_img_data;
  test_img.data_size = size;

  // invoke ImageConversion with pseudo data
  horizon::vision::util::ImageConversion(test_img);

  delete[] tmp_img_data;

  // pseudo RawBGR Img
  tmp_img_data = new uint8_t[64];
  test_img.pixel_format = kHorizonVisionPixelFormatRawBGR;
  test_img.data = tmp_img_data;
  test_img.data_size = 64;
  test_img.width = 4;
  test_img.height = 4;

  // invoke ImageConversion with pseudo data
  horizon::vision::util::ImageConversion(test_img);

  delete[] tmp_img_data;
}

}  // namespace
