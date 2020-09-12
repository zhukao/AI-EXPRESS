/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-02-06 22:20:15
 * @Version: v0.0.1
 * @Brief: Scheduler reimplementation based hobotsdk.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-02-07 22:42:36
 */

#ifndef HOBOTXSTREAM_HOBOTSDK_SCHEDULERV2_H_
#define HOBOTXSTREAM_HOBOTSDK_SCHEDULERV2_H_

#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <mutex>
#include "NodeV2.h"
#include "hobot/hobot.h"
#include "hobotxstream/xstream_config.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotxsdk/xstream_error.h"
#include "BaseMessage.h"

namespace xstream {
class SchedulerV2 {  // 调度模块
 public:
  SchedulerV2() = default;

  virtual ~SchedulerV2();

  int Init(XStreamConfigPtr config,
           ProfilerPtr profiler);

  int UpdateConfig(std::string unique_name, InputParamPtr param_ptr);

  InputParamPtr GetConfig(const std::string &unique_name) const;

  int SetCallback(XStreamCallback callback);

  int SetCallback(XStreamCallback callback, const std::string &name);

  int SetFreeMemery(bool is_enable);

  int64_t Input(InputDataPtr data, void *sync_context);

  std::string GetVersion(const std::string &unique_name) const;

 private:
  int CreateNodes();

  int CreateWorkflow();

  void NodePostInit();

  void Start();

  void StartOnce();

  void PreRun();

  class ResultObserver : public hobot::RunObserver {
   public:
    explicit ResultObserver(SchedulerV2* scheduler)
        : scheduler_(scheduler) {}
    void OnResult(hobot::Module *from, int output_slot,
                  hobot::spMessage output);

   private:
    SchedulerV2* scheduler_;
  };

  std::unique_ptr<ResultObserver> result_ob_;
  hobot::spRunContext run_task_;

  bool IsSingleOutputDone(const FrameDataInfoPtr &framedata,
                          const std::string &output_type) const;
  bool IsFrameDone(const FrameDataInfoPtr &framedata) const;
  bool IsNodeReady(const FrameDataInfoPtr &framedata,
                   const std::string &node_name);
  bool IsObserveOutput(const std::string &node_name,
                       const std::string &slot_name);
  void OnResult(FrameDataInfoPtr frame_data, const std::string &node_name);
  void RemovePostedOutput(FrameDataInfoPtr frame_data);
  std::set<std::string> GetNotPostedOutput(FrameDataInfoPtr framedata);
  std::atomic_ullong global_sequence_id_{0};
  std::vector<std::shared_ptr<std::atomic_ullong>> sequence_id_list_;

  // 多路输出时，封装单路输出，用于异步调用
  OutputDataPtr SingleOutput(FrameDataInfoPtr data,
                             const std::string &output_type);

  // 多路整体输出，用于同步调用
  std::vector<OutputDataPtr> MultipleOutput(FrameDataInfoPtr data);

  int OutputMethodResult(FrameDataInfoPtr data, const std::string &node_name);

  std::vector<int> CreateSlot(const std::vector<std::string> &datas);

  std::unordered_map<std::string, NodeV2Ptr> name2ptr_;
  std::unordered_map<NodeV2Ptr, std::vector<int>> node_input_slots_;
  std::unordered_map<NodeV2Ptr, std::vector<int>> node_output_slots_;
  std::unordered_map<std::string, int> data_slots_;
  std::vector<std::string> data_slot_names_;
  // 支持多路输出
  std::map<std::string, std::vector<std::string>> output_type_names_;
  // xstream输出集合
  std::vector<std::string> xstream_output_names_;
  std::set<std::string> xstream_input_names_;
  struct SlotInfo {
    std::pair<NodeV2Ptr, int> prod_node_;
    std::set<std::pair<NodeV2Ptr, int>> dep_nodes_;
  };
  std::vector<SlotInfo> slot_infos_;

  // 是否需要释放帧内无用数据
  bool is_need_free_data_ = false;
  XStreamCallback callback_;
  std::unordered_map<NodeV2Ptr, XStreamCallback> node_callbacks_;
  SchedulerConfigPtr scheduler_config_;

  bool is_init_{false};
  std::atomic_bool start_flag_{false};
  std::mutex lock_;

  std::unique_ptr<hobot::Engine> engine_;
  std::unique_ptr<hobot::Workflow> workflow_;
  ProfilerPtr profiler_;
};

typedef std::shared_ptr<SchedulerV2> SchedulerV2Ptr;

}  // namespace xstream

#endif  // HOBOTXSTREAM_HOBOTSDK_SCHEDULERV2_H_
