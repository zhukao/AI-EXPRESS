/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-24 21:15:12
 * @Version: v0.0.1
 * @Brief: default dispatch module implementation.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 06:15:07
 */

#include "base_modules/DefaultDispatchModule.h"

#include <memory>
#include <vector>

#include "BaseMessage.h"
#include "base_modules/DispatchModule.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {
FORWARD_DEFINE(DefaultDispatchModule, 0) {
  /**
   *  １.检查InputParam，输入是否需要passthrough,如果需要passthrough,把passthrough的结果通过output
   *  slot0发出去，然后module return;
   *  ２． 根据DisaptchStrategy分发输入消息．
   * */
  auto xstream_input = std::make_shared<XStreamMethodInputMessage>();
  auto in0 = std::static_pointer_cast<XStreamInputParamMessage>((*input[0])[0]);

  if (in0->msg_type_ == XStreamMsgType::INIT) {
    for (int i = 0; i < inst_num_; ++i) {
      xstream_input->SyncXStreamData(in0);
      workflow->Return(this, i + 1, hobot::spMessage(xstream_input), context);
    }
    return;
  }
  xstream_input->input_param_.push_back(in0->input_param_);
  SourceContext src_ctx;
  for (int batch_size = 0; batch_size < 1; batch_size++) {
    std::vector<BaseDataPtr> in_vec;
    for (size_t i = 1; i < input.size(); i++) {
      auto in_i = std::static_pointer_cast<XStreamInputMessage>((*input[i])[0]);
      in_vec.push_back(in_i->input_);
      HOBOT_CHECK(in_i->feed_id == in0->feed_id);
      if (i == 1) {
        src_ctx = in_i->src_ctx_;
      } else {
        HOBOT_CHECK(src_ctx.source_id_ == in_i->src_ctx_.source_id_ &&
                    src_ctx.seq_id_ == in_i->src_ctx_.seq_id_);
      }
    }
    xstream_input->data_list_.push_back(in_vec);
  }

  // passthrough support.
  auto inputs = xstream_input->data_list_[0];
  auto param = xstream_input->input_param_[0];
  if (IsNeedSkip(inputs, param)) {
    auto fake_res = FakeResult(inputs, param);
    fake_res->SyncXStreamData(in0);
    workflow->Return(this, 0, hobot::spMessage(fake_res), context);
    return;
  }
  // Dispatch to MethodModules.
  xstream_input->SyncXStreamData(in0);
  int select_idx = GetSelectInstanceIdx();
  workflow->Return(this, select_idx + 1, hobot::spMessage(xstream_input),
                   context);
}
}  // namespace xstream
