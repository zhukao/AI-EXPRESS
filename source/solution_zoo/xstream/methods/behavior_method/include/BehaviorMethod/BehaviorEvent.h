/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: BehaviorEvent.h
 * @Brief: declaration of the BehaviorEvent
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-06-02 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-06-02 16:01:54
 */

#ifndef BEHAVIORMETHOD_BEHAVIOREVENT_H_
#define BEHAVIORMETHOD_BEHAVIOREVENT_H_

#include <map>
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <unordered_map>
#include "hobotxstream/method.h"
#include "json/json.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

enum class Behavior {
  RAISE_HAND,  // 举手
  STAND,       // 站立
  SQUAT        // 蹲下
};

const std::map<std::string, Behavior> g_behavior_map = {
    {"raise_hand", Behavior::RAISE_HAND},
    {"stand", Behavior::STAND},
    {"squat", Behavior::SQUAT}};

class BehaviorEvent {
 public:
  BehaviorEvent() {}
  virtual ~BehaviorEvent() {}

  virtual int Init(const Json::Value &config);

  std::vector<std::vector<BaseDataPtr>> Process(
      const std::vector<std::vector<BaseDataPtr> > &input,
      const std::vector<xstream::InputParamPtr> &param);

  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                      std::vector<BaseDataPtr> &frame_output);

  virtual bool IsEvent(hobot::vision::Landmarks kps) = 0;

  // 计算夹角
  float CalculateAngle(hobot::vision::Point pSrc,
                 hobot::vision::Point p1,
                 hobot::vision::Point p2);
  // 计算倾斜角度
  float CalculateSlope(float y, float x);

  // 滑动窗口
  void AdjustQueue(const XStreamData<
                       hobot::vision::Attribute<int32_t>> &vote_info,
                   uint32_t track_id);
  // 投票
  void Vote(std::shared_ptr<XStreamData<
                hobot::vision::Attribute<int32_t>>> &vote_info_ptr,
            uint32_t track_id);

 public:
  const float PI = 3.14159265f;
  // 滑动窗口参数
  int max_slide_window_size_ = 10;
  float behavior_voting_threshold_ = 0.5;
  std::unordered_map<uint32_t, std::deque<xstream::XStreamData<
      hobot::vision::Attribute<int32_t>>>> slide_window_map_;  // key:tarck_id
};
}  // namespace xstream

#endif  // BEHAVIORMETHOD_BEHAVIOREVENT_H_
