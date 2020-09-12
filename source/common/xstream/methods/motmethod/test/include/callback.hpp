/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     callback.hpp
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2017.9.25
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <vector>
#include <string>
#include "hobotxsdk/xstream_data.h"

class TestCallback {
 public:
  TestCallback() = default;

  ~TestCallback() = default;

  std::string &GetLog();

  void SaveOutputLog(const xstream::OutputDataPtr &output);

  void IsSameBBox(const std::vector<xstream::BaseDataPtr> &input,
                  const std::vector<xstream::BaseDataPtr> &output,
                  bool has_id);
 private:
  std::string id_bbox_track_res_;
};

#endif  // CALLBACK_H_
