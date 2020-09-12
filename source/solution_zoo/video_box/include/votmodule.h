/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#ifndef INCLUDE_VOTMODULE_H_
#define INCLUDE_VOTMODULE_H_

#include <array>
#include <vector>
#include <string>
#include <mutex>
#include "opencv2/opencv.hpp"

#include "hb_vio_interface.h"
#include "mediapipemanager/basicmediamoudle.h"
#include "smartplugin_box/traffic_info.h"
#include "smartplugin_box/character_font.h"
#include "hobotxsdk/xstream_data.h"
#include "xstream/vision_type/include/horizon/vision_type/vision_type.h"

using horizon::vision::xproto::smartplugin::VehicleInfo;

namespace horizon {
namespace vision {

struct VotData {
  uint32_t channel;
  std::vector<std::array<int, 4>> boxes;
  std::vector<std::array<int, 2>> points;
  std::vector<VehicleInfo> vehicle_infos;
  char *y_virtual_addr;
  char *uv_virtual_addr;
};

typedef struct smart_vo_s {
  float box_face_thr = 0.95;
  float box_head_thr = 0.95;
  float box_body_thr = 0.95;
  float lmk_thr = 0.0;
  float kps_thr = 0.50;
  float box_veh_thr = 0.995;
  bool plot_fps = false;
} smart_vo_cfg_t;

// class VotModule : public BasicMediaModule
// {
// public:
//   VotModule(/* args */);
//   ~VotModule();
//   virtual int Init(uint32_t group_id,
//                    const PipeModuleInfo *module_info) override;
//   virtual int Start() override;
//   virtual int Input(void *data) override;
//   virtual int Output(void **data) override;
//   virtual int OutputBufferFree(void *data) override;
//   virtual int Stop() override;
//   virtual int DeInit() override;

// protected:

// private:
//   uint32_t group_id_;
//   uint32_t timeout_;
//   uint32_t image_width_;
//   uint32_t image_height_;
//   char *buffer_[4];
//   // uint32_t frameDepth_;
//   // uint32_t buffer_index_;
//   // std::vector<pym_buffer_t> buffers_;
// };

class VotModule {
public:
  VotModule(/* args */);
  ~VotModule();
  int Init(uint32_t group_id, const PipeModuleInfo *module_info,
           const smart_vo_cfg_t& smart_vo_cfg);
  int Start();
  int Input(void *data);
  int Input(void *data, const xstream::OutputDataPtr& xstream_out);
  int Output(void **data);
  int OutputBufferFree(void *data);
  int Stop();
  int DeInit();

protected:
private:
  uint32_t group_id_;
  uint32_t timeout_;
  uint32_t image_width_;
  uint32_t image_height_;
  char *buffer_;
  // uint32_t frameDepth_;
  // uint32_t buffer_index_;
  // std::vector<pym_buffer_t> buffers_;
  smart_vo_cfg_t vo_plot_cfg_;

 private:
  typedef struct logo_img_cache_s {
    int top_image_width_;
    int top_image_height_;
    cv::Mat top_yuv_mat_;
    cv::Mat top_bgr_mat_;
    cv::Mat top_bgr_mat_left_;
    cv::Mat top_bgr_mat_right_;

    int bottom_image_width_;
    int bottom_image_height_;
    cv::Mat bottom_yuv_mat_;
    cv::Mat bottom_bgr_mat_;
    cv::Mat bottom_bgr_mat_left_;
    cv::Mat bottom_bgr_mat_right_;
  } logo_img_cache_t;
  logo_img_cache_t logo_img_cache_;
  int ParseLogoImg(const std::string& file_name_top,
                   const std::string& file_name_bottom,
                   int pad_width = 1920, int pad_height = 1080);
  int ParseBottomLogoImg(const std::string& file_name_bottom_left,
                   const std::string& file_name_bottom_rigth);
  void padding_logo(char *buf, int pad_width = 1920, int pad_height = 1080);

  int PlotFont(char *y, const char* font_buf,
               int x0, int y0, int bg_width = 1920, int bg_height = 1080);

  // create 960*540 bgr
  int PlotSmartData(
          cv::Mat& bgr,
          bool face, bool head, bool body, bool kps, bool veh,
          VotData *vot_data, const xstream::OutputDataPtr& xstream_out);
  // padding 540p bgr to 1080p nv12
  void bgr_540p_to_nv12(cv::Mat& bgr_mat, char *buf, int channel);
  // convert 540p bgr to 540p nv12
  void bgr_to_nv12(uint8_t *bgr, uint8_t *buf);

  // position 0:top, 1:bottom
  // left_right 0:left, 1:right
  int Drawlogo(const cv::Mat &logo, cv::Mat *bgr,
               int position, int left_right = 0);
};

}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_VOTMODULE_H_