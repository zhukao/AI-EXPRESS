/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: BehaviorEvent.cpp
 * @Brief: definition of the BehaviorEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-06-02 14:27:05
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-06-02 16:17:08
 */

#include "BehaviorMethod/BehaviorEvent.h"
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int BehaviorEvent::Init(const Json::Value &config) {
  // 获取滑动窗口
  if (config.isMember("max_slide_window_size")) {
    max_slide_window_size_ = config["max_slide_window_size"].asInt();
    HOBOT_CHECK(max_slide_window_size_ > 0);
  }
  if (config.isMember("behavior_voting_threshold")) {
    behavior_voting_threshold_ = config["behavior_voting_threshold"].asFloat();
    HOBOT_CHECK(behavior_voting_threshold_ > 0);
  }
  return 0;
}

std::vector<std::vector<BaseDataPtr>> BehaviorEvent::Process(
      const std::vector<std::vector<BaseDataPtr> > &input,
      const std::vector<xstream::InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> output;
  HOBOT_CHECK(!input.empty());

  int batch_size = input.size();
  output.resize(batch_size);
  for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    auto &input_data = input[batch_idx];
    auto &output_data = output[batch_idx];

    RunSingleFrame(input_data, output_data);
  }
  return output;
}

void BehaviorEvent::RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                      std::vector<BaseDataPtr> &frame_output) {
  HOBOT_CHECK(frame_input.size() == 3);  // BBox, disappeard track_id and kps
  auto out_infos = std::make_shared<BaseDataVector>();
  frame_output.resize(1);  // behavior
  frame_output[0] = out_infos;

  std::vector<BaseDataPtr> boxes = std::static_pointer_cast<
      BaseDataVector>(frame_input[0])->datas_;
  std::vector<BaseDataPtr> disappeared_track_ids = std::static_pointer_cast<
      BaseDataVector>(frame_input[1])->datas_;
  std::vector<BaseDataPtr> kpses = std::static_pointer_cast<
      BaseDataVector>(frame_input[2])->datas_;
  if (boxes.size() != kpses.size()) {
    LOGE << "boxes.size(): " << boxes.size()
         << ", kpses.size(): " << kpses.size();
  }

  for (size_t i = 0; i < boxes.size(); ++i) {
    const auto &box = std::static_pointer_cast<
                          XStreamData<hobot::vision::BBox>>(boxes[i])->value;

    if (box.id < 0) {
      auto attribute_result = std::make_shared<XStreamData<
                                hobot::vision::Attribute<int32_t>>>();
      attribute_result->value.value = 0;
      out_infos->datas_.push_back(attribute_result);
      continue;
    }

    auto kps = std::static_pointer_cast<XStreamData<
                 hobot::vision::Landmarks>>(kpses[i]);
    XStreamData<hobot::vision::Attribute<int32_t>> vote_info;
    if (IsEvent(kps->value)) {
      vote_info.value.value = 1;  // TODO(zhe.sun) score
    } else {
      vote_info.value.value = 0;
    }
    uint32_t track_id = static_cast<uint32_t>(box.id);

    // adjust queue
    AdjustQueue(vote_info, track_id);

    // vote
    auto vote_info_ptr = std::make_shared<XStreamData<
                           hobot::vision::Attribute<int32_t>>>();
    Vote(vote_info_ptr, track_id);
    // output
    out_infos->datas_.push_back(vote_info_ptr);
  }

  // disappeared_track_ids
  for (const auto &data : disappeared_track_ids) {
    auto disappeared_track_id =
        std::static_pointer_cast<XStreamData<uint32_t>>(data)->value;
    auto iter = slide_window_map_.find(disappeared_track_id);
    if (iter != slide_window_map_.end()) {
      slide_window_map_.erase(iter);
    }
  }
}

void BehaviorEvent::AdjustQueue(
    const XStreamData<hobot::vision::Attribute<int32_t>> &vote_info,
    uint32_t track_id) {
  auto iter = slide_window_map_.find(track_id);
  if (iter == slide_window_map_.end()) {
      slide_window_map_[track_id].push_back(vote_info);
  } else {
    // 已有该track_id
    int queue_size = slide_window_map_[track_id].size();
    if (queue_size < max_slide_window_size_) {
      slide_window_map_[track_id].push_back(vote_info);
    } else if (queue_size == max_slide_window_size_) {
      slide_window_map_[track_id].pop_front();
      assert(slide_window_map_[track_id].size()
        == static_cast<std::size_t>(queue_size - 1));
      slide_window_map_[track_id].push_back(vote_info);
    } else {
      HOBOT_CHECK(0) << "impossible.";
    }
  }
}

void BehaviorEvent::Vote(
    std::shared_ptr<XStreamData<
        hobot::vision::Attribute<int32_t>>> &vote_info_ptr,
    uint32_t track_id) {
  auto iter = slide_window_map_.find(track_id);
  if (iter == slide_window_map_.end()) {
    vote_info_ptr->value.value = 0;  // 非任何行为事件
  } else {
    auto &slide_window = slide_window_map_[track_id];
    int window_size = slide_window.size();
    if (window_size < max_slide_window_size_) {
      vote_info_ptr->value.value = 0;
    } else if (window_size == max_slide_window_size_) {
      int behavior_num = 0;
      // 计算行为事件、非行为事件在滑动窗口中的数量
      for (const auto &behavior : slide_window) {
        if (behavior.value.value == 1) {
          ++behavior_num;
        }
      }
      float voting_ratio = static_cast<float>(behavior_num) /
                                static_cast<float>(max_slide_window_size_);
      if (voting_ratio > behavior_voting_threshold_) {
        vote_info_ptr->value.value = 1;
      } else {
        vote_info_ptr->value.value = 0;
      }
    } else {
      HOBOT_CHECK(0) << "impossible.";
    }
  }
}

float BehaviorEvent::CalculateAngle(hobot::vision::Point pSrc,
                              hobot::vision::Point p1,
                              hobot::vision::Point p2) {
  float angle = 0.0f;  // 夹角

  // 向量Vector a的(x, y)坐标
  float va_x = p1.x - pSrc.x;
  float va_y = p1.y - pSrc.y;

  // 向量b的(x, y)坐标
  float vb_x = p2.x - pSrc.x;
  float vb_y = p2.y - pSrc.y;

  float productValue = (va_x * vb_x) + (va_y * vb_y);   // 向量的乘积
  float va_val = std::sqrt(va_x * va_x + va_y * va_y);  // 向量a的模
  float vb_val = std::sqrt(vb_x * vb_x + vb_y * vb_y);  // 向量b的模
  float cosValue = productValue / (va_val * vb_val);    // 余弦公式

  // acos的输入参数范围必须在[-1, 1]之间
  if (cosValue < -1 && cosValue > -2) {
    cosValue = -1;
  } else if (cosValue > 1 && cosValue < 2) {
    cosValue = 1;
  }

  // acos返回的是弧度值，转换为角度值
  angle = std::acos(cosValue) * 180.0f / PI;
  return angle;
}

float BehaviorEvent::CalculateSlope(float y, float x) {
  if (x == 0) {
    return 90.0;
  }
  float tan = y / x;
  auto arctan = std::atan(std::abs(tan)) * 180.0f / PI;
  return arctan;
}

}  // namespace xstream
