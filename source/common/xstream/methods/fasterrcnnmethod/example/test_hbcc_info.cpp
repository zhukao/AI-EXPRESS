//
// Created by yaoyao.sun on 2019-05-18.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <iostream>
#include "bpu_predict/bpu_predict.h"

int TestHBCCInfo(int argc, char **argv) {
  const char *model_file_path = "./models/faceMultitask.hbm";
  const char *bpu_config = "./configs/bpu_config.json";

  BPUHandle bpu_handle;
  int ret = BPU_loadModel(model_file_path, &bpu_handle, bpu_config);
  if (ret != 0) {
    std::cout << "here load bpu model failed: "
              << BPU_getLastError(bpu_handle) << std::endl;
    return 1;
  }
  const char **model_names;
  int model_num;
  ret = BPU_getModelNameList(bpu_handle, &model_names, &model_num);
  if (ret != 0) {
    std::cout << "here get name list failed: "
              << BPU_getLastError(bpu_handle) << std::endl;
    return 1;
  }

  for (int i = 0; i < model_num; i++) {
     std::cout << "model name:" << model_names[i] << std::endl;
  }
  return 0;
}

