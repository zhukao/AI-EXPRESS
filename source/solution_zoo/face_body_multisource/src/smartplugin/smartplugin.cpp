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

#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/json_key.h"
#include "horizon/vision/util.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"
#include "smartplugin/convert.h"
#include "smartplugin/runtime_monitor.h"
#include "smartplugin/smart_config.h"
#include "smartplugin/smartplugin.h"
#include "xproto_msgtype/protobuf/x2.pb.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::basic_msgtype::VioMessage;
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;

using horizon::iot::Convertor;

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_SMART_MESSAGE)

void CustomSmartMessage::Serialize_Print(Json::Value &root) {
  LOGW << "Frame id: " << frame_id;
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

  Json::Value targets;
  xstream::BaseDataVector *body_data = nullptr;
  std::set<int> id_set;
  for (const auto &output : smart_result->datas_) {
    auto prefix = name_prefix(output->name_);
    auto postfix = name_postfix(output->name_);
    if (prefix == "body") {
      body_data = dynamic_cast<xstream::BaseDataVector *>(output.get());
    }
    if (output->name_ == "face_bbox_list" || output->name_ == "head_box" ||
        output->name_ == "body_box" || postfix == "box") {
      auto bboxs = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "box size: " << bboxs->datas_.size();
      for (size_t i = 0; i < bboxs->datas_.size(); ++i) {
        Json::Value target;
        auto bbox =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                bboxs->datas_[i]);

        if (prefix == "face" || prefix == "head" || prefix == "body") {
          target[prefix].append(static_cast<int>(bbox->value.x1));
          target[prefix].append(static_cast<int>(bbox->value.y1));
          target[prefix].append(static_cast<int>(bbox->value.x2));
          target[prefix].append(static_cast<int>(bbox->value.y2));
        } else {
          LOGE << "unsupport box name: " << output->name_;
        }
        targets[std::to_string(bbox->value.id)].append(target);
        id_set.insert(bbox->value.id);
      }
    }
    if (output->name_ == "kps") {
      auto lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "kps size: " << lmks->datas_.size();
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
  std::string proto_str;
  x2::FrameMessage proto_frame_message;
  auto smart_msg = proto_frame_message.mutable_smart_msg_();
  smart_msg->set_timestamp_(time_stamp);
  smart_msg->set_error_code_(0);
  // user-defined output parsing declaration.
  xstream::BaseDataVector *bboxs = nullptr;
  xstream::BaseDataVector *lmks = nullptr;
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

  for (const auto &output : smart_result->datas_) {
    LOGD << "output name: " << output->name_;
    auto prefix = name_prefix(output->name_);
    auto postfix = name_postfix(output->name_);
    if (output->name_ == "face_bbox_list" || output->name_ == "head_box" ||
        output->name_ == "body_box" || postfix == "box") {
      bboxs = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "box size: " << bboxs->datas_.size();
      for (size_t i = 0; i < bboxs->datas_.size(); ++i) {
        auto bbox =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                bboxs->datas_[i]);
        LOGW << output->name_ << " id: " << bbox->value.id
             << " x1: " << bbox->value.x1 << " y1: " << bbox->value.y1
             << " x2: " << bbox->value.x2 << " y2: " << bbox->value.y2;

        auto target = smart_msg->add_targets_();
        auto proto_box = target->add_boxes_();

        if (prefix == "face" || prefix == "head" || prefix == "body") {
          target->set_type_(prefix);
          proto_box->set_type_(prefix);
        } else {
          LOGE << "unsupport box name: " << output->name_;
        }
        target->set_track_id_(bbox->value.id);

        auto point1 = proto_box->mutable_top_left_();
        point1->set_x_(bbox->value.x1);
        point1->set_y_(bbox->value.y1);
        point1->set_score_(bbox->value.score);
        auto point2 = proto_box->mutable_bottom_right_();
        point2->set_x_(bbox->value.x2);
        point2->set_y_(bbox->value.y2);
        point2->set_score_(bbox->value.score);
      }
    }
    if (output->name_ == "kps") {
      lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "kps size: " << lmks->datas_.size();
      for (size_t i = 0; i < lmks->datas_.size(); ++i) {
        auto lmk = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Landmarks>>(lmks->datas_[i]);
        auto target = smart_msg->add_targets_();
        target->set_type_("kps");
        auto proto_points = target->add_points_();
        proto_points->set_type_("landmarks");
        for (size_t i = 0; i < lmk->value.values.size(); ++i) {
          auto point = proto_points->add_points_();
          point->set_x_(lmk->value.values[i].x);
          point->set_y_(lmk->value.values[i].y);
          point->set_score_(lmk->value.values[i].score);
          LOGW << "x: " << std::round(lmk->value.values[i].x)
               << " y: " << std::round(lmk->value.values[i].y)
               << " score: " << lmk->value.values[i].score << "\n";
        }
      }
    }
    if (output->name_ == "age") {
      auto ages = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "age size: " << ages->datas_.size();
      for (size_t i = 0; i < ages->datas_.size(); ++i) {
        auto age =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::Age>>(
                ages->datas_[i]);
        if (age->state_ != xstream::DataState::VALID) {
          LOGE << "-1 -1 -1";
        }
        auto target = smart_msg->mutable_targets_(i);
        auto attrs = target->add_attributes_();
        attrs->set_type_("age");
        attrs->set_value_((age->value.min + age->value.max) / 2);
        attrs->set_score_(age->value.score);

        LOGW << " " << age->value.min << " " << age->value.max;
      }
    }
    if (output->name_ == "gender") {
      auto genders = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "gender size: " << genders->datas_.size();
      for (size_t i = 0; i < genders->datas_.size(); ++i) {
        auto gender = std::static_pointer_cast<
            xstream::XStreamData<hobot::vision::Gender>>(genders->datas_[i]);
        if (genders->state_ != xstream::DataState::VALID) {
          LOGE << "-1";
        }
        auto target = smart_msg->mutable_targets_(i);
        auto attrs = target->add_attributes_();
        attrs->set_type_("gender");
        attrs->set_value_(gender->value.value);
        attrs->set_score_(gender->value.score);
        LOGW << " " << gender->value.value;
      }
    }
  }
  proto_frame_message.SerializeToString(&proto_str);
  return proto_str;
}

SmartPlugin::SmartPlugin(const std::string &config_file) {
  config_file_ = config_file;
  LOGI << "smart config file:" << config_file_;
  Json::Value cfg_jv;
  std::ifstream infile(config_file_);
  infile >> cfg_jv;
  config_.reset(new JsonConfigWrapper(cfg_jv));

  if (config_->HasMember("enable_result_to_json")) {
    result_to_json = config_->GetBoolValue("enable_result_to_json");
  }
}

// 覆盖workflow的source id配置
int SmartPlugin::OverWriteSourceNum(const std::string &cfg_file,
                                    int source_num) {
  std::fstream cfg_file_stream(cfg_file, std::fstream::out | std::fstream::in);
  LOGI << "overwrite workflow cfg_file: " << cfg_file;
  if (!cfg_file_stream.good()) {
    LOGE << "Open failed:" << cfg_file;
    return -1;
  }
  Json::Value config;
  cfg_file_stream >> config;
  if (source_num <= 0) {
    config.removeMember(xstream::kSourceNum);
  } else {
    config[xstream::kSourceNum] = source_num;
  }
  cfg_file_stream.seekp(0, std::ios_base::beg);
  cfg_file_stream << config;
  cfg_file_stream.close();
  return 0;
}

int SmartPlugin::Init() {
  auto workflows_cfg = config_->GetSubConfig("workflows");

  // 初始化相关字段
  int workflow_cnt = workflows_cfg->ItemCount();
  LOGI << "workflow count: " << workflow_cnt;

  sdk_.resize(workflow_cnt);
  root.resize(workflow_cnt);
  source_map_.resize(workflow_cnt);

  monitor_.reset(new RuntimeMonitor());

  for (int i = 0; i < workflow_cnt; i++) {
    auto item_cfg = workflows_cfg->GetSubConfig(i);
    auto xstream_workflow_file =
        item_cfg->GetSTDStringValue("xstream_workflow_file");
    auto source_list = item_cfg->GetIntArray("source_list");

    sdk_[i].reset(xstream::XStreamSDK::CreateSDK());

    if (item_cfg->GetBoolValue("overwrite")) {
      // int max_source_id = 0;
      // for (unsigned int i = 0; i < source_list.size(); i++) {
      //   max_source_id =
      //       source_list[i] > max_source_id ? source_list[i] : max_source_id;
      // }
      // FIXME(zhuoran.rong) xstream 必须是source id从0开始
      // OverWriteSourceNum(xstream_workflow_file, max_source_id+1);
      OverWriteSourceNum(xstream_workflow_file, source_list.size());
    }

    sdk_[i]->SetConfig("config_file", xstream_workflow_file);

    if (item_cfg->GetBoolValue("enable_profile")) {
      sdk_[i]->SetConfig("profiler", "on");
      sdk_[i]->SetConfig("profiler_file",
                         item_cfg->GetSTDStringValue("profile_log_path"));
    }

    if (sdk_[i]->Init() != 0) {
      return kHorizonVisionInitFail;
    }

    sdk_[i]->SetCallback(
        std::bind(&SmartPlugin::OnCallback, this, std::placeholders::_1));

    // 建立source id到workflow的映射关系
    for (unsigned int j = 0; j < source_list.size(); j++) {
      auto &tmp = source_target_[source_list[j]];
      auto iter = std::find(tmp.begin(), tmp.end(), i);
      if (iter == tmp.end()) {
        tmp.push_back(i);
      }
      // 加入到对应xstream instance的source list中
      source_map_[i].push_back(source_list[j]);
    }
  }

  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&SmartPlugin::Feed, this, std::placeholders::_1));
  return XPluginAsync::Init();
}

// feed video frame to xstreamsdk.
int SmartPlugin::Feed(XProtoMessagePtr msg) {
  // 1. parse valid frame from msg
  auto valid_frame = std::static_pointer_cast<VioMessage>(msg);

  int source_id = valid_frame->image_[0]->channel_id;
  int frame_id = valid_frame->image_[0]->frame_id;

  auto iter = source_target_.find(source_id);
  if (iter == source_target_.end()) {
    LOGW << "Unknow Source ID: " << source_id;
    return 0;
  }
  LOGD << "Source ID: " << source_id;

  auto target_sdk = iter->second;
  for (unsigned int i = 0; i < target_sdk.size(); i++) {
    int sdk_idx = target_sdk[i];
    // 创建xstream的输入
    xstream::InputDataPtr input = Convertor::ConvertInput(valid_frame.get());
    // 设置Source ID
    for (unsigned int j = 0; j < source_map_[sdk_idx].size(); j++) {
      if (source_map_[sdk_idx][j] == source_id) {
        input->source_id_ = j;
        break;
      }
    }

    input->context_ = (const void *)((uintptr_t)sdk_idx);  // NOLINT

    SmartInput *input_wrapper = new SmartInput();
    input_wrapper->frame_info = valid_frame;
    input_wrapper->context = input_wrapper;
    monitor_->PushFrame(input_wrapper);

    if (sdk_[sdk_idx]->AsyncPredict(input) <= 0) {
      LOGW << "Async failed: " << sdk_idx << ", source id: " << source_id;

      auto input = monitor_->PopFrame(source_id, frame_id);
      if (input.ref_count == 0) {
        delete static_cast<SmartInput *>(input.context);
      }

      continue;
      // return kHorizonVisionFailure;
    }
  }

  return 0;
}

int SmartPlugin::Start() {
  LOGW << "SmartPlugin Start";
  // json记录
  for (unsigned int i = 0; i < sdk_.size(); i++) {
    root[i].clear();
  }
  return 0;
}

int SmartPlugin::Stop() {
  if (result_to_json) {
    for (unsigned int i=0; i < sdk_.size(); i++) {
      std::stringstream name;
      name << "smart_data_" << i << ".json";
      remove(name.str().c_str());
      Json::StreamWriterBuilder builder;
      std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
      std::ofstream outputFileStream(name.str().c_str());
      writer->write(root[i], &outputFileStream);
    }
  }
  LOGW << "SmartPlugin Stop";
  return 0;
}

void SmartPlugin::OnCallback(xstream::OutputDataPtr xstream_out) {
  // On xstream async-predict returned,
  // transform xstream standard output to smart message.
  HOBOT_CHECK(!xstream_out->datas_.empty()) << "Empty XStream Output";

  using xstream::BaseDataVector;
  LOGI << "============Output Call Back============";
  LOGI << "—seq: " << xstream_out->sequence_id_;
  LOGI << "—output_type: " << xstream_out->output_type_;
  LOGI << "—error_code: " << xstream_out->error_code_;
  LOGI << "—error_detail_: " << xstream_out->error_detail_;
  LOGI << "—datas_ size: " << xstream_out->datas_.size();
  for (auto data : xstream_out->datas_) {
    LOGI << "——output data " << data->name_ << " state:"
         << static_cast<std::underlying_type<xstream::DataState>::type>(
                data->state_);

    if (data->error_code_ < 0) {
      LOGI << "——data error: " << data->error_code_;
      continue;
    }
    LOGI << "——data type:" << data->type_ << " name:" << data->name_;
    // BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
    // LOGI << "pdata size: " << pdata->datas_.size();
  }
  LOGI << "============Output Call Back End============";

  int sdk_idx = (int)((uintptr_t)xstream_out->context_);  // NOLINT
  int source_id = source_map_[sdk_idx][xstream_out->source_id_];

  XStreamImageFramePtr *rgb_image = nullptr;

  for (const auto &output : xstream_out->datas_) {
    LOGD << output->name_ << ", type is " << output->type_;
    if (output->name_ == "rgb_image" || output->name_ == "image") {
      rgb_image = dynamic_cast<XStreamImageFramePtr *>(output.get());
    }
  }
  HOBOT_CHECK(rgb_image);

  auto smart_msg = std::make_shared<CustomSmartMessage>(xstream_out);
  // Set origin input named "image" as output always.
  smart_msg->time_stamp = rgb_image->value->time_stamp;
  smart_msg->frame_id = rgb_image->value->frame_id;
  PushMsg(smart_msg);

  auto input = monitor_->PopFrame(source_id, smart_msg->frame_id);
  if (input.ref_count == 0) {
    delete static_cast<SmartInput *>(input.context);
  }

  smart_msg->Serialize();
  if (result_to_json) {
    /// output structure data
    smart_msg->Serialize_Print(root[source_id]);
  }
}

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
