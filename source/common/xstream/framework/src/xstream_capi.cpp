/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xsoul c framework interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.14
 */

#include <cstring>
#include <fstream>
#include <map>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/version.h"
#ifdef ENABLE_SECURE
#include "secure/VisionGuard.hpp"
#endif
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_capi.h"
#include "hobotxsdk/xstream_capi_type_helper.h"
#include "hobotxsdk/xstream_sdk.h"

static std::map<std::string, std::string> *gconfig_map = nullptr;
static char *license_path_ = nullptr;
static char *license_info_buf_ = nullptr;
#ifdef ENABLE_SECURE
hobot::VisionGuard *vision_guard_ = nullptr;
#endif

const char *HobotXStreamCapiGetVersion() { return HOBOTXSDK_VERSION; }

void HobotXStreamCapiSetLicensePath(const char *path) {
  if (license_path_) {
    delete[] license_path_;
    license_path_ = nullptr;
  }
  auto buflen = std::strlen(path);
  auto tmp_pstr = new char[buflen + 1];
  std::strncpy(tmp_pstr, path, buflen);
  license_path_ = tmp_pstr;
  tmp_pstr[buflen] = '\0';
  LOGV << "set license: " << license_path_;
#ifdef ENABLE_SECURE
  if (vision_guard_) {
    delete vision_guard_;
    vision_guard_ = nullptr;
  }
  vision_guard_ = new hobot::VisionGuard(license_path_);
#endif
  LOGV << "end of set license";
}

const char *HobotXStreamCapiGetLicenseInfo() {
  LOGI << license_path_;
  std::string tmp_license_path(license_path_);
  tmp_license_path += "/license.txt";
  std::ifstream ifile(tmp_license_path);
  std::string license = "";

  if (ifile.is_open()) {
    std::string line = "";
    while (ifile >> line) {
    }
    ifile.close();
    license = line;
  } else {
    license = "INVALID INFO";
  }
  if (license_info_buf_) {
    delete[] license_info_buf_;
    license_info_buf_ = nullptr;
  }
  license_info_buf_ = new char[license.size() + 1];
  std::strncpy(license_info_buf_, license.c_str(), license.size());
  license_info_buf_[license.size()] = '\0';
  return license_info_buf_;
}

int HobotXStreamCapiSetGlobalConfig(const char *cfg_name, const char *value) {
  // TODO(jet) 此函数有可能用于Log输出配置等
  if (!gconfig_map) {
    gconfig_map = new std::map<std::string, std::string>();
  }
  (*gconfig_map)[cfg_name] = value;
  return 0;
}

int HobotXStreamCapiInit(HobotXStreamCapiHandle *handle) {
  if (!handle) {
    return -1;
  }
#ifdef ENABLE_SECURE
  if (!vision_guard_) LOGF << "Error: please set license path first";
  vision_guard_->Guard();
#endif
  auto sdk = xstream::XStreamSDK::CreateSDK();
  sdk->SetConfig("config_file", (*gconfig_map)["config_file"]);
  sdk->Init();
  *handle = sdk;
  return 0;
}

int HobotXStreamCapiFinalize(HobotXStreamCapiHandle handle) {
  if (!handle) {
    return -1;
  }
  auto sdk = reinterpret_cast<xstream::XStreamSDK *>(handle);
  delete sdk;
  return 0;
}

int HobotXStreamCapiProcessSync(HobotXStreamCapiHandle handle,
                                const HobotXStreamCapiInputList *inputs,
                                HobotXStreamCapiDataList **outputs) {
  if (!handle || !inputs || !outputs) {
    return -1;
  }
  auto sdk = reinterpret_cast<xstream::XStreamSDK *>(handle);
  // sdk Init should be called inside SetConfig
  auto cppout = sdk->SyncPredict(xstream::InputList2Cpp(inputs));
  if (cppout->error_code_ < 0) {
    return cppout->error_code_;
  }
  *outputs = xstream::Output2CList(cppout);
  return 0;
}

int HobotXStreamCapiSetCallback(HobotXStreamCapiHandle handle,
                                HobotXStreamCapiCallback callback,
                                void *pUserData) {
  if (!handle) {
    return -1;
  }
  auto sdk = reinterpret_cast<xstream::XStreamSDK *>(handle);
  return sdk->SetCallback(
      [sdk, callback, pUserData](xstream::OutputDataPtr outputs) {
        HobotXStreamCapiCallbackData cbdata;
        cbdata.sequence_id_ = outputs->sequence_id_;
        cbdata.inputs_ = reinterpret_cast<HobotXStreamCapiInputList *>(
            const_cast<void *>(outputs->context_));
        cbdata.outputs_ = xstream::Output2CList(outputs);
        callback(sdk, &cbdata, pUserData);
      });
}

int64_t HobotXStreamCapiProcessAsync(HobotXStreamCapiHandle handle,
                                     const HobotXStreamCapiInputList *inputs) {
  if (!handle || !inputs) {
    return -1;
  }
  auto sdk = reinterpret_cast<xstream::XStreamSDK *>(handle);
  return sdk->AsyncPredict(xstream::InputList2Cpp(inputs));
}
