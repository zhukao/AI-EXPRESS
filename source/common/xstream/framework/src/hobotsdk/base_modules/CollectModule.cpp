/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-12 01:30:21
 * @Version: v0.0.1
 * @Brief: CollectModule　implementation.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 02:44:38
 */

#include "base_modules/CollectModule.h"
#include <memory>
#include "BaseMessage.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

FORWARD_DEFINE(CollectModule, 0) {
  LOGI << "enter CollectModule";
  /**
   * 1.分拆打包的结果，分发到对应output slot
   */
  for (size_t i = 0; i < input.size(); ++i) {
    for (size_t j = 0; j < input[i]->size(); j++) {
      auto in_msg =
          std::static_pointer_cast<XStreamMethodOutputMessage>((*input[i])[j]);
      for (size_t batch_size = 0; batch_size < 1; batch_size++) {
        auto outputs = in_msg->data_list_[batch_size];
        for (size_t output_slot = 0; output_slot < outputs.size();
             output_slot++) {
          auto out_msg =
              std::make_shared<XStreamInputMessage>(outputs[output_slot]);
          out_msg->SyncXStreamData(in_msg);
          workflow->Return(this, output_slot, hobot::spMessage(out_msg),
                           context);
        }  // for (int output_slot=...)
      }    // for (size_t batch_size=...)
    }      // for (size_t j = ...)
  }        // for (size_t i = ...)
}
}  // namespace xstream
