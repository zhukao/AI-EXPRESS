/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-06 22:50:22
 * @Version: V0.0.1
 * @Brief: scheduler reimplementation based hobotsdk.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 03:49:59
 */

#include "schedulerV2.h"

#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BaseMessage.h"
#include "hobotxstream/profiler.h"
#include "hobotxstream/json_key.h"

namespace xstream {

int SchedulerV2::Init(XStreamConfigPtr config,
                      ProfilerPtr profiler) {
  if (is_init_) {
    LOGE << "Scheduler already Init";
    return -2;
  }
  LOGD << "Scheduler Init";
  profiler_ = profiler;
  scheduler_config_ = std::make_shared<SchedulerConfig>(config);

  int ret = scheduler_config_->CheckConfigValid();
  if (0 != ret) {
    return ret;
  }

  engine_.reset(hobot::Engine::NewInstance());
  workflow_.reset(engine_->NewWorkflow());

  if (0 != CreateNodes()) {
    LOGE << "CreateNodes failed";
    return -1;
  }
  sequence_id_list_.clear();
  for (size_t i = 0; i < scheduler_config_->GetSourceNumber(); ++i)
    sequence_id_list_.push_back(std::make_shared<std::atomic_ullong>(0));

  is_init_ = true;
  return 0;
}

SchedulerV2::~SchedulerV2() {
#if 1
  if (workflow_) {
    workflow_->Reset();
  }
  //   HOBOT_CHECK(false) << "destory";
  engine_.reset(nullptr);
#endif
}

int SchedulerV2::SetCallback(XStreamCallback callback) {
  callback_ = callback;
  return 0;
}

int SchedulerV2::SetCallback(XStreamCallback callback,
                             const std::string &name) {
  auto node_ptr_i = name2ptr_.find(name);
  if (node_ptr_i == name2ptr_.end()) {
    LOGE << "Failed to find " << name << " in the configs";
    return HOBOTXSTREAM_ERROR_INVALID_PARAM;
  } else {
    if (callback) {
      LOGI << "Set callback for " << name;
      node_callbacks_[node_ptr_i->second] = callback;
    } else {
      LOGI << "Unset callback for " << name;
      node_callbacks_.erase(node_ptr_i->second);
    }
    return 0;
  }
}

int SchedulerV2::SetFreeMemery(bool is_enable) { return 0; }
int SchedulerV2::UpdateConfig(std::string unique_name,
                              InputParamPtr param_ptr) {
  auto iter = name2ptr_.find(unique_name);
  if (iter != name2ptr_.end()) {
    return iter->second->SetParameter(param_ptr);
  }
  return 1;
}

InputParamPtr SchedulerV2::GetConfig(const std::string &unique_name) const {
  auto iter = name2ptr_.find(unique_name);
  if (iter != name2ptr_.end()) {
    return iter->second->GetParameter();
  }
  return nullptr;
}

std::string SchedulerV2::GetVersion(const std::string &unique_name) const {
  auto iter = name2ptr_.find(unique_name);
  if (iter != name2ptr_.end()) {
    return iter->second->GetVersion();
  }
  return std::string{};
}

bool SchedulerV2::IsSingleOutputDone(const FrameDataInfoPtr &framedata,
                                     const std::string &output_type) const {
  // 判断该路输出是否存在
  auto itr = output_type_names_.find(output_type);
  HOBOT_CHECK(itr != output_type_names_.end())
      << "failed to find " << output_type << " in the config";

  bool is_output_ready = true;
  auto single_outputs = itr->second;
  for (auto output_name : single_outputs) {
    if (!framedata->outputs_.count(output_name)) {
      is_output_ready = false;
      break;
    }
  }
  return is_output_ready;
}

bool SchedulerV2::IsFrameDone(const FrameDataInfoPtr &framedata) const {
  bool is_ready = true;
  for (auto &output_name : xstream_output_names_) {
    if (!framedata->outputs_.count(output_name)) {
      is_ready = false;
      break;
    }
  }

  return is_ready;
}
bool SchedulerV2::IsNodeReady(const FrameDataInfoPtr &framedata,
                              const std::string &node_name) {
  bool is_ready = true;
  auto node = name2ptr_[node_name];
  for (auto slot_id : node_output_slots_[node]) {
    auto slot_name = data_slot_names_[slot_id];
    if (!framedata->outputs_.count(slot_name)) {
      is_ready = false;
      break;
    }
  }
  return is_ready;
}

/// 将用户输入数据转换成框架数据
int64_t SchedulerV2::Input(InputDataPtr input, void *sync_context) {
  RUN_FPS_PROFILER_WITH_PROFILER(profiler_, "workflow input")
  auto framedata = std::make_shared<FrameDataInfo>();
  framedata->context_ = input->context_;
  framedata->sync_context_ = sync_context;
  framedata->global_sequence_id_ = global_sequence_id_++;
  framedata->sequence_id_ = (*sequence_id_list_[input->source_id_])++;
  framedata->timestamp_ = framedata->sequence_id_;
  HOBOT_CHECK(input->source_id_ < scheduler_config_->GetSourceNumber())
      << "source id " << input->source_id_ << " is out of range (0-"
      << scheduler_config_->GetSourceNumber() - 1;
  framedata->source_id_ = input->source_id_;
  std::vector<std::tuple<hobot::Module *, int, int, hobot::spMessage>>
      feed_messages;
  for (const auto &base_data : input->datas_) {
    HOBOT_CHECK(xstream_input_names_.count(base_data->name_))
        << "failed to find " << base_data->name_ << " in the config";
    hobot::spMessage in_msg(new XStreamInputMessage(base_data));
    {
      auto real_msg = static_cast<XStreamBaseMessage *>(in_msg.get());
      real_msg->src_ctx_ =
          SourceContext(framedata->source_id_, framedata->sequence_id_);
      real_msg->framedata_ = framedata;
    }
    auto dep_nodes = slot_infos_[data_slots_[base_data->name_]].dep_nodes_;
    for (auto &node_entry : dep_nodes) {
      feed_messages.push_back(
          std::make_tuple(node_entry.first->GetInputModule(), 0,
                          node_entry.second + 1, in_msg));
    }
    auto iter = std::find(xstream_output_names_.begin(),
                          xstream_output_names_.end(), base_data->name_);
    if (iter != xstream_output_names_.end()) {
      framedata->SetOutput(base_data->name_, base_data);
    }
  }

  std::unordered_map<std::string, InputParamPtr> method_params;
  for (const auto &param : input->params_) {
    method_params[param->unique_name_] = param;
  }

  for (const auto &nodeName : scheduler_config_->GetNodesName()) {
    auto node = name2ptr_[nodeName];
    auto iter = method_params.find(nodeName);
    if (iter != method_params.end()) {
      hobot::spMessage in_msg(new XStreamInputParamMessage(iter->second));
      {
        auto real_msg = static_cast<XStreamBaseMessage *>(in_msg.get());
        real_msg->src_ctx_ =
            SourceContext(framedata->source_id_, framedata->sequence_id_);
        real_msg->framedata_ = framedata;
      }
      feed_messages.push_back(
          std::make_tuple(node->GetInputModule(), 0, 0, in_msg));
    } else {
      hobot::spMessage null_msg(new XStreamInputParamMessage(nullptr));
      {
        auto real_msg = static_cast<XStreamBaseMessage *>(null_msg.get());
        real_msg->src_ctx_ =
            SourceContext(framedata->source_id_, framedata->sequence_id_);
        real_msg->framedata_ = framedata;
      }
      feed_messages.push_back(
          std::make_tuple(node->GetInputModule(), 0, 0, null_msg));
    }
  }

  if (!IsFrameDone(framedata)) {
    workflow_->Feed(run_task_, feed_messages, framedata->global_sequence_id_);
  }

  OnResult(framedata, "__input__");

  return framedata->sequence_id_;
}
OutputDataPtr SchedulerV2::SingleOutput(FrameDataInfoPtr frame_data,
                                        const std::string &output_type) {
  // RUN_FPS_PROFILER(output_type + " output")
  OutputDataPtr result(new OutputData());
  result->context_ = frame_data->context_;
  result->sequence_id_ = frame_data->sequence_id_;
  result->global_sequence_id_ = frame_data->global_sequence_id_;
  result->source_id_ = frame_data->source_id_;
  result->error_code_ = 0;
  result->output_type_ = output_type;
  for (const auto &name : output_type_names_[output_type]) {
    if (frame_data->outputs_.find(name) == frame_data->outputs_.end()) {
      result->error_code_ += HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY;
      result->error_detail_ += (name + " is not ready;");
      continue;
    }
    auto &output_basedata = frame_data->outputs_[name];
    if (!output_basedata) {
      result->error_code_ += HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY;
      result->error_detail_ += (name + " is not ready;");
      continue;
    }
    output_basedata->name_ = name;
    result->datas_.push_back(output_basedata);
    result->error_code_ += output_basedata->error_code_;
    result->error_detail_ += output_basedata->error_detail_;
  }
  return result;
}
std::vector<OutputDataPtr> SchedulerV2::MultipleOutput(
    FrameDataInfoPtr frame_data) {
  // RUN_FPS_PROFILER("workflow output")
  std::vector<OutputDataPtr> multiple_result;
  for (auto single_output : output_type_names_) {
    std::string output_type = single_output.first;
    OutputDataPtr result(new OutputData());
    result->context_ = frame_data->context_;
    result->sequence_id_ = frame_data->sequence_id_;
    result->global_sequence_id_ = frame_data->global_sequence_id_;
    result->source_id_ = frame_data->source_id_;
    result->error_code_ = 0;
    result->output_type_ = output_type;
    // 封装单路输出
    for (const auto &name : output_type_names_[output_type]) {
      if (frame_data->outputs_.find(name) == frame_data->outputs_.end()) {
        result->error_code_ += HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY;
        result->error_detail_ += (name + " is not ready;");
        continue;
      }
      auto &output_basedata = frame_data->outputs_[name];
      if (!output_basedata) {
        result->error_code_ += HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY;
        result->error_detail_ += (name + " is not ready;");
        continue;
      }
      output_basedata->name_ = name;
      result->datas_.push_back(output_basedata);
      result->error_code_ += output_basedata->error_code_;
      result->error_detail_ += output_basedata->error_detail_;
    }
    multiple_result.push_back(result);
  }
  return multiple_result;
}

int SchedulerV2::OutputMethodResult(FrameDataInfoPtr framedata,
                                    const std::string &node_name) {
  RUN_FPS_PROFILER_WITH_PROFILER(profiler_,
                             node_name + "method output")
  if (framedata->sync_context_ != nullptr) {
    return -1;
  } else {
    auto readyNode = name2ptr_[node_name];
    auto node_callback_i = node_callbacks_.find(readyNode);
    if (node_callbacks_.end() != node_callback_i) {
      auto &node_callback = node_callback_i->second;
      OutputDataPtr result(new OutputData());
      result->context_ = framedata->context_;
      result->sequence_id_ = framedata->sequence_id_;
      result->global_sequence_id_ = framedata->global_sequence_id_;
      result->source_id_ = framedata->source_id_;
      result->unique_name_ = node_name;
      result->output_type_ = kNodeOutputName;
      result->error_code_ = 0;
      auto out_slots = node_output_slots_[readyNode];
      for (auto slot : out_slots) {
        auto slot_name = data_slot_names_[slot];
        auto &output_basedata = framedata->outputs_[slot_name];
        if (!output_basedata) {
          result->error_code_ += HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY;
          result->error_detail_ += (slot_name + " is not ready;");
          continue;
        }
        output_basedata->name_ = slot_name;
        result->datas_.push_back(output_basedata);
        result->error_code_ += output_basedata->error_code_;
        result->error_detail_ += output_basedata->error_detail_;
      }
      node_callback(result);
    }
  }
  return 0;
}

std::set<std::string> SchedulerV2::GetNotPostedOutput(
    FrameDataInfoPtr framedata) {
  std::set<std::string> not_posted;
  for (auto item : node_callbacks_) {
    auto node_name = item.first->GetUniqueName();
    if (!framedata->posted_node_.count(node_name)) {
      for (auto slotid : node_output_slots_[item.first]) {
        not_posted.insert(data_slot_names_[slotid]);
      }
    }
  }

  for (auto &single_output : output_type_names_) {
    auto output_type = single_output.first;
    if (!framedata->posted_out_.count(output_type)) {
      for (auto &output_name : single_output.second) {
        not_posted.insert(output_name);
      }
    }
  }

  return not_posted;
}

void SchedulerV2::RemovePostedOutput(FrameDataInfoPtr framedata) {
  auto not_posted = GetNotPostedOutput(framedata);
  std::vector<std::string> unused;
  for (auto &item : framedata->outputs_) {
    if (!not_posted.count(item.first)) {
      unused.emplace_back(item.first);
    }
  }
  for (auto &item : unused) {
    framedata->outputs_.erase(item);
  }
}

void SchedulerV2::OnResult(FrameDataInfoPtr framedata,
                           const std::string &node_name) {
  bool is_posted = false;
  if (name2ptr_.count(node_name) && IsNodeReady(framedata, node_name)) {
    OutputMethodResult(framedata, node_name);
    framedata->posted_node_.insert(node_name);
    is_posted = true;
  }

  if (framedata->is_frame_done_) return;

  if (framedata->sync_context_ == nullptr) {
    for (auto &single_output : output_type_names_) {
      auto output_type = single_output.first;
      if (!framedata->posted_out_.count(output_type) &&
          IsSingleOutputDone(framedata, output_type)) {
        callback_(SingleOutput(framedata, output_type));
        framedata->posted_out_.insert(output_type);
        is_posted = true;
      }
    }
  }

  if (IsFrameDone(framedata)) {
    if (framedata->sync_context_ != nullptr) {
      auto promise = static_cast<std::promise<std::vector<OutputDataPtr>> *>(
          framedata->sync_context_);
      promise->set_value(MultipleOutput(framedata));
    }
    is_posted = true;
    framedata->is_frame_done_ = true;
    LOGD << "FRAME " << framedata->global_sequence_id_ << ", done";
  }
  if (is_posted) {
    RemovePostedOutput(framedata);
  }
}

bool SchedulerV2::IsObserveOutput(const std::string &node_name,
                                  const std::string &slot_name) {
  auto node = name2ptr_[node_name];
  if (node_callbacks_.find(node) != node_callbacks_.end()) {
    LOGD << "OBSERVE: NODE:" << node_name << ", slot_name:" << slot_name;
    return true;
  }
  if (std::find(xstream_output_names_.begin(), xstream_output_names_.end(),
                slot_name) != xstream_output_names_.end()) {
    LOGD << "OBSERVE:" << slot_name;
    return true;
  }
  LOGD << "NOT OBSERVE:" << slot_name;
  return false;
}
void SchedulerV2::ResultObserver::OnResult(hobot::Module *from, int output_slot,
                                           hobot::spMessage output) {
  auto instance_name = from->GetFullInstanceName();
  auto pos = instance_name.find(".");
  auto node_name = instance_name.substr(pos + 1);
  auto node = scheduler_->name2ptr_[node_name];
  auto slot_name =
      scheduler_
          ->data_slot_names_[scheduler_->node_output_slots_[node][output_slot]];
  if (!scheduler_->IsObserveOutput(node_name, slot_name)) {
    return;
  }
  auto out_msg = std::static_pointer_cast<XStreamInputMessage>(output);
  auto framedata = out_msg->framedata_;
  {
    std::lock_guard<std::mutex> lock(framedata->lock_);
    framedata->SetOutput(slot_name, out_msg->input_);
    scheduler_->OnResult(framedata, node_name);
  }
}

int SchedulerV2::CreateNodes() {
  auto node_runctx = std::make_shared<NodeV2RunContext>(
      engine_.get(), workflow_.get(),
      scheduler_config_->GetSharedConfg(), profiler_);
  for (const auto &nodeName : scheduler_config_->GetNodesName()) {
    if (name2ptr_.find(nodeName) != name2ptr_.end()) {
      LOGE << "node name'" << nodeName << " is not unique!";
      return -1;
    }
    auto node = std::make_shared<xstream::NodeV2>(nodeName);
    auto inputs = scheduler_config_->GetNodeInputs(nodeName);
    auto outputs = scheduler_config_->GetNodeOutputs(nodeName);
    auto inputSlot = CreateSlot(inputs);
    auto outputSlot = CreateSlot(outputs);
    node_input_slots_[node] = inputSlot;
    node_output_slots_[node] = outputSlot;
    std::vector<std::string> inputSlot_name, outputSlot_name;
    int cnt = -1;
    for (auto slot : inputSlot) {
      cnt++;
      if (slot < 0) continue;
      if (slot_infos_.size() <= static_cast<size_t>(slot)) {
        slot_infos_.resize(slot + 1);
      }
      slot_infos_[slot].dep_nodes_.insert(std::make_pair(node, cnt));
      inputSlot_name.push_back(data_slot_names_[slot]);
    }

    cnt = -1;
    for (auto slot : outputSlot) {
      cnt++;
      if (slot < 0) continue;
      if (slot_infos_.size() <= static_cast<size_t>(slot)) {
        slot_infos_.resize(slot + 1);
      }
      slot_infos_[slot].prod_node_ = std::make_pair(node, cnt);
      outputSlot_name.push_back(data_slot_names_[slot]);
    }

    node->Init(inputSlot_name, outputSlot_name,
               scheduler_config_->GetNodeConfig(nodeName), node_runctx);
    name2ptr_[nodeName] = node;
  }
  xstream_output_names_ = scheduler_config_->GetFlowOutputsUnion();
  output_type_names_ = scheduler_config_->GetFlowOutputs();
  auto flow_input_names = scheduler_config_->GetFlowInputs();
  xstream_input_names_.insert(flow_input_names.begin(), flow_input_names.end());

  // Build link
  CreateWorkflow();

  // Node Post Init.
  NodePostInit();

  Start();

  PreRun();

  return 0;
}

void SchedulerV2::PreRun() {
  auto framedata = std::make_shared<FrameDataInfo>();
  framedata->context_ = nullptr;
  std::promise<std::vector<OutputDataPtr>> promise;
  auto future = promise.get_future();
  framedata->sync_context_ = &promise;
  framedata->global_sequence_id_ = 0;
  framedata->sequence_id_ = 0;
  framedata->timestamp_ = framedata->sequence_id_;

  framedata->source_id_ = 0;
  std::vector<std::tuple<hobot::Module *, int, int, hobot::spMessage>>
      feed_messages;
  for (auto &input_name : xstream_input_names_) {
    auto base_data = std::make_shared<BaseData>();
    hobot::spMessage in_msg(new XStreamInputMessage(base_data));
    {
      auto real_msg = static_cast<XStreamBaseMessage *>(in_msg.get());
      real_msg->framedata_ = framedata;
      real_msg->msg_type_ = XStreamMsgType::INIT;
    }
    auto dep_nodes = slot_infos_[data_slots_[input_name]].dep_nodes_;
    for (auto &node_entry : dep_nodes) {
      feed_messages.push_back(
          std::make_tuple(node_entry.first->GetInputModule(), 0,
                          node_entry.second + 1, in_msg));
    }
    auto iter = std::find(xstream_output_names_.begin(),
                          xstream_output_names_.end(), input_name);
    if (iter != xstream_output_names_.end()) {
      framedata->SetOutput(input_name, base_data);
    }
  }

  std::unordered_map<std::string, InputParamPtr> method_params;
  for (const auto &nodeName : scheduler_config_->GetNodesName()) {
    auto node = name2ptr_[nodeName];
    hobot::spMessage null_msg(new XStreamInputParamMessage(nullptr));
    {
      auto real_msg = static_cast<XStreamBaseMessage *>(null_msg.get());
      real_msg->framedata_ = framedata;
      real_msg->msg_type_ = XStreamMsgType::INIT;
    }
    feed_messages.push_back(
        std::make_tuple(node->GetInputModule(), 0, 0, null_msg));
  }

  workflow_->Feed(run_task_, feed_messages, 0);
  // sync
  auto out = future.get();
}

std::vector<int> SchedulerV2::CreateSlot(
    const std::vector<std::string> &dataNames) {
  LOGD << "CreateSlot";
  std::vector<int> dataSlots;
  int index = data_slots_.size();
  for (const auto &dataName : dataNames) {
    if (data_slots_.find(dataName) == data_slots_.end()) {
      data_slots_[dataName] = index++;
      data_slot_names_.push_back(dataName);
    }
    dataSlots.push_back(data_slots_[dataName]);
  }
  return dataSlots;
}

int SchedulerV2::CreateWorkflow() {
  for (const auto &nodeName : scheduler_config_->GetNodesName()) {
    auto node = name2ptr_[nodeName];
    int cnt = -1;
    for (auto slot : node_input_slots_[node]) {
      cnt++;
      if (xstream_input_names_.count(data_slot_names_[slot])) {
        continue;
      }
      Link(slot_infos_[slot].prod_node_.first,
           slot_infos_[slot].prod_node_.second, node, cnt);
    }
  }
  return 0;
}

void SchedulerV2::NodePostInit() {
  for (const auto &nodeName : scheduler_config_->GetNodesName()) {
    auto node = name2ptr_[nodeName];
    node->PostInit();
  }
}

#define HOBOTSDK_OBSERVE_ALL TRUE

void SchedulerV2::Start() {
  std::vector<std::pair<hobot::Module *, int>> result_modules;
  std::set<std::pair<hobot::Module *, int>> result_modules_set;
#ifndef HOBOTSDK_OBSERVE_ALL
  for (const auto &slotName : xstream_output_names_) {
    if (xstream_input_names_.count(slotName)) {
      continue;
    }
    auto node = slot_infos_[data_slots_[slotName]].prod_node_.first;
    int output_slot = slot_infos_[data_slots_[slotName]].prod_node_.second;
    auto module = node->GetOutputModule();
    result_modules_set.insert(std::make_pair(module, output_slot));
  }

  for (auto &item : node_callbacks_) {
    auto node = item.first;
    auto module = node->GetOutputModule();
    for (int i = 0; i < node_output_slots_[node].size(); i++) {
      result_modules_set.insert(std::make_pair(module, i));
    }
  }
#else
  for (auto item : name2ptr_) {
    auto node = item.second;
    auto module = node->GetOutputModule();
    for (size_t i = 0; i < node_output_slots_[node].size(); ++i) {
      auto slot_name = data_slot_names_[node_output_slots_[node][i]];
      result_modules_set.insert({module, i});
    }
  }
#endif
  result_modules.insert(result_modules.begin(), result_modules_set.begin(),
                        result_modules_set.end());
  result_ob_.reset(new ResultObserver(this));
  run_task_ = workflow_->Run(result_modules, result_ob_.get());
  run_task_->Init();
}

void SchedulerV2::StartOnce() {
  // double check to ensure start Once.
  if (!start_flag_) {
    std::lock_guard<std::mutex> lock(lock_);
    if (!start_flag_) {
      Start();
      start_flag_ = true;
    }
  }
}
}  // namespace xstream
