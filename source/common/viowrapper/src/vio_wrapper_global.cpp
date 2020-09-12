/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: hb_vio_fb_wrapper.h
 * @Brief: declaration of the hb_vio_fb_wrapper
 * @Author: hangjun.yang
 * @Email: hangjun.yang@horizon.ai
 * @Date: 2020-05-25 14:18:28
 * @Last Modified by: hangjun.yang
 * @Last Modified time: 2020-05-25 22:00:00
 */

#include "./vio_wrapper_global.h"
#include <string>
#include <memory>
#include "hobotxstream/image_tools.h"

#ifdef X3_X2_VIO
#include "./hb_cam_interface.h"
#endif
#ifdef X3_IOT_VIO
#include "./hb_cam_interface.h"  // NOLINT
#include "vioplugin/iot_vio_api.h"
#endif

#ifdef X3_X2_VIO
static void Convert(img_info_t *img, PymImageFrame &pym_img) {
  if (nullptr == img) {
    return;
  }
  pym_img.ds_pym_total_layer = img->ds_pym_layer;
  pym_img.us_pym_total_layer = img->us_pym_layer;
  pym_img.frame_id = img->frame_id;
  pym_img.time_stamp = img->timestamp;
  pym_img.context = static_cast<void *>(img);
  for (int i = 0; i < DOWN_SCALE_MAX; ++i) {
    pym_img.down_scale[i].width = img->down_scale[i].width;
    pym_img.down_scale[i].height = img->down_scale[i].height;
    pym_img.down_scale[i].stride = img->down_scale[i].step;
    pym_img.down_scale[i].y_paddr = img->down_scale[i].y_paddr;
    pym_img.down_scale[i].c_paddr = img->down_scale[i].c_paddr;
    pym_img.down_scale[i].y_vaddr = img->down_scale[i].y_vaddr;
    pym_img.down_scale[i].c_vaddr = img->down_scale[i].c_vaddr;
  }
  for (int i = 0; i < UP_SCALE_MAX; ++i) {
    pym_img.up_scale[i].width = img->up_scale[i].width;
    pym_img.up_scale[i].height = img->up_scale[i].height;
    pym_img.up_scale[i].stride = img->up_scale[i].step;
    pym_img.up_scale[i].y_paddr = img->up_scale[i].y_paddr;
    pym_img.up_scale[i].c_paddr = img->up_scale[i].c_paddr;
    pym_img.up_scale[i].y_vaddr = img->up_scale[i].y_vaddr;
    pym_img.up_scale[i].c_vaddr = img->up_scale[i].c_vaddr;
  }
  for (int i = 0; i < DOWN_SCALE_MAIN_MAX; ++i) {
    pym_img.down_scale_main[i].width = img->down_scale_main[i].width;
    pym_img.down_scale_main[i].height = img->down_scale_main[i].height;
    pym_img.down_scale_main[i].stride = img->down_scale_main[i].step;
    pym_img.down_scale_main[i].y_paddr = img->down_scale_main[i].y_paddr;
    pym_img.down_scale_main[i].c_paddr = img->down_scale_main[i].c_paddr;
    pym_img.down_scale_main[i].y_vaddr = img->down_scale_main[i].y_vaddr;
    pym_img.down_scale_main[i].c_vaddr = img->down_scale_main[i].c_vaddr;
  }
}
#endif

#ifdef X3_IOT_VIO
static void Convert(pym_buffer_t *pym_buffer, PymImageFrame &pym_img) {
  if (nullptr == pym_buffer) {
    return;
  }
  pym_img.ds_pym_total_layer = DOWN_SCALE_MAX;
  pym_img.us_pym_total_layer = UP_SCALE_MAX;
  pym_img.frame_id = pym_buffer->pym_img_info.frame_id;
  pym_img.time_stamp = pym_buffer->pym_img_info.time_stamp;
  pym_img.context = static_cast<void *>(pym_buffer);
  for (int i = 0; i < DOWN_SCALE_MAX; ++i) {
    address_info_t *pym_addr = NULL;
    if (i % 4 == 0) {
      pym_addr = reinterpret_cast<address_info_t *>(&pym_buffer->pym[i / 4]);
    } else {
      pym_addr = reinterpret_cast<address_info_t *>(
          &pym_buffer->pym_roi[i / 4][i % 4 - 1]);
    }
    pym_img.down_scale[i].width = pym_addr->width;
    pym_img.down_scale[i].height = pym_addr->height;
    pym_img.down_scale[i].stride = pym_addr->stride_size;
    pym_img.down_scale[i].y_paddr = pym_addr->paddr[0];
    pym_img.down_scale[i].c_paddr = pym_addr->paddr[1];
    pym_img.down_scale[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_addr->addr[0]);
    pym_img.down_scale[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_addr->addr[1]);
  }
  for (int i = 0; i < UP_SCALE_MAX; ++i) {
    pym_img.up_scale[i].width = pym_buffer->us[i].width;
    pym_img.up_scale[i].height = pym_buffer->us[i].height;
    pym_img.up_scale[i].stride = pym_buffer->us[i].stride_size;
    pym_img.up_scale[i].y_paddr = pym_buffer->us[i].paddr[0];
    pym_img.up_scale[i].c_paddr = pym_buffer->us[i].paddr[1];
    pym_img.up_scale[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->us[i].addr[0]);
    pym_img.up_scale[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->us[i].addr[1]);
  }
  for (int i = 0; i < DOWN_SCALE_MAIN_MAX; ++i) {
    pym_img.down_scale_main[i].width = pym_buffer->pym[i].width;
    pym_img.down_scale_main[i].height = pym_buffer->pym[i].height;
    pym_img.down_scale_main[i].stride = pym_buffer->pym[i].stride_size;
    pym_img.down_scale_main[i].y_paddr = pym_buffer->pym[i].paddr[0];
    pym_img.down_scale_main[i].c_paddr = pym_buffer->pym[i].paddr[1];
    pym_img.down_scale_main[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->pym[i].addr[0]);
    pym_img.down_scale_main[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->pym[i].addr[1]);
  }
}
#endif

HbVioFbWrapperGlobal::HbVioFbWrapperGlobal(std::string hb_vio_cfg) {
  init_ = false;
  hb_vio_cfg_ = hb_vio_cfg;
}

HbVioFbWrapperGlobal::~HbVioFbWrapperGlobal() {
  if (init_) {
    DeInit();
    init_ = false;
  }
}

int HbVioFbWrapperGlobal::Init() {
#ifdef X3_X2_VIO
  if (hb_vio_init(hb_vio_cfg_.c_str()) < 0)
    return -1;
  if (hb_vio_start() < 0)
    return -1;
  init_ = true;
  return 0;
#endif
#ifdef X3_IOT_VIO
  if (iot_vio_init(hb_vio_cfg_.c_str()) < 0)
    return -1;
  if (iot_vio_start() < 0)
    return -1;
  init_ = true;
  return 0;
#endif
}

int HbVioFbWrapperGlobal::DeInit() {
#ifdef X3_X2_VIO
  if (hb_vio_stop() < 0)
    return -1;
  if (hb_vio_deinit() < 0)
    return -1;
  init_ = false;
  return 0;
#endif
#ifdef X3_IOT_VIO
  if (iot_vio_stop() < 0)
    return -1;
  if (iot_vio_deinit() < 0)
    return -1;
  init_ = false;
  return 0;
#endif
}

std::shared_ptr<PymImageFrame>
HbVioFbWrapperGlobal::GetImgInfo(std::string rgb_file, uint32_t *effective_w,
                                 uint32_t *effective_h) {
#ifdef X3_X2_VIO
  if (hb_vio_get_info(HB_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0) {
    std::cout << "get fb src fail!!!" << std::endl;
    return nullptr;
  }
  cv::Mat img = cv::imread(rgb_file);
  cv::Mat img_1080p;
  int width = 1920;
  int height = 1080;
  TransImage(&img, &img_1080p, width, height, effective_w, effective_h);

  uint8_t *output_data = nullptr;
  int output_size, output_1_stride, output_2_stride;

  HobotXStreamConvertImage(img_1080p.data, height * width * 3, width, height,
                           width * 3, 0, IMAGE_TOOLS_RAW_BGR,
                           IMAGE_TOOLS_RAW_YUV_NV12, &output_data, &output_size,
                           &output_1_stride, &output_2_stride);

  // adapter to x3 api, y and uv address is standalone
  int y_out_len = output_size / 3 * 2;
  int uv_out_len = output_size / 3;
  std::cout << "y_out_len:" << y_out_len << ", uv_out_len:" << uv_out_len
            << ", total_output_size:" << output_size << std::endl;

  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.y_vaddr),
         output_data, y_out_len);
  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.c_vaddr),
         output_data + y_out_len, uv_out_len);

  HobotXStreamFreeImage(output_data);
  hb_vio_set_info(HB_VIO_FEEDBACK_FLUSH, &process_info_);
  if (hb_vio_pym_process(&process_info_) < 0) {
    std::cout << "fb process fail!!!" << std::endl;
    return nullptr;
  }
  img_info_t *fb_img = new img_info_t();
  if (hb_vio_get_info(HB_VIO_PYM_INFO, fb_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return nullptr;
  }
  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  Convert(fb_img, *pym_image_frame_ptr);
  return pym_image_frame_ptr;
#endif
#ifdef X3_IOT_VIO
  if (iot_vio_get_info(IOT_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0) {
    std::cout << "get fb src fail!!!" << std::endl;
    return nullptr;
  }
  cv::Mat img = cv::imread(rgb_file);
  cv::Mat img_1080p;
  int width = 1920;
  int height = 1080;
  TransImage(&img, &img_1080p, width, height, effective_w, effective_h);

  uint8_t *output_data = nullptr;
  int output_size, output_1_stride, output_2_stride;

  HobotXStreamConvertImage(img_1080p.data, height * width * 3, width, height,
                           width * 3, 0, IMAGE_TOOLS_RAW_BGR,
                           IMAGE_TOOLS_RAW_YUV_NV12, &output_data, &output_size,
                           &output_1_stride, &output_2_stride);

  // adapter to x3 api, y and uv address is standalone
  int y_out_len = output_size / 3 * 2;
  int uv_out_len = output_size / 3;
  std::cout << "y_out_len:" << y_out_len << ", uv_out_len:" << uv_out_len
            << ", total_output_size:" << output_size << std::endl;
  memcpy(reinterpret_cast<uint8_t *>(process_info_.img_addr.addr[0]),
         output_data, y_out_len);
  memcpy(reinterpret_cast<uint8_t *>(process_info_.img_addr.addr[1]),
         output_data + y_out_len, uv_out_len);

  HobotXStreamFreeImage(output_data);
  iot_vio_set_info(IOT_VIO_FEEDBACK_FLUSH, &process_info_);
  if (iot_vio_pym_process(&process_info_) < 0) {
    std::cout << "fb process fail!!!" << std::endl;
    return nullptr;
  }
  pym_buffer_t *pym_img = new pym_buffer_t();
  if (iot_vio_get_info(IOT_VIO_PYM_INFO, pym_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return nullptr;
  }
  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  Convert(pym_img, *pym_image_frame_ptr);
  return pym_image_frame_ptr;
#endif
}

std::shared_ptr<PymImageFrame> HbVioFbWrapperGlobal::GetImgInfo(uint8_t *nv12,
                                                                int w, int h) {
#ifdef X3_X2_VIO
  if (hb_vio_get_info(HB_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0) {
    std::cout << "get fb src fail!!!" << std::endl;
    return nullptr;
  }

  // adapter to x3 api, y and uv address is standalone
  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.y_vaddr), nv12,
         w * h);
  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.c_vaddr),
         nv12 + w * h, w * h / 2);

  hb_vio_set_info(HB_VIO_FEEDBACK_FLUSH, &process_info_);
  if (hb_vio_pym_process(&process_info_) < 0) {
    std::cout << "fb process fail!!!" << std::endl;
    return nullptr;
  }
  img_info_t *fb_img = new img_info_t();
  if (hb_vio_get_info(HB_VIO_PYM_INFO, fb_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return nullptr;
  }
  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  Convert(fb_img, *pym_image_frame_ptr);
  return pym_image_frame_ptr;
#endif
#ifdef X3_IOT_VIO
  if (iot_vio_get_info(IOT_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0) {
    std::cout << "get fb src fail!!!" << std::endl;
    return nullptr;
  }

  // adapter to x3 api, y and uv address is standalone
  memcpy(reinterpret_cast<uint8_t *>(process_info_.img_addr.addr[0]), nv12,
         w * h);
  memcpy(reinterpret_cast<uint8_t *>(process_info_.img_addr.addr[1]),
         nv12 + w * h, w * h / 2);

  iot_vio_set_info(IOT_VIO_FEEDBACK_FLUSH, &process_info_);
  if (iot_vio_pym_process(&process_info_) < 0) {
    std::cout << "fb process fail!!!" << std::endl;
    return nullptr;
  }
  pym_buffer_t *pym_img = new pym_buffer_t();
  if (iot_vio_get_info(IOT_VIO_PYM_INFO, pym_img) < 0) {
    std::cout << "iot_vio_get_info fail!!!" << std::endl;
    return nullptr;
  }
  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  Convert(pym_img, *pym_image_frame_ptr);
  return pym_image_frame_ptr;
#endif
}

int HbVioFbWrapperGlobal::FreeImgInfo(
    std::shared_ptr<PymImageFrame> pym_image) {
#ifdef X3_X2_VIO
  // src image buffer need be free by next module
  if (hb_vio_free_info(HB_VIO_SRC_INFO, &process_info_) < 0)
    return -1;
  img_info_t *fb_img = static_cast<img_info_t *>(pym_image->context);
  if (fb_img) {
    int ret = hb_vio_free(fb_img);
    delete fb_img;
    pym_image->context = nullptr;
    return ret;
  }
  return 0;
#endif
#ifdef X3_IOT_VIO
  // src image buffer need be free by next module
  if (iot_vio_free_info(IOT_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0)
    return -1;
  pym_buffer_t *pym_img = static_cast<pym_buffer_t *>(pym_image->context);
  if (pym_img) {
    int ret = iot_vio_free(pym_img);
    delete pym_img;
    pym_image->context = nullptr;
    return ret;
  }
  return 0;
#endif
}

void HbVioFbWrapperGlobal::TransImage(cv::Mat *src_mat, cv::Mat *dst_mat,
                                      int dst_w, int dst_h,
                                      uint32_t *effective_w,
                                      uint32_t *effective_h) {
  if (src_mat->rows == dst_h && src_mat->cols == dst_w) {
    *dst_mat = src_mat->clone();
    *effective_h = dst_h;
    *effective_w = dst_w;
  } else {
    *dst_mat = cv::Mat(dst_h, dst_w, CV_8UC3, cv::Scalar::all(0));
    int x2 = dst_w < src_mat->cols ? dst_w : src_mat->cols;
    int y2 = dst_h < src_mat->rows ? dst_h : src_mat->rows;
    (*src_mat)(cv::Rect(0, 0, x2, y2))
        .copyTo((*dst_mat)(cv::Rect(0, 0, x2, y2)));
    *effective_w = x2;
    *effective_h = y2;
  }
}

HbVioMonoCameraGlobal::HbVioMonoCameraGlobal(std::string hb_vio_cfg,
                                             std::string camera_cfg) {
  init_ = false;
  hb_vio_cfg_ = hb_vio_cfg;
  camera_cfg_ = camera_cfg;
  camera_idx_ = 0;
}

HbVioMonoCameraGlobal::~HbVioMonoCameraGlobal() {
  if (init_) {
    DeInit();
    init_ = false;
  }
}

int HbVioMonoCameraGlobal::Init() {
#ifdef X3_X2_VIO
  if (hb_cam_init(camera_idx_, camera_cfg_.c_str()) < 0)
    return -1;
  if (hb_vio_init(hb_vio_cfg_.c_str()) < 0)
    return -1;
  if (hb_cam_start(camera_idx_) < 0)
    return -1;
  if (hb_vio_start() < 0)
    return -1;
  init_ = true;
  return 0;
#endif
#ifdef X3_IOT_VIO
  if (iot_vio_init(hb_vio_cfg_.c_str()) < 0) return -1;
  if (iot_cam_init(camera_idx_, camera_cfg_.c_str()) < 0) return -1;
  if (iot_vio_start() < 0) return -1;
  if (iot_cam_start(camera_idx_) < 0) return -1;
  init_ = true;
  return 0;
#endif
}

int HbVioMonoCameraGlobal::DeInit() {
#ifdef X3_X2_VIO
  if (hb_vio_stop() < 0)
    return -1;
  if (hb_cam_stop(camera_idx_) < 0)
    return -1;
  if (hb_vio_deinit() < 0)
    return -1;
  if (hb_cam_deinit(camera_idx_) < 0)
    return -1;
  init_ = false;
  return 0;
#endif
#ifdef X3_IOT_VIO
  if (iot_vio_stop() < 0) return -1;
  if (iot_cam_stop(camera_idx_) < 0) return -1;
  if (iot_vio_deinit() < 0) return -1;
  if (iot_cam_deinit(camera_idx_) < 0) return -1;
  init_ = false;
  return 0;
#endif
}

std::shared_ptr<PymImageFrame> HbVioMonoCameraGlobal::GetImage() {
#ifdef X3_X2_VIO
  img_info_t *pym_img = new img_info_t();
  if (hb_vio_get_info(HB_VIO_PYM_INFO, pym_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    delete pym_img;
    return nullptr;
  }
  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  Convert(pym_img, *pym_image_frame_ptr);
  return pym_image_frame_ptr;
#endif
#ifdef X3_IOT_VIO
  pym_buffer_t *pym_img = new pym_buffer_t();
  if (iot_vio_get_info(HB_VIO_PYM_INFO, pym_img) < 0) {
    std::cout << "iot_vio_get_info fail!!!" << std::endl;
    delete pym_img;
    return nullptr;
  }
  auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
  Convert(pym_img, *pym_image_frame_ptr);
  return pym_image_frame_ptr;
#endif
}

int HbVioMonoCameraGlobal::Free(std::shared_ptr<PymImageFrame> pym_image) {
#ifdef X3_X2_VIO
  img_info_t *pym_img = static_cast<img_info_t *>(pym_image->context);
  if (pym_img) {
    int ret = hb_vio_free(pym_img);
    delete pym_img;
    pym_image->context = nullptr;
    return ret;
  }
  return 0;
#endif
#ifdef X3_IOT_VIO
  pym_buffer_t *pym_img = static_cast<pym_buffer_t *>(pym_image->context);
  if (pym_img) {
    int ret = iot_vio_free_info(HB_VIO_PYM_INFO, pym_img);
    delete pym_img;
    pym_image->context = nullptr;
    return ret;
  }
  return 0;
#endif
}
