/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-25 00:24:42
 * @Version: v0.0.1
 * @Brief: Reorder + disaptch module impl.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 06:16:12
 */

#include "base_modules/ReorderDispatchModule.h"
#include <list>
#include <memory>
#include "BaseMessage.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {
bool ReorderDispatchModule::Sponge::Sop(const PackageInputPtr &data,
                                        std::list<PackageInputPtr> *ready) {
  ready->clear();
  if (expected_sequence_id_ == data->sequence_id_) {
    expected_sequence_id_ = data->sequence_id_ + 1;
    ready->push_back(data);
    while (!cache_list_.empty()) {
      auto front = cache_list_.front();
      if (front->sequence_id_ == expected_sequence_id_) {
        ready->emplace_back(front);
        cache_list_.pop_front();
        ++expected_sequence_id_;
      } else if ((front->sequence_id_ < expected_sequence_id_ &&
                  (expected_sequence_id_ - front->sequence_id_ <
                   cache_max_size_)) ||
                 ((front->sequence_id_ > expected_sequence_id_) &&
                  (front->sequence_id_ - expected_sequence_id_ >
                   0xFFFFFFFFFFF0000))) {
        // when expected_sequence_id_ == curr data->sequence_id_, drop
        // all old frame:
        // 1. cache seqid < expected_sequence_id && distance smaller
        //    than cache_max_size_;
        // 2. cache seqid > expected_sequence_id && distance larger than
        //    0xFFFFFFFFFFF0000, which happens on sequence id overflow.
        ready->emplace_back(front);
        cache_list_.pop_front();
      } else {
        break;
      }
    }
    return true;
  } else {
    if (expected_sequence_id_ > data->sequence_id_) {
      auto res = expected_sequence_id_ - data->sequence_id_;
      if (res > 0xFFFFFFFFFFF0000) {
        // cache
        auto it = cache_list_.begin();
        for (; it != cache_list_.end(); it++) {
          if ((*it)->sequence_id_ < 0xFFFF &&
              (*it)->sequence_id_ > data->sequence_id_) {
            break;
          }
        }
        cache_list_.insert(it, data);
        if (cache_list_.size() > cache_max_size_) {
          ready->swap(cache_list_);
          expected_sequence_id_ = (*ready->rbegin())->sequence_id_ + 1;
        }
        return true;
      }
    } else {
      auto res = data->sequence_id_ - expected_sequence_id_;
      if (res < 0xFFFF) {
        // cache
        auto it = cache_list_.begin();
        for (; it != cache_list_.end(); it++) {
          if ((*it)->sequence_id_ > data->sequence_id_) {
            break;
          }
        }
        cache_list_.insert(it, data);
        if (cache_list_.size() > cache_max_size_) {
          ready->swap(cache_list_);
          expected_sequence_id_ = (*ready->rbegin())->sequence_id_ + 1;
        }
        return true;
      }
    }
    return false;
  }
  return false;
}

FORWARD_DEFINE(ReorderDispatchModule, 0) {
  auto package_input = std::make_shared<PackageInput>();
  package_input->input_param_ =
      std::static_pointer_cast<XStreamInputParamMessage>((*input[0])[0]);
  // broadcast INIT message to all method modules.
  if (package_input->input_param_->msg_type_ == XStreamMsgType::INIT) {
    auto xstream_input = std::make_shared<XStreamMethodInputMessage>();
    xstream_input->SyncXStreamData(package_input->input_param_);
    for (int i = 0; i < inst_num_; ++i) {
      workflow->Return(this, i + 1, hobot::spMessage(xstream_input), context);
    }
    return;
  }

  SourceContext src_ctx;
  for (size_t i = 1; i < input.size(); i++) {
    auto in_i = std::static_pointer_cast<XStreamInputMessage>((*input[i])[0]);
    package_input->inputs_.push_back(in_i);
    HOBOT_CHECK(in_i->feed_id == package_input->input_param_->feed_id);
    if (i == 1) {
      src_ctx = in_i->src_ctx_;
    } else {
      HOBOT_CHECK(src_ctx.source_id_ == in_i->src_ctx_.source_id_ &&
                  src_ctx.seq_id_ == in_i->src_ctx_.seq_id_);
    }
  }
  package_input->sequence_id_ = src_ctx.seq_id_;
  package_input->source_id_ = src_ctx.source_id_;

  std::list<PackageInputPtr> ready;
  if (is_reorder_) {
    if (!sponge_list_[src_ctx.source_id_].Sop(package_input, &ready)) {
      LOGW << "Sop failed, curr source_id" << src_ctx.source_id_
           << ", seqid=" << src_ctx.seq_id_;
      auto fake_res =
          FakeResult(package_input->GetInputs(), package_input->GetParam());
      fake_res->SyncXStreamData(package_input->input_param_);
      fake_res->feed_id = package_input->input_param_->feed_id;
      workflow->Return(this, 0, hobot::spMessage(fake_res), context);
      return;
    }
  } else {
    ready.emplace_back(package_input);
  }

  for (auto &package_input : ready) {
    auto inputs = package_input->GetInputs();
    auto param = package_input->GetParam();
    SourceContext src_ctx(package_input->source_id_,
                          package_input->sequence_id_);
    if (IsNeedSkip(inputs, param)) {
      auto fake_res = FakeResult(inputs, param);
      fake_res->SyncXStreamData(package_input->input_param_);
      fake_res->feed_id = package_input->input_param_->feed_id;
      workflow->Return(this, 0, hobot::spMessage(fake_res), context);
      continue;
    }
    auto xstream_input = std::make_shared<XStreamMethodInputMessage>();
    xstream_input->input_param_.push_back(param);
    xstream_input->data_list_.push_back(inputs);
    xstream_input->SyncXStreamData(package_input->input_param_);
    // sync feedid explicitly.
    xstream_input->feed_id = package_input->input_param_->feed_id;
    size_t source_id = src_ctx.source_id_;
    int select_idx = GetSelectInstanceIdx(&source_id);
    workflow->Return(this, select_idx + 1, hobot::spMessage(xstream_input),
                     context);
  }
}
}  // namespace xstream
