/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-09-26 17:21:53
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#include <unistd.h>
#include <ctime>
#include <fstream>

#include "vioplugin/vioprocess.h"

#include "opencv2/opencv.hpp"

#include "hobotlog/hobotlog.hpp"

#if defined(X3_X2_VIO)
void Convert(img_info_t *img, PymImageFrame &pym_img) {
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
#endif  // X3_X2_VIO

#ifdef X3_IOT_VIO
void Convert(pym_buffer_t *pym_buffer, PymImageFrame &pym_img) {
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
    LOGD << "dxd1 : " << pym_addr->width;
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
    LOGD << "dxd2 : " << pym_buffer->pym[i].width;
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
#endif  // X3_IOT_VIO

int HorizonConvertImage(HorizonVisionImage *in_img, HorizonVisionImage *dst_img,
                        HorizonVisionPixelFormat dst_format) {
  if (!in_img || !dst_img)
    return kHorizonVisionErrorParam;
  dst_img->width = in_img->width;
  dst_img->height = in_img->height;
  dst_img->stride = in_img->stride;
  dst_img->stride_uv = in_img->stride_uv;
  dst_img->pixel_format = dst_format;
  if (dst_format != in_img->pixel_format) {
    switch (in_img->pixel_format) {
    case kHorizonVisionPixelFormatRawRGB: {
      cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB565: {
        auto dst_data_size = in_img->data_size / 3 * 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb565(in_img->height, in_img->width, CV_8UC2, dst_img->data);
        cv::cvtColor(rgb888, rgb565, CV_BGR2BGR565);
      } break;
      case kHorizonVisionPixelFormatRawBGR: {
        dst_img->data_size = in_img->data_size;
        dst_img->data =
            reinterpret_cast<uint8_t *>(std::malloc(dst_img->data_size));
        cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(rgb888, bgr888, CV_RGB2BGR);
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawRGB565: {
      cv::Mat rgb565(in_img->height, in_img->width, CV_8UC2, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB: {
        auto dst_data_size = (in_img->data_size >> 1) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(rgb565, rgb888, CV_BGR5652BGR);
      } break;
      case kHorizonVisionPixelFormatRawBGR: {
        auto dst_data_size = (in_img->data_size >> 1) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(rgb565, bgr888, CV_BGR5652RGB);
      } break;

      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawBGR: {
      cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB: {
        auto dst_data_size = in_img->data_size;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(bgr888, rgb888, CV_BGR2RGB);
      } break;
      case kHorizonVisionPixelFormatRawRGB565: {
        auto dst_data_size = in_img->data_size / 3 * 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb565(in_img->height, in_img->width, CV_8UC2, dst_img->data);
        cv::cvtColor(bgr888, rgb565, CV_RGB2BGR565);
      } break;
      case kHorizonVisionPixelFormatRawNV12: {
        auto dst_data_size = (in_img->height * in_img->width) * 3 / 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat yuv420_mat;
        cv::cvtColor(bgr888, yuv420_mat, cv::COLOR_BGR2YUV_I420);
        // copy y data
        int y_size = in_img->height * in_img->width;
        auto *yuv420_ptr = yuv420_mat.ptr<uint8_t>();
        memcpy(dst_img->data, yuv420_ptr, y_size);
        // copy uv data
        int uv_stride = in_img->width * in_img->height / 4;
        uint8_t *uv_data = dst_img->data + y_size;
        for (int i = 0; i < uv_stride; ++i) {
          *(uv_data++) = *(yuv420_ptr + y_size + i);
          *(uv_data++) = *(yuv420_ptr + y_size + uv_stride + i);
        }
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawBGRA: {
      cv::Mat bgra(in_img->height, in_img->width, CV_8UC4, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB: {
        auto dst_data_size = (in_img->height * in_img->width) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(bgra, rgb888, CV_BGRA2RGB);
      } break;
      case kHorizonVisionPixelFormatRawBGR: {
        auto dst_data_size = (in_img->height * in_img->width) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(bgra, bgr888, CV_BGRA2BGR);
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawNV12: {
      cv::Mat nv12((in_img->height * 3) >> 1, in_img->width, CV_8UC1,
                   in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawBGR: {
        auto dst_data_size = (in_img->height * in_img->width) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgr(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(nv12, bgr, CV_YUV2BGR_NV12);
      } break;
      case kHorizonVisionPixelFormatRawBGRA: {
        auto dst_data_size = (in_img->height * in_img->width) << 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgra(in_img->height, in_img->width, CV_8UC4, dst_img->data);
        cv::cvtColor(nv12, bgra, CV_YUV2BGRA_NV12);
      } break;
      case kHorizonVisionPixelFormatRawRGBA: {
        auto dst_data_size = (in_img->height * in_img->width) << 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgba(in_img->height, in_img->width, CV_8UC4, dst_img->data);
        cv::cvtColor(nv12, rgba, CV_YUV2RGBA_NV12);
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    default:
      return kHorizonVisionErrorNoImpl;
    }
  } else {
    dst_img->data_size = in_img->data_size;
    dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(in_img->data_size));
    std::memcpy(dst_img->data, in_img->data, in_img->data_size);
  }
  return kHorizonVisionSuccess;
}

int HorizonSave2File(HorizonVisionImage *img, const char *file_path) {
  if (!img || !file_path)
    return kHorizonVisionErrorParam;
  if (0 == strlen(file_path))
    return kHorizonVisionErrorParam;
  if (kHorizonVisionPixelFormatRawBGR == img->pixel_format) {
    cv::Mat bgr_img_mat(img->height, img->width, CV_8UC3, img->data);
    cv::imwrite(file_path, bgr_img_mat);
    return kHorizonVisionSuccess;
  }
  HorizonVisionImage *bgr_img;
  HorizonVisionAllocImage(&bgr_img);
  int ret = HorizonConvertImage(img, bgr_img, kHorizonVisionPixelFormatRawBGR);
  if (ret != kHorizonVisionSuccess)
    return ret;
  // save to file
  cv::Mat bgr_img_mat(bgr_img->height, bgr_img->width, CV_8UC3, bgr_img->data);
  cv::imwrite(file_path, bgr_img_mat);
  HorizonVisionFreeImage(bgr_img);
  return kHorizonVisionSuccess;
}

int HorizonFillFromFile(const char *file_path, HorizonVisionImage **ppimg) {
  if (!ppimg) {
    return kHorizonVisionErrorParam;
  }
  if (*ppimg || !file_path)
    return kHorizonVisionErrorParam;

  if (!strcmp(file_path, "")) {
    return kHorizonVisionErrorParam;
  }
  if (access(file_path, F_OK) != 0) {
    LOGE << "file not exist: " << file_path;
    return kHorizonVisionOpenFileFail;
  }

  try {
    auto bgr_mat = cv::imread(file_path);
    if (!bgr_mat.data) {
      LOGF << "Failed to call imread for " << file_path;
      return kHorizonVisionFailure;
    }
    HorizonVisionAllocImage(ppimg);
    auto &img = *ppimg;
    img->pixel_format = kHorizonVisionPixelFormatRawBGR;
    img->data_size = static_cast<uint32_t>(bgr_mat.total() * 3);
    // HorizonVisionFreeImage call std::free to free data
    img->data = reinterpret_cast<uint8_t *>(
        std::calloc(img->data_size, sizeof(uint8_t)));
    std::memcpy(img->data, bgr_mat.data, img->data_size);
    img->width = static_cast<uint32_t>(bgr_mat.cols);
    img->height = static_cast<uint32_t>(bgr_mat.rows);
    img->stride = static_cast<uint32_t>(bgr_mat.cols);
    img->stride_uv = static_cast<uint32_t>(bgr_mat.cols);
  } catch (const cv::Exception &e) {
    LOGF << "Exception to call imread for " << file_path;
    return kHorizonVisionOpenFileFail;
  }
  return kHorizonVisionSuccess;
}
