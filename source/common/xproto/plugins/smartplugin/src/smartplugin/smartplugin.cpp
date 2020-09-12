/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-01 20:38:52
 * @Version: v0.0.1
 * @Brief: smartplugin impl based on xstream.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-29 05:04:11
 */

#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto/utils/profile.h"

#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/profiler.h"
#include "horizon/vision/util.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"
#include "smartplugin/convert.h"
#include "smartplugin/runtime_monitor.h"
#include "smartplugin/smart_config.h"
#include "smartplugin/smartplugin.h"
#include "xproto_msgtype/protobuf/x2.pb.h"
#include "smartplugin/convertpb.h"
#include "xproto_msgtype/protobuf/x3.pb.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "websocketplugin/attribute_convert/attribute_convert.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::websocketplugin::AttributeConvert;
using hobot::vision::BBox;
using hobot::vision::Segmentation;

using horizon::vision::xproto::basic_msgtype::VioMessage;
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;
using xstream::XStreamData;

using horizon::iot::Convertor;

enum CameraFeature {
  NoneFeature = 0,                // 无
  TrafficConditionFeature = 0b1,  // 拥堵事故
  SnapFeature = 0b10,             // 抓拍
  AnomalyFeature = 0b100,         // 违法行为
  CountFeature = 0b1000,          // 车流计数
  GisFeature = 0b10000,           // 定位
};


XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_SMART_MESSAGE)

VehicleSmartMessage::VehicleSmartMessage() { camera_type = 0; }

void VehicleSmartMessage::Serialize_Print(Json::Value &root) {
  LOGD << "Frame id: " << frame_id << " vehicle_infos: "
       << vehicle_infos.size();

  Json::Value targets;
  for (const auto &vehicle_info : vehicle_infos) {
    Json::Value target;

    target["id"].append(vehicle_info.track_id);
    target["vehicle_bbox"].append(static_cast<int>(vehicle_info.box.x1));
    target["vehicle_bbox"].append(static_cast<int>(vehicle_info.box.y1));
    target["vehicle_bbox"].append(static_cast<int>(vehicle_info.box.x2));
    target["vehicle_bbox"].append(static_cast<int>(vehicle_info.box.y2));
    target["vehicle_bbox"].append(static_cast<int>(
      vehicle_info.box.score * 100));

    const auto &plate = vehicle_info.plate_info;
    if (plate.box.x1 != 0 || plate.box.y1 != 0 || plate.box.x2 != 0 ||
      plate.box.y2 != 0) {
      target["vehicle_plate_bbox"].append(static_cast<int>(plate.box.x1));
      target["vehicle_plate_bbox"].append(static_cast<int>(plate.box.y1));
      target["vehicle_plate_bbox"].append(static_cast<int>(plate.box.x2));
      target["vehicle_plate_bbox"].append(static_cast<int>(plate.box.y2));
      target["vehicle_plate_bbox"].append(static_cast<int>(
        plate.box.score * 100));
      if (plate.plate_num != "") {
        target["vehicle_plate_num"] = plate.plate_num;
      }
    }
    targets["vehicle"].append(target);
  }
  root[std::to_string(frame_id)] = targets;
}

std::string VehicleSmartMessage::Serialize(int ori_w, int ori_h, int dst_w,
                                           int dst_h) {
  HOBOT_CHECK(ori_w > 0 && ori_h > 0 && dst_w > 0 && dst_h > 0)
      << "Serialize param error";
  float x_ratio = 1.0 * dst_w / ori_w;
  float y_ratio = 1.0 * dst_h / ori_h;
  x3::FrameMessage frame_msg_x3;
  frame_msg_x3.set_timestamp_(time_stamp);
  frame_msg_x3.set_sequence_id_(frame_id);
  auto smart_msg_x3 = frame_msg_x3.mutable_smart_msg_();
  smart_msg_x3->set_timestamp_(time_stamp);
  smart_msg_x3->set_error_code_(0);

  LOGD << "VehicleSmartMessage::Serialize with ratio" << std::endl;
  LOGD << "vehicle_infos: " << vehicle_infos.size()
       << " person_infos: " << person_infos.size()
       << " nomotor_infos: " << nomotor_infos.size()
       << " anomaly_infos: " << anomaly_infos.size()
       << " capture_infos: " << capture_infos.size() << std::endl;

  /* 逐帧车辆结构化信息 */
  for (const auto &vehicle_info : vehicle_infos) {
    auto vehicle_pb_x3 = smart_msg_x3->add_targets_();
    convertVehicleInfo(vehicle_pb_x3, vehicle_info, x_ratio, y_ratio);
  }

  /* 逐帧人的结构化信息 */
  for (const auto &person_info : person_infos) {
    auto person_pb = smart_msg_x3->add_targets_();
    convertPerson(person_pb, person_info, x_ratio, y_ratio);
  }

  /* 逐帧非机动车的结构化信息 */
  for (const auto &nomotor_info : nomotor_infos) {
    auto nomotor_pb = smart_msg_x3->add_targets_();
    convertNonmotor(nomotor_pb, nomotor_info, x_ratio, y_ratio);
  }

  return frame_msg_x3.SerializeAsString();
}

std::string VehicleSmartMessage::Serialize() {
  x3::FrameMessage frame_msg_x3;
  frame_msg_x3.set_timestamp_(time_stamp);
  frame_msg_x3.set_sequence_id_(frame_id);
  auto smart_msg_x3 = frame_msg_x3.mutable_smart_msg_();
  smart_msg_x3->set_timestamp_(time_stamp);
  smart_msg_x3->set_error_code_(0);
  LOGD << "vehicle_infos: " << vehicle_infos.size()
       << " person_infos: " << person_infos.size()
       << " nomotor_infos: " << nomotor_infos.size()
       << " anomaly_infos: " << anomaly_infos.size()
       << " capture_infos: " << capture_infos.size() << std::endl;
  /* 逐帧车辆结构化信息 */
  for (const auto &vehicle_info : vehicle_infos) {
    auto vehicle_pb_x3 = smart_msg_x3->add_targets_();
    convertVehicleInfo(vehicle_pb_x3, vehicle_info, 1.0, 1.0);
  }

  /* 逐帧人的结构化信息 */
  for (const auto &person_info : person_infos) {
    auto person_pb = smart_msg_x3->add_targets_();
    convertPerson(person_pb, person_info, 1.0, 1.0);
  }

  /* 逐帧非机动车的结构化信息 */
  for (const auto &nomotor_info : nomotor_infos) {
    auto nomotor_pb = smart_msg_x3->add_targets_();
    convertNonmotor(nomotor_pb, nomotor_info, 1.0, 1.0);
  }

  // x3::FrameMessage frame_msg_x3;
  // convertVehicle(frame_msg_x3, frame_msg);
  // std::string x3 = frame_msg_x3.SerializeAsString();

  // std::ofstream file_proto;
  // file_proto.open("./vehicle.bin", std::ios::out| std:: ios_base::ate);
  // file_proto << x3;
  // file_proto.flush();
  // file_proto.close();

  return frame_msg_x3.SerializeAsString();
}

std::map<int, int> CustomSmartMessage::fall_state_;
std::mutex CustomSmartMessage::fall_mutex_;
std::map<int, int> CustomSmartMessage::gesture_state_;
std::map<int, float> CustomSmartMessage::gesture_start_time_;
bool gesture_as_event = false;
void CustomSmartMessage::Serialize_Print(Json::Value &root) {
  LOGD << "Frame id: " << frame_id;
  auto name_prefix = [](const std::string name) -> std::string {
    auto pos = name.find('_');
    if (pos == std::string::npos)
      return "";

    return name.substr(0, pos);
  };

  auto name_postfix = [](const std::string name)  -> std::string {
    auto pos = name.rfind('_');
    if (pos == std::string::npos)
      return "";

    return name.substr(pos + 1);
  };

  Json::Value targets;
  xstream::BaseDataVector * body_data = nullptr;
  std::set<int> id_set;
  for (const auto &output : smart_result->datas_) {
    auto prefix = name_prefix(output->name_);
    auto postfix = name_postfix(output->name_);
    if (prefix == "body") {
      body_data = dynamic_cast<xstream::BaseDataVector *>(output.get());
    }
    if (output->name_ == "face_bbox_list" || output->name_ == "head_box" ||
        output->name_ == "body_box" || postfix == "box") {
      auto face_boxes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "box size: " << face_boxes->datas_.size();
      for (size_t i = 0; i < face_boxes->datas_.size(); ++i) {
        Json::Value target;
        auto face_box =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                face_boxes->datas_[i]);

        if (prefix == "face" || prefix == "head" || prefix == "body") {
          target[prefix].append(static_cast<int>(face_box->value.x1));
          target[prefix].append(static_cast<int>(face_box->value.y1));
          target[prefix].append(static_cast<int>(face_box->value.x2));
          target[prefix].append(static_cast<int>(face_box->value.y2));
        } else {
          LOGE << "unsupport box name: " << output->name_;
        }
        targets[std::to_string(face_box->value.id)].append(target);
        id_set.insert(face_box->value.id);
      }
    }
    if (output->name_ == "kps") {
      auto lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "kps size: " << lmks->datas_.size();
      for (size_t i = 0; i < lmks->datas_.size(); ++i) {
        Json::Value target;
        auto lmk = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(lmks->datas_[i]);
        auto body_box =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                body_data->datas_[i]);
        for (size_t i = 0; i < lmk->value.values.size(); ++i) {
          const auto &point = lmk->value.values[i];
          if (point.score > 2) {
            target["kps"].append(static_cast<int>(point.x));
            target["kps"].append(static_cast<int>(point.y));
          } else {
            target["kps"].append(0);
            target["kps"].append(0);
          }
        }
        targets[std::to_string(body_box->value.id)].append(target);
      }
    }
  }

  /// transform targets dict to person list
  Json::Value person;
  for (auto it = id_set.begin(); it != id_set.end(); ++it) {
    auto &item = targets[std::to_string(*it)];
    Json::Value trk_id;
    trk_id["id"] = *it;
    item.append(trk_id);
    person["person"].append(item);
  }
  root[std::to_string(frame_id)] = person;
}

std::string CustomSmartMessage::Serialize() {
  // serialize smart message using defined smart protobuf.
  // smart result coordinate to origin image size(pymrid 0 level)
  std::string proto_str;
  x3::FrameMessage proto_frame_message;
  proto_frame_message.set_timestamp_(time_stamp);
  auto smart_msg = proto_frame_message.mutable_smart_msg_();
  smart_msg->set_timestamp_(time_stamp);
  smart_msg->set_error_code_(0);
  // user-defined output parsing declaration.
  xstream::BaseDataVector *face_boxes = nullptr;
  xstream::BaseDataVector *lmks = nullptr;
  xstream::BaseDataVector *mask = nullptr;
  auto name_prefix = [](const std::string name) -> std::string {
    auto pos = name.find('_');
    if (pos == std::string::npos)
      return "";

    return name.substr(0, pos);
  };

  auto name_postfix = [](const std::string name) -> std::string {
    auto pos = name.rfind('_');
    if (pos == std::string::npos)
      return "";

    return name.substr(pos + 1);
  };

  std::vector<std::shared_ptr<
      xstream::XStreamData<hobot::vision::BBox>>> face_box_list;
  std::vector<std::shared_ptr<
      xstream::XStreamData<hobot::vision::BBox>>> head_box_list;
  std::vector<std::shared_ptr<
      xstream::XStreamData<hobot::vision::BBox>>> body_box_list;
  std::map<int, x3::Target*> smart_target;
  for (const auto &output : smart_result->datas_) {
    LOGD << "output name: " << output->name_;
    auto prefix = name_prefix(output->name_);
    auto postfix = name_postfix(output->name_);
    if (output->name_ == "face_bbox_list" || output->name_ == "head_box" ||
        output->name_ == "body_box" || postfix == "box") {
      face_boxes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "box type: " << output->name_
           << ", box size: " << face_boxes->datas_.size();
      for (size_t i = 0; i < face_boxes->datas_.size(); ++i) {
        auto face_box =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                face_boxes->datas_[i]);
        LOGD << output->name_ << " id: " << face_box->value.id
             << " x1: " << face_box->value.x1 << " y1: " << face_box->value.y1
             << " x2: " << face_box->value.x2 << " y2: " << face_box->value.y2;
        if (prefix == "face") {
          face_box_list.push_back(face_box);
        } else if (prefix == "head") {
          head_box_list.push_back(face_box);
        } else if (prefix == "body") {
          body_box_list.push_back(face_box);
        } else {
          LOGE << "unsupport box name: " << output->name_;
        }

        if (face_box->value.id != -1) {
          if (smart_target.find(face_box->value.id) ==
                smart_target.end()) {  // 新track_id
            auto target = smart_msg->add_targets_();
            target->set_track_id_(face_box->value.id);
            target->set_type_("person");
            smart_target[face_box->value.id] = target;
          }
          auto proto_box = smart_target[face_box->value.id]->add_boxes_();
          proto_box->set_type_(prefix);  // "face", "head", "body"
          auto point1 = proto_box->mutable_top_left_();
          point1->set_x_(face_box->value.x1);
          point1->set_y_(face_box->value.y1);
          point1->set_score_(face_box->value.score);
          auto point2 = proto_box->mutable_bottom_right_();
          point2->set_x_(face_box->value.x2);
          point2->set_y_(face_box->value.y2);
          point2->set_score_(face_box->value.score);

          // body_box在前
          if (prefix == "body" &&
              smart_target[face_box->value.id]->boxes__size() > 1) {
            auto body_box_index =
                smart_target[face_box->value.id]->boxes__size() - 1;
            auto body_box = smart_target[face_box->value.id]->
                mutable_boxes_(body_box_index);
            auto first_box =
                smart_target[face_box->value.id]->mutable_boxes_(0);
            first_box->Swap(body_box);
          }
        }
      }
    }
    if (output->name_ == "kps") {
      lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "kps size: " << lmks->datas_.size();
      if (lmks->datas_.size() != body_box_list.size()) {
        LOGE << "kps size: " << lmks->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < lmks->datas_.size(); ++i) {
        auto lmk = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(lmks->datas_[i]);
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          auto target = smart_target[body_box_list[i]->value.id];
          auto proto_points = target->add_points_();
          proto_points->set_type_("body_landmarks");
          for (size_t i = 0; i < lmk->value.values.size(); ++i) {
            auto point = proto_points->add_points_();
            point->set_x_(lmk->value.values[i].x);
            point->set_y_(lmk->value.values[i].y);
            point->set_score_(lmk->value.values[i].score);
            LOGD << "x: " << std::round(lmk->value.values[i].x)
                 << " y: " << std::round(lmk->value.values[i].y)
                 << " score: " << lmk->value.values[i].score << "\n";
          }
        }
      }
    }
    if (output->name_ == "mask") {
      mask = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "mask size: " << mask->datas_.size();
      if (mask->datas_.size() != body_box_list.size()) {
        LOGE << "mask size: " << mask->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < mask->datas_.size(); ++i) {
        auto one_mask = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Segmentation>>(mask->datas_[i]);
        if (one_mask->state_ != xstream::DataState::VALID) {
          continue;
        }
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
            smart_target.end()) {
          LOGE << "Not found the track_id target";
          continue;
        } else {
          auto target = smart_target[body_box_list[i]->value.id];
          auto float_matrix = target->add_float_matrixs_();
          float_matrix->set_type_("mask");
          int mask_value_idx = 0;
          for (int mask_height = 0; mask_height < one_mask->value.height;
               ++mask_height) {
            auto float_array = float_matrix->add_arrays_();
            for (int mask_width = 0; mask_width < one_mask->value.width;
                 ++mask_width) {
              float_array->add_value_(one_mask->value.values[mask_value_idx++]);
            }
          }
        }
      }
    }
    if (output->name_ == "age") {
      auto ages = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "age size: " << ages->datas_.size();
      if (ages->datas_.size() != face_box_list.size()) {
        LOGE << "ages size: " << ages->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < ages->datas_.size(); ++i) {
        auto age =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::Age>>(
                ages->datas_[i]);
        // 查找对应的track_id
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(face_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (age->state_ != xstream::DataState::VALID) {
            LOGE << "-1 -1 -1";
            continue;
          }
          auto target = smart_target[face_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("age");
          attrs->set_value_((age->value.min + age->value.max) / 2);
          attrs->set_score_(age->value.score);
          LOGD << " " << age->value.min << " " << age->value.max;
        }
      }
    }
    if (output->name_ == "gender") {
      auto genders = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "gender size: " << genders->datas_.size();
      if (genders->datas_.size() != face_box_list.size()) {
        LOGE << "genders size: " << genders->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < genders->datas_.size(); ++i) {
        auto gender = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Gender>>(genders->datas_[i]);
        // 查找对应的track_id
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(face_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (genders->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[face_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("gender");
          attrs->set_value_(gender->value.value);
          attrs->set_score_(gender->value.score);
          LOGD << " " << gender->value.value;
        }
      }
    }
    if (output->name_ == "action") {
      auto actions = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "action size: " << actions->datas_.size();
      for (size_t i = 0; i < actions->datas_.size(); ++i) {
        auto action =
            std::static_pointer_cast<xstream::XStreamData<std::string>>(
                actions->datas_[i]);
        if (action->state_ != xstream::DataState::VALID) {
          LOGE << "not valid : -1";
          continue;
        }
        // i 仅可能为 0，默认放在第一个target中; 防止target被过滤
        if (static_cast<int>(i) >= smart_msg->targets__size()) {
          break;
        }
        auto target = smart_msg->mutable_targets_(i);
        auto attrs = target->add_attributes_();
        attrs->set_type_("action");
        float action_index = 0;
        if (action->value == "other") {
          action_index = 1;
        } else if (action->value == "stand") {
          action_index = 2;
        } else if (action->value == "run") {
          action_index = 3;
        } else if (action->value == "attack") {
          action_index = 4;
        }
        LOGD << "smartplugin, value = " << action->value
             << ", action_index = " << action_index << std::endl;
        attrs->set_value_(action_index);
        attrs->set_score_(0.8);
      }
    }
    if (output->name_ == "fall_vote") {
      auto fall_votes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "fall_votes size: " << fall_votes->datas_.size();
      if (fall_votes->datas_.size() != body_box_list.size()) {
        LOGE << "fall_vote size: " << fall_votes->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < fall_votes->datas_.size(); ++i) {
        auto fall_vote = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
            fall_votes->datas_[i]);
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (fall_vote->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[body_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("fall");
          attrs->set_value_(fall_vote->value.value);
          attrs->set_score_(fall_vote->value.score);
          LOGD << " " << fall_vote->value.value;
        }
      }
    }
    if (output->name_ == "raise_hand" ||
        output->name_ == "stand" ||
        output->name_ == "squat") {
      auto attributes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << attributes->datas_.size();
      if (attributes->datas_.size() != body_box_list.size()) {
        LOGE << "behavior attributes size: " << attributes->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < attributes->datas_.size(); ++i) {
        auto attribute = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
                attributes->datas_[i]);
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (attribute->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[body_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_(output->name_);
          attrs->set_value_(attribute->value.value);
          attrs->set_score_(attribute->value.score);
          LOGD << "value: " << attribute->value.value;
        }
      }
    }
    if (output->name_ == "face_mask") {
      auto attributes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << attributes->datas_.size();
      if (attributes->datas_.size() != face_box_list.size()) {
        LOGE << "face mask size: " << attributes->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < attributes->datas_.size(); ++i) {
        auto attribute = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
                attributes->datas_[i]);
        // 查找对应的track_id
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(face_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (attribute->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[face_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_(output->name_);
          attrs->set_value_(attribute->value.value);
          attrs->set_score_(attribute->value.score);
          LOGD << "value: " << attribute->value.value;
        }
      }
    }
  }
  proto_frame_message.SerializeToString(&proto_str);
  LOGD << "smart result serial success";
  return proto_str;
}

std::string CustomSmartMessage::Serialize(int ori_w, int ori_h, int dst_w,
                                          int dst_h) {
  HOBOT_CHECK(ori_w > 0 && ori_h > 0 && dst_w > 0 && dst_h > 0)
      << "Serialize param error";
  float x_ratio = 1.0 * dst_w / ori_w;
  float y_ratio = 1.0 * dst_h / ori_h;
  // serialize smart message using defined smart protobuf.
  std::string proto_str;
  x3::FrameMessage proto_frame_message;
  proto_frame_message.set_timestamp_(time_stamp);
  auto smart_msg = proto_frame_message.mutable_smart_msg_();
  smart_msg->set_timestamp_(time_stamp);
  smart_msg->set_error_code_(0);
  // add fps to output
  auto static_msg = proto_frame_message.mutable_statistics_msg_();
  auto fps_attrs = static_msg->add_attributes_();
  fps_attrs->set_type_("fps");
  fps_attrs->set_value_(frame_fps);
  fps_attrs->set_value_string_(std::to_string(frame_fps));
  // user-defined output parsing declaration.
  xstream::BaseDataVector *face_boxes = nullptr;
  xstream::BaseDataVector *lmks = nullptr;
  xstream::BaseDataVector *mask = nullptr;
  auto name_prefix = [](const std::string name) -> std::string {
    auto pos = name.find('_');
    if (pos == std::string::npos)
      return "";

    return name.substr(0, pos);
  };

  auto name_postfix = [](const std::string name) -> std::string {
    auto pos = name.rfind('_');
    if (pos == std::string::npos)
      return "";

    return name.substr(pos + 1);
  };

  std::vector<std::shared_ptr<
      xstream::XStreamData<hobot::vision::BBox>>> face_box_list;
  std::vector<std::shared_ptr<
      xstream::XStreamData<hobot::vision::BBox>>> head_box_list;
  std::vector<std::shared_ptr<
      xstream::XStreamData<hobot::vision::BBox>>> body_box_list;
  std::vector<std::shared_ptr<xstream::XStreamData<hobot::vision::BBox>>>
      hand_box_list;
  std::map<int, x3::Target*> smart_target;
  for (const auto &output : smart_result->datas_) {
    LOGD << "output name: " << output->name_;
    auto prefix = name_prefix(output->name_);
    auto postfix = name_postfix(output->name_);
    if (output->name_ == "face_bbox_list" || output->name_ == "head_box" ||
        output->name_ == "body_box" || postfix == "box") {
      face_boxes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "box type: " << output->name_
           << ", box size: " << face_boxes->datas_.size();
      bool track_id_valid = true;
      if (face_boxes->datas_.size() > 1) {
        track_id_valid = false;
        for (size_t i = 0; i < face_boxes->datas_.size(); ++i) {
          auto face_box = std::static_pointer_cast<
              xstream::XStreamData<hobot::vision::BBox>>(face_boxes->datas_[i]);
          if (face_box->value.id != 0) {
            track_id_valid = true;
            break;
          }
        }
      }
      for (size_t i = 0; i < face_boxes->datas_.size(); ++i) {
        auto face_box =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                face_boxes->datas_[i]);
        if (!track_id_valid) {
          face_box->value.id = i + 1;
        }
        LOGD << output->name_ << " id: " << face_box->value.id
             << " x1: " << face_box->value.x1 << " y1: " << face_box->value.y1
             << " x2: " << face_box->value.x2 << " y2: " << face_box->value.y2;
        if (prefix == "face") {
          face_box_list.push_back(face_box);
        } else if (prefix == "head") {
          head_box_list.push_back(face_box);
        } else if (prefix == "body") {
          body_box_list.push_back(face_box);
        } else if (prefix == "hand") {
          hand_box_list.push_back(face_box);
        } else {
          LOGE << "unsupport box name: " << output->name_;
        }
        if (face_box->value.id != -1) {
          if (smart_target.find(face_box->value.id) ==
                smart_target.end()) {  // 新track_id
            auto target = smart_msg->add_targets_();
            target->set_track_id_(face_box->value.id);
            if (prefix == "hand") {
              target->set_type_("hand");
            } else {
              target->set_type_("person");
            }
            smart_target[face_box->value.id] = target;
          }
          auto proto_box = smart_target[face_box->value.id]->add_boxes_();
          proto_box->set_type_(prefix);  // "face", "head", "body"
          auto point1 = proto_box->mutable_top_left_();
          point1->set_x_(face_box->value.x1 * x_ratio);
          point1->set_y_(face_box->value.y1 * y_ratio);
          point1->set_score_(face_box->value.score);
          auto point2 = proto_box->mutable_bottom_right_();
          point2->set_x_(face_box->value.x2 * x_ratio);
          point2->set_y_(face_box->value.y2 * y_ratio);
          point2->set_score_(face_box->value.score);

          // body_box在前
          if (prefix == "body" &&
              smart_target[face_box->value.id]->boxes__size() > 1) {
            auto body_box_index =
                smart_target[face_box->value.id]->boxes__size() - 1;
            auto body_box = smart_target[face_box->value.id]->
                mutable_boxes_(body_box_index);
            auto first_box =
                smart_target[face_box->value.id]->mutable_boxes_(0);
            first_box->Swap(body_box);
          }
        }
      }
    }
    if (output->name_ == "bound_rect_filter") {
      auto bound_rect = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "bound_rect size: " << bound_rect->datas_.size();
      if (bound_rect->datas_.size() == 2) {
        auto bound_box =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                bound_rect->datas_[0]);
        auto show_enable = std::static_pointer_cast<xstream::XStreamData<bool>>(
            bound_rect->datas_[1]);
        LOGD << output->name_ << " id: " << bound_box->value.id
             << " x1: " << bound_box->value.x1 << " y1: " << bound_box->value.y1
             << " x2: " << bound_box->value.x2 << " y2: " << bound_box->value.y2
             << ", show_enable: " << show_enable->value;
        if (show_enable->value) {
          auto target = smart_msg->add_targets_();
          target->set_track_id_(0);
          target->set_type_("bound");
          auto proto_box = target->add_boxes_();
          proto_box->set_type_("bound_rect");
          auto point1 = proto_box->mutable_top_left_();
          point1->set_x_(bound_box->value.x1 * x_ratio);
          point1->set_y_(bound_box->value.y1 * y_ratio);
          auto point2 = proto_box->mutable_bottom_right_();
          point2->set_x_(bound_box->value.x2 * x_ratio);
          point2->set_y_(bound_box->value.y2 * y_ratio);
        }
      }
    }
    if (output->name_ == "kps") {
      lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "kps size: " << lmks->datas_.size();
      if (lmks->datas_.size() != body_box_list.size()) {
        LOGE << "kps size: " << lmks->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < lmks->datas_.size(); ++i) {
        auto lmk = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(lmks->datas_[i]);
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          auto target = smart_target[body_box_list[i]->value.id];
          auto proto_points = target->add_points_();
          proto_points->set_type_("body_landmarks");
          for (size_t i = 0; i < lmk->value.values.size(); ++i) {
            auto point = proto_points->add_points_();
            point->set_x_(lmk->value.values[i].x * x_ratio);
            point->set_y_(lmk->value.values[i].y * y_ratio);
            point->set_score_(lmk->value.values[i].score);
            LOGD << "x: " << std::round(lmk->value.values[i].x)
                 << " y: " << std::round(lmk->value.values[i].y)
                 << " score: " << lmk->value.values[i].score << "\n";
          }
        }
      }
    }
    if (output->name_ == "mask") {
      mask = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "mask size: " << mask->datas_.size();
      if (mask->datas_.size() != body_box_list.size()) {
        LOGE << "mask size: " << mask->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < mask->datas_.size(); ++i) {
        auto one_mask = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Segmentation>>(mask->datas_[i]);
        if (one_mask->state_ != xstream::DataState::VALID) {
          continue;
        }
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
            smart_target.end()) {
          LOGE << "Not found the track_id target";
          continue;
        } else {
          auto body_box = body_box_list[i]->value;
          int x1 = body_box.x1;
          int y1 = body_box.y1;
          int x2 = body_box.x2;
          int y2 = body_box.y2;
          auto mask = one_mask->value;
          int h_w = sqrt(mask.values.size());
          cv::Mat mask_mat(h_w, h_w, CV_32F);

          for (int h = 0; h < h_w; ++h) {
            float *ptr = mask_mat.ptr<float>(h);
            for (int w = 0; w < h_w; ++w) {
              *(ptr + w) = (mask.values)[h * h_w + w];
            }
          }
          int width = x2 - x1;
          int height = y2 - y1;
          cv::resize(mask_mat, mask_mat, cv::Size(width, height));

          cv::Mat mask_gray(height, width, CV_8UC1);
          mask_gray.setTo(0);
          for (int h = 0; h < height; ++h) {
            uchar *p_gray = mask_gray.ptr<uchar>(h);
            const float *p_mask = mask_mat.ptr<float>(h);
            for (int w = 0; w < width; ++w) {
              if (p_mask[w] > 0) {
                // 这个点在人体内
                p_gray[w] = 1;
              } else {
              }
            }
          }
          mask_mat.release();
          std::vector<std::vector<cv::Point>> contours;
          cv::findContours(mask_gray, contours, cv::noArray(), cv::RETR_CCOMP,
                           cv::CHAIN_APPROX_NONE);
          mask_gray.release();
          auto target = smart_target[body_box_list[i]->value.id];
          auto Points = target->add_points_();
          Points->set_type_("mask");
          for (size_t i = 0; i < contours.size(); i++) {
            auto one_line = contours[i];
            for (size_t j = 0; j < one_line.size(); j++) {
              auto point = Points->add_points_();
              point->set_x_((contours[i][j].x + x1) * x_ratio);
              point->set_y_((contours[i][j].y + y1) * y_ratio);
              point->set_score_(0);
            }
          }
          contours.clear();
          std::vector<std::vector<cv::Point>>(contours).swap(contours);
        }
      }
    }

    if (output->name_ == "age") {
      auto ages = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "age size: " << ages->datas_.size();
      if (ages->datas_.size() != face_box_list.size()) {
        LOGE << "ages size: " << ages->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < ages->datas_.size(); ++i) {
        auto age =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::Age>>(
                ages->datas_[i]);
        // 查找对应的track_id
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(face_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (age->state_ != xstream::DataState::VALID) {
            LOGE << "-1 -1 -1";
            continue;
          }
          auto target = smart_target[face_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("age");
          attrs->set_value_((age->value.min + age->value.max) / 2);
          attrs->set_score_(age->value.score);
          attrs->set_value_string_(
            std::to_string((age->value.min + age->value.max) / 2));
          LOGD << " " << age->value.min << " " << age->value.max;
        }
      }
    }

    if (output->name_ == "gender") {
      auto genders = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "gender size: " << genders->datas_.size();
      if (genders->datas_.size() != face_box_list.size()) {
        LOGE << "genders size: " << genders->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < genders->datas_.size(); ++i) {
        auto gender = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Gender>>(genders->datas_[i]);
        // 查找对应的track_id
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(face_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (genders->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[face_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("gender");
          attrs->set_value_(gender->value.value);
          attrs->set_score_(gender->value.score);
          auto gender_des = AttributeConvert::Instance().GetAttrDes(
            "gender", gender->value.value);
          attrs->set_value_string_(gender_des);
          LOGD << " " << gender->value.value;
        }
      }
    }

    // 每帧输出action数量为1，无track_id
    if (output->name_ == "action") {
      auto actions = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "action size: " << actions->datas_.size();
      for (size_t i = 0; i < actions->datas_.size(); ++i) {
        auto action =
            std::static_pointer_cast<xstream::XStreamData<std::string>>(
                actions->datas_[i]);
        if (action->state_ != xstream::DataState::VALID) {
          LOGE << "not valid : -1";
          continue;
        }
        // i 仅可能为 0，默认放在第一个target中; 防止target被过滤
        if (static_cast<int>(i) >= smart_msg->targets__size()) {
          break;
        }
        auto target = smart_msg->mutable_targets_(i);
        auto attrs = target->add_attributes_();
        attrs->set_type_("action");
        float action_index = 0;
        if (action->value == "other") {
          action_index = 1;
        } else if (action->value == "stand") {
          action_index = 2;
        } else if (action->value == "run") {
          action_index = 3;
        } else if (action->value == "attack") {
          action_index = 4;
        }
        LOGD << "smartplugin, value = " << action->value
             << ", action_index = " << action_index << std::endl;
        attrs->set_value_(action_index);
        attrs->set_score_(0.8);
      }
    }

    if (output->name_ == "fall_vote") {
      auto fall_votes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "fall_votes size: " << fall_votes->datas_.size();
      if (fall_votes->datas_.size() != body_box_list.size()) {
        LOGE << "fall_vote size: " << fall_votes->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < fall_votes->datas_.size(); ++i) {
        auto fall_vote = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
            fall_votes->datas_[i]);
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (fall_vote->state_ != xstream::DataState::VALID) {
            continue;
          }
          auto target = smart_target[body_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("fall");
          {
            std::lock_guard<std::mutex> fall_lock(fall_mutex_);
            auto id = body_box_list[i]->value.id;
            if (fall_vote->value.value == 1) {
              fall_state_[id] = 1;
            } else {
              if (fall_state_.find(id) != fall_state_.end()) {
                if (fall_state_[id] > 0) {
                  fall_vote->value.value = 1;
                  fall_state_[id]++;
                }
                if (fall_state_[id] > 10) {
                  fall_state_.erase(id);
                }
              }
            }
          }
          attrs->set_value_(fall_vote->value.value);
          attrs->set_score_(fall_vote->value.score);
          LOGD << " " << fall_vote->value.value;
        }
      }
    }

    if (output->name_ == "raise_hand" ||
        output->name_ == "stand" ||
        output->name_ == "squat") {
      auto attributes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << attributes->datas_.size();
      if (attributes->datas_.size() != body_box_list.size()) {
        LOGE << "behavior attributes size: " << attributes->datas_.size()
             << ", body_box size: " << body_box_list.size();
      }
      for (size_t i = 0; i < attributes->datas_.size(); ++i) {
        auto attribute = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
                attributes->datas_[i]);
        // 查找对应的track_id
        if (body_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(body_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (attribute->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[body_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_(output->name_);
          attrs->set_value_(attribute->value.value);
          attrs->set_score_(attribute->value.score);
          LOGD << "value: " << attribute->value.value;
        }
      }
    }

    if (output->name_ == "face_mask") {
      auto attributes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << attributes->datas_.size();
      if (attributes->datas_.size() != face_box_list.size()) {
        LOGE << "face mask size: " << attributes->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < attributes->datas_.size(); ++i) {
        auto attribute = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
                attributes->datas_[i]);
        // 查找对应的track_id
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(face_box_list[i]->value.id) ==
              smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (attribute->state_ != xstream::DataState::VALID) {
            LOGE << "-1";
            continue;
          }
          auto target = smart_target[face_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_(output->name_);
          attrs->set_value_(attribute->value.value);
          attrs->set_score_(attribute->value.score);
          LOGD << "value: " << attribute->value.value;
        }
      }
    }
    if (output->name_ == "face_feature") {
      auto features = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << features->datas_.size();
      if (features->datas_.size() != face_box_list.size()) {
        LOGE << "face feature size: " << features->datas_.size()
             << ", face_box size: " << face_box_list.size();
      }
      for (size_t i = 0; i < features->datas_.size(); ++i) {
        if (face_box_list[i]->value.id == -1) {
          continue;
        }
        // 查找对应的track_id
        if (smart_target.find(face_box_list[i]->value.id) ==
            smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          auto one_person_feature =
              std::static_pointer_cast<xstream::BaseDataVector>(
                  features->datas_[i]);
          auto target = smart_target[face_box_list[i]->value.id];
          for (size_t idx = 0; idx < one_person_feature->datas_.size(); idx++) {
            auto one_feature = std::static_pointer_cast<
                xstream::XStreamData<hobot::vision::Feature>>(
                one_person_feature->datas_[idx]);
            if (one_feature->state_ != xstream::DataState::VALID) {
              LOGE << "-1";
              continue;
            }
            auto feature = target->add_float_arrays_();
            feature->set_type_(output->name_);
            for (auto item : one_feature->value.values) {
              feature->add_value_(item);
            }
          }
        }
      }
    }
    if (output->name_ == "hand_lmk") {
      lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "hand_lmk size: " << lmks->datas_.size();
      if (lmks->datas_.size() != hand_box_list.size()) {
        LOGE << "hand_lmk size: " << lmks->datas_.size()
             << ", hand_box size: " << hand_box_list.size();
      }
      for (size_t i = 0; i < lmks->datas_.size(); ++i) {
        auto lmk = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(lmks->datas_[i]);
        // 查找对应的track_id
        if (hand_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(hand_box_list[i]->value.id) ==
            smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          auto target = smart_target[hand_box_list[i]->value.id];
          auto proto_points = target->add_points_();
          proto_points->set_type_("hand_landmarks");
          for (size_t i = 0; i < lmk->value.values.size(); ++i) {
            auto point = proto_points->add_points_();
            point->set_x_(lmk->value.values[i].x * x_ratio);
            point->set_y_(lmk->value.values[i].y * y_ratio);
            point->set_score_(lmk->value.values[i].score);
            LOGD << "x: " << std::round(lmk->value.values[i].x)
                 << " y: " << std::round(lmk->value.values[i].y)
                 << " score: " << lmk->value.values[i].score << "\n";
          }
        }
      }
    }
    if (output->name_ == "gesture_vote") {
      auto gesture_votes =
          dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << "gesture_vote size: " << gesture_votes->datas_.size();
      if (gesture_votes->datas_.size() != hand_box_list.size()) {
        LOGE << "gesture_vote size: " << gesture_votes->datas_.size()
             << ", body_box size: " << hand_box_list.size();
      }
      for (size_t i = 0; i < gesture_votes->datas_.size(); ++i) {
        auto gesture_vote = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
            gesture_votes->datas_[i]);
        // 查找对应的track_id
        if (hand_box_list[i]->value.id == -1) {
          continue;
        }
        if (smart_target.find(hand_box_list[i]->value.id) ==
            smart_target.end()) {
          LOGE << "Not found the track_id target";
        } else {
          if (gesture_vote->state_ != xstream::DataState::VALID) {
            LOGD << "state = invalid";
            continue;
          }
          int gesture_ret = -1;
          // gesture event
          if (gesture_as_event) {
            auto hand_id = hand_box_list[i]->value.id;
            auto cur_microsec =
              std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            float cur_sec = cur_microsec / 1000000.0;
            if (gesture_state_.find(hand_id) == gesture_state_.end()) {
              gesture_ret = gesture_vote->value.value;
              gesture_state_[hand_id] = gesture_ret;
              gesture_start_time_[hand_id] = cur_sec;
            } else {
              auto last_gesture = gesture_state_[hand_id];
              auto cur_gesture = gesture_vote->value.value;
              if (last_gesture != cur_gesture &&
                  cur_sec - gesture_start_time_[hand_id] >= 0.5) {
                gesture_ret = cur_gesture;
                gesture_state_[hand_id] = gesture_ret;
                gesture_start_time_[hand_id] = cur_sec;
              }
            }
          } else {
            gesture_ret = gesture_vote->value.value;
          }
          // end gesture event
          auto target = smart_target[hand_box_list[i]->value.id];
          auto attrs = target->add_attributes_();
          attrs->set_type_("gesture");
          attrs->set_value_(gesture_ret);
          auto gesture_des = AttributeConvert::Instance().GetAttrDes(
              "gesture", gesture_ret);
          attrs->set_value_string_(gesture_des);
          LOGD << " " << gesture_ret << ", des: " << gesture_des;
        }
      }
    }
    if (output->name_ == "hand_disappeared_track_id_list") {
      auto disappeared_hands =
        dynamic_cast<xstream::BaseDataVector *>(output.get());
      for (size_t i = 0; i < disappeared_hands->datas_.size(); ++i) {
        auto disappeared_hand = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Attribute<int>>>(
            disappeared_hands->datas_[i])->value.value;
        if (gesture_state_.find(disappeared_hand) != gesture_state_.end()) {
          gesture_state_.erase(disappeared_hand);
          LOGD << "erase " << disappeared_hand << " from gesture_state";
        }
        if (gesture_start_time_.find(disappeared_hand) !=
            gesture_start_time_.end()) {
          gesture_start_time_.erase(disappeared_hand);
          LOGD << "erase " << disappeared_hand << " from timestamp_start";
        }
      }
    }
  }

  proto_frame_message.SerializeToString(&proto_str);
  LOGD << "smart result serial success";
  return proto_str;
}

SmartPlugin::SmartPlugin(const std::string &config_file) {
  config_file_ = config_file;
  LOGI << "smart config file:" << config_file_;
  monitor_.reset(new RuntimeMonitor());
  Json::Value cfg_jv;
  std::ifstream infile(config_file_);
  infile >> cfg_jv;
  config_.reset(new JsonConfigWrapper(cfg_jv));
  ParseConfig();
}

void SmartPlugin::ParseConfig() {
  xstream_workflow_cfg_file_ =
      config_->GetSTDStringValue("xstream_workflow_file");
  enable_profile_ = config_->GetBoolValue("enable_profile");
  profile_log_file_ = config_->GetSTDStringValue("profile_log_path");
  if (config_->HasMember("enable_result_to_json")) {
    result_to_json = config_->GetBoolValue("enable_result_to_json");
  }
  LOGI << "xstream_workflow_file:" << xstream_workflow_cfg_file_;
  LOGI << "enable_profile:" << enable_profile_
       << ", profile_log_path:" << profile_log_file_;
  if (config_->HasMember("gesture_as_event")) {
    gesture_as_event = config_->GetBoolValue("gesture_as_event");
  }
}

int SmartPlugin::Init() {
  // init for xstream sdk
  sdk_.reset(xstream::XStreamSDK::CreateSDK());
  sdk_->SetConfig("config_file", xstream_workflow_cfg_file_);
  if (sdk_->Init() != 0) {
    return kHorizonVisionInitFail;
  }
  if (enable_profile_) {
    sdk_->SetConfig("profiler", "on");
    sdk_->SetConfig("profiler_file", profile_log_file_);
  }
  sdk_->SetCallback(
      std::bind(&SmartPlugin::OnCallback, this, std::placeholders::_1));

  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&SmartPlugin::Feed, this, std::placeholders::_1));
  return XPluginAsync::Init();
}

int SmartPlugin::Feed(XProtoMessagePtr msg) {
  // feed video frame to xstreamsdk.
  // 1. parse valid frame from msg
  LOGI << "smart plugin got one msg";
  auto valid_frame = std::static_pointer_cast<VioMessage>(msg);
  if (valid_frame->profile_ != nullptr) {
    valid_frame->profile_->UpdatePluginStartTime(desc());
  }
  xstream::InputDataPtr input = Convertor::ConvertInput(valid_frame.get());
  SmartInput *input_wrapper = new SmartInput();
  input_wrapper->frame_info = valid_frame;
  input_wrapper->context = input_wrapper;
  monitor_->PushFrame(input_wrapper);
  if (sdk_->AsyncPredict(input) != 0) {
    return kHorizonVisionFailure;
  }
  LOGI << "feed one task to xtream workflow";

  return 0;
}


int SmartPlugin::Start() {
  LOGI << "SmartPlugin Start";
  root.clear();
  return 0;
}

int SmartPlugin::Stop() {
  if (result_to_json) {
    remove("smart_data.json");
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream("smart_data.json");
    writer->write(root, &outputFileStream);
  }
  Profiler::Release();
  LOGI << "SmartPlugin Stop";
  return 0;
}

void SmartPlugin::OnCallback(xstream::OutputDataPtr xstream_out) {
  // On xstream async-predict returned,
  // transform xstream standard output to smart message.
  LOGI << "smart plugin got one smart result";
  HOBOT_CHECK(!xstream_out->datas_.empty()) << "Empty XStream Output";

  XStreamImageFramePtr *rgb_image = nullptr;

  for (const auto &output : xstream_out->datas_) {
    LOGD << output->name_ << ", type is " << output->type_;
    if (output->name_ == "rgb_image" || output->name_ == "image") {
      rgb_image = dynamic_cast<XStreamImageFramePtr *>(output.get());
    }
  }

  auto smart_msg = std::make_shared<CustomSmartMessage>(xstream_out);
  // Set origin input named "image" as output always.
  HOBOT_CHECK(rgb_image);
  smart_msg->time_stamp = rgb_image->value->time_stamp;
  smart_msg->frame_id = rgb_image->value->frame_id;
  LOGI << "smart result frame_id = " << smart_msg->frame_id << std::endl;
  auto input = monitor_->PopFrame(smart_msg->frame_id);
  auto vio_msg = input.vio_msg;
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStopTime(desc());
  }
  delete static_cast<SmartInput *>(input.context);
  monitor_->FrameStatistic();
  smart_msg->frame_fps = monitor_->GetFrameFps();
  PushMsg(smart_msg);
  // smart_msg->Serialize();
  if (result_to_json) {
    /// output structure data
    smart_msg->Serialize_Print(root);
  }
}
}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
