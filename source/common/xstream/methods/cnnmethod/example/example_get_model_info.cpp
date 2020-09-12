/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: example_get_model_info.cpp
 * @Brief:
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-04-15 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-04-15 15:18:10
 */
#include <stdint.h>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "CNNMethod/util/ModelInfo.h"
#include "hobotlog/hobotlog.hpp"
#include "bpu_predict/bpu_predict.h"

static void Usage() {
  std::cout << "./example get_model_info model_file bpu_config" << std::endl;
}
int GetModelInfo(int argc, char** argv) {
  if (argc < 3) {
    Usage();
    return -1;
  }
  std::string model_file = argv[1];
  std::string bpu_config = argv[2];
  BPUHandle bpu_handle;
  int ret = BPU_loadModel(model_file.c_str(), &bpu_handle, bpu_config.c_str());
  if (ret != 0) {
    std::cout << "here load bpu model failed" << std::endl;
    return 1;
  }
  std::cout << "here load bpu model OK" << std::endl;

  // get bpu version
  const char* version = BPU_getVersion(bpu_handle);
  if (version == nullptr) {
    std::cout << "here get bpu version failed: " << BPU_getLastError(bpu_handle)
              << std::endl;
    return 1;
  }
  std::cout << "here get bpu version: " << version << std::endl;

  // get model names
  const char** name_list;
  int name_cnt;
  ret = BPU_getModelNameList(bpu_handle, &name_list, &name_cnt);
  if (ret != 0) {
    std::cout << "here get name list failed: " << BPU_getLastError(bpu_handle)
              << std::endl;
    return 1;
  }


  BPU_MODEL_S* bpu_model;
  // load model
  {
    std::ifstream ifs(model_file.c_str(), std::ios::in | std::ios::binary);
    if (!ifs) {
      HOBOT_CHECK(0) << "Open model file: " << model_file << " failed";
    }
    ifs.seekg(0, std::ios::end);
    int model_length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    char *model_bin = new char[sizeof(char) * model_length];
    ifs.read(model_bin, model_length);
    ifs.close();
    bpu_model = new BPU_MODEL_S();
    int ret = HB_BPU_loadModel(model_bin, model_length, bpu_model);
    HOBOT_CHECK(ret == 0) << "Load model failed";
    delete[] model_bin;
  }

  for (int i = 0; i < name_cnt; i++) {
    xstream::ModelInfo model_info;
    model_info.Init(bpu_model, -1);
    std::cout << model_info;
  }

  return 0;
}
