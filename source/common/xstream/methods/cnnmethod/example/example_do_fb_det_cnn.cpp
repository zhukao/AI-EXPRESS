/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: example_do_pose_lmk.cpp
 * @Brief:
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-04-15 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-04-15 15:18:10
 */

#include <stdint.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "CNNMethod/CNNMethod.h"
#include "CNNMethod/util/util.h"
#include "FasterRCNNMethod/FasterRCNNMethod.h"
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
#include "hobotxstream/method.h"
#include "hobotxstream/method_factory.h"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "opencv2/opencv.hpp"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::XStreamData;
using hobot::vision::BBox;
static void Usage() {
  std::cout << "./example do_fb_det_cnn [pose_lmk|age_gender|anti_spf] "
               "xstream_cfg_file fb_cfg img_list out_file\n";
}
extern void DumpAgeGender(std::vector<BaseDataPtr> &, std::ostream &,
                          std::string);
extern void DumpPoseLmk(std::vector<BaseDataPtr> &, std::ostream &,
                        std::string);
void PrintPoseLmk(std::vector<BaseDataPtr> &);
void DumpAntiSpfAndRoi(std::vector<BaseDataPtr> &, std::ostream &, std::string);

int DoFbDetCNN(int argc, char **argv) {
  if (argc < 6) {
    Usage();
    return 1;
  }
  std::string model_name(argv[1]);
  std::string cfg_file(argv[2]);
  std::string fb_cfg(argv[3]);
  std::string img_list(argv[4]);
  std::string output_file(argv[5]);

  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", cfg_file.c_str());
  flow->Init();
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(fb_cfg);
  fb_handle.Init();

  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::ofstream output(output_file, std::ios::out);
  while (getline(img_list_file, img_path)) {
    std::vector<std::string> strs;
    xstream::split_string(img_path, strs, " ");
    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(strs[0], &feed_back_info, &effective_w, &effective_h);
    xstream::InputDataPtr inputdata(new xstream::InputData());

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    if (model_name == "pose_lmk") {
      DumpPoseLmk(out->datas_, output, strs[0]);
      PrintPoseLmk(out->datas_);
    } else if (model_name == "age_gender") {
      DumpAgeGender(out->datas_, output, strs[0]);
    } else if (model_name == "anti_spf") {
      DumpAntiSpfAndRoi(out->datas_, output, strs[0]);
    }
    fb_handle.FreeImgInfo(&feed_back_info);
  }
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  fb_handle.Init();

  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::ofstream output(output_file, std::ios::out);
  while (getline(img_list_file, img_path)) {
    std::vector<std::string> strs;
    xstream::split_string(img_path, strs, " ");
    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(strs[0], &effective_w, &effective_h);
    xstream::InputDataPtr inputdata(new xstream::InputData());
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    if (model_name == "pose_lmk") {
      DumpPoseLmk(out->datas_, output, strs[0]);
      PrintPoseLmk(out->datas_);
    } else if (model_name == "age_gender") {
      DumpAgeGender(out->datas_, output, strs[0]);
    } else if (model_name == "anti_spf") {
      DumpAntiSpfAndRoi(out->datas_, output, strs[0]);
    }
    fb_handle.FreeImgInfo(py_img);
  }
#endif
  delete flow;
  return 0;
}

void PrintPoseLmk(std::vector<xstream::BaseDataPtr> &result) {
  auto lmks = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  auto poses = std::static_pointer_cast<xstream::BaseDataVector>(result[1]);
  auto xstream_pyramid =
      std::static_pointer_cast<xstream::XStreamData<ImageFramePtr>>(result[2]);
  auto pyramid = std::static_pointer_cast<hobot::vision::PymImageFrame>(
      xstream_pyramid->value);
  auto rois = std::static_pointer_cast<xstream::BaseDataVector>(result[3]);

  static int dump_index = 0;
#ifdef X2
  auto &img_info = pyramid->img.src_img;
  auto height = img_info.height;
  auto width = img_info.width;
  auto y_addr = img_info.y_vaddr;
  auto uv_addr = img_info.c_vaddr;
#endif
#ifdef X3
  auto height = pyramid->Height();
  auto width = pyramid->Width();
  auto y_addr = pyramid->Data();
  auto uv_addr = pyramid->DataUV();
#endif
  cv::Mat yuv420p;
  yuv420p.create(height * 3 / 2, width, CV_8UC1);
  memcpy(yuv420p.data, reinterpret_cast<uint8_t *>(y_addr), height * width);
  memcpy(yuv420p.data + height * width,
         reinterpret_cast<uint8_t *>(uv_addr),
         height * width / 2);
  cv::Mat img;
  cv::cvtColor(yuv420p, img, CV_YUV2BGR_NV12);
  int target_size = lmks->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto lmk = std::static_pointer_cast<xstream::XStreamData<
               hobot::vision::Landmarks>>(lmks->datas_[target_idx]);
    auto pose =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::Pose3D>>(
            poses->datas_[target_idx]);
    if (lmk->state_ != xstream::DataState::VALID) {
      std::cout << "lmk invalid";
      continue;
    }
    for (auto &point : lmk->value.values) {
      cv::rectangle(img, cv::Point(point.x - 2, point.y - 2),
                    cv::Point(point.x + 2, point.y + 2), CV_RGB(255, 0, 0), 2);
    }
  }
  for (size_t roi_idx = 0; roi_idx < rois->datas_.size(); roi_idx++) {
    auto roi =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
            rois->datas_[roi_idx]);
    cv::rectangle(img,
                  cv::Point(roi->value.x1,
                  roi->value.y1),
                  cv::Point(roi->value.x2,
                  roi->value.y2),
                  CV_RGB(0, 255, 0),
                  2);
  }
  cv::imwrite(std::to_string(dump_index++) + "_pose_lmk.jpg", img);
}

void DumpAntiSpfAndRoi(std::vector<xstream::BaseDataPtr> &result,
                       std::ostream &os, std::string id) {
  os << id;
  auto anti_spfs = std::static_pointer_cast<BaseDataVector>(result[0]);
  auto rois = std::static_pointer_cast<BaseDataVector>(result[1]);
  auto target_size = anti_spfs->datas_.size();
  for (size_t target_idx = 0; target_idx < target_size; target_idx++) {
    auto anti_spf = std::static_pointer_cast<
        xstream::XStreamData<hobot::vision::Attribute<int>>>(
        anti_spfs->datas_[target_idx]);
    auto p_roi =
        std::static_pointer_cast<XStreamData<BBox>>(rois->datas_[target_idx]);
    if (anti_spf->state_ == xstream::DataState::VALID) {
      os << " " << p_roi->value.x1 << " " << p_roi->value.y1 << " "
         << p_roi->value.x2 << " " << p_roi->value.y2;
      os << " " << anti_spf->value.score;
    }
  }
  os << std::endl;
}
