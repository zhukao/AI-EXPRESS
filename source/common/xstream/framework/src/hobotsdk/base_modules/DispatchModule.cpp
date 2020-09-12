/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-11 21:39:34
 * @Version: v0.0.1
 * @Brief: DispatchModule implemenation.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 22:08:25
 */
#include <vector>
#include <memory>
#include "base_modules/DispatchModule.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/hobotsdk/BaseMessage.h"

namespace xstream {
bool DispatchModule::IsNeedSkip(const std::vector<BaseDataPtr> &inputs,
                                const InputParamPtr &param) {
  bool need_skip = false;
  do {
    // filter by input data state
    for (auto &input : inputs) {
      if (!input || 0 != input->error_code_) {
        need_skip = true;
        break;
      }
    }
    // filter by parameter
    if (param && !param->is_enable_this_method_) {
      need_skip = true;
      break;
    }
  } while (0);

  return need_skip;
}

std::shared_ptr<XStreamMethodOutputMessage> DispatchModule::FakeResult(
    const std::vector<BaseDataPtr> &inputs, const InputParamPtr &input_param) {
  LOGV << "skip " << unique_name_;
  auto res = std::make_shared<XStreamMethodOutputMessage>();
  std::vector<BaseDataPtr> outputs;
  outputs.resize(output_slot_num_);

  if (input_param && !input_param->is_enable_this_method_) {
    auto param = dynamic_cast<DisableParam *>(input_param.get());
    if (param) {
      switch (param->mode_) {
        case DisableParam::Mode::PassThrough: {
          if (inputs.size() == output_slot_num_) {
            for (uint32_t i = 0; i < output_slot_num_; ++i) {
              outputs[i] = inputs[i];
            }
          } else {
            LOGE << unique_name_
                 << " : input slot size is not equal to the output data , skip "
                    "data pass through ";
          }
          break;
        }
        case DisableParam::Mode::BestEffortPassThrough: {
          for (uint32_t i = 0; i < output_slot_num_; ++i) {
            if (inputs.size() > i) {
              outputs[i] = inputs[i];
            } else {
              BaseDataPtr invalid_data(new BaseData());
              invalid_data->state_ = DataState::INVALID;
              outputs[i] = invalid_data;
            }
          }
          break;
        }
        case DisableParam::Mode::UsePreDefine: {
          if (param->pre_datas_.size() == output_slot_num_) {
            for (uint32_t i = 0; i < output_slot_num_; ++i) {
              outputs[i] = param->pre_datas_[i];
            }
          } else {
            LOGI << unique_name_ << " : predefine data size is "
                 << param->pre_datas_.size()
                 << ", while method output data size is " << output_slot_num_
                 << ", skip data pass through ";
          }
          break;
        }
        case DisableParam::Mode::Invalid: {
          for (uint32_t i = 0; i < output_slot_num_; ++i) {
            BaseDataPtr invalid_data(new BaseData());
            invalid_data->state_ = DataState::INVALID;
            outputs[i] = invalid_data;
          }
          break;
        }
      }
    }
  }
  res->data_list_.push_back(outputs);
  return res;
}

int DispatchModule::GetSelectInstanceIdx(void *key) {
  int select_idx = 0;
  switch (stgy_) {
    case DispatchStrategy::ROUND_ROBIN: {
      if (cur_post_pos_ >= inst_num_) {
        cur_post_pos_ = 0;
      }
      select_idx = cur_post_pos_++;
      break;
    }
    case DispatchStrategy::KEY_MATCHING: {
      if (key == nullptr) {
        // return error, can not post the task since no key is inputed
        LOGE << "key is Null in KEY_MATCHING mode.";
        return -1;
      }
    //  LOGI << GetFullInstanceName()
    //       << ", match key = " << *(static_cast<size_t*>(key));
      select_idx = key_match_func_(key);
      if (select_idx < 0) {
        LOGE << "can not fine valid key matching index.";
        return -1;
      }
      HOBOT_CHECK(select_idx < inst_num_)
        << "key matching: overflow select index:" << select_idx;
      break;
    }
    default: {
      // not support other post strategy currently.
      HOBOT_CHECK(false) << "Doesn't support this " << (uint16_t)stgy_
                         << " post strategy currently";
    }
  }
  return select_idx;
}
}  // namespace xstream
