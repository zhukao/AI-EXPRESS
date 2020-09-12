/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * @file traffic_info.h
 * @brief smart中输出的结构定义,与proto的版本对应
 * @author peibo.zhao, peibo.zhao@horizon.ai
 * @version v2.0.0
 * @date 2019-11-11
 */

#ifndef SRC_SMARTPLUGIN_VEHICLE_INFO_H_
#define SRC_SMARTPLUGIN_VEHICLE_INFO_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include "horizon/vision_type/vision_type.hpp"

using hobot::vision::ImageFrame;

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {

/**
 * @brief 坐标点
 */
struct Point {
  float x;
  float y;
  float score;

  Point() {
    x = y = 0;
    score = 0;
  }
};

/**
 * @brief 矩形框
 */
struct Box {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;

  Box() {
    x1 = y1 = x2 = y2 = 0;
    score = 0;
  }
};

/**
 * @brief 定位信息
 */
struct Gis {
  float longitude;
  float latitude;
  float width;
  float height;
  float orientation;

  Gis() {
    longitude = 0;
    latitude = 0;
    width = 0;
    height = 0;
    orientation = 0;
  }
};

/**
 * @brief 车牌信息
 */
struct Plate {
  Box box;
  std::vector<Point> points;
  std::string plate_num;
  int32_t is_double_plate;
  int32_t color;
  int32_t type;

  Plate() {
    is_double_plate = -1;
    color = -1;
    type = -1;
  }
};

/**
 * @brief 车辆信息
 */
struct VehicleInfo {
  uint64_t track_id;
  int32_t type;
  int32_t color;
  Box box;
  std::vector<Point> points;
  Plate plate_info;
  Point location;
  float speed;
  uint32_t land_id;
  std::shared_ptr<Gis> gis_info;

  VehicleInfo() {
    track_id = -1;
    type = -1;
    color = -1;
    speed = 0;
    land_id = -1;
  }
};

/**
  * @brief 车俩的抓拍信息
  */
struct VehicleCapture {
  VehicleInfo vehicle_info;
  std::shared_ptr<ImageFrame> image;  // 抓拍图像

  VehicleCapture() {
    image = nullptr;
  }
};

/**
 * @brief 违法信息
 */
struct AnomalyInfo {
  int32_t type;                                  // 违法类型
  std::vector<VehicleCapture> vehicle_captures;  // 违法时的信息

  AnomalyInfo() {
    type = -1;
  }
};

/**
 * @brief 道路情况
 */
struct TrafficConditionInfo {
  uint32_t type;

  TrafficConditionInfo() {
    type = 0;
  }
};

/**
 * @brief 输出的车流信息
 */
struct TrafficStatisticsInfo {
  int hour_index;  // 时段 小于0表示不需要发送车流信息
  uint32_t cycle_count;
  uint32_t cycle_minute;
  uint32_t vehicle_sum;
  uint32_t big_vehicle_sum;
  uint32_t small_vehicle_sum;
  float mean_speed;

  TrafficStatisticsInfo() {
    hour_index = -1;
    cycle_count = 0;
    cycle_minute = 0;
    vehicle_sum = 0;
    big_vehicle_sum = 0;
    small_vehicle_sum = 0;
    mean_speed = 0;
  }
};

/**
 * @brief 非机动车信息
 */
struct  NoMotorVehicleInfo {
  uint64_t track_id;
  Box box;
  std::shared_ptr<Gis> gis_info;

  NoMotorVehicleInfo() {
    track_id = -1;
  }
};

/**
 * @brief 人的信息
 */
struct  PersonInfo {
  uint64_t track_id;
  Box box;
  std::shared_ptr<Gis> gis_info;

  PersonInfo() {
    track_id = -1;
  }
};

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // ifndef SRC_SMARTPLUGIN_VEHICLE_INFO_H_
