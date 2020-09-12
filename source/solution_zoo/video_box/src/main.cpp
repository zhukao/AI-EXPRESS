/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file main.cpp
 * @brief
 * @author fei.cheng
 * @email fei.cheng@horizon.ai
 *
 *
 * */

#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <memory>
// #include "hbipcplugin/hbipcplugin.h"
#include "hobotlog/hobotlog.hpp"
#include "rtspplugin/rtspplugin.h"
#include "smartplugin_box/smartplugin.h"
#include "vioplugin/vioplugin.h"
#include "visualplugin/visualplugin.h"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using std::chrono::seconds;

// using horizon::vision::xproto::hbipcplugin::HbipcPlugin;
using horizon::vision::xproto::rtspplugin::RtspPlugin;
using horizon::vision::xproto::smartplugin_multiplebox::SmartPlugin;
using horizon::vision::xproto::vioplugin::VioPlugin;
using horizon::vision::xproto::visualplugin::VisualPlugin;
static bool exit_ = false;

static void signal_handle(int param) {
  std::cout << "recv signal " << param << ", stop" << std::endl;
  if (param == SIGINT) {
    exit_ = true;
  }
  if (param == SIGSEGV) {
    std::cout << "recv segment fault, exit exe, maybe need reboot..."
              << std::endl;
    exit(-1);
  }
}

int main(int argc, char **argv) {
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

  signal(SIGINT, signal_handle);
  signal(SIGPIPE, signal_handle);
  signal(SIGSEGV, signal_handle);

  // auto vio_plg = std::make_shared<VioPlugin>(vio_config_file);
  // auto visual_plg = std::make_shared<VisualPlugin>(visual_config_file);
  auto rtsp_plg = std::make_shared<RtspPlugin>();

  // auto ret = vio_plg->Init();
  // if (ret != 0) {
  //   LOGE << "Failed to init vio";
  //   return 1;
  // }

  // if (visual_plg)
  //   visual_plg->Init();

  // vio_plg->Start();
  rtsp_plg->Init();
  auto ret = rtsp_plg->Start();
  if (ret < 0) {
    LOGE << "Failed to start rtsp plugin ret:" << ret;
    rtsp_plg->Stop();
    if (-2 == ret) {
      LOGE << "ERROR!!! open rtsp fail, please check url";
    }
    return 0;
  } else {
    LOGI << "rtsp plugin start success";
  }

  auto smart_plg = std::make_shared<SmartPlugin>(
          "./video_box/configs/body_solution.json");
  ret = smart_plg->Init();
  if (ret != 0) {
    LOGE << "Failed to init smart plugin";
    return 2;
  }
  smart_plg->Start();

  // if (visual_plg)
  //   visual_plg->Start();

  if (run_mode == "ut") {
    std::this_thread::sleep_for(std::chrono::seconds(30));
  } else {
    while (!exit_) {
      // std::this_thread::sleep_for(std::chrono::microseconds(40));
      std::this_thread::sleep_for(std::chrono::milliseconds(40));
      LOGI << "wait to quit";
    }
  }
  // vio_plg->Stop();
  rtsp_plg->Stop();
  smart_plg->Stop();
  // if (visual_plg)
  //   visual_plg->Stop();

  // vio_plg->DeInit();
  smart_plg->DeInit();
  // visual_plg->DeInit();

  return 0;
}
