/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-02 03:42:16
 * @Version: v0.0.1
 * @Brief:  convert to xroc inputdata from input VioMessage
 * @Note:  extracted from xperson repo.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-08-02 05:22:15
 */
#ifndef INCLUDE_SMARTPLUGIN_CONVERT_H_
#define INCLUDE_SMARTPLUGIN_CONVERT_H_

#include <string>
#include "xproto_msgtype/vioplugin_data.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "opencv2/opencv.hpp"
#ifdef X3_MEDIA_CODEC
#include "media_codec/media_codec_manager.h"
#endif

namespace horizon {
namespace vision {
namespace xproto {
namespace visualplugin {

using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::basic_msgtype::SmartMessage;

class Convertor {
 public:
  /**
   * @brief get yuv data from VioMessage (only for mono)
   * @param out yuv_img - cv::Mat with yuv type
   * @param in vio_msg
   * @param in layer - number
   * @return error code
   */
  static int GetYUV(cv::Mat &yuv_img, VioMessage *vio_msg, int level);

#ifdef X3_MEDIA_CODEC
  /**
   * @brief get yuv data from VioMessage (only for mono)
   * @param out frame_buf - iot_venc_src_buf_t with yuv type
   * @param in vio_msg
   * @param in layer - number
   * @param in use_vb - if use vb memory or not
   * @return error code
   */
  static int GetYUV(iot_venc_src_buf_t *frame_buf, VioMessage *vio_msg,
          int level, int use_vb);
#endif

  /**
   * @brief convert cv::Mat(format of yuv) to JPG data
   * @param out img_buf
   * @param in yuv_img
   * @param in quality
   * @return bool
   */
  static bool YUV2JPG(std::vector<uchar> &img_buf,
      cv::Mat &yuv_img, int quality);

  /**
   * @brief package smart messge to proto
   * @param out data - protobuf serialized string
   * @param in smart_msg
   * @param in type - VisualConfig::SmartType
   * @return error code
   */
  static int PackSmartMsg(std::string &data, SmartMessage *smart_msg, int type);

 private:

};

}  // namespace visualplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  //  INCLUDE_SMARTPLUGIN_CONVERT_H_
