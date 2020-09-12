/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PostProcessorFactory.cc
 * @Brief: definition of the PostProcessorFactory
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-22 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-22 16:19:08
 */

#ifndef POSTPROCESSMETHOD_POSTPROCESSOR_POSTPROCESSORFACTORY_H_
#define POSTPROCESSMETHOD_POSTPROCESSOR_POSTPROCESSORFACTORY_H_

#include "PostProcessMethod/PostProcessor/PostProcessor.h"
#include "PostProcessMethod/PostProcessor/DetectPostProcessor.h"
#include "PostProcessMethod/PostProcessConst.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

class PostProcessorFactory {
 public:
  static PostProcessor* GetPostProcessorFactory(PostProcessType post_type) {
    switch (post_type) {
      case PostProcessType::DETECT:
        return new DetectPostProcessor();
      default: {
         HOBOT_CHECK(false) << "Invalid predictor type";
         return nullptr;
      }
    }
    return nullptr;
  }
};

}  // namespace xstream
#endif  // POSTPROCESSMETHOD_POSTPROCESSOR_POSTPROCESSORFACTORY_H_
