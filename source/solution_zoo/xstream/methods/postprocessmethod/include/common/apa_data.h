/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: apa_data.h
 * @Brief: declaration of the apa_data
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-09-03 9:47:31
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-09-03 10:22:45
 */

#ifndef COMMON_APADATA_H_
#define COMMON_APADATA_H_

#define PI 3.1415926

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <numeric>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "horizon/vision_type/vision_type.hpp"

using hobot::vision::BBox;

namespace xstream {

struct Parking3DBBox {
  void Init() {
    tp[0].y = w / 2;
    tp[0].x = l / 2;
    tp[1].y = -w / 2;
    tp[1].x = l / 2;
    tp[2].y = -w / 2;
    tp[2].x = -l / 2;
    tp[3].y = w / 2;
    tp[3].x = -l / 2;
    float min_x = 575.0f, min_y = 575.0f;
    float max_x = -100.0f, max_y = -100.0f;
    for (int j = 0; j < 4; j++) {
      cv::Point2f rpt;
      rpt.y = y + tp[j].x * cosf(theta) - tp[j].y * sinf(theta);
      rpt.x = x + tp[j].x * sinf(theta) + tp[j].y * cosf(theta);
      if (rpt.x < min_x) {
        min_x = rpt.x;
      }
      if (rpt.x > max_x) {
        max_x = rpt.x;
      }
      if (rpt.y < min_y) {
        min_y = rpt.y;
      }
      if (rpt.y > max_y) {
        max_y = rpt.y;
      }
      tp[j].x = rpt.x;
      tp[j].y = rpt.y;
    }
    outing_box.score = score;
    outing_box.x1 = min_x;
    outing_box.x2 = max_x;
    outing_box.y1 = min_y;
    outing_box.y2 = max_y;
    tp[4].y = w / 2;
    tp[4].x = l / 2;
    tp[5].y = -w / 2;
    tp[5].x = l / 2;
    tp[6].y = -w / 2;
    tp[6].x = -l / 2;
    tp[7].y = w / 2;
    tp[7].x = -l / 2;
    for (int j = 4; j < 8; j++) {
      cv::Point2f rpt;
      rpt.y = y + tp[j].x * cosf(theta) - tp[j].y * sinf(theta);
      rpt.x = x + tp[j].x * sinf(theta) + tp[j].y * cosf(theta);
      tp[j].x = rpt.x;
      tp[j].y = rpt.y;
    }
    tp[8].x = z;
    tp[8].y = z + h;
  }
  inline Parking3DBBox() { }

  inline Parking3DBBox(float score, float center_x, float center_y,
                       float center_z, float l, float h, float w, float theta)
      : score(score), x(center_x), y(center_y), z(center_z), l(l), w(w), h(h),
        theta(theta) {
    Init();
  }
  inline friend std::ostream &operator<<(std::ostream &out,
                                         Parking3DBBox &bbox) {
    out << "( x1: " << bbox.outing_box.x1 << " y1: " << bbox.outing_box.y1
        << " x2: " << bbox.outing_box.x2 << " y2: " << bbox.outing_box.y2
        << " score: " << bbox.score << " )";
    out << std::endl;
    out << "(" << bbox.x << "," << bbox.y << "," << bbox.z << ") ";
    out << " " << bbox.h << " " << bbox.l << " "
        << bbox.w << " " << bbox.theta;
    out << std::endl;
    return out;
  }
  inline static bool greater(const Parking3DBBox &a, const Parking3DBBox &b) {
    return a.score > b.score;
  }
  BBox outing_box;
  cv::Point2f tp[9];
  float score, x, y, z, l, w, h, theta;
};

static void Parking_NMS_local_iou(std::vector<Parking3DBBox> &candidates,
                                  std::vector<Parking3DBBox> &result,
                                  const float overlap_ratio, const int top_N,
                                  const bool addScore) {
  if (candidates.size() == 0) {
    return;
  }
  std::vector<bool> skip(candidates.size(), false);
  std::stable_sort(candidates.begin(), candidates.end(),
                   Parking3DBBox::greater);

  int count = 0;
  for (size_t i = 0; count < top_N && i < skip.size(); ++i) {
    if (skip[i]) {
      continue;
    }
    skip[i] = true;
    ++count;

    float area_i = (candidates[i].outing_box.x2 - candidates[i].outing_box.x1) *
                   (candidates[i].outing_box.y2 - candidates[i].outing_box.y1);

    // suppress the significantly covered bbox
    for (size_t j = i + 1; j < skip.size(); ++j) {
      if (skip[j]) {
        continue;
      }
      // get intersections
      float xx1 =
          std::max(candidates[i].outing_box.x1, candidates[j].outing_box.x1);
      float yy1 =
          std::max(candidates[i].outing_box.y1, candidates[j].outing_box.y1);
      float xx2 =
          std::min(candidates[i].outing_box.x2, candidates[j].outing_box.x2);
      float yy2 =
          std::min(candidates[i].outing_box.y2, candidates[j].outing_box.y2);
      float area_intersection = (xx2 - xx1) * (yy2 - yy1);
      bool area_intersection_valid = (area_intersection > 0) && (xx2 - xx1 > 0);

      if (area_intersection_valid) {
        // compute overlap
        float area_j =
            (candidates[j].outing_box.x2 - candidates[j].outing_box.x1) *
            (candidates[j].outing_box.y2 - candidates[j].outing_box.y1);
        float o = area_intersection / (area_i + area_j - area_intersection);

        if (o > overlap_ratio) {
          skip[j] = true;
          if (addScore) {
            candidates[i].score += candidates[j].score;
          }
        }
      }
    }
    result.push_back(candidates[i]);
  }
  return;
}
// convert to 2D box
static void Center_To_Corner_Box2d(std::vector<Parking3DBBox> result,
                                   std::vector<cv::Mat> &out_2dbox) {
  for (auto parking_box : result) {
    // float x = parking_box.x;
    // float y = parking_box.y;
    // float w = parking_box.w;
    // float l = parking_box.l;
    // float r = -parking_box.theta;
    // r = r * 180.0f / PI - 180.0f;
    // if (r < -90.0f) {
    //   r += 90.0f;
    //   float tmp = w;
    //   w = l;
    //   l = tmp;
    // }
    // cv::RotatedRect rot_rect(cv::Point2f(x, y), cv::Size2f(w, l), r);
    cv::RotatedRect rot_rect(
        cv::Point2f(parking_box.x, parking_box.y),
        cv::Size2f(parking_box.l, parking_box.w),
        parking_box.theta);  // 顺时针旋转r
    cv::Mat boxPts;
    cv::boxPoints(rot_rect, boxPts);
    out_2dbox.push_back(boxPts);
  }
}

struct BPUParkingAnchorBox {
  int8_t x;
  int8_t y;
  int8_t z;
  int8_t l;
  int8_t w;
  int8_t h;
  int8_t theta;
};
union BPUParkingAnchorData {
  int8_t raw_data[7];
  BPUParkingAnchorBox data;
};
struct BPUParkingAnchorResult {
  uint32_t w;
  uint32_t h;
  uint32_t c;
  int8_t anchor;
  int8_t conf;
  BPUParkingAnchorData box;
};

struct LidarAnchor {
  float x;
  float y;
  float z;
  float l;
  float w;
  float h;
  float t;
  float d;
};

}  // namespace xstream

#endif  // COMMON_APADATA_H_
