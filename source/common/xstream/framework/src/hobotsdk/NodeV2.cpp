/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-07 22:49:56
 * @Version: v0.0.1
 * @Brief: Node reimplemetation based hobotsdk.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-25 02:57:24
 */

#include "NodeV2.h"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseMessage.h"
#include "hobotxstream/json_key.h"
#include "hobotxstream/method_factory.h"

namespace xstream {
int Link(NodeV2Ptr src, int src_index, NodeV2Ptr dest, int dest_index) {
  auto workflow = src->node_ctx_->GetWorkflow();
  HOBOT_CHECK(src->node_ctx_ == dest->node_ctx_) << "Not Same graph node!";

  workflow->From(src->collect_module_.get(), src_index)
      ->To(dest->dispatch_module_.get(), dest_index + 1, 0);
  return 0;
}

void NodeV2::Init(const std::vector<std::string> &inputSlots,
                  const std::vector<std::string> &outputSlots,
                  const Config &config,
                  std::shared_ptr<NodeV2RunContext> run_context) {
  /**
   * 1.创建DispatchModule，设置SyncOnFeed属性，并根据Method属性设置reorder参数;
   * 2.根据Config配置创建对应数量的MethodModule(如在Threadsafe=false,创建thread_num个MethodModule实例等)；
   * 3.创建CollectModule;
   * 4.建立DispatchModule,n*MethodModue,CollectModule之间的连接；
   * 5.根据配置设置MethodModule的执行线程；
   * 6.设置DispatchModule与CollectModule的执行线程（为两个固定的预留线程num）;
   */
  unique_name_ = config[kUniqueName].asString();
  method_type_ = config[kMethodType].asString();
  input_slots_ = inputSlots;
  output_slots_ = outputSlots;
  node_ctx_ = run_context;
  config_ = config;
  shared_config_ = run_context->GetSharedConfig();
  engine_ = run_context->GetEngine();
  workflow_ = run_context->GetWorkflow();
  profiler_ = run_context->GetProfiler();

  CreateMethodModules();

  collect_module_ = std::make_shared<CollectModule>(unique_name_);

  // link dispatch Module to CollectModule + MethodModules.
  workflow_->From(dispatch_module_.get(), 0)->To(collect_module_.get(), 0, 0);
  for (size_t i = 0; i < method_modules_.size(); ++i) {
    workflow_->From(dispatch_module_.get(), i + 1)
        ->To(method_modules_[i].get(), 0, 0);
  }
  // link MethodModules to CollectModule.
  for (size_t i = 0; i < method_modules_.size(); ++i) {
    workflow_->From(method_modules_[i].get(), 0)
        ->To(collect_module_.get(), i + 1, 0);
  }
}

int NodeV2::CreateDispatchModule() {
  if (method_info_.is_src_ctx_dept) {
    dispatch_module_ = std::make_shared<ReorderDispatchModule>(
        shared_config_.source_num_, unique_name_);
    auto real_module =
        static_cast<ReorderDispatchModule *>(dispatch_module_.get());
    real_module->SetReorder(method_info_.is_need_reorder);
  } else {
    dispatch_module_ = std::make_shared<DefaultDispatchModule>(unique_name_);
  }
  dispatch_module_->SetOutputSlotNum(output_slots_.size());
  return 0;
}

int NodeV2::CreateMethodModules() {
  // create MethodModules.
  auto temp_method = shared_config_.method_factory_->CreateMethod(method_type_);
  method_info_ = temp_method->GetMethodInfo();

  // when source_num <= 0, disable source context attribute.
  if (shared_config_.source_num_ <= 1) {
    method_info_.is_src_ctx_dept = false;
  }

  CreateDispatchModule();

  size_t thread_count = 1;
  if (!config_.isMember(kThreadNum) && !config_.isMember(kTheadList)) {
    // 如果没有thread num的信息或者 threadlist信息。default 认为thread count为1
    // 创建 source_num_个线程,否则创建1个线程。
    thread_count = 1;
  } else {
    // 至少一个thread setting存在
    HOBOT_CHECK(!(config_.isMember(kThreadNum) && config_.isMember(kTheadList)))
        << "Thread setting can choose only one from thread_list and "
           "thread_count";
    // 向前兼容，以thread list 为主
    thread_count = config_.isMember(kTheadList) ? config_[kTheadList].size()
                                                : config_[kThreadNum].asUInt();
  }
  // 根据threadcount和其他配置生成线程
  if (config_.isMember(kTheadList)) {
    auto thread_list = config_[kTheadList];
    HOBOT_CHECK(thread_count > 0)
        << "node " << unique_name_ << "thread list is none";
    // TODO(SONGSHAN) :判断同一列表的thread_idx唯一
    for (uint i = 0; i < thread_list.size(); i++) {
      uint32_t thread_idx = thread_list[i].asUInt();
      threads_.push_back(thread_idx);
    }
  } else {
    // no "thread_list", create auto-produce thread
    for (size_t i = 0; i < thread_count; ++i) {
      threads_.push_back(node_ctx_->GetAutoGenThreadIdx());
    }
  }
  int32_t method_instance_count = 1;
  if (method_info_.is_src_ctx_dept) {
    // 检查 thread count与 source num的关系是否合法
    HOBOT_CHECK(thread_count <= static_cast<size_t>(shared_config_.source_num_))
        << "When method is source context dependent， thread count"
        << thread_count << " can not be larger than source number "
        << shared_config_.source_num_;
    // 当method跟输入源的上下文有关的时候，method的实例个数必须跟source个数一致
    method_instance_count = shared_config_.source_num_;
  } else {
    if (!method_info_.is_thread_safe_) {
      method_instance_count = thread_count;
    } else {
      // 为向前兼容，当不支持多输入的时候，method 实例数为1
      method_instance_count = 1;
    }
  }
  methods_.reserve(method_instance_count);
  methods_.push_back(std::make_shared<ThreadSafeMethod>(temp_method));

  size_t extra_instance_count = method_instance_count - 1;
  for (size_t i = 0; i < extra_instance_count; ++i) {
    methods_.push_back(std::make_shared<ThreadSafeMethod>(
        shared_config_.method_factory_->CreateMethod(method_type_)));
  }

  if (method_info_.is_thread_safe_) {
    for (size_t i = 0; i < methods_.size(); ++i) {
      if ((config_.isMember(kMethodCfgString) &&
           0 != methods_[0]->InitFromJsonString(
                    config_[kMethodCfgString].toStyledString())) ||
          (config_.isMember(kMethodCfgPath) &&
           0 != methods_[0]->Init(config_[kMethodCfgPath].asString()))) {
        LOGE << unique_name_ << ", method " << i << " initial failed";
        exit(1);
      }
    }
  }
  size_t methodmodule_num = std::max(thread_count, methods_.size());
  for (size_t i = 0; i < methodmodule_num; ++i) {
    auto instance_name = method_type_ + "_" + unique_name_;
    bool is_use_config_file_path{true};
    if (config_.isMember(kMethodCfgString)) {
      method_modules_.push_back(std::make_shared<MethodModule>(
          methods_[i % methods_.size()],
          config_[kMethodCfgString].toStyledString(),
          method_info_.is_thread_safe_, profiler_, unique_name_,
          instance_name));
    } else if (config_.isMember(kMethodCfgPath)) {
      method_modules_.push_back(std::make_shared<MethodModule>(
          methods_[i % methods_.size()], config_[kMethodCfgPath].asString(),
          method_info_.is_thread_safe_, profiler_, unique_name_,
          is_use_config_file_path, instance_name));
    } else {
      HOBOT_CHECK(false) << "invalid method config.";
    }
  }
  dispatch_module_->SetInstanceNum(method_modules_.size());
  if (method_info_.is_src_ctx_dept) {
    dispatch_module_->SetStrategy(DispatchStrategy::KEY_MATCHING);
    dispatch_module_->SetKeyMatchingFunc([methodmodule_num](void *key) -> int {
      if (key == nullptr) {
        return -1;
      }
      size_t source_id = *(static_cast<size_t *>(key));
      HOBOT_CHECK(source_id < methodmodule_num)
          << "source index overflow, curr source_id = " << source_id
          << ", source_num=" << methodmodule_num;
      return static_cast<int>(source_id);
    });
  }

  return 0;
}

/**
 * @brief Set Module thread id.
 *
 */
void NodeV2::PostInit() {
  engine_->ExecuteOnThread(dispatch_module_.get(), 0,
                           XSTREAM_DISPATCH_THREAD_IDX);
  for (size_t i = 0; i < method_modules_.size(); ++i) {
    engine_->ExecuteOnThread(method_modules_[i].get(), 0,
                             threads_[i % threads_.size()]);
    method_modules_[i]->SetOutputSlotNum(output_slots_.size());
    method_modules_[i]->SetInstId(i);
  }

  hobot::Expression *condition = hobot::Expression::Require(0, 1);

  LOGI << "METHODMODULE.size=" << method_modules_.size();
  for (size_t i = 0; i < method_modules_.size(); ++i) {
    condition = condition->Or(hobot::Expression::Require(i + 1, 1));
  }
  workflow_->SetCondition(collect_module_.get(), condition, 0);
  engine_->ExecuteOnThread(collect_module_.get(), 0,
                           XSTREAM_COLLECTION_THREAD_IDX);

  // Set dispath_module, SyncOnFeed.
  std::vector<int> sync_slot_arr;
  // input param link
  sync_slot_arr.push_back(0);
  hobot::Expression *dispatch_condition =
      hobot::Expression::Require(0, 1, false);
  for (size_t i = 0; i < input_slots_.size(); i++) {
    sync_slot_arr.push_back(i + 1);
    dispatch_condition =
        dispatch_condition->And(hobot::Expression::Require(i + 1, 1, false));
  }
  workflow_->SetCondition(dispatch_module_.get(), dispatch_condition, 0);
  workflow_->SetSyncOnFeed(
      dispatch_module_.get(), 0, sync_slot_arr,
      !method_info_.is_src_ctx_dept && method_info_.is_need_reorder);
}

int NodeV2::SetParameter(InputParamPtr ptr) {
  for (size_t i = 0; i < methods_.size(); ++i) {
    methods_[i]->UpdateParameter(ptr);
  }
  return 0;
}

InputParamPtr NodeV2::GetParameter() const {
  if (methods_.empty()) {
    return nullptr;
  }
  return methods_[0]->GetParameter();
}

std::string NodeV2::GetVersion() const {
  if (methods_.empty()) {
    return nullptr;
  }
  return methods_[0]->GetVersion();
}

}  // namespace xstream
