/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 */
#include <turbojpeg.h>
#include <fstream>
#include <string>
#include "xproto_msgtype/protobuf/x3.pb.h"
#include "smartplugin/traffic_info.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {

void setAttribute(x3::Attributes* attribute,
                  std::string type, float value,
                  std::string value_string, float score);

// VehicleInfo
void convertVehicleInfo(
    x3::Target* target,
    const vision::xproto::smartplugin::VehicleInfo& vehicle_info,
    float x_ratio, float y_ratio);

// Nonmotor
void convertNonmotor(
    x3::Target* target,
    const vision::xproto::smartplugin::NoMotorVehicleInfo& nomotor_info,
    float x_ratio, float y_ratio);

// Person
void convertPerson(x3::Target* target,
                   const vision::xproto::smartplugin::PersonInfo& person_info,
                   float x_ratio, float y_ratio);

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
