/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CNNMethodCreator.h
 * @Brief: declaration of the CNNMethodCreator
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-02-06 14:52:31
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-02-06 16:22:44
 */

#ifndef INCLUDE_CNNMETHODCREATOR_H_
#define INCLUDE_CNNMETHODCREATOR_H_

#include <map>
#include <string>
#include <vector>

namespace xstream {
#define DEFINE_MethodCreator(method_name, class_name) \
            MethodPtr method_name##_creator() { \
                return MethodPtr(new class_name); \
            }

#define DECLARE_MethodCreator(method_name) \
  extern MethodPtr method_name##_creator();
}  // namespace xstream
#endif  // INCLUDE_CNNMETHODCREATOR_H_
