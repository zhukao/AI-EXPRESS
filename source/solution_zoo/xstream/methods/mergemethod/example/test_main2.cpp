/*
 * @Description: implement of data_type
 * @Author: peng02.li@horizon.ai
 * @Date: 2019-4-12 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:57:21
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;

typedef xstream::XStreamData<uint32_t> XStreamUint32;
typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<hobot::vision::Landmarks> XStreamKps;
typedef xstream::XStreamData<hobot::vision::Landmarks> XStreamLandmarks;
typedef std::shared_ptr<xstream::BaseDataVector> BaseDataVectorPtr;

struct BoxKpsList {
  BoxKpsList(BaseDataVectorPtr box_list_, BaseDataVectorPtr kps_list_) {
    box_list = box_list_;
    kps_list = kps_list_;
  }
  BaseDataVectorPtr box_list;
  BaseDataVectorPtr kps_list;
};

std::string get_image_name(const std::string &info) {
  std::istringstream ss(info);
  std::string image_name;
  ss >> image_name;
  return image_name;
}

BaseDataVectorPtr parsing_input_str_stream(const std::string &info) {
  // std::cout << info << std::endl;
  std::istringstream ss(info);
  int x1, y1, x2, y2;
  float track_id;
  std::string image_name;
  ss >> image_name;
  auto box_list = std::make_shared<BaseDataVector>();
  while (ss >> x1 >> y1 >> x2 >> y2 >> track_id) {
    auto box1 = std::make_shared<XStreamBBox>();
    box1->type_ = "BBox";
    box1->value.id = track_id;
    box1->value.x1 = x1;
    box1->value.y1 = y1;
    box1->value.x2 = x2;
    box1->value.y2 = y2;
    box_list->datas_.emplace_back(BaseDataPtr(box1));
  }
  return box_list;
}

BoxKpsList parsing_person_det_stream(const std::string &info) {
  std::istringstream ss(info);
  int x1, y1, x2, y2;
  double track_id;
  auto box_list = std::make_shared<BaseDataVector>();
  auto kps_list = std::make_shared<BaseDataVector>();
  BoxKpsList pair_res = BoxKpsList(box_list, kps_list);
  std::string image_name;
  ss >> image_name;
  while (ss >> x1 >> y1 >> x2 >> y2 >> track_id) {
    auto box1 = std::make_shared<XStreamBBox>();
    box1->type_ = "BBox";
    box1->value.id = track_id;
    box1->value.x1 = x1;
    box1->value.y1 = y1;
    box1->value.x2 = x2;
    box1->value.y2 = y2;
    box_list->datas_.emplace_back(BaseDataPtr(box1));

    int kp_x, kp_y;
    float kp_score;
    auto body_kps1 = std::make_shared<XStreamKps>();
    body_kps1->type_ = "Kps";
    body_kps1->value.values.resize(17);

    for (size_t kp_idx = 0; kp_idx < 17; ++kp_idx) {
      ss >> kp_x >> kp_y >> kp_score;
      body_kps1->value.values[kp_idx].x = kp_x;
      body_kps1->value.values[kp_idx].y = kp_y;
      body_kps1->value.values[kp_idx].score = kp_score;
    }
    kps_list->datas_.emplace_back(BaseDataPtr(body_kps1));

    // feat
    double feat_value;
    for (size_t feat_idx = 0; feat_idx < 128; ++feat_idx) {
      ss >> feat_value;
    }
  }
  return pair_res;
}

int main(int argc, char const *argv[]) {
  SetLogLevel(HOBOT_LOG_INFO);
  std::string head_track_file = "./data/head_body/head_track_result.txt";
  std::ifstream fid_in(head_track_file, std::ios::in);
  std::string info;

  std::string face_det_file = "./data/head_body/face_track_result.txt";
  std::ifstream face_fid_in(face_det_file, std::ios::in);
  std::string face_info;

  std::string body_det_file = "./data/head_body/body_track_result.txt";
  std::ifstream body_fid_in(body_det_file, std::ios::in);
  std::string body_info;

  std::string face_in_file = "./data/head_body/output/merged_face_track.txt";
  std::string body_in_file = "./data/head_body/output/merged_body_track.txt";
  std::fstream face_res_f(face_in_file, std::ios::out);
  std::fstream body_res_f(body_in_file, std::ios::out);

  static xstream::XStreamSDK *flow;
  flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", "./config/merge_flow2.json");
  flow->Init();
  auto version = flow->GetVersion("merge_method");
  LOGI << "Merge Method Version: " << version;

  while (getline(fid_in, info) && getline(face_fid_in, face_info) &&
         getline(body_fid_in, body_info)) {
    InputDataPtr input(new InputData());
    std::string image_name = get_image_name(info);
    LOGD << "image name: " << image_name;
    face_res_f << image_name;
    body_res_f << image_name;

    auto head_box_list = parsing_input_str_stream(info);
    auto face_box_list = parsing_input_str_stream(face_info);
    BoxKpsList pair_res = parsing_person_det_stream(body_info);
    auto person_box_list = pair_res.box_list;
    auto person_kps_list = pair_res.kps_list;

    face_box_list->name_ = "face_box";
    input->datas_.push_back(BaseDataPtr(face_box_list));
    head_box_list->name_ = "head_box";
    input->datas_.push_back(BaseDataPtr(head_box_list));
    person_box_list->name_ = "body_box";
    input->datas_.push_back(BaseDataPtr(person_box_list));
    person_kps_list->name_ = "body_kps";
    input->datas_.push_back(BaseDataPtr(person_kps_list));

    auto seed = static_cast<uint32_t>(time(NULL));
    auto disappeared_head_id = std::make_shared<BaseDataVector>();
    disappeared_head_id->name_ = "disappeared_head_id";
    auto head_id = std::make_shared<XStreamUint32>();
    head_id->value = rand_r(&seed) % 1000;
    LOGD << "lost head id: " << head_id->value;
    disappeared_head_id->datas_.emplace_back(BaseDataPtr(head_id));
    input->datas_.push_back(BaseDataPtr(disappeared_head_id));

    auto disappeared_face_id = std::make_shared<BaseDataVector>();
    disappeared_face_id->name_ = "disappeared_face_id";
    auto face_id = std::make_shared<XStreamUint32>();
    face_id->value = rand_r(&seed) % 1000;
    LOGD << "lost face id: " << face_id->value;
    disappeared_face_id->datas_.emplace_back(BaseDataPtr(face_id));
    input->datas_.push_back(BaseDataPtr(disappeared_face_id));

    auto disappeared_body_id = std::make_shared<BaseDataVector>();
    disappeared_body_id->name_ = "disappeared_body_id";
    auto body_id = std::make_shared<XStreamUint32>();
    body_id->value = rand_r(&seed) % 1000;
    LOGD << "lost body id: " << body_id->value;
    disappeared_body_id->datas_.emplace_back(BaseDataPtr(body_id));
    input->datas_.push_back(BaseDataPtr(disappeared_body_id));

    // 构建输入参数
    auto out = flow->SyncPredict(input);
    for (int i = 0; i < 3; ++i) {
      auto &data = out->datas_[i];
      auto pdata = std::static_pointer_cast<BaseDataVector>(data);
      if (pdata->name_ == "merged_face_box") {
        for (const auto &element : pdata->datas_) {
          auto bbox = std::static_pointer_cast<XStreamBBox>(element);
          LOGD << "face " << bbox->value.x1 << " " << bbox->value.y1 << " "
               << bbox->value.x2 << " " << bbox->value.y2 << " "
               << bbox->value.id;
          face_res_f << " " << bbox->value.x1 << " " << bbox->value.y1 << " "
                     << bbox->value.x2 << " " << bbox->value.y2 << " "
                     << bbox->value.id;
        }
      } else if (pdata->name_ == "merged_body_box") {
        for (const auto &element : pdata->datas_) {
          auto bbox = std::static_pointer_cast<XStreamBBox>(element);
          LOGD << "body " << bbox->value.x1 << " " << bbox->value.y1 << " "
               << bbox->value.x2 << " " << bbox->value.y2 << " "
               << bbox->value.id;
          body_res_f << " " << bbox->value.x1 << " " << bbox->value.y1 << " "
                     << bbox->value.x2 << " " << bbox->value.y2 << " "
                     << bbox->value.id;
        }
      } else if (pdata->name_ == "merged_disappeared_id") {
        for (const auto &element : pdata->datas_) {
          auto id = std::static_pointer_cast<XStreamUint32>(element);
          LOGD << "disappeared id " << id->value;
        }
      }
    }
    face_res_f << std::endl;
    body_res_f << std::endl;
  }
  face_res_f.close();
  body_res_f.close();
  return 0;
}
