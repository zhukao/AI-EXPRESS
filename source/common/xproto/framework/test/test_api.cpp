/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     test_api.cpp
 * \Author   Yingmin Li
 * \Mail     yingmin.li-horizon.ai
 * \Version  1.0.0.0
 * \Date     2019/1/10
 * \Brief    implement of test_api.cpp
 */
#include <sys/utsname.h>

#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"

namespace {

TEST(xproto, api) {
  SetLogLevel(HOBOT_LOG_INFO);
  struct utsname name;
  if (uname(&name)) {
    exit(-1);
  }
  LOGI << "Hello! Your remote computer's OS is " << name.sysname << " "
       << name.release;
  LOGI << "Test xproto api file success";
}

}  // namespace
