/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-05 00:56:39
 * @Version: v0.0.1
 * @Brief: xstream base message definition.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 22:37:29
 */

#ifndef HOBOTXSTREAM_HOBOTSDK_BASEMESSAGE_H_
#define HOBOTXSTREAM_HOBOTSDK_BASEMESSAGE_H_

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "common/rw_mutex.h"
#include "hobot/hobot.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"

namespace xstream {
/// 记录单帧数据信息
struct FrameDataInfo {
  /// 每个slot的数据
  std::map<std::string, BaseDataPtr> outputs_;
  std::set<std::string> posted_out_;
  std::set<std::string> posted_node_;
  /// SDK Input透传数据
  const void *context_ = nullptr;
  /// 用于SDK等待同步结果的辅助字段
  void *sync_context_ = nullptr;
  /// 时间戳
  uint64_t timestamp_;
  /// 用来做reorder
  uint64_t sequence_id_;
  uint32_t source_id_ = 0;
  uint64_t global_sequence_id_ = 0;
  bool is_frame_done_{false};
  // lock outside.
  std::mutex lock_;
  FrameDataInfo() = default;
  void SetOutput(const std::string &name, BaseDataPtr data) {
    HOBOT_CHECK(outputs_.find(name) == outputs_.end())
        << "Produce duplicate output";
    outputs_[name] = data;
  }
  void DelData(const std::string &name) {
    if (outputs_.find(name) == outputs_.end()) {
      return;
    }
    outputs_.erase(name);
  }
};
using FrameDataInfoPtr = std::shared_ptr<FrameDataInfo>;
struct SourceContext {
  uint32_t source_id_;
  uint64_t seq_id_;
  explicit SourceContext(uint32_t src_id = 0, uint64_t seq_id = 0)
      : source_id_(src_id), seq_id_(seq_id) {}
};

enum class XStreamMsgType { NORMAL, INIT };

struct XStreamBaseMessage : public hobot::Message {
  SourceContext src_ctx_;
  FrameDataInfoPtr framedata_;
  XStreamMsgType msg_type_{XStreamMsgType::NORMAL};
  void SyncXStreamData(const std::shared_ptr<XStreamBaseMessage> &src) {
    src_ctx_ = src->src_ctx_;
    framedata_ = src->framedata_;
    msg_type_ = src->msg_type_;
  }
};
using XStreamBaseMessagePtr = std::shared_ptr<XStreamBaseMessage>;

struct XStreamInputMessage : public XStreamBaseMessage {
  BaseDataPtr input_;

  XStreamInputMessage() = default;
  explicit XStreamInputMessage(BaseDataPtr input) : input_(input) {}
};

struct XStreamInputParamMessage : public XStreamBaseMessage {
  InputParamPtr input_param_;
  // TODO(SONGSHAN.GONG): maybe input params also
  // need support multi-input source.

  XStreamInputParamMessage() = default;
  explicit XStreamInputParamMessage(InputParamPtr input_param)
      : input_param_(input_param) {}
};

struct XStreamMethodInputMessage : public XStreamBaseMessage {
 public:
  std::vector<std::vector<BaseDataPtr>> data_list_;
  std::vector<InputParamPtr> input_param_;

  XStreamMethodInputMessage() = default;
  explicit XStreamMethodInputMessage(
      const std::vector<std::vector<BaseDataPtr>> &data,
      const std::vector<InputParamPtr> &input_param)
      : data_list_(data), input_param_(input_param) {}
};

struct XStreamMethodOutputMessage : public XStreamBaseMessage {
  std::vector<std::vector<BaseDataPtr>> data_list_;
  XStreamMethodOutputMessage() = default;
  explicit XStreamMethodOutputMessage(
      const std::vector<std::vector<BaseDataPtr>> &data)
      : data_list_(data) {}
};
}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_BASEMESSAGE_H_
