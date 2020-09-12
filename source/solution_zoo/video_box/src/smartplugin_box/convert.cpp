/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-02 04:05:52
 * @Version: v0.0.1
 * @Brief: implemenation of converter.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-08-02 06:26:01
 */
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision/util.h"
#include <turbojpeg.h>
#include <string>
#include <vector>
#include "hobotlog/hobotlog.hpp"
#include "smartplugin_box/convert.h"
#include "xproto_msgtype/vioplugin_data.h"
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
namespace horizon {
namespace iot {
namespace smartplugin_multiplebox {

using horizon::vision::util::ImageFrameConversion;

// 默认值由smartplugin.cpp中设置
int Convertor::image_compress_quality = 50;
xstream::InputDataPtr Convertor::ConvertInput(const VioMessage *input) {
  xstream::InputDataPtr inputdata(new xstream::InputData());
  HOBOT_CHECK(input != nullptr && input->num_ > 0 && input->is_valid_uri_);

  // \todo need a better way to identify mono or semi cameras
  for (uint32_t image_index = 0; image_index < 1; ++image_index) {
    xstream::BaseDataPtr xstream_input_data;
    if (input->num_ > image_index) {
#ifdef X2
      auto xstream_img = ImageFrameConversion(input->image_[image_index]);
      xstream_input_data = xstream::BaseDataPtr(xstream_img);
      LOGI << "Input Frame ID = " << xstream_img->value->frame_id
           << ", Timestamp = " << xstream_img->value->time_stamp;
#endif

#ifdef X3
      std::shared_ptr<hobot::vision::PymImageFrame> pym_img =
          input->image_[image_index];
      LOGI << "vio message, frame_id = " << pym_img->frame_id;
      for (uint32_t i = 0; i < DOWN_SCALE_MAX; ++i) {
        LOGD << "vio message, pym_level_" << i
             << ", width=" << pym_img->down_scale[i].width
             << ", height=" << pym_img->down_scale[i].height
             << ", stride=" << pym_img->down_scale[i].stride;
      }
      auto xstream_img =
          std::make_shared<xstream::XStreamData<ImageFramePtr>>();
      xstream_img->type_ = "ImageFrame";
      xstream_img->value =
          std::static_pointer_cast<hobot::vision::ImageFrame>(pym_img);
      LOGI << "Input Frame ID = " << xstream_img->value->frame_id
           << ", Timestamp = " << xstream_img->value->time_stamp;
      xstream_input_data = xstream::BaseDataPtr(xstream_img);
#endif
    } else {
      xstream_input_data = std::make_shared<xstream::BaseData>();
      xstream_input_data->state_ = xstream::DataState::INVALID;
    }

    if (image_index == uint32_t{0}) {
      if (input->num_ == 1) {
        xstream_input_data->name_ = "image";
      } else {
        xstream_input_data->name_ = "rgb_image";
      }
    } else {
      xstream_input_data->name_ = "nir_image";
    }
    LOGI << "input name:" << xstream_input_data->name_;
    inputdata->datas_.emplace_back(xstream_input_data);
  }

  return inputdata;
}

std::vector<vision::xproto::smartplugin::NoMotorVehicleInfo>
Convertor::ParseXstreamOutputToNoMotorInfo(
        const xstream::OutputDataPtr xstream_outputs) {
  std::vector<vision::xproto::smartplugin::NoMotorVehicleInfo>
          nomotor_infos;

  for (const auto &xstream_output : xstream_outputs->datas_) {
    if (xstream_output->name_ == "vehicle_un_bbox_list") {
      auto nomotor_boxes =
              std::dynamic_pointer_cast<xstream::BaseDataVector>
                      (xstream_output);
      int nomotor_boxe_size = nomotor_boxes->datas_.size();

      for (int box_idx = 0; box_idx < nomotor_boxe_size; ++box_idx) {
        auto nomotor_box =
                std::dynamic_pointer_cast<
                        xstream::XStreamData<hobot::vision::BBox>>(
                        nomotor_boxes->datas_[box_idx]);
        vision::xproto::smartplugin::NoMotorVehicleInfo no_motor_info;
        uint64_t track_id = nomotor_box->value.id;
        no_motor_info.track_id = track_id;
        no_motor_info.box.x1 = nomotor_box->value.x1;
        no_motor_info.box.y1 = nomotor_box->value.y1;
        no_motor_info.box.x2 = nomotor_box->value.x2;
        no_motor_info.box.y2 = nomotor_box->value.y2;
        no_motor_info.box.score = nomotor_box->value.score;

        nomotor_infos.push_back(no_motor_info);
      }
    }
  }

  return nomotor_infos;
}

std::vector<vision::xproto::smartplugin::PersonInfo>
Convertor::ParseXstreamOutputToPersonInfo(
        const xstream::OutputDataPtr xstream_outputs) {
  std::vector<vision::xproto::smartplugin::PersonInfo> person_infos;

  for (const auto &xstream_output : xstream_outputs->datas_) {
    if (xstream_output->name_ == "person_bbox_list") {
      auto person_boxes =
              std::dynamic_pointer_cast<xstream::BaseDataVector>
                      (xstream_output);

      int person_boxes_size = person_boxes->datas_.size();

      for (int box_idx = 0; box_idx < person_boxes_size; ++box_idx) {
        auto person_box =
                std::dynamic_pointer_cast<
                        xstream::XStreamData<hobot::vision::BBox>>(
                        person_boxes->datas_[box_idx]);

        vision::xproto::smartplugin::PersonInfo person_info;
        uint64_t track_id = person_box->value.id;
        person_info.track_id = track_id;
        person_info.box.x1 = person_box->value.x1;
        person_info.box.y1 = person_box->value.y1;
        person_info.box.x2 = person_box->value.x2;
        person_info.box.y2 = person_box->value.y2;
        person_info.box.score = person_box->value.score;

        person_infos.push_back(person_info);
      }
    }
  }

  return person_infos;
}

std::vector<vision::xproto::smartplugin::VehicleInfo>
Convertor::ParseXstreamOutputToVehicleInfo(
        const xstream::OutputDataPtr xstream_outputs) {
  std::vector<vision::xproto::smartplugin::VehicleInfo> vehicle_infos;
  for (const auto &xstream_output : xstream_outputs->datas_) {
    // 需要通过车辆框去找其他的信息
    if ((xstream_output->name_ == "vehicle_box_match") ||
        (xstream_output->name_ == "vehicle_bbox_list")) {
      auto xstream_vehicle_boxes =
              std::dynamic_pointer_cast<xstream::BaseDataVector>
                      (xstream_output);
      std::size_t vehicle_box_size = xstream_vehicle_boxes->datas_.size();
      for (std::size_t vehicle_box_idx = 0; vehicle_box_idx < vehicle_box_size;
           ++vehicle_box_idx) {
        auto xstream_vehicle_box =
                std::dynamic_pointer_cast<
                        xstream::XStreamData<hobot::vision::BBox>>(
                        xstream_vehicle_boxes->datas_[vehicle_box_idx]);
        int32_t track_id = xstream_vehicle_box->value.id;
        vision::xproto::smartplugin::VehicleInfo vehicle_info;
        vehicle_info.track_id = track_id;
        vehicle_info.box.x1 = xstream_vehicle_box->value.x1;
        vehicle_info.box.y1 = xstream_vehicle_box->value.y1;
        vehicle_info.box.x2 = xstream_vehicle_box->value.x2;
        vehicle_info.box.y2 = xstream_vehicle_box->value.y2;
        vehicle_info.box.score = xstream_vehicle_box->value.score;

        // 查找车的其他结构化信息
        for (const auto &xstream_sub_output : xstream_outputs->datas_) {
          if (xstream_sub_output->name_ == "plate_box_match" ||
                  xstream_sub_output->name_ == "plate_box_filter") {
            // 车牌框
            auto xstream_plate_boxes =
                    std::dynamic_pointer_cast<xstream::BaseDataVector>(
                            xstream_sub_output);
            for (std::size_t plate_box_idx = 0;
                 plate_box_idx < xstream_plate_boxes->datas_.size();
                 ++plate_box_idx) {  // 车牌的下标
              auto xstream_plate_box = std::dynamic_pointer_cast<
                      xstream::XStreamData<hobot::vision::BBox>>(
                      xstream_plate_boxes->datas_[plate_box_idx]);
              if (xstream_plate_box->value.id == track_id) {
                LOGI << "find plate box " << track_id;
                vision::xproto::smartplugin::Plate &plate_info =
                        vehicle_info.plate_info;
                plate_info.box.x1 = xstream_plate_box->value.x1;
                plate_info.box.y1 = xstream_plate_box->value.y1;
                plate_info.box.x2 = xstream_plate_box->value.x2;
                plate_info.box.y2 = xstream_plate_box->value.y2;
                plate_info.box.score = xstream_plate_box->value.score;

                // 查找车牌的其他结构化信息
                for (const auto &xstream_sub_sub_output :
                        xstream_outputs->datas_) {
                  if (xstream_sub_sub_output->name_ == "plate_type_match") {
                    // 是否是双行牌照
                    auto xstream_plate_types =
                            std::dynamic_pointer_cast<xstream::BaseDataVector>(
                                    xstream_sub_sub_output);
                    if (xstream_plate_types->datas_.size() !=
                        xstream_plate_boxes->datas_.size()) {
                      LOGE << "plate type and plate box size is not same"
                           << xstream_plate_types->datas_.size() << " "
                           << xstream_plate_boxes->datas_.size();
                      plate_info.is_double_plate = -1;
                    } else {
                      auto xstream_plate_type = std::dynamic_pointer_cast<
                              xstream::XStreamData
                                      <hobot::vision::Attribute<int>>>(
                              xstream_plate_types->datas_[plate_box_idx]);
                      plate_info.is_double_plate =
                              xstream_plate_type->state_ ==
                              xstream::DataState::INVALID
                              ? -1
                              : xstream_plate_type->value.value;
                    }
                  } else if (xstream_sub_sub_output->name_ ==
                             "plate_num_vote") {
                    // 车牌号
                    auto xstream_plate_nums =
                            std::dynamic_pointer_cast<xstream::BaseDataVector>(
                                    xstream_sub_sub_output);
                    if (xstream_plate_nums->datas_.size() !=
                        xstream_plate_boxes->datas_.size()) {
                      LOGE << "plate num and plate box size is not same"
                           << xstream_plate_nums->datas_.size() << " "
                           << xstream_plate_boxes->datas_.size();
                    } else {
                      auto xstream_plate_num = std::dynamic_pointer_cast<
                              xstream::XStreamData<std::vector<int>>>(
                              xstream_plate_nums->datas_[plate_box_idx]);
                      if (!xstream_plate_num->value.empty() &&
                          xstream_plate_num->state_ !=
                          xstream::DataState::INVALID) {
                        plate_info.plate_num =
                                ConvertPlateNumber(xstream_plate_num->value);
                        plate_info.plate_idx = xstream_plate_num->value.front();
                      } else {
                        LOGI << "plate num is empty " << track_id;
                      }
                    }
                  } else if (xstream_sub_sub_output->name_ ==
                             "plate_color_vote") {
                    // 车牌颜色
                    auto xstream_plate_colors =
                            std::dynamic_pointer_cast<xstream::BaseDataVector>(
                                    xstream_sub_sub_output);
                    if (xstream_plate_colors->datas_.size() !=
                        xstream_plate_boxes->datas_.size()) {
                      LOGE << "plate color and plate box size is not same "
                           << xstream_plate_colors->datas_.size() << " "
                           << xstream_plate_boxes->datas_.size();
                    } else {
                      auto xstream_plate_color = std::dynamic_pointer_cast<
                              xstream::XStreamData
                                      <hobot::vision::Attribute<int>>>(
                              xstream_plate_colors->datas_[plate_box_idx]);
                      if (xstream_plate_color != nullptr) {
                        plate_info.color = xstream_plate_color->value.value;
                      }
                    }
                  } else if (xstream_sub_sub_output->name_ ==
                             "plate_lmk_match") {
                    // 车牌关键点
                    auto xstream_plate_lmks =
                            std::dynamic_pointer_cast<xstream::BaseDataVector>(
                                    xstream_sub_sub_output);
                    auto lmk_size = xstream_plate_lmks->datas_.size();
                    if (plate_box_idx < lmk_size) {
                      auto xstream_vehicle_lmk = std::dynamic_pointer_cast<
                              xstream::XStreamData<hobot::vision::Landmarks>>(
                              xstream_plate_lmks->datas_[plate_box_idx]);
                      plate_info.points.clear();
                      if (xstream_vehicle_lmk == nullptr) {
                        LOGI << "lmk num is empty " << track_id;
                      }
                      for (auto point : xstream_vehicle_lmk->value.values) {
                        vision::xproto::smartplugin::Point point_smart;
                        point_smart.x = point.x;
                        point_smart.y = point.y;
                        point_smart.score = point.score;
                        plate_info.points.emplace_back(point_smart);
                      }
                    } else {
                      plate_info.points = {};
                    }
                  }
                }
                break;
              }
            }
          } else if ((xstream_sub_output->name_ == "vehicle_lmk_match") ||
                     (xstream_sub_output->name_ == "vehicle_lmk")) {
            // 车辆关键点
            auto xstream_vehicle_lmks =
                    std::dynamic_pointer_cast<xstream::BaseDataVector>(
                            xstream_sub_output);
            auto lmk_size = xstream_vehicle_lmks->datas_.size();
            if (vehicle_box_idx < lmk_size) {
              auto xstream_vehicle_lmk = std::dynamic_pointer_cast<
                      xstream::XStreamData<hobot::vision::Landmarks>>(
                      xstream_vehicle_lmks->datas_[vehicle_box_idx]);
              vehicle_info.points.clear();
              for (auto point : xstream_vehicle_lmk->value.values) {
                vision::xproto::smartplugin::Point point_smart;
                point_smart.x = point.x;
                point_smart.y = point.y;
                point_smart.score = point.score;
                vehicle_info.points.emplace_back(point_smart);
              }
            } else {
              vehicle_info.points = {};
            }
          } else if (xstream_sub_output->name_ == "vehicle_type_vote") {
            // 车辆类型
            auto xstream_vehicle_types =
                    std::dynamic_pointer_cast<xstream::BaseDataVector>(
                            xstream_sub_output);
            if (xstream_vehicle_types->datas_.size() != vehicle_box_size) {
              LOGE << "vehicle type and vehicle box size is not same "
                   << xstream_vehicle_types->datas_.size() << " "
                   << vehicle_box_size;
              vehicle_info.type = -1;
            } else {
              auto xstream_vehicle_type =
                      std::dynamic_pointer_cast<xstream::XStreamData<int>>(
                              xstream_vehicle_types->datas_[vehicle_box_idx]);
              vehicle_info.type = xstream_vehicle_type->value;
            }
          } else if (xstream_sub_output->name_ == "vehicle_color_vote") {
            // 车辆颜色
            auto xstream_vehicle_colors =
                    std::dynamic_pointer_cast<xstream::BaseDataVector>(
                            xstream_sub_output);
            if (xstream_vehicle_colors->datas_.size() !=
                    vehicle_box_size) {
              LOGE << "vehicle color and vehicle box size is not same "
                   << xstream_vehicle_colors->datas_.size() << " "
                   << vehicle_box_size;
              vehicle_info.color = -1;
            } else {
              auto xstream_vehicle_color =
                      std::dynamic_pointer_cast<xstream::XStreamData<int>>(
                          xstream_vehicle_colors->datas_[vehicle_box_idx]);
              vehicle_info.color = xstream_vehicle_color->value;
            }
          }
        }
        vehicle_infos.push_back(vehicle_info);
      }
    }
  }
  return vehicle_infos;
}  // namespace iot

std::vector<uint64_t> Convertor::ParseXstreamOutputToLostTrack(
        const xstream::OutputDataPtr xstream_outputs) {
  std::vector<uint64_t> lost_track_ids;
  for (const auto &xstream_output : xstream_outputs->datas_) {
    if (xstream_output->name_ == "vehicle_disappeared_track_id_list") {
      // lost的track
      auto xstream_lost_tracks =
              std::dynamic_pointer_cast<xstream::BaseDataVector>
                      (xstream_output);
      for (auto xstream_lost_track : xstream_lost_tracks->datas_) {
        auto track_id =
                std::dynamic_pointer_cast<xstream::XStreamData<uint32_t>>(
                        xstream_lost_track);
        lost_track_ids.push_back(track_id->value);
      }
    }
  }
  return lost_track_ids;
}

std::string Convertor::ConvertPlateNumber(
        const std::vector<int> &plate_num_array_model) {
  static std::vector<std::string> plate_num_map{
          " ",  "A",  "B",  "C",  "D",  "E",  "F",  "G",  "H",  "J",  "K",
          "L",  "M",  "N",  "P",  "Q",  "R",  "S",  "T",  "U",  "V",  "W",
          "X",  "Y",  "Z",  "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",
          "8",  "9",  "京", "津", "沪", "渝", "黑", "吉", "辽", "冀", "晋",
          "鲁", "豫", "陕", "甘", "青", "苏", "浙", "皖", "鄂", "湘", "闽",
          "赣", "川", "贵", "云", "粤", "琼", "蒙", "宁", "新", "桂", "藏",
          "学", "警", "领", "军", "使", "港", "挂", "澳"};
  std::string plate_num;
  for (auto plate_num_model : plate_num_array_model) {
    if (static_cast<std::size_t>(plate_num_model) >= plate_num_map.size()) {
      return "";
    }
    plate_num.append(plate_num_map[plate_num_model]);
  }
  return plate_num;
}

//    std::string Convertor::JsonToUnStyledString(const Json::Value &jv) {
//      Json::StreamWriterBuilder writer_builder;
//      writer_builder["indentation"] = "";
//      return Json::writeString(writer_builder, jv);
//    }

}  // namespace smartplugin_multiplebox
}  // namespace iot
}  // namespace horizon
