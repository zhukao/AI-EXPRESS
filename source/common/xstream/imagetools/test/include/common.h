/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     common operate
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.10
 */
#ifndef TEST_INCLUDE_COMMON_H_
#define TEST_INCLUDE_COMMON_H_

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/common.h"


namespace xstream {

inline bool SaveFile(const std::string file_name,
                     const uint8_t *data,
                     const int data_size) {
  if (nullptr == data || data_size <= 0) {
    return false;
  }
  if (file_name.empty()) {
    return false;
  }
  std::ofstream strm;
  strm.open(file_name, std::ios::out | std::ios::binary);
  if (!strm.good()) {
    return false;
  }
  strm.write(reinterpret_cast<char *>(const_cast<uint8_t *>(data)), data_size);
  return true;
}

inline bool CheckOutput(const ImageToolsFormatData &output,
                        const int width,
                        const int height) {
  bool ret = true;
  ret = (output.width_ == width)
        && (output.height_ == height)
        && (output.data_[0] != nullptr);
  if (!ret) {
    return false;
  }
  if (IMAGE_TOOLS_RAW_RGB == output.format_
      || IMAGE_TOOLS_RAW_BGR == output.format_) {
    ret = (output.first_stride_ == width * 3)
          && (output.data_size_[0] == width * height * 3);
  } else if (IMAGE_TOOLS_RAW_GRAY == output.format_) {
    ret = (output.first_stride_ == width)
          && (output.data_size_[0] == width * height);
  } else if (IMAGE_TOOLS_RAW_YUV_I420 == output.format_) {
    ret = (output.first_stride_ == width)
          && (output.second_stride_ == width / 2)
          && (output.data_size_[0] == width * height * 3 / 2);
  } else {
    ret = (output.first_stride_ == width)
          && (output.second_stride_ == width)
          && (output.data_size_[0] == width * height * 3 / 2);
  }
  return ret;
}

inline std::int64_t getMilliSecond() {
  auto time_now = std::chrono::system_clock::now();
  auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      time_now.time_since_epoch());
  return duration_in_ms.count();
}

}  // namespace xstream

#endif  // TEST_INCLUDE_COMMON_H_
