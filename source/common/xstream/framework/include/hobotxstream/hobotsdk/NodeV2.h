/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-06 01:08:05
 * @Version: V0.0.1
 * @Brief: Node reimplemention based on hobotsdk.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-24 22:02:23
 */
#ifndef HOBOTXSTREAM_HOBOTSDK_NODEV2_H_
#define HOBOTXSTREAM_HOBOTSDK_NODEV2_H_

#include <vector>
#include <string>
#include <memory>
#include <atomic>

#include "hobot/hobot.h"
#include "base_modules/CollectModule.h"
#include "base_modules/DispatchModule.h"
#include "base_modules/DefaultDispatchModule.h"
#include "base_modules/ReorderDispatchModule.h"
#include "base_modules/MethodModule.h"
#include "ThreadSafeMethod.h"
#include "hobotxstream/profiler.h"
#include "hobotxstream/xstream_config.h"

namespace xstream {
// user thread idx: 0~999;
// reserved thread_idx: 1000 ~1099
// auto-produce thread_idx:1100~INT_MAX
#define XSTREAM_DISPATCH_THREAD_IDX 1000
#define XSTREAM_COLLECTION_THREAD_IDX 1001
#define XSTREAM_AUTO_PRODUCE_THREAD_IDX_BASE 1100

struct NodeV2RunContext {
 private:
  hobot::Engine* engine_;
  hobot::Workflow *workflow_;
  XStreamSharedConfig sharedconfig_;
  ProfilerPtr profiler_;
  std::atomic_ulong auto_gen_thr_idx_{XSTREAM_AUTO_PRODUCE_THREAD_IDX_BASE};

 public:
  NodeV2RunContext(hobot::Engine* engine,
                   hobot::Workflow *workflow,
                   XStreamSharedConfig sharedconfig,
                   ProfilerPtr profiler) :
                 engine_(engine),
                 workflow_(workflow),
                 sharedconfig_(sharedconfig),
                 profiler_(profiler) {}
  hobot::Engine* GetEngine() const {
    return engine_;
  }
  hobot::Workflow* GetWorkflow() const {
    return workflow_;
  }
  XStreamSharedConfig GetSharedConfig() const {
    return sharedconfig_;
  }
  ProfilerPtr GetProfiler() const {
    return profiler_;
  }
  uint32_t GetAutoGenThreadIdx() {
    return auto_gen_thr_idx_++;
  }
};
class NodeV2 {
 public:
  explicit NodeV2(const std::string &unique_name) : unique_name_(unique_name) {}
  ~NodeV2() = default;

  void Init(const std::vector<std::string> &inputSlots,
            const std::vector<std::string> &outputSlots, const Config &config,
            std::shared_ptr<NodeV2RunContext> run_context);
  void PostInit();

  int SetParameter(InputParamPtr ptr);

  InputParamPtr GetParameter() const;

  std::string GetVersion() const;

  std::string GetUniqueName() const {
    return unique_name_;
  }

  hobot::Module *GetInputModule() const { return dispatch_module_.get(); }

  hobot::Module *GetOutputModule() const { return collect_module_.get(); }

 private:
  int CreateMethodModules();

  int CreateDispatchModule();

  ProfilerPtr profiler_;
  std::shared_ptr<DispatchModule> dispatch_module_;
  std::vector<std::shared_ptr<MethodModule>> method_modules_;
  std::shared_ptr<CollectModule> collect_module_;
  std::shared_ptr<NodeV2RunContext> node_ctx_;
  MethodInfo method_info_;

  std::vector<ThreadSafeMethodPtr> methods_;
  Config config_;
  // configuration shared within all nodes
  XStreamSharedConfig shared_config_;

  std::string method_type_;
  std::string unique_name_;
  std::vector<std::string> input_slots_;
  std::vector<std::string> output_slots_;
  hobot::Engine *engine_;
  hobot::Workflow *workflow_;
  std::vector<int> threads_;

  friend int Link(std::shared_ptr<NodeV2> src, int src_index,
                  std::shared_ptr<NodeV2> dest, int dest_index);
};

using NodeV2Ptr = std::shared_ptr<NodeV2>;

// 建立两个Node之间的连接关系．
int Link(NodeV2Ptr src, int src_index, NodeV2Ptr dest, int dest_index);
}  // namespace xstream
#endif  // HOBOTXSTREAM_HOBOTSDK_NODEV2_H_
