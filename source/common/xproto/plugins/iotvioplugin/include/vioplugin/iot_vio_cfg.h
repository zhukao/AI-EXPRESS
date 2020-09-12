/***************************************************************************
 * COPYRIGHT NOTICE
 * Copyright 2019 Horizon Robotics, Inc.
 * All rights reserved.
 ***************************************************************************/
#ifndef INCLUDE_IOT_VIO_CFG_H_
#define INCLUDE_IOT_VIO_CFG_H_

#include <string>
#include <mutex>
#include "json/json.h"
#include "vioplugin/iot_cfg_type.h"

class IotVioConfig {
 public:
     IotVioConfig() = default;
     explicit IotVioConfig(const std::string &path) : path_(path) {}
     std::string GetStringValue(const std::string &key) const;
     int GetIntValue(const std::string &key) const;
     Json::Value GetJson() const;
     bool LoadConfig();
     bool ParserConfig();
     bool PrintConfig();

 private:
     std::string path_;
     Json::Value json_;
     mutable std::mutex mutex_;
     iot_vio_cfg_t iot_vio_cfg_ = { 0 };
};

#endif  // INCLUDE_IOT_VIO_CFG_H_
