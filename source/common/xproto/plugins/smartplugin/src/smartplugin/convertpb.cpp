/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 */

#include "smartplugin/convertpb.h"
#include <fstream>
#include <string>

#include "xproto_msgtype/protobuf/x3.pb.h"
#include "websocketplugin/attribute_convert/attribute_convert.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {
using horizon::vision::xproto::websocketplugin::AttributeConvert;

void setAttribute(x3::Attributes* attribute,
                  std::string type, float value,
                  std::string value_string, float score) {
    auto value_description = value_string;
    if (value_description.empty()) {
      auto key_value = static_cast<int32_t>(value);
      value_description =
          AttributeConvert::Instance().GetAttrDes(type, key_value);
    }
    attribute->set_type_(type);
    attribute->set_value_(value);
    attribute->set_value_string_(value_description);
    attribute->set_score_(score);
}

// VehicleInfo
void convertVehicleInfo(
    x3::Target *target,
    const vision::xproto::smartplugin::VehicleInfo &vehicle_info, float x_ratio,
    float y_ratio) {
  target->set_type_("vehicle");
  // track_id
  target->set_track_id_(vehicle_info.track_id);
  // type
  if (vehicle_info.type != -1) {
    auto type = target->add_attributes_();
    setAttribute(type, "vehicle_type", vehicle_info.type, "", 0);
  }
  // color
  if (vehicle_info.color != -1) {
    auto color = target->add_attributes_();
    setAttribute(color, "vehicle_color", vehicle_info.color, "", 0);
  }

  // vehicle_key_points
  {
    if (!vehicle_info.points.empty()) {
      auto points = target->add_points_();
      points->set_type_("vehicle_key_points");
      for (const auto &vehicle_point : vehicle_info.points) {
        auto vehicle_point_pb = points->add_points_();
        vehicle_point_pb->set_x_(vehicle_point.x * x_ratio);
        vehicle_point_pb->set_y_(vehicle_point.y * y_ratio);
        vehicle_point_pb->set_score_(vehicle_point.score);
      }
    }
  }

  // vehicle_box
  auto vehicle_box = target->add_boxes_();
  vehicle_box->set_type_("vehicle_box");
  vehicle_box->mutable_top_left_()->set_x_(vehicle_info.box.x1 * x_ratio);
  vehicle_box->mutable_top_left_()->set_y_(vehicle_info.box.y1 * y_ratio);
  vehicle_box->mutable_bottom_right_()->set_x_(vehicle_info.box.x2 * x_ratio);
  vehicle_box->mutable_bottom_right_()->set_y_(vehicle_info.box.y2 * y_ratio);
  vehicle_box->set_score(vehicle_info.box.score);

  // plate_info 车牌信息
  if (vehicle_info.plate_info.box.x1 != 0 &&
      vehicle_info.plate_info.box.y1 != 0 &&
      vehicle_info.plate_info.box.x2 != 0 &&
      vehicle_info.plate_info.box.y2 != 0) {
    auto vehicle_plate = target->add_sub_targets_();
    vehicle_plate->set_type_("plate");
    auto plate_box = vehicle_plate->add_boxes_();
    plate_box->set_type_("plate_box");
    plate_box->mutable_top_left_()->set_x_(vehicle_info.plate_info.box.x1 *
                                          x_ratio);
    plate_box->mutable_top_left_()->set_y_(vehicle_info.plate_info.box.y1 *
                                          y_ratio);
    plate_box->mutable_bottom_right_()->set_x_(vehicle_info.plate_info.box.x2 *
                                              x_ratio);
    plate_box->mutable_bottom_right_()->set_y_(vehicle_info.plate_info.box.y2 *
                                              y_ratio);
    // 是否双排车牌
    auto is_double_plate = vehicle_plate->add_attributes_();
    setAttribute(is_double_plate, "is_double_plate",
                vehicle_info.plate_info.is_double_plate, "", 0);
    // 车牌关键点
    if (!vehicle_info.points.empty()) {
      auto plate_points_pb = vehicle_plate->add_points_();
      plate_points_pb->set_type_("plate_key_points");
      for (const auto &plate_point : vehicle_info.points) {
        auto plate_point_pb = plate_points_pb->add_points_();
        plate_point_pb->set_x_(plate_point.x * x_ratio);
        plate_point_pb->set_y_(plate_point.y * y_ratio);
        plate_point_pb->set_score_(plate_point.score);
      }
    }
    // plate_num 车牌号
    if (!vehicle_info.plate_info.plate_num.empty()) {
      auto plate_num = vehicle_plate->add_attributes_();
      setAttribute(plate_num, "plate_num", 0,
                   vehicle_info.plate_info.plate_num, 0);
    }
    // color 车牌颜色
    auto plate_color = vehicle_plate->add_attributes_();
    setAttribute(plate_color, "plate_color", vehicle_info.plate_info.color, "",
                0);
    // type 车牌类型
    auto plate_type = vehicle_plate->add_attributes_();
    setAttribute(plate_type, "plate_type", vehicle_info.plate_info.type, "", 0);
  }
}

// Nonmotor
void convertNonmotor(
    x3::Target *target,
    const vision::xproto::smartplugin::NoMotorVehicleInfo &nomotor_info,
    float x_ratio, float y_ratio) {
  target->set_type_("no-motor");
  // track_id
  target->set_track_id_(nomotor_info.track_id);
  // box 非机动车框
  auto nomotor_box = target->add_boxes_();
  nomotor_box->set_type_("no-motor_box");
  nomotor_box->mutable_top_left_()->set_x_(nomotor_info.box.x1 * x_ratio);
  nomotor_box->mutable_top_left_()->set_y_(nomotor_info.box.y1 * y_ratio);
  nomotor_box->mutable_bottom_right_()->set_x_(nomotor_info.box.x2 * x_ratio);
  nomotor_box->mutable_bottom_right_()->set_y_(nomotor_info.box.y2 * y_ratio);
  nomotor_box->set_score(nomotor_info.box.score);
}

// Person
void convertPerson(x3::Target *target,
                   const vision::xproto::smartplugin::PersonInfo &person_info,
                   float x_ratio, float y_ratio) {
  target->set_type_("person");
  // tarck_id
  target->set_track_id_(person_info.track_id);
  // box
  auto person_box = target->add_boxes_();
  person_box->set_type_("person_box");
  person_box->mutable_top_left_()->set_x_(person_info.box.x1 * x_ratio);
  person_box->mutable_top_left_()->set_y_(person_info.box.y1 * y_ratio);
  person_box->mutable_bottom_right_()->set_x_(person_info.box.x2 * x_ratio);
  person_box->mutable_bottom_right_()->set_y_(person_info.box.y2 * y_ratio);
  person_box->set_score(person_info.box.score);
}

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
