/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xstream framework
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2019.01.17
 */

#include "hobotxstream/method.h"

int xstream::Method::UpdateParameter(xstream::InputParamPtr ptr) {
  return 0;
}

xstream::MethodInfo xstream::Method::GetMethodInfo() {
  return MethodInfo();
}

xstream::Method::~Method() = default;
