/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: util.h
 * @Brief: declaration of the common func
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-26 9:47:31
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-26 10:22:45
 */

#ifndef COMMON_UTIL_H_
#define COMMON_UTIL_H_

#include <string>
#include <memory>
#include <numeric>

using hobot::vision::Feature;

namespace xstream {

// Align by 16
#define ALIGN_16(v) ((v + (16 - 1)) / 16 * 16)

inline void MemZero(void* p, size_t n)
{
  memset(p, 0, n);
}

inline float SigMoid(const float &input) {
  return 1 / (1 + std::exp(-1 * input));
}

inline float GetFloatByInt(int32_t value, uint32_t shift) {
  return (static_cast<float>(value)) / (static_cast<float>(1 << shift));
}

// coordinate transform.
// fasterrcnn model's input size maybe not eqaul to origin image size,
// needs coordinate transform for detection result.
template<typename T>
inline void CoordinateTransform(
    T &x, T &y,
    int src_image_width, int src_image_height,
    int model_input_width, int model_input_hight) {
  x = x * 1.0 * src_image_width / model_input_width;
  y = y * 1.0 * src_image_height / model_input_hight;
}

inline std::string GetParentPath(const std::string &path) {
  auto pos = path.rfind('/');
  if (std::string::npos != pos) {
    auto parent = path.substr(0, pos);
    return parent + "/";
  } else {
    return std::string("./");
  }
}

}  // namespace xstream

#endif  // COMMON_UTIL_H_
