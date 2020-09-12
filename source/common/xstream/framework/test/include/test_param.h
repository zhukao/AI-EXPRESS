/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file test_param.h
 * @brief
 * @author tangji.sun
 * @email tangji.sun@horizon.ai
 * @date 2019/10/8
 */
#ifndef TEST_INCLUDE_TEST_PARAM_H_
#define TEST_INCLUDE_TEST_PARAM_H_

#include <string>
#include "hobotxsdk/xstream_capi_type.h"
#include "hobotxsdk/xstream_data.h"
namespace xstream {
class TestParam : public InputParam {
 public:
  explicit TestParam(std::string unique_name) : InputParam(unique_name) {}
  virtual ~TestParam() = default;

  std::string Format() override { return ""; }
};

}  // namespace xstream

#endif  // TEST_INCLUDE_TEST_PARAM_H_
