/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-09-26 22:54:07
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_VIOPROCESS_VIOPROCESS_H_
#define INCLUDE_VIOPROCESS_VIOPROCESS_H_

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_msg.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type_util.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hb_vio_interface.h"

using hobot::vision::PymImageFrame;
#if defined(X3_X2_VIO)
struct VioFeedbackContext {
  src_img_info_t src_info;
  img_info_t pym_img_info;
};
void Convert(img_info_t *img, PymImageFrame &pym_img);
#endif
#if defined(X3_IOT_VIO)
struct VioFeedbackContext {
  hb_vio_buffer_t src_info;
  pym_buffer_t pym_img_info;
};
void Convert(pym_buffer_t *pym_buffer, PymImageFrame &pym_img);
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 转换图像格式,
 * 支持将RGB转成RGB565、BGR
 *    将RGB565转成RGB、BGR
 *    将BGR转成RGB、RGB565、NV12
 *    将BGRA转成RGB、BGR、
 *    将NV12转成BGR、BGRA、RGBA
 * @param in_img [in] 输入图像
 * @param dst_img [out] 输出图像，会覆盖dst_img->data
 * @param dst_format [in] 转出图像的格式
 * @return int 0表示成功，<0则为错误码
 */
int HorizonConvertImage(HorizonVisionImage *in_img, HorizonVisionImage *dst_img,
                        HorizonVisionPixelFormat dst_format);

/**
 * @brief 保存到文件
 *        支持将BGR、NV12格式的数据保持至jpg/png（通过后缀名区分）文件中
 *
 * @param img [in] 输入图像
 * @param file_path [in] 保存文件路径
 * @return int 0表示成功，<0则为错误码
 */
int HorizonSave2File(HorizonVisionImage *img, const char *file_path);

/**
 * @brief 从文件加载至内存
 *        将jpg/png文件，加载至内存BGR中
 * @param file_path [in] 文件路径
 * @param img [in] 要填充的image 指针
 * @return int 0表示成功，<0则为错误码
 */
int HorizonFillFromFile(const char *file_path, HorizonVisionImage **img);

#ifdef __cplusplus
}
#endif

#endif
