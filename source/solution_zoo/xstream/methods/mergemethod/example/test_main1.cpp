/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Merge Method
 * @author    ruoting.ding, chao.yang
 * @email     ruoting.ding@horizon.ai, chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.21
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<uint32_t> XStreamUint32;

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;

namespace {
void Dump(const xstream::OutputDataPtr &output) {
  using xstream::BaseDataVector;
  std::cout << "======================" << std::endl;
  std::cout << "seq: " << output->sequence_id_ << std::endl;
  std::cout << "error_code: " << output->error_code_ << std::endl;
  std::cout << "error_detail_: " << output->error_detail_ << std::endl;
  std::cout << "datas_ size: " << output->datas_.size() << std::endl;

  assert(output->datas_.size() >= 3);

  std::vector<int> face_id, head_id, disappeared_id;
  for (auto i = 0; i < 2; i++) {
    auto &data = output->datas_[i];
    if (data->error_code_ < 0) {
      std::cout << "data error: " << data->error_code_ << std::endl;
      continue;
    }
    std::cout << "data type_name : " << data->type_ << " " << data->name_
              << std::endl;
    auto pdata = std::static_pointer_cast<BaseDataVector>(data);
    if (!pdata->datas_.empty()) {
      for (const auto &element : pdata->datas_) {
        auto bbox = std::static_pointer_cast<XStreamBBox>(element);
        if (pdata->name_ == "merged_face_box") {
          face_id.emplace_back(bbox->value.id);
        } else if (pdata->name_ == "merged_head_box") {
          head_id.emplace_back(bbox->value.id);
        }
      }
    }
  }

  auto &data = output->datas_[2];
  if (data->error_code_ < 0) {
    std::cout << "data error: " << data->error_code_ << std::endl;
    return;
  }
  std::cout << "data type_name : " << data->type_ << " " << data->name_
            << std::endl;
  auto pdata = std::static_pointer_cast<BaseDataVector>(data);
  if (!pdata->datas_.empty()) {
    for (const auto &element : pdata->datas_) {
      auto num = std::static_pointer_cast<XStreamUint32>(element);
      disappeared_id.emplace_back(num->value);
    }
  }
  if (!face_id.empty()) {
    std::cout << " face:";
    for (const auto &id : face_id) {
      std::cout << " [" << id << "]";
    }
  }
  if (!head_id.empty()) {
    std::cout << " head:";
    for (const auto &id : head_id) {
      std::cout << " [" << id << "]";
    }
  }
  if (!disappeared_id.empty()) {
    std::cout << " disappeared id:";
    for (const auto &id : disappeared_id) {
      std::cout << " [" << id << "]";
    }
  }
  if (!head_id.empty() || !face_id.empty()) {
    std::cout << std::endl;
  }
}

std::unordered_map<int, int> face_disappeared_count_;
std::unordered_map<int, int> head_disappeared_count_;

void Predict(xstream::XStreamSDK *flow, const std::string &result_file) {
  std::ifstream in(result_file);
  if (!in.is_open()) {
    std::cout << "failed to open " << result_file << std::endl;
    return;
  }
  std::string file;
  int total_count = 100;
  if (in) {
    while (getline(in, file)) {
      total_count--;
      if (total_count < 0) {
        break;
      }
      std::istringstream iss(file);
      std::vector<std::string> elements;
      std::string tmp;
      int tmp_seq = 0;
      while (std::getline(iss, tmp, ' ')) {
        if (tmp_seq > 0) {
          elements.emplace_back(tmp);
        }
        tmp_seq++;
      }
      int size = elements.size() / 6;
      if (size < 1) {
        continue;
      }
      InputDataPtr input(new InputData());
      auto face_box_list = std::make_shared<BaseDataVector>();
      face_box_list->name_ = "face_box";
      input->datas_.push_back(BaseDataPtr(face_box_list));

      auto disappeared_face_id_list = std::make_shared<BaseDataVector>();
      disappeared_face_id_list->name_ = "disappeared_face_id";
      input->datas_.push_back(BaseDataPtr(disappeared_face_id_list));

      auto head_box_list = std::make_shared<BaseDataVector>();
      head_box_list->name_ = "head_box";
      input->datas_.push_back(BaseDataPtr(head_box_list));

      auto disappeared_head_id_list = std::make_shared<BaseDataVector>();
      disappeared_head_id_list->name_ = "disappeared_head_id";
      input->datas_.push_back(BaseDataPtr(disappeared_head_id_list));

      for (auto i = 0; i < size; ++i) {
        auto box = std::make_shared<XStreamBBox>();
        box->type_ = "BBox";
        box->value.id = atof(elements[i * 6 + 0].c_str());
        box->value.x1 = atof(elements[i * 6 + 2].c_str());
        box->value.y1 = atof(elements[i * 6 + 3].c_str());
        box->value.x2 = atof(elements[i * 6 + 4].c_str());
        box->value.y2 = atof(elements[i * 6 + 5].c_str());

        if (box->value.id >= 255) {
          return;
        }
        if (elements[i * 6 + 1] == "1") {  // Head
          head_box_list->datas_.emplace_back(BaseDataPtr(box));
          auto &id = box->value.id;
          auto itr = head_disappeared_count_.find(id);
          if (itr != head_disappeared_count_.end()) {
            itr->second = 0;
          } else {
            head_disappeared_count_.emplace(id, 0);
          }
        } else {  // Face
          face_box_list->datas_.emplace_back(BaseDataPtr(box));
          auto &id = box->value.id;
          auto itr = face_disappeared_count_.find(id);
          if (itr != face_disappeared_count_.end()) {
            itr->second = 0;
          } else {
            face_disappeared_count_.emplace(id, 0);
          }
        }
      }  // get all rects
      for (auto &pair : face_disappeared_count_) {
        pair.second++;
        if (pair.second == 10) {
          auto track_id = std::make_shared<XStreamUint32>();
          track_id->value = pair.first;
          disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(track_id));
        }
      }
      for (auto &pair : head_disappeared_count_) {
        pair.second++;
        if (pair.second == 10) {
          auto track_id = std::make_shared<XStreamUint32>();
          track_id->value = pair.first;
          disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(track_id));
        }
      }
      auto out = flow->SyncPredict(input);
      Dump(out);
    }  // a frame result
  }    // get all frame results
}
}  // end of namespace

int main(int argc, char const *argv[]) {
  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", "./config/face_head.json");
  flow->Init();
  Predict(flow, "./data/ipc_dump_tracking.txt");
  // 初始化sdk
  delete flow;
  return 0;
}
