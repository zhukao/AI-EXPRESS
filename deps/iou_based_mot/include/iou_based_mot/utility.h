//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef IOU_BASED_MOT_UTILITY_H_
#define IOU_BASED_MOT_UTILITY_H_

#include "iou_based_mot/target.h"

namespace hobot {
namespace iou_mot {

uint32_t CalculateIntersectionArea1(const sp_BBox& box1, const sp_BBox& box2);

uint32_t CalculateBBoxArea1(const sp_BBox &bbox);

double CalculateBBoxOverlap(const sp_BBox &bbox1, const sp_BBox &bbox2);

double CalculateWithBBox1Overlap(const sp_BBox &bbox1, const sp_BBox &bbox2);

double CalculateBBoxDistance(const sp_BBox &bbox1, const sp_BBox &bbox2);

}  // namespace iou_mot
}  // namespace hobot

#endif  // IOU_BASED_MOT_UTILITY_H_
