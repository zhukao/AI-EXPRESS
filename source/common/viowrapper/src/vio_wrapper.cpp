/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: hb_vio_fb_wrapper.h
 * @Brief: declaration of the hb_vio_fb_wrapper
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-05-23 14:18:28
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-23 16:16:58
 */
#include "./vio_wrapper.h"

#include "hobotxstream/image_tools.h"
#ifdef X2
#include "x2_camera.h"
#endif

int HbVioFbWrapper::Init() {
  if (hb_vio_init(hb_vio_cfg_.c_str()) < 0) return -1;
  if (hb_vio_start() < 0) return -1;
  init_ = true;
  return 0;
}

int HbVioFbWrapper::DeInit() {
  if (hb_vio_stop() < 0) return -1;
  if (hb_vio_deinit() < 0) return -1;
  init_ = false;
  return 0;
}

int HbVioFbWrapper::FreeImgInfo(img_info_t *fb_img) {
  // src image buffer need be free by next module 
  if (hb_vio_free_info(HB_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0)
    return -1;
  if (hb_vio_free(fb_img) < 0) return -1;

  return 0;
}

int HbVioFbWrapper::GetImgInfo(std::string rgb_file, img_info_t *fb_img,
                               uint32_t *effective_w, uint32_t *effective_h) {
  if (hb_vio_get_info(HB_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0) {
    std::cout << "get fb src fail!!!" << std::endl;
    return -1;
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
  std::cout << "y_out_len:"  << y_out_len << ", uv_out_len:" << uv_out_len
            << ", total_output_size:" << output_size << std::endl;

  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.y_vaddr),
         output_data, y_out_len);
  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.c_vaddr),
         output_data+y_out_len, uv_out_len);

  HobotXStreamFreeImage(output_data);
  hb_vio_set_info(HB_VIO_FEEDBACK_FLUSH, &process_info_);
  if (hb_vio_pym_process(&process_info_) < 0) {
    std::cout << "fb process fail!!!" << std::endl;
    return -1;
  }
  if (hb_vio_get_info(HB_VIO_PYM_INFO, fb_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return -1;
  }
  return 0;
}

int HbVioFbWrapper::GetImgInfo(uint8_t *nv12, int w, int h,
                               img_info_t *fb_img) {
  if (hb_vio_get_info(HB_VIO_FEEDBACK_SRC_INFO, &process_info_) < 0) {
    std::cout << "get fb src fail!!!" << std::endl;
    return -1;
  }

  // adapter to x3 api, y and uv address is standalone
  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.y_vaddr), nv12,
         w * h);
  memcpy(reinterpret_cast<uint8_t *>(process_info_.src_img.c_vaddr), nv12 + w * h,
         w * h / 2);

  hb_vio_set_info(HB_VIO_FEEDBACK_FLUSH, &process_info_);
  if (hb_vio_pym_process(&process_info_) < 0) {
    std::cout << "fb process fail!!!" << std::endl;
    return -1;
  }
  if (hb_vio_get_info(HB_VIO_PYM_INFO, fb_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return -1;
  }
  return 0;
}

void HbVioFbWrapper::TransImage(cv::Mat *src_mat, cv::Mat *dst_mat, int dst_w,
                                int dst_h, uint32_t *effective_w,
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

int HbVioMonoCamera::Init() {
  if (hb_cam_init(camera_idx_, camera_cfg_.c_str()) < 0) return -1;
  if (hb_vio_init(hb_vio_cfg_.c_str()) < 0) return -1;
  if (hb_cam_start(camera_idx_) < 0) return -1;
  if (hb_vio_start() < 0) return -1;
  init_ = true;
  return 0;
}

int HbVioMonoCamera::DeInit() {
  if (hb_vio_stop() < 0) return -1;
  if (hb_cam_stop(camera_idx_) < 0) return -1;
  if (hb_vio_deinit() < 0) return -1;
  if (hb_cam_deinit(camera_idx_) < 0) return -1;
  init_ = false;
  return 0;
}

int HbVioMonoCamera::GetImage(img_info_t *pyd_img) {
  if (hb_vio_get_info(HB_VIO_PYM_INFO, pyd_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return -1;
  }
  return 0;
}

int HbVioMonoCamera::Free(img_info_t *pyd_img) { return hb_vio_free(pyd_img); }

int HbVioDualCamera::Init() {
  if (hb_cam_init(camera_idx_, camera_cfg_.c_str()) < 0) return -1;
  if (hb_vio_init(hb_vio_cfg_.c_str()) < 0) return -1;
  if (hb_cam_start(camera_idx_) < 0) return -1;
  if (hb_vio_start() < 0) return -1;
  init_ = true;
  return 0;
}
int HbVioDualCamera::DeInit() {
  if (hb_vio_stop() < 0) return -1;
  if (hb_vio_deinit() < 0) return -1;
  if (hb_cam_stop(camera_idx_) < 0) return -1;
  if (hb_cam_deinit(camera_idx_) < 0) return -1;
  init_ = false;
  return 0;
}

int HbVioDualCamera::GetMultiImage(mult_img_info_t *pyd_img) {
  if (hb_vio_get_info(HB_VIO_PYM_MULT_INFO, pyd_img) < 0) {
    std::cout << "hb_vio_get_info fail!!!" << std::endl;
    return -1;
  }
  return 0;
}

int HbVioDualCamera::Free(mult_img_info_t *pyd_img) {
  return hb_vio_mult_free(pyd_img);
}

