/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-24 20:30:16
 * @Version: v0.0.1
 * @Brief: Rorder on different source + dispatch.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 02:12:06
 */

#ifndef HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_REORDERDISPATCHMODULE_H_
#define HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_REORDERDISPATCHMODULE_H_

#include <vector>
#include <list>
#include <string>
#include <memory>

#include "hobot/hobot.h"
#include "base_modules/DispatchModule.h"
#include "BaseMessage.h"

namespace xstream {
class ReorderDispatchModule : public DispatchModule {
 public:
  explicit ReorderDispatchModule(uint32_t source_num,
                                 std::string instance_name = "")
      : DispatchModule(instance_name, "xstream::ReorderDispatchModule") {
    sponge_list_.resize(source_num);
  }
  /**
   * Input Message:
   *  0: InputParam, type: XStreamInputParamMessage
   *  1- : Node inputs, type:XStreamInputMessage
   * Output Message:
   *  0: PassThrough Message,type:XStreamMethodOutputMessage
   *  1- : MethodModule.size() * Method Input Message,
   * type:XStreamMethodInputMessage
   */
  FORWARD_DECLARE(ReorderDispatchModule, 0);

  void SetReorder(bool on) {
    is_reorder_ = on;
  }

 private:
  struct PackageInput {
    std::shared_ptr<XStreamInputParamMessage> input_param_;
    std::vector<std::shared_ptr<XStreamInputMessage>> inputs_;
    uint64_t sequence_id_;
    uint32_t source_id_;
    std::vector<BaseDataPtr> GetInputs() const {
      std::vector<BaseDataPtr> ret_vec;
      for (size_t i = 0; i < inputs_.size(); ++i) {
        ret_vec.push_back(inputs_[i]->input_);
      }
      return ret_vec;
    }
    InputParamPtr GetParam() const {
      return input_param_->input_param_;
    }
    uint32_t GetSourceId() const {
      return source_id_;
    }
    uint64_t GetSequence_id() const {
      return sequence_id_;
    }
  };
  using PackageInputPtr = std::shared_ptr<PackageInput>;
  class Sponge {
   public:
    explicit Sponge(int16_t max_size = 128) : cache_max_size_(max_size) {}
    ~Sponge() = default;
    bool Sop(const PackageInputPtr &data, std::list<PackageInputPtr> *ready);
   private:
    uint16_t cache_max_size_;
    uint64_t expected_sequence_id_{0};
    std::list<PackageInputPtr> cache_list_;
  };
  std::vector<Sponge> sponge_list_;
  bool is_reorder_{false};
};
}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_BASE_MODULES_REORDERDISPATCHMODULE_H_
