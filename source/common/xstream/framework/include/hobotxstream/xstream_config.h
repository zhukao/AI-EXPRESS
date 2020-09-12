/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     the config of xsoul framework
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */
#ifndef HOBOTXSTREAM_XSTREAM_CONFIG_H_
#define HOBOTXSTREAM_XSTREAM_CONFIG_H_

#include <limits.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_error.h"
#include "hobotxstream/method_factory.h"
#include "json/json.h"
namespace xstream {

enum CONFIG_ERROR {
  CONFIG_OK = 0,
  INPUT_UNFEED_ERROR,
  OUTPUT_USELESS_ERROR,
  WORKFLOW_CIRCLE_ERROR,
  OUTPUT_REPEATED_ERROR,
  NODE_NAME_ERROR,
};

enum PARSE_CONFIG_ERROR {
  FILE_PATH_ERROR = 1100,
  EXCEED_MAX_NESTED_LAYER_ERROR = 1101,
  PARAMETER_NAME_NOT_EXIST_ERROR = 1102,
  INCLUDE_EMPTY_FILEPATH_ERROR = 1103,
  TEMPLATES_FORMAT_ERROR = 1104,
  TEMPLATES_NAME_REPEATED_ERROR = 1105,
  TEMPLATES_NAME_INVALID_ERROR = 1106,
  UNDEFINITION_TYPE_ERROR = 1107,
  TEMPLATEREF_NOT_CORRESPOND_ERROR = 1108,
  UNKOWN_TYPE_ERROR = 1109,
};

typedef Json::Value Config;

class ParseConfig {
 public:
  std::string folder_path_;  // @include拼接文件路径
  // function: load json file's content
  int LoadContent(const std::string &file_path, Json::Value &config);

  // function：template_ref, template -> template_instance
  int GetTemplateInstance(Json::Value &template_conf,
                          const Json::Value &template_ref_conf);

  // function: check parameters of template_ref and template
  bool CheckTemplateParameters(const Json::Value &template_conf,
                               const Json::Value &template_ref_conf);

  // function: @include
  int ConvertIncludePath2Content(Json::Value &config, int level = 0);

  // function: create map of templates
  int GetTemplatesMap(Json::Value &templates_array,
                      std::map<std::string, Json::Value> &templates);

  // function：parse workflows
  int ParseWorkflows(Json::Value &workflows,
                     std::map<std::string, Json::Value> &templates,
                     bool &repleat_parsing);

  void UpdateSlotsName(const Json::Value &workflow_slots,
                       const Json::Value &workflow_argslots, std::string &path,
                       Json::Value &node_slots);

  void GetSubNodes(Json::Value &root, Json::Value &out, std::string path,
                   const Json::Value &workflow_inputs,
                   const Json::Value &workflow_outputs,
                   const Json::Value &workflow_arginputs,
                   const Json::Value &workflow_argoutputs);

  void GetNodes(Json::Value &root, Json::Value &output);

  // main feature: IN: config, OUT: out
  int GetWorkflows(Json::Value &config, Json::Value &out);

 private:
  int GenerateWorkFlowTplInternalArgList(Json::Value &template_conf);

  bool IsTemplateRefObject(const Json::Value &config);
  bool IsWorkflowObject(const Json::Value &config);
};

class XStreamConfig {  // 配置模块
 public:
  XStreamConfig() = default;

  virtual ~XStreamConfig() {}

 public:
  ParseConfig parse_config_;
  int LoadFile(const std::string &file_path);
  std::string folder_path_;
  MethodFactoryPtr method_factory_;
  Config cfg_jv_;
};

typedef std::shared_ptr<XStreamConfig> XStreamConfigPtr;

struct XStreamSharedConfig {
  int source_num_ = 1;
  // method factory
  MethodFactoryPtr method_factory_ = nullptr;
};
typedef std::shared_ptr<XStreamSharedConfig> XStreamSharedConfigPtr;
class SchedulerConfig {  // 配置模块
 public:
  explicit SchedulerConfig(XStreamConfigPtr config);

  virtual ~SchedulerConfig() {}

 public:
  std::vector<std::string> GetFlowInputs() const { return flow_inputs_; }
  std::map<std::string, std::vector<std::string>> GetFlowOutputs() const {
    return flow_outputs_;
  }
  std::vector<std::string> GetFlowOutputsUnion() const {
    return flow_outputs_union_;
  }

  std::vector<std::string> GetNodesName() const { return nodes_names_; }

  std::vector<std::string> GetNodeInputs(const std::string &node_name) const {
    auto itr = nodes_inputs_.find(node_name);
    if (itr != nodes_inputs_.end()) {
      return itr->second;
    }
    return {};
  }

  std::vector<std::string> GetNodeOutputs(const std::string &node_name) {
    auto itr = nodes_outputs_.find(node_name);
    if (itr != nodes_outputs_.end()) {
      return itr->second;
    }
    return {};
  }

  Config &GetNodeConfig(const std::string &node_name) {
    return nodes_config_[node_name];
  }

  XStreamSharedConfig GetSharedConfg() const {
    XStreamSharedConfig config;
    config.source_num_ = source_num_;
    config.method_factory_ = config_->method_factory_;
    return config;
  }

  int GetMaxRunningCount() const { return max_running_count_; }

  uint32_t GetSourceNumber() const { return source_num_; }

  std::string GetFolderPath() const { return config_->folder_path_; }

  Config &GetOptionalConfig() { return optional_config_; }

  // int CheckOutputRedundant();
  int CheckInputValid();
  int CheckUniqueName();
  int CheckIsCircle();
  int CheckRepeatedOutput();

  int CheckConfigValid() {
    if (CheckUniqueName()) {
      return NODE_NAME_ERROR;
    } else if (CheckInputValid()) {
      return INPUT_UNFEED_ERROR;
    } else if (CheckIsCircle()) {
      return WORKFLOW_CIRCLE_ERROR;
    } else if (CheckRepeatedOutput()) {
      return OUTPUT_REPEATED_ERROR;
    }

    return 0;
  }

 protected:
  XStreamConfigPtr config_;
  int max_running_count_ = INT_MAX;
  uint32_t source_num_ = 1;
  std::vector<std::string> nodes_names_;
  std::vector<std::string> flow_inputs_;
  // 支持多路输出
  std::map<std::string, std::vector<std::string>> flow_outputs_;
  // 多路输出并集
  std::vector<std::string> flow_outputs_union_;
  std::map<std::string, std::vector<std::string>> nodes_inputs_;
  std::map<std::string, std::vector<std::string>> nodes_outputs_;
  std::map<std::string, Config> nodes_config_;
  Config optional_config_;
};

typedef std::shared_ptr<SchedulerConfig> SchedulerConfigPtr;

}  //  namespace xstream

#endif  //  XSTREAM_FRAMEWORK_INCLUDE_HOBOTXSTREAM_XSTREAM_CONFIG_H_
