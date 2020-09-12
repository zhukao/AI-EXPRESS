/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file TestInitMethod.h
 * @brief
 * @author zhe.sun
 * @email zhe.sun@horizon.ai
 * @date 2020/2/11
 */

#ifndef XSTREAM_FRAMEWORK_TEST_INCLUDE_TESTINITMETHOD_H_
#define XSTREAM_FRAMEWORK_TEST_INCLUDE_TESTINITMETHOD_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "json/json.h"
namespace xstream {

class TestInitMethod : public Method {
 private:
  int threshold_;

 public:
  int Init(const std::string &config_file_path) override {
    std::ifstream infile(config_file_path);
    HOBOT_CHECK(infile.good())
        << "config_file_path error: " << config_file_path;
    Json::Value cfg_jv;
    infile >> cfg_jv;
    HOBOT_CHECK(cfg_jv.isMember("threshold") && cfg_jv["threshold"].isInt());
    threshold_ = cfg_jv["threshold"].asInt();
    return 0;
  }

  int InitFromJsonString(const std::string &json_config_string) override {
    Json::CharReaderBuilder reader_builder;
    Json::Value config;
    JSONCPP_STRING errs;
    std::shared_ptr<Json::CharReader> json_reader(
        reader_builder.newCharReader());
    bool ret = json_reader->parse(
        json_config_string.c_str(),
        json_config_string.c_str() + json_config_string.size(), &config, &errs);
    if (!ret || !errs.empty()) {
      std::cout << "parse json error:" << errs << std::endl;
    } else {
      HOBOT_CHECK(config.isMember("threshold") && config["threshold"].isInt());
      threshold_ = config["threshold"].asInt();
    }
    return 0;
  }

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override {
    std::cout << "threshold: " << threshold_ << std::endl;
    return input;
  }

  void Finalize() override {}

  int UpdateParameter(InputParamPtr ptr) override { return 0; }

  InputParamPtr GetParameter() const override { return InputParamPtr(); }

  std::string GetVersion() const override { return "0.0.0"; }

  void OnProfilerChanged(bool on) override {}
};

}  //  namespace xstream

#endif  //  XSTREAM_FRAMEWORK_TEST_INCLUDE_TESTINITMETHOD_H_
