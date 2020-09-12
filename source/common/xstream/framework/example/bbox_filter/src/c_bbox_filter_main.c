
/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xsoul framework
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.22
 */

#include <stdio.h>

#include "hobotxsdk/xstream_capi.h"
#include "hobotxsdk/xstream_error.h"
#include "hobotxstream/c_data_types/bbox.h"

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Usage : ./c_bbox_filter_main config\n");
    printf("Example : ./c_bbox_filter_main ./config/filter.json\n");
    return -1;
  }
  const char *config = argv[1];
  printf("sdk version: %s\n", HobotXStreamCapiGetVersion());
  HobotXStreamCapiSetLicensePath("../config/");
  printf("sdk license info: %s\n", HobotXStreamCapiGetLicenseInfo());

  HobotXStreamCapiSetGlobalConfig("config_file", config);
  // 初始化sdk
  HobotXStreamCapiHandle handle;
  int ret = HobotXStreamCapiInit(&handle);
  if (ret != HOBOTXSTREAM_ERROR_CODE_OK) {
    return ret;
  }
  // 准备inputs
  HobotXStreamCapiInputList *inputs = HobotXStreamCapiDataListAlloc(1);
  // 填充BBox
  HobotXStreamCapiBaseDataVector *rects =
      HobotXStreamCapiBaseDataVectorAlloc(2);
  rects->parent_.name_ = "face_head_box";
  inputs->datas_[0] = &(rects->parent_);
  HobotXStreamCapiBBox *bbox1 = HobotXStreamCapiBBoxAlloc();
  bbox1->values_[0] = 0;
  bbox1->values_[1] = 0;
  bbox1->values_[2] = 1000;
  bbox1->values_[3] = 1000;
  rects->datas_->datas_[0] = &(bbox1->parent_);
  HobotXStreamCapiBBox *bbox2 = HobotXStreamCapiBBoxAlloc();
  bbox2->values_[0] = 0;
  bbox2->values_[1] = 0;
  bbox2->values_[2] = 10;
  bbox2->values_[3] = 10;
  rects->datas_->datas_[1] = &(bbox2->parent_);
  // Hobot
  // 送给sdk计算
  HobotXStreamCapiDataList *outputs = NULL;
  ret = HobotXStreamCapiProcessSync(handle, inputs, &outputs);
  if (ret == HOBOTXSTREAM_ERROR_CODE_OK && outputs != NULL) {
    // 解析sdk的结果
    if (outputs && 1 == outputs->datas_size_) {
      HobotXStreamCapiDataList *rect_list =
          ((HobotXStreamCapiBaseDataVector *)(outputs->datas_[0]))->datas_;
      // 解析BBox
      int i;
      for (i = 0; i < rect_list->datas_size_; ++i) {
        HobotXStreamCapiBBox *bbox =
            (HobotXStreamCapiBBox *)(rect_list->datas_[i]);
        printf("%d : [%f, %f, %f, %f]\n", i, bbox->values_[0], bbox->values_[1],
               bbox->values_[2], bbox->values_[3]);
      }
    } else {
      puts("Error: outputs data size check failed");
    }
    // 释放sdk生成的结果
    HobotXStreamCapiDataListFree(&outputs);
  } else {
    printf("HobotXStreamCapiProcessSync error: %d\n", ret);
  }
  // 释放sdk
  HobotXStreamCapiDataListFree(&inputs);
  HobotXStreamCapiFinalize(handle);
  return 0;
}
