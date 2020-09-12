/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: zhe sun
 * @Mail: zhe.sun@horizon.ai
 * @Date: 2019-12-26 16:15:22
 * @Version: v0.0.1
 * @Brief: test subworkflow
 */

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/xstream_config.h"
#include "hobotxstream/xstream.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxsdk/xstream_sdk.h"

namespace subworkflow_test {
TEST(WorkflowTest, ValidTemplate) {
  LOGD << "test subworkflow" << std::endl;
  auto config = std::make_shared<xstream::XStreamConfig>();
  EXPECT_EQ(0, config->LoadFile("./test/configs_subworkflow/config.json"));
  Json::Value cfg_jv;
  std::ifstream infile("./test/configs_subworkflow/config_gt.json");
  HOBOT_CHECK(infile.good()) << "error config config_gt.json, please check it";
  infile >> cfg_jv;
  infile.close();
  EXPECT_EQ(cfg_jv, config->cfg_jv_);
}

TEST(WorkflowTest, TemplateDefinitionFile) {
  LOGD << "loading template definition from file" << std::endl;
  auto config = std::make_shared<xstream::XStreamConfig>();
  EXPECT_EQ(0, config->LoadFile("./test/configs_subworkflow/config_1.json"));
  Json::Value cfg_jv;
  std::ifstream infile("./test/configs_subworkflow/config_1_gt.json");
  HOBOT_CHECK(infile.good()) << "error config config_gt.json, please check it";
  infile >> cfg_jv;
  infile.close();
  EXPECT_EQ(cfg_jv, config->cfg_jv_);
}

TEST(WorkflowTest, templateEbeded) {
  LOGD << "templete definition includes anothor template_ref" << std::endl;
  auto config = std::make_shared<xstream::XStreamConfig>();
  EXPECT_EQ(0, config->LoadFile("./test/configs_subworkflow/config_2.json"));
  Json::Value cfg_jv;
  std::ifstream infile("./test/configs_subworkflow/config_2_gt.json");
  HOBOT_CHECK(infile.good()) << "error config config_gt.json, please check it";
  infile >> cfg_jv;
  infile.close();
  EXPECT_EQ(cfg_jv, config->cfg_jv_);
}

TEST(WorkflowTest, CommonTemplate) {
  LOGD << "Common template(not subworkflow template)" << std::endl;
  auto config = std::make_shared<xstream::XStreamConfig>();
  EXPECT_EQ(0, config->LoadFile("./test/configs_subworkflow/config_3.json"));

  Json::Value cfg_jv;
  std::ifstream infile("./test/configs_subworkflow/config_3_gt.json");
  HOBOT_CHECK(infile.good()) << "error config config_gt.json, please check it";
  infile >> cfg_jv;
  infile.close();
  EXPECT_EQ(cfg_jv, config->cfg_jv_);
}

TEST(WorkflowTest, MultiOutput) {
  LOGD << "test subworkflow" << std::endl;
  auto config = std::make_shared<xstream::XStreamConfig>();
  EXPECT_EQ(0, config->LoadFile("./test/configs_subworkflow/config_4.json"));
  Json::Value cfg_jv;
  std::ifstream infile("./test/configs_subworkflow/config_4_gt.json");
  HOBOT_CHECK(infile.good()) << "error config config_gt.json, please check it";
  infile >> cfg_jv;
  infile.close();
  EXPECT_EQ(cfg_jv, config->cfg_jv_);
}
}  // namespace subworkflow_test
