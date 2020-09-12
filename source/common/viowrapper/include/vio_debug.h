/***************************************************************************
 *   Copyright (C) 2019 by horizon.                                        *
 *   xudong.du@horizon.ai                                                  *
 *                                                                         *
 *   header file.                                                          *
 *                                                                         *
 ***************************************************************************/
#ifndef INCLUDE_VIO_DEBUG_H_
#define INCLUDE_VIO_DEBUG_H_

#include <string.h>

#include <memory>
#include <string>
#ifdef X3
#include "./x3_vio_patch.h"
#include "horizon/vision_type/vision_type.hpp"
using hobot::vision::PymImageFrame;
#endif

namespace vio_debug {
inline void print_info(const img_info_t &data) {
  printf("data slot_id = %d\n", data.slot_id);
  printf("data frame_id = %d\n", data.frame_id);
  // printf("data1 timestamp = %llx\n", data.timestamp);
  // printf("data2 timestamp = %lld\n", data.timestamp);
  printf("data img_format = %d\n", data.img_format);
  printf("data ds_pym_layer = %d\n", data.ds_pym_layer);
  printf("data us_pym_layer = %d\n", data.us_pym_layer);
  printf("s w = %d\n", data.src_img.width);
  printf("s h = %d\n", data.src_img.height);
  printf("s s = %d\n", data.src_img.step);
  printf("s y_p = %p\n", reinterpret_cast<uint8_t *>(data.src_img.y_paddr));
  printf("s c_p = %p\n", reinterpret_cast<uint8_t *>(data.src_img.c_paddr));
  printf("s y_v = %p\n", reinterpret_cast<uint8_t *>(data.src_img.y_vaddr));
  printf("s c_v = %p\n", reinterpret_cast<uint8_t *>(data.src_img.c_vaddr));
  int i;
  for (i = 0; i < data.ds_pym_layer; i++) {
    printf("ds[%d] w = %d\n", i, data.down_scale[i].width);
    printf("ds[%d] h = %d\n", i, data.down_scale[i].height);
    printf("ds[%d] s = %d\n", i, data.down_scale[i].step);
    printf("ds[%d] y_p = %p\n", i,
           reinterpret_cast<uint8_t *>(data.down_scale[i].y_paddr));
    printf("ds[%d] c_p = %p\n", i,
           reinterpret_cast<uint8_t *>(data.down_scale[i].c_paddr));
    printf("ds[%d] y_v = %p\n", i,
           reinterpret_cast<uint8_t *>(data.down_scale[i].y_vaddr));
    printf("ds[%d] c_v = %p\n", i,
           reinterpret_cast<uint8_t *>(data.down_scale[i].c_vaddr));
  }
}

inline int save_img(int y_len, int uv_len, uint8_t *y_vaddr, uint8_t *uv_vaddr,
                    std::string name) {
  FILE *fd = fopen(name.c_str(), "wb+");
  if (!fd) {
    printf("open file name:%s failure!\n", name.c_str());
    return -1;
  }
  printf("[%s] y_len:%d, uv_len:%d, y_vaddr:%p, uv_vaddr:%p\n", name.c_str(),
         y_len, uv_len, y_vaddr, uv_vaddr);

  uint8_t *img_ptr = static_cast<uint8_t *>(malloc(y_len + uv_len));
  memcpy(img_ptr, y_vaddr, y_len);
  memcpy(img_ptr + y_len, uv_vaddr, uv_len);

  fwrite(img_ptr, sizeof(char), y_len + uv_len, fd);
  fflush(fd);
  fclose(fd);
  free(img_ptr);

  return 0;
}

inline void dump_pym_nv12(img_info_t *data, std::string yuv_prefix) {
  /*1. dump src img */
  std::string src_name = yuv_prefix + "_src_nv12.yuv";
  int src_y_width = data->src_img.width;
  int src_y_stride = data->src_img.step;  // stride
  int src_y_height = data->src_img.height;
  printf("[%s] w:%d, stride:%d, h:%d\n", src_name.c_str(), src_y_width,
         src_y_stride, src_y_height);
  int src_y_len = src_y_stride * src_y_height;
  int src_uv_len = src_y_stride * src_y_height / 2;
  uint8_t *src_y_vaddr = reinterpret_cast<uint8_t *>(data->src_img.y_vaddr);
  uint8_t *src_uv_vaddr = reinterpret_cast<uint8_t *>(data->src_img.c_vaddr);
  save_img(src_y_len, src_uv_len, src_y_vaddr, src_uv_vaddr, src_name);

  /*2. dump pym img */
  for (int k = 0; k < 5; k++) {
    std::string pym_name;
    int pym_y_width = data->down_scale[4 * k].width;
    int pym_y_stride = data->down_scale[4 * k].step;  // stride
    int pym_y_height = data->down_scale[4 * k].height;
    int pym_y_len = pym_y_stride * pym_y_height;
    int pym_uv_len = pym_y_stride * pym_y_height / 2;
    pym_name = pym_name + yuv_prefix + "_" + std::to_string(k) + "_nv12.yuv";
    printf("[%s] w:%d, stride:%d, h:%d\n", pym_name.c_str(), pym_y_width,
           pym_y_stride, pym_y_height);
    uint8_t *pym_y_vaddr =
        reinterpret_cast<uint8_t *>(data->down_scale[4 * k].y_vaddr);
    uint8_t *pym_uv_vaddr =
        reinterpret_cast<uint8_t *>(data->down_scale[4 * k].c_vaddr);
    save_img(pym_y_len, pym_uv_len, pym_y_vaddr, pym_uv_vaddr, pym_name);
  }
}
#ifdef X3
inline void dump_pym_nv12(std::shared_ptr<PymImageFrame> data,
                          std::string yuv_prefix) {
  /*1. dump pym img */
  for (int k = 0; k < 5; k++) {
    std::string pym_name;
    int pym_y_width = data->down_scale[4 * k].width;
    int pym_y_stride = data->down_scale[4 * k].stride;  // stride
    int pym_y_height = data->down_scale[4 * k].height;
    int pym_y_len = pym_y_stride * pym_y_height;
    int pym_uv_len = pym_y_stride * pym_y_height / 2;
    pym_name = pym_name + yuv_prefix + "_" + std::to_string(k) + "_nv12.yuv";
    printf("dump_pym_nv12: [%s] w:%d, stride:%d, h:%d\n", pym_name.c_str(),
           pym_y_width, pym_y_stride, pym_y_height);
    uint8_t *pym_y_vaddr =
        reinterpret_cast<uint8_t *>(data->down_scale[4 * k].y_vaddr);
    uint8_t *pym_uv_vaddr =
        reinterpret_cast<uint8_t *>(data->down_scale[4 * k].c_vaddr);
    save_img(pym_y_len, pym_uv_len, pym_y_vaddr, pym_uv_vaddr, pym_name);
  }
}
#endif
}  // namespace vio_debug
#endif  // INCLUDE_VIO_DEBUG_H_
