/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file main.cpp
 * @brief
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 *
 *
 * */

#include <signal.h>
#include <unistd.h>
#include <malloc.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "json/json.h"

// #include "hbipcplugin/hbipcplugin.h"
#include "hobotlog/hobotlog.hpp"
#include "smartplugin/smartplugin.h"
#include "vioplugin/vioplugin.h"
#include "visualplugin/visualplugin.h"
#include "websocketplugin/websocketplugin.h"
#ifdef X3
#include "uvcplugin/uvcplugin.h"
#endif
using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using std::chrono::seconds;

// using horizon::vision::xproto::hbipcplugin::HbipcPlugin;
using horizon::vision::xproto::smartplugin::SmartPlugin;
using horizon::vision::xproto::vioplugin::VioPlugin;
using horizon::vision::xproto::visualplugin::VisualPlugin;
using horizon::vision::xproto::websocketplugin::WebsocketPlugin;
#ifdef X3
using horizon::vision::xproto::Uvcplugin::UvcPlugin;
#endif
static bool exit_ = false;

static void signal_handle(int param) {
  std::cout << "recv signal " << param << ", stop" << std::endl;
  if (param == SIGINT) {
    exit_ = true;
  }
}

int main(int argc, char **argv) {
  mallopt(M_TRIM_THRESHOLD, 128 * 1024);
  std::string run_mode = "ut";

  if (argc < 5) {
    std::cout << "Usage: smart_main vio_config_file "
              << "xstream_config_file visualplugin_config "
              << "[-i/-d/-w/-f] " << std::endl;
    return 0;
  }
  std::string vio_config_file = std::string(argv[1]);
  std::string smart_config_file = std::string(argv[2]);
  std::string visual_config_file = std::string(argv[3]);

  std::string log_level(argv[4]);
  if (log_level == "-i") {
    SetLogLevel(HOBOT_LOG_INFO);
  } else if (log_level == "-d") {
    SetLogLevel(HOBOT_LOG_DEBUG);
  } else if (log_level == "-w") {
    SetLogLevel(HOBOT_LOG_WARN);
  } else if (log_level == "-e") {
    SetLogLevel(HOBOT_LOG_ERROR);
  } else if (log_level == "-f") {
    SetLogLevel(HOBOT_LOG_FATAL);
  } else {
    LOGE << "log option: [-i/-d/-w/-f] ";
    return 0;
  }

  if (argc == 6) {
    run_mode.assign(argv[5]);
    if (run_mode != "ut" && run_mode != "normal") {
      LOGE << "not support mode: " << run_mode;
      return 0;
    }
  }
  // parse output display mode config
  int display_mode = -1;
  std::ifstream ifs(visual_config_file);
  if (!ifs.is_open()) {
    LOGF << "open config file " << visual_config_file << " failed";
    return 0;
  }
  Json::CharReaderBuilder builder;
  std::string err_json;
  Json::Value json_obj;
  try {
    bool ret = Json::parseFromStream(builder, ifs, &json_obj, &err_json);
    if (!ret) {
      LOGF << "invalid config file " << visual_config_file;
      return 0;
    }
  } catch (std::exception &e) {
    LOGF << "exception while parse config file " << visual_config_file << ", "
         << e.what();
    return 0;
  }
  if (json_obj.isMember("display_mode")) {
    display_mode = json_obj["display_mode"].asUInt();
  } else {
    LOGF << visual_config_file << " should set display mode";
    return 0;
  }
  if (display_mode < 0) {
    LOGF << visual_config_file << " set display mode failed";
    return 0;
  }

  signal(SIGINT, signal_handle);
  signal(SIGPIPE, signal_handle);
  signal(SIGSEGV, signal_handle);

  auto vio_plg = std::make_shared<VioPlugin>(vio_config_file);
  auto smart_plg = std::make_shared<SmartPlugin>(smart_config_file);
  std::shared_ptr<XPluginAsync> output_plg = nullptr;
  // create output plugin: QtVisualPlugin/WebsocketPlugin/UvcPlugin
  if (0 == display_mode) {
    LOGI << "create QT Visual Plugin";
    output_plg = std::make_shared<VisualPlugin>(visual_config_file);
  } else if (1 == display_mode) {
    LOGI << "create WebSocket plugin";
    output_plg = std::make_shared<WebsocketPlugin>(visual_config_file);
  }
#ifdef X3
  if (2 == display_mode) {
    LOGI << "create UVC plugin";
    output_plg = std::make_shared<UvcPlugin>(visual_config_file);
  }
#endif
  auto ret = vio_plg->Init();
  if (ret != 0) {
    LOGE << "Failed to init vio";
    return 1;
  }
  ret = smart_plg->Init();
  if (ret != 0) {
    LOGE << "Failed to init vio";
    return 2;
  }
  if (output_plg) {
    ret = output_plg->Init();
    if (ret != 0) {
      LOGE << "output plugin init failed";
      return 3;
    }
    ret = output_plg->Start();
    if (ret != 0) {
      LOGE << "output plugin start failed";
      return 3;
    }
  }
  vio_plg->Start();
  smart_plg->Start();

  if (run_mode == "ut") {
    std::this_thread::sleep_for(std::chrono::seconds(60));
  } else {
    while (!exit_) {
      std::this_thread::sleep_for(std::chrono::microseconds(40));
    }
  }
  vio_plg->Stop();
  vio_plg->DeInit();
  vio_plg = nullptr;
  smart_plg->Stop();
  if (output_plg) {
    output_plg->Stop();
  }
  smart_plg->DeInit();
  output_plg->DeInit();
  return 0;
}
