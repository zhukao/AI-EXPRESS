//
// Created by kai01.han on 6/1/19.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef MOT_MODULE_H
#define MOT_MODULE_H

#include <vector>
#include <memory>
#include "horizon/vision_type/vision_type.hpp"
#include "hobot/hobot.h"

namespace hobot {
namespace iou_mot {
class Tracker;

class MultipleObjectTrackingModule : public hobot::Module {
 public:
//  explicit MultipleObjectTrackingModule(std::string instance_name = "")
//  : hobot::Module(instance_name, "MultipleObjectTrackingModule") { }
//
//  ~MultipleObjectTrackingModule() override;
  MultipleObjectTrackingModule() : Module ("", "MultipleObjectTracking"){};
  ~MultipleObjectTrackingModule() {};

  int Init(RunContext *context) override;

  void Reset()  {};

  FORWARD_DECLARE(MultipleObjectTrackingModule, 0);

 private:
  Tracker* tracker_;
};

class TimeStampMessage : public hobot::Message {
 public:
  TimeStampMessage() {}
  ~TimeStampMessage() {}
  time_t GetTimeStamp() { return time_stamp_; }
  void SetTimeStamp(const time_t &time) {time_stamp_ = time;}
 private:
  time_t time_stamp_ = 0;
};
typedef std::shared_ptr<TimeStampMessage> spTimeStampMessage;

class ImgInfoMessage :public hobot::Message{
 public:
  ImgInfoMessage() {}
  int img_width_;
  int img_height_;
};
typedef std::shared_ptr<ImgInfoMessage> sp_ImgInfoMessage;

}  // namespace iou_mot
}  // namespace hobot

#endif //MOT_MODULE_H
