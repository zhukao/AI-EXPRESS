/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.11.02
 */

#ifndef TEST_INCLUDE_UTILS_TEST_HPP_
#define TEST_INCLUDE_UTILS_TEST_HPP_

#include "vehicle_common_utility/data_type.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

ImageFramePtr crop_image(const ImageFramePtr &img_ptr,
                         const vision::BBox &crop_rect,
                         const uint32_t &output_width,
                         const uint32_t &output_height,
                         const bool &need_resize);


VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // TEST_INCLUDE_UTILS_TEST_HPP_
