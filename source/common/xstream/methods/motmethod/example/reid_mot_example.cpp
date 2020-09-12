/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xsoul framework
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.20
 */

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <cassert>

#include "hobotxstream/data_types/number.h"
#include "hobotxstream/data_types/bbox.h"
#include "hobotxstream/data_types/tracklet.h"
#include "hobotxstream/data_types/image_frame.h"
#include "hobotxstream/data_types/landmark.h"
#include "hobotxsdk/xstream_sdk.h"
#include "MOTMethod/MOT/ReID.h"


class Callback {
 public:
  Callback() = default;

  ~Callback() = default;

  uint64_t timestamp_ = 0;

  cv::Mat img_;

  void Using_TrackID_Bbox(const xstream::OutputDataPtr &output) {
    using xstream::BaseDataVector;

    assert(output->datas_.size() >= 4);
    auto &track_id_list = output->datas_[0];
    auto &bbox_list = output->datas_[1];
    auto &disappeared_track_id_list = output->datas_[2];

    if (track_id_list->error_code_ < 0 || bbox_list->error_code_ < 0) {
      std::cout << "track_id_list data error: " <<
      track_id_list->error_code_ << std::endl;
      std::cout << "bbox_list data error: " <<
      bbox_list->error_code_ << std::endl;
    }
    std::cout << "track_id_list data type_name : " << track_id_list->type_
              << " " << track_id_list->name_ << std::endl;
    std::cout << "bbox_list data type_name : " << bbox_list->type_
              << " " << bbox_list->name_ << std::endl;
    std::cout << "disappeared_track_id_list data type_name : "
              << bbox_list->type_
              << " " << disappeared_track_id_list->name_ << std::endl;
    auto *track_id_data = dynamic_cast<BaseDataVector *>(track_id_list.get());
    auto *bbox_data = dynamic_cast<BaseDataVector *>(bbox_list.get());
    auto *disappeared_track_id_data =
        dynamic_cast<BaseDataVector *>(disappeared_track_id_list.get());
    std::cout << "track_id_list data size: "
    << track_id_data->datas_.size() << std::endl;
    std::cout << "bbox_list data data size: "
    << bbox_data->datas_.size() << std::endl;
    std::cout << "disappeared_track_id_list data data size: "
              << disappeared_track_id_data->datas_.size() << std::endl;

    assert(track_id_data->datas_.size() == bbox_data->datas_.size());

    for (auto i = 0; i < track_id_data->datas_.size(); i++) {
      auto &ptrack_id = track_id_data->datas_[i];
      auto &pbbox = bbox_data->datas_[i];
      assert("Number" == ptrack_id->type_);
      assert("BBox" == pbbox->type_);
      auto track_id = std::static_pointer_cast<xstream::Number>(ptrack_id);
      auto bbox = std::static_pointer_cast<xstream::BBox>(pbbox);

      auto x0 = static_cast<int>(bbox->values_[0]);
      auto y0 = static_cast<int>(bbox->values_[1]);
      auto x1 = static_cast<int>(bbox->values_[2]);
      auto y1 = static_cast<int>(bbox->values_[3]);

#define DRAW_RES
#ifdef DRAW_RES
      rectangle(img_, cv::Point(x0, y0),
      cv::Point(x1, y1), cv::Scalar(0, 255, 255), 1, 8);
      char i_ch[10];
      snprintf(i_ch, sizeof(i_ch), "%d",
      static_cast<unsigned>(track_id->value_));
      ShowText(img_, i_ch,
               cv::Point((x0 + x1)/2 - 5, (y0 + y1)/2 - 30),
               cv::Scalar(0, 255, 255), 1);
#endif
      std::cout << "id and bbox:"
                << " track_id:" << track_id->value_
                << " bbox:" << bbox->values_[0]
                << "," << bbox->values_[1]
                << "," << bbox->values_[2]
                << "," << bbox->values_[3]
                << "," << bbox->score_
                << std::endl;
    }
#ifdef DRAW_RES
    cv::imwrite("./img_output/result"+
    std::to_string(timestamp_) + ".jpg", img_);
#endif
  }

  void Using_Tracktet(const xstream::OutputDataPtr &output) {
    using xstream::BaseDataVector;
    assert(output->datas_.size() >= 4);

    auto &tracklet_list = output->datas_[3];
    if (tracklet_list->error_code_ < 0) {
      std::cout << "tracklet_list data error: "
      << tracklet_list->error_code_ << std::endl;
    }
    std::cout << "data type_name : "
    << tracklet_list->type_ << " " << tracklet_list->name_
              << std::endl;
    auto *pdata = dynamic_cast<BaseDataVector *>(tracklet_list.get());
    std::cout << "tracklet_list data size: "
    << pdata->datas_.size() << std::endl;
    for (const auto &item : pdata->datas_) {
      assert("Tracklet" == item->type_);
      auto tracklet = std::static_pointer_cast<xstream::Tracklet>(item);

      auto tail_frame_id = tracklet->GetTailFrameId();
      auto tail_bbox = tracklet->GetTailBBox();
      auto tail_reid_feature = tracklet->GetTailReidFeature();
      auto tail_orientation = tracklet->GetTailOrientation();
      auto tail_occulusion = tracklet->GetTailOcclusion();
      auto tail_skeleton = tracklet->GetTailSkeleton();

      xstream::BBox timestamp_bbox;
      xstream::Feature timestamp_feature;
      xstream::Orientation timestamp_orientation;
      bool timestamp_occlusion;
      xstream::Landmark timestamp_skeleton;

      /// Warning: This interface is not thread safe
      /// now and only used for debugging and evaluation.
      tracklet->GetBBoxOfTimeStamp(timestamp_, timestamp_bbox);

      /// Warning: This interface is not thread safe
      /// now and only used for debugging and evaluation.
      tracklet->GetReidFeatureOfTimeStamp(timestamp_, timestamp_feature);

      /// Warning: This interface is not thread safe
      /// now and only used for debugging and evaluation.
      tracklet->GetOrientationOfTimeStamp(timestamp_, timestamp_orientation);

      /// Warning: This interface is not thread safe
      /// now and only used for debugging and evaluation.
      tracklet->GetOcclusionOfTimeStamp(timestamp_, timestamp_occlusion);

      /// Warning: This interface is not thread safe
      /// now and only used for debugging and evaluation.
      tracklet->GetSkeletonOfTimeStamp(timestamp_, timestamp_skeleton);

      std::cout << "tracklet:"
                << " track_id:" << tracklet->track_id_
                << " tail_bbox:" << tail_bbox->values_[0]
                << "," << tail_bbox->values_[1]
                << "," << tail_bbox->values_[2]
                << "," << tail_bbox->values_[3]
                << "," << tail_bbox->score_
                << std::endl;
    }
  }

  void OnCallback(const xstream::OutputDataPtr &output) {
    using xstream::BaseDataVector;
    std::cout << "======================" << std::endl;
    std::cout << "seq: " << output->sequence_id_ << std::endl;
    std::cout << "error_code: " << output->error_code_ << std::endl;
    std::cout << "error_detail_: " << output->error_detail_ << std::endl;
    std::cout << "datas_ size: " << output->datas_.size() << std::endl;

    // example of using track id and bbox
    Using_TrackID_Bbox(output);

    // example of using tracklet
    Using_Tracktet(output);
  }

 private:
  void ShowText(cv::Mat &frame, char* buf, const cv::Point& pos,
                       const cv::Scalar &color = cv::Scalar(0, 0, 255),
                       const double font_scale = 0.2,
                       const int thickness = 2) {
    putText(frame, buf, pos, CV_FONT_HERSHEY_COMPLEX,
    font_scale, color, thickness);
  }
};


int main(int argc, char* argv[]) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;
  using xstream::ImageFrame;
  using xstream::PixelFormat;
  using xstream::BBox;
  using xstream::Landmark;
  using xstream::Feature;


  Callback callback;

  // 初始化sdk
  xstream::XStreamSDK* flow = xstream::XStreamSDK::CreateSDK();
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow->SetConfig("config_file", "../../example/config/reid_mot.json");
  flow->Init();

  std::string postv_str;
  std::string track_str;

  float detection_score_threshold = 0.9;
  int min_height = 80;
  size_t feature_length = 128;
  int skeleton_point_num = 17;
  unsigned int time_per_frame = 40;
  int end_time = 7500;

  std::string det_result_path =
     "/home/dev/chao.yang/data/ch3_easy/det/"
     "ch3_easy_det_kps_resnet-34_256_128_128_reid_hobot.txt";
  std::string image_folder = "/home/dev/chao.yang/data/ch3_easy/images/";

  std::ifstream fid_roi(det_result_path, std::fstream::binary);
  std::ifstream img_list(det_result_path, std::fstream::binary);

  if (fid_roi) {
    std::string roi_info, image_name;
    unsigned int frame_id = 0;
    while (std::getline(fid_roi, roi_info)) {
      if (roi_info.empty()) {
        std::cout << "roi_info is empty" << std::endl;
        break;
      }

      std::istringstream ss(roi_info);
      ss >> image_name;
      auto image_path = image_folder + image_name;
      ++frame_id;

      auto rgb_mat = cv::imread(image_path);
      callback.img_ = rgb_mat;

      cv::Mat yuv_mat;
      cv::cvtColor(rgb_mat, yuv_mat, CV_RGB2YUV_I420);

      InputDataPtr inputdata(new InputData());

      auto *img_frame =
          new ImageFrame(static_cast<uint32_t>(rgb_mat.rows *
          rgb_mat.cols * 3 / 2), nullptr);
      img_frame->name_ = "img_frame";
      img_frame->pixel_format_ = PixelFormat::RAW_YUV_I420;
      img_frame->height_ = (uint32_t)rgb_mat.rows;
      img_frame->width_ = (uint32_t)rgb_mat.cols;
      img_frame->stride_uv_ = (uint32_t)rgb_mat.cols;
      img_frame->data_size_ = static_cast<uint32_t>(
        rgb_mat.rows * rgb_mat.cols * 3 / 2);
      memcpy(img_frame->data_, yuv_mat.data, img_frame->data_size_);
      img_frame->time_stamp_ = frame_id * time_per_frame;
      img_frame->frame_id_ = frame_id;

      callback.timestamp_ = img_frame->time_stamp_;

      inputdata->datas_.push_back(BaseDataPtr(img_frame));

      auto *face_head_box = new BaseDataVector();
      auto *body_box = new BaseDataVector();
      auto *reid_features = new BaseDataVector();
      auto *skeletons = new BaseDataVector();

      face_head_box->name_ = "face_head_box_list";
      body_box->name_ = "body_box_list";
      reid_features->name_ = "reid_feature_list";
      skeletons->name_ = "skeleton_list";

      float x1, y1, x2, y2, score;

      while ( ss >> x1 >> y1 >> x2 >> y2 >> score ) {
        if (score < detection_score_threshold || (y2 - y1) < min_height) {
          // skip kps
          for (size_t k_p = 0; k_p < skeleton_point_num; ++k_p) {
            float x, y, s;
            ss >> x >> y >> s;
          }
          // skip reid feature
          float feature;
          for (size_t i = 0; i < feature_length; ++i) {
            ss >> feature;
          }
          continue;
        }

        // body bboxs
        std::shared_ptr<BBox> box(new BBox);
        box->values_.resize(4);
        box->values_[0] = x1;
        box->values_[1] = y1;
        box->values_[2] = x2;
        box->values_[3] = y2;
        box->score_ = score;
        body_box->datas_.push_back(box);

        // kps
        std::shared_ptr<Landmark> human_skeleton(new Landmark);
        human_skeleton->points_.resize(static_cast<size_t>(skeleton_point_num));
        assert(human_skeleton->points_.size() == skeleton_point_num);
        for (size_t k_p = 0; k_p < skeleton_point_num; ++k_p) {
          float x, y, s;
          ss >> x >> y >> s;
          human_skeleton->points_[k_p].values_.resize(2);
          human_skeleton->points_[k_p].values_[0] = x;
          human_skeleton->points_[k_p].values_[1] = y;
          human_skeleton->points_[k_p].score_ = s;
        }
        skeletons->datas_.push_back(human_skeleton);

        // reid feature
        auto reid = std::make_shared<Feature>();
        reid->values_.resize(feature_length);
        for (int i = 0; i < feature_length; i++) {
          float feature;
          ss >> feature;
          reid->values_[i] = feature;
        }
//        reid_features->datas_.push_back(reid);
      }

      inputdata->datas_.push_back(BaseDataPtr(face_head_box));
      inputdata->datas_.push_back(BaseDataPtr(body_box));
      inputdata->datas_.push_back(BaseDataPtr(reid_features));
      inputdata->datas_.push_back(BaseDataPtr(skeletons));

      auto out = flow->SyncPredict(inputdata);
      callback.OnCallback(out);
    }

    fid_roi.close();
  } else {
    return -1;
  }
  return 0;
}
