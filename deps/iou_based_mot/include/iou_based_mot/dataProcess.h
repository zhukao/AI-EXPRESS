//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef IOU_BASED_MOT_DATAPROCESS_H_
#define IOU_BASED_MOT_DATAPROCESS_H_

#include "target.h"
#include <Eigen/Dense>
// Eigen3

namespace hobot {
namespace iou_mot {

#define myMax(a, b)  (((a) > (b)) ? (a) : (b))
#define myMin(a, b)  (((a) > (b)) ? (b) : (a))

Eigen::VectorXd convertBBox2VectorXd(const sp_BBox &bbox);

sp_BBox convertVectorXd2BBox(const Eigen::VectorXd &vec);

}  // namespace iou_mot
}  // namespace hobot

#endif  // IOU_BASED_MOT_DATAPROCESS_H_
