/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: BehaviorMethod.h
 * @Brief: declaration of the BehaviorMethod
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-05-25 14:18:28
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-05-25 16:01:54
 */

#ifndef BEHAVIORMETHOD_BEHAVIORMETHOD_H_
#define BEHAVIORMETHOD_BEHAVIORMETHOD_H_

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "hobotxstream/method.h"
#include "BehaviorMethod/BehaviorEvent.h"
#include "json/json.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

class BehaviorMethod : public Method {
 public:
  BehaviorMethod() {}
  virtual ~BehaviorMethod() {}

  virtual int Init(const std::string &cfg_path);
  virtual void Finalize();

  virtual std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param);
  virtual int UpdateParameter(xstream::InputParamPtr ptr);
  virtual InputParamPtr GetParameter() const;
  virtual std::string GetVersion() const;
  virtual void OnProfilerChanged(bool on) {}

 private:
  Json::Value config_;
  std::shared_ptr<BehaviorEvent> behavior_event_;
};
}  // namespace xstream

#endif  // BEHAVIORMETHOD_BEHAVIORMETHOD_H_
