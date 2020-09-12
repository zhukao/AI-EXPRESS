/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     bbox base data type only for example
 * @author    wenhao.zou
 * @email     wenhao.zou@horizon.ai
 * @version   0.0.0.1
 * @date      2020.02.08
 */

#ifndef XSTREAM_TUTORIALS_STAGE4_METHOD_BBOX_H_
#define XSTREAM_TUTORIALS_STAGE4_METHOD_BBOX_H_

#include <string>
#include "hobotxsdk/xstream_data.h"

namespace hobot {
namespace vision {

template <typename Dtype>
struct BBox_ {
  inline BBox_() {}
  inline BBox_(Dtype x1_, Dtype y1_, Dtype x2_, Dtype y2_,
               int32_t id_ = -1) {
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
    id = id_;
  }
  inline Dtype Width() const { return (x2 - x1); }
  inline Dtype Height() const { return (y2 - y1); }
  inline Dtype CenterX() const { return (x1 + (x2 - x1) / 2); }
  inline Dtype CenterY() const { return (y1 + (y2 - y1) / 2); }

  inline friend std::ostream &operator<<(std::ostream &out, BBox_ &bbox) {
    out << "( x1: " << bbox.x1 << " y1: " << bbox.y1 << " x2: " << bbox.x2
        << " y2: " << bbox.y2 << " )";
    return out;
  }

  inline friend std::ostream &operator<<(std::ostream &out, const BBox_ &bbox) {
    out << "( x1: " << bbox.x1 << " y1: " << bbox.y1 << " x2: " << bbox.x2
        << " y2: " << bbox.y2 << " )";
    return out;
  }

  Dtype x1 = 0;
  Dtype y1 = 0;
  Dtype x2 = 0;
  Dtype y2 = 0;
  int32_t id = 0;
};
typedef BBox_<float> BBox;

}  // namespace vision
}  // namespace hobot

namespace xstream {
  typedef XStreamData<hobot::vision::BBox> BBox;
}  // namespace xstream

#endif  // XSTREAM_TUTORIALS_STAGE4_METHOD_BBOX_H_
