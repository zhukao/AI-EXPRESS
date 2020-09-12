/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     图像处理接口的example程序
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.15
 */

#include <stdio.h>
#include <stdlib.h>

#include "hobotxstream/image_tools.h"

int main() {
  FILE *fp = fopen("test.jpg", "rb");
  if (fp == NULL) {
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  int jpg_size = ftell(fp);
  unsigned char *jpg_data = (unsigned char *)malloc(jpg_size);
  rewind(fp);
  fread(jpg_data, 1, jpg_size, fp);
  fclose(fp);
  int width = 0;
  int height = 0;
  int first_stride = 0;
  int second_stride = 0;
  uint8_t *output_data = NULL;
  int output_size = 0;

  // 解码值BGR格式
  int ret = HobotXStreamDecodeImage(jpg_data,
                        jpg_size,
                        IMAGE_TOOLS_RAW_BGR,
                        &output_data,
                        &output_size,
                        &width,
                        &height,
                        &first_stride,
                        &second_stride);
  if (ret == 0) {
    fprintf(stdout, "decode to bgr format, result is :\n"
                    "width = %d\n"
                    "height = %d\n"
                    "first_stride = %d\n"
                    "second_stride = %d\n"
                    "output_data_size = %d\n\n",
                    width, height,
                    first_stride, second_stride, output_size);
    free(jpg_data);
  } else {
    free(jpg_data);
    fprintf(stderr, "decode error\n");
    return -1;
  }

  // 抠图
  uint8_t *crop_data = NULL;
  int crop_data_size = 0;
  ret = HobotXStreamCropImage(output_data, output_size,
                  width, height,
                  first_stride, second_stride,
                  IMAGE_TOOLS_RAW_BGR,
                  10, 560, 601, 931,
                  &crop_data, &crop_data_size,
                  &width, &height,
                  &first_stride, &second_stride);
  if (ret == 0) {
    fprintf(stdout, "cropr image, result is :\n"
                    "width = %d\n"
                    "height = %d\n"
                    "first_stride = %d\n"
                    "second_stride = %d\n"
                    "output_data_size = %d\n\n",
                    width, height,
                    first_stride, second_stride, crop_data_size);
    // 释放解码得到的内存
    HobotXStreamFreeImage(output_data);
  } else {
    fprintf(stderr, "resize format error\n");
    HobotXStreamFreeImage(output_data);
    return -1;
  }

  // 缩放至400 x 300, 保持宽高比
  uint8_t *scale_data = NULL;
  int scale_data_size = 0;
  struct HobotXStreamImageToolsResizeInfo resize_info;
  ret = HobotXStreamResizeImage(crop_data, crop_data_size,
                    width, height,
                    first_stride, second_stride,
                    IMAGE_TOOLS_RAW_BGR,
                    1,
                    400, 300,
                    &scale_data, &scale_data_size,
                    &first_stride, &second_stride,
                    &resize_info);
  if (ret == 0) {
    fprintf(stdout, "resize, result is :\n"
                    "width = 400\n"
                    "height = 300\n"
                    "first_stride = %d\n"
                    "second_stride = %d\n"
                    "output_data_size = %d\n"
                    "padding_right = %d\n"
                    "padding_bottom = %d\n"
                    "width_ratio = %f\n"
                    "height_ratio = %f\n\n",
                    first_stride, second_stride, scale_data_size,
                    resize_info.padding_right_,
                    resize_info.padding_bottom_,
                    resize_info.width_ratio_,
                    resize_info.height_ratio_);
    // 释放抠图得到的内存
    HobotXStreamFreeImage(crop_data);
  } else {
    fprintf(stderr, "resize format error\n");
    HobotXStreamFreeImage(crop_data);
    return -1;
  }

  // padding
  uint8_t *padding_data = NULL;
  int padding_data_size = 0;
  uint8_t padding_rgb[3] = {255, 0, 0};
  ret = HobotXStreamPadImage(scale_data, scale_data_size,
                 400, 300,
                 first_stride, second_stride,
                 IMAGE_TOOLS_RAW_BGR,
                 100, 300, 200, 400,
                 padding_rgb,
                 &padding_data, &padding_data_size,
                 &width, &height,
                 &first_stride, &second_stride);
  if (ret == 0) {
    fprintf(stdout, "pedding, result is :\n"
                    "width = %d\n"
                    "height = %d\n"
                    "first_stride = %d\n"
                    "second_stride = %d\n"
                    "output_data_size = %d\n\n",
                    width, height,
                    first_stride, second_stride, padding_data_size);
    // 释放 缩放得到的内存
    HobotXStreamFreeImage(scale_data);
  } else {
    fprintf(stderr, "resize format error\n");
    HobotXStreamFreeImage(scale_data);
    return -1;
  }

  // 格式转换
  uint8_t *i420_data = NULL;
  int i420_data_size = 0;
  ret = HobotXStreamConvertImage(padding_data, padding_data_size,
                     width, height,
                     first_stride, second_stride,
                     IMAGE_TOOLS_RAW_BGR,
                     IMAGE_TOOLS_RAW_YUV_I420,
                     &i420_data, &i420_data_size,
                     &first_stride, &second_stride);
  if (ret == 0) {
    fprintf(stdout, "convert, result is :\n"
                    "width = %d\n"
                    "height = %d\n"
                    "first_stride = %d\n"
                    "second_stride = %d\n"
                    "output_data_size = %d\n\n",
                    width, height,
                    first_stride, second_stride, i420_data_size);
    // 释放 pedding得到的内存
    HobotXStreamFreeImage(padding_data);
  } else {
    fprintf(stderr, "resize format error\n");
    HobotXStreamFreeImage(padding_data);
    return -1;
  }

  FILE *result = fopen("example.i420", "wb+");
  fwrite(i420_data, 1, i420_data_size, result);
  fclose(result);
  // 释放格式转换得到的内存
  HobotXStreamFreeImage(i420_data);
  return 0;
}
