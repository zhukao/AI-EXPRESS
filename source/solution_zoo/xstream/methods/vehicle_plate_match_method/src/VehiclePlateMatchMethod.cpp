/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.23
 */

#include "VehiclePlateMatchMethod/VehiclePlateMatchMethod.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

// #include "hobotxstream/data_types/bbox.h"
#include <random>
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"

namespace xstream {
typedef XStreamData<hobot::vision::BBox> XStreamBBox;

int VehiclePlateMatchMethod::Init(const std::string &config_file_path) {
  LOGI << "VehiclePlateMatchMethod::Init " << config_file_path;
  matcher = hobot::vehicle_snap_strategy::PairsMatchAPI::NewPairsMatchAPI(
      config_file_path);
  if (nullptr == matcher)
    return -1;
  return 0;
}

int VehiclePlateMatchMethod::UpdateParameter(InputParamPtr ptr) { return 0; }

InputParamPtr VehiclePlateMatchMethod::GetParameter() const {
  // method name is useless here
  // auto param = std::make_shared<FilterParam>("");

  return nullptr;  // param;
}

std::vector<std::vector<BaseDataPtr>> VehiclePlateMatchMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<InputParamPtr> &param) {
  LOGI << "VehiclePlateMatchMethod::DoProcess";
  std::vector<std::vector<BaseDataPtr>> output;

  output.resize(input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];

    auto in_vehicle_rects =
        std::dynamic_pointer_cast<BaseDataVector>(in_batch_i[0]);
    auto in_plate_rects =
        std::dynamic_pointer_cast<BaseDataVector>(in_batch_i[1]);
    auto in_plate_type =
        std::dynamic_pointer_cast<BaseDataVector>(in_batch_i[2]);
    auto in_plate_color =
        std::dynamic_pointer_cast<BaseDataVector>(in_batch_i[3]);
    auto in_vehicle_lmk = std::make_shared<BaseDataVector>();
    auto in_plate_lmk = std::make_shared<BaseDataVector>();
    if (in_batch_i.size() == 6) {
      in_vehicle_lmk =
          std::dynamic_pointer_cast<BaseDataVector>(in_batch_i[4]);
      in_plate_lmk =
          std::dynamic_pointer_cast<BaseDataVector>(in_batch_i[5]);
    }

    assert("BaseDataVector" == in_vehicle_rects->type_);
    assert("BaseDataVector" == in_plate_rects->type_);
    LOGI << "match method input vehicle size : "
         << in_vehicle_rects->datas_.size();

    auto out_vehicle_rects = std::make_shared<BaseDataVector>();
    auto out_vehicle_lmk = std::make_shared<BaseDataVector>();
    auto out_plate_rects = std::make_shared<BaseDataVector>();
    auto out_plate_lmk = std::make_shared<BaseDataVector>();
    auto out_plate_type = std::make_shared<BaseDataVector>();
    auto out_plate_color = std::make_shared<BaseDataVector>();
    out_batch_i.push_back(
        std::dynamic_pointer_cast<BaseData>(out_vehicle_rects));
    out_batch_i.push_back(
        std::dynamic_pointer_cast<BaseData>(out_vehicle_lmk));
    out_batch_i.push_back(
        std::dynamic_pointer_cast<BaseData>(out_plate_rects));
    out_batch_i.push_back(
        std::dynamic_pointer_cast<BaseData>(out_plate_lmk));
    out_batch_i.push_back(
        std::dynamic_pointer_cast<BaseData>(out_plate_type));
    out_batch_i.push_back(
        std::dynamic_pointer_cast<BaseData>(out_plate_color));

    hobot::vehicle_snap_strategy::VehicleListPtr vehicle_list =
        std::make_shared<hobot::vehicle_snap_strategy::VehicleList>();
    hobot::vehicle_snap_strategy::PlateListPtr plate_list =
        std::make_shared<hobot::vehicle_snap_strategy::PlateList>();

    std::size_t vehicle_data_size = in_vehicle_rects->datas_.size();
    for (std::size_t vehicle_idx = 0; vehicle_idx < vehicle_data_size;
         ++vehicle_idx) {
      hobot::vehicle_snap_strategy::Vehicle veh_tmp;
      auto bbox = std::dynamic_pointer_cast<XStreamBBox>(
          in_vehicle_rects->datas_[vehicle_idx]);
      if (!bbox) {
        LOGW << "bbox is null";
        continue;
      }
      hobot::vehicle_snap_strategy::BBoxPtr box =
          std::make_shared<hobot::vision::BBox>(
            bbox->value.x1, bbox->value.y1, bbox->value.x2, bbox->value.y2);
      if (in_vehicle_lmk && !in_vehicle_lmk->datas_.empty() &&
          vehicle_idx < in_vehicle_lmk->datas_.size()) {
        auto vehicle_lmk = std::dynamic_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(
            in_vehicle_lmk->datas_[vehicle_idx]);
        veh_tmp.vehicle_landmarks_ptr = vehicle_lmk;
      } else {
        LOGW << "no matching vehicle lmks";
      }

      veh_tmp.bbox = box;
      veh_tmp.bbox->id = bbox->value.id;
      veh_tmp.vehicleBox = bbox;
      vehicle_list->emplace_back(std::move(veh_tmp));
    }

    for (std::size_t plate_idx = 0; plate_idx < in_plate_rects->datas_.size();
         ++plate_idx) {
      hobot::vehicle_snap_strategy::Plate plate_tmp;
      auto bbox =
          std::dynamic_pointer_cast<XStreamBBox>(
              in_plate_rects->datas_[plate_idx]);
      auto plate_type = std::dynamic_pointer_cast<
          xstream::XStreamData<hobot::vision::Attribute<int>>>(
          in_plate_type->datas_[plate_idx]);
      auto plate_color = std::dynamic_pointer_cast<
          xstream::XStreamData<hobot::vision::Attribute<int>>>(
          in_plate_color->datas_[plate_idx]);
      if (in_plate_lmk && !in_plate_lmk->datas_.empty() &&
          plate_idx < in_plate_lmk->datas_.size()) {
        auto plate_lmk = std::dynamic_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(
            in_plate_lmk->datas_[plate_idx]);
        plate_tmp.plate_landmarks_ptr = plate_lmk;
      } else {
        LOGW << "no match plate lmks";
      }

      if (!bbox || !plate_type) {
        LOGW << "bbox or plate_type is null";
        continue;
      }

      hobot::vehicle_snap_strategy::BBoxPtr box =
          std::make_shared<hobot::vision::BBox>(
        bbox->value.x1, bbox->value.y1, bbox->value.x2, bbox->value.y2);
      plate_tmp.bbox = box;
      plate_tmp.bbox->id = bbox->value.id;
      plate_tmp.plateBox = bbox;
      plate_tmp.is_double = plate_type->value.value;
      plate_tmp.plateType = plate_type;
      plate_tmp.plate_color = *plate_color;
      plate_list->emplace_back(std::move(plate_tmp));
    }
    LOGI << "matcher input vehicle list size : " << vehicle_list->size();
    LOGI << "matcher input plate list size : " << plate_list->size();
    matcher->Process(vehicle_list, plate_list);
    LOGI << "matcher output vehicle list size : " << vehicle_list->size();
    for (const auto &itr : *vehicle_list) {
      out_vehicle_rects->datas_.push_back(itr.vehicleBox);
      out_vehicle_lmk->datas_.push_back(itr.vehicle_landmarks_ptr);
      if (itr.plate != nullptr) {
        itr.plate->plateBox->value.id = itr.vehicleBox->value.id;
        out_plate_rects->datas_.push_back(itr.plate->plateBox);
        out_plate_lmk->datas_.push_back(itr.plate->plate_landmarks_ptr);
        out_plate_type->datas_.push_back(itr.plate->plateType);
        out_plate_color->datas_.push_back(
            std::make_shared<
                xstream::XStreamData<hobot::vision::Attribute<int>>>(
                itr.plate->plate_color));
      }
    }
  }
  return output;
}

void VehiclePlateMatchMethod::Finalize() { LOGI << "VehicleMatch::Finalize"; }

std::string VehiclePlateMatchMethod::GetVersion() const { return "v0.0.1"; }

void VehiclePlateMatchMethod::OnProfilerChanged(bool on) {}

}  // namespace xstream
