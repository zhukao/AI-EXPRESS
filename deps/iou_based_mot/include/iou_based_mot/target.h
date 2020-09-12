//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//
#ifndef IOU_BASED_MOT_TARGET_H
#define IOU_BASED_MOT_TARGET_H

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <printf.h>

namespace hobot{
namespace iou_mot{
/* this structure describes bouding box information */
struct BBox_s
{
  BBox_s() {}
  BBox_s(int x1_, int y1_, int x2_, int y2_) {
    x1 = x1_; y1 = y1_;
    x2 = x2_; y2 = y2_;
  }
  BBox_s(int x1_, int y1_, int x2_, int y2_, float score_) {
    x1 = x1_; y1 = y1_;
    x2 = x2_; y2 = y2_;
    score = score_;
  }
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  float score = 1;
  int box_id = -1;
};
typedef std::shared_ptr<BBox_s> sp_BBox;

struct Target
{
  Target() = default;
  Target(const sp_BBox &body_bbox,
         const double dist_with_others,
         const float overlap_with_others,
         const int img_width,
         const int img_height) {
    this->img_width = img_width;
    this->img_height = img_height;
    this->body_bbox = body_bbox;
    body_width = body_bbox->x2 - body_bbox->x1;
    body_height = body_bbox->y2 - body_bbox->y1;
    this->dist_with_others = dist_with_others;
    this->overlap_with_others = overlap_with_others;
  }

  int img_width = -1, img_height = -1;
  sp_BBox body_bbox;
  int body_width = 0;
  int body_height = 0;
  bool det_valid = true;
  double dist_with_others = -1.;
  float overlap_with_others = -1.f;
};
typedef std::shared_ptr<Target> sp_Target;

}  // namespace iou_mot
}  // namespace hobot

#endif //IOU_BASED_MOT_TARGET_H
