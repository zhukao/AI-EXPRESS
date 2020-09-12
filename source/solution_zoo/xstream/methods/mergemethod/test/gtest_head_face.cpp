/*
 * @Description: implement of data_type
 * @Author: yaoyao.sun@horizon.ai
 * @Date: 2019-4-12 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:00:32
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<hobot::vision::Landmarks> XStreamLandmarks;
typedef xstream::XStreamData<uint32_t> XStreamUint32;

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;

int CompareBBox(const XStreamBBox &bbox1, const XStreamBBox &bbox2) {
  return ((bbox1.value.x1 == bbox2.value.x1) &&
          (bbox1.value.y1 == bbox2.value.y1) &&
          (bbox1.value.x2 == bbox2.value.x2) &&
          (bbox1.value.y2 == bbox2.value.y2));
}

int CheckResult(
    const std::vector<XStreamBBox> &face_box_s,
    const std::vector<XStreamBBox> &head_box_s,
    const std::vector<std::pair<XStreamBBox, XStreamBBox>> &gt_bbox) {
  for (unsigned int i = 0; i < face_box_s.size(); ++i) {
    XStreamBBox corresponding_gt_nir_box;
    bool get_corresponding_nir_box = false;
    bool get_corresponding_gt_rgb_box = false;
    for (unsigned int j = 0; j < head_box_s.size(); ++j) {
      if (face_box_s[i].value.id == head_box_s[j].value.id) {
        get_corresponding_nir_box = true;
        for (size_t k = 0; k < gt_bbox.size(); ++k) {
          if (CompareBBox(face_box_s[i], gt_bbox[k].first)) {
            get_corresponding_gt_rgb_box = true;
            corresponding_gt_nir_box = gt_bbox[k].second;
            if (CompareBBox(corresponding_gt_nir_box, head_box_s[j])) {
              break;
            } else {
              LOGE << "Camera Calibration performance is bad.";
              return -1;
            }
          }
        }
        if (!get_corresponding_gt_rgb_box) {
          LOGE << "Data chaos! RGB Result Box is not in RGB GT Box list";
          return -1;
        }
      }
    }
    if (!get_corresponding_nir_box) {
      LOGE << "Sadly, rgb box & nir box even not matched!";
      return -1;
    }
  }
  return 0;
}

class XStreamSDKTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    SetLogLevel(HOBOT_LOG_VERBOSE);
    flow = xstream::XStreamSDK::CreateSDK();
    flow->SetConfig("config_file", "./config/merge_flow1.json");
    flow->Init();
    auto version = flow->GetVersion("merge_method");
    EXPECT_EQ(version, "0.0.17");
  }
  static void TearDownTestCase() { delete flow; }
  static xstream::XStreamSDK *flow;
};

xstream::XStreamSDK *XStreamSDKTest::flow = nullptr;

TEST_F(XStreamSDKTest, Basic) {
  InputDataPtr input(new InputData());
  auto face_box_list = std::make_shared<BaseDataVector>();
  face_box_list->name_ = "face_box";
  input->datas_.push_back(BaseDataPtr(face_box_list));

  auto disappeared_face_id_list = std::make_shared<BaseDataVector>();
  disappeared_face_id_list->name_ = "disappeared_face_id";
  input->datas_.push_back(BaseDataPtr(disappeared_face_id_list));

  auto head_box_list = std::make_shared<BaseDataVector>();
  head_box_list->name_ = "head_box";
  input->datas_.push_back(BaseDataPtr(head_box_list));

  auto disappeared_head_id_list = std::make_shared<BaseDataVector>();
  disappeared_head_id_list->name_ = "disappeared_head_id";
  input->datas_.push_back(BaseDataPtr(disappeared_head_id_list));

  auto rgb_lmk = std::make_shared<BaseDataVector>();
  rgb_lmk->name_ = "rgb_lmk";
  input->datas_.push_back(BaseDataPtr(rgb_lmk));

  auto nir_lmk = std::make_shared<BaseDataVector>();
  nir_lmk->name_ = "nir_lmk";
  input->datas_.push_back(BaseDataPtr(nir_lmk));

  auto face_box1 = std::make_shared<XStreamBBox>();
  face_box1->type_ = "BBox";
  face_box1->value.id = 10;
  face_box1->value.x1 = 100;
  face_box1->value.y1 = 100;
  face_box1->value.x2 = 300;
  face_box1->value.y2 = 200;

  auto face_box2 = std::make_shared<XStreamBBox>();
  face_box2->type_ = "BBox";
  face_box2->value.id = 12;
  face_box2->value.x1 = 400;
  face_box2->value.y1 = 100;
  face_box2->value.x2 = 500;
  face_box2->value.y2 = 200;

  auto face_box3 = std::make_shared<XStreamBBox>();
  face_box3->type_ = "BBox";
  face_box3->value.id = 14;
  face_box3->value.x1 = 700;
  face_box3->value.y1 = 100;
  face_box3->value.x2 = 800;
  face_box3->value.y2 = 200;

  face_box_list->datas_.emplace_back(BaseDataPtr(face_box1));
  face_box_list->datas_.emplace_back(BaseDataPtr(face_box2));
  face_box_list->datas_.emplace_back(BaseDataPtr(face_box3));

  auto head_box1 = std::make_shared<XStreamBBox>();
  head_box1->type_ = "BBox";
  head_box1->value.id = 11;
  head_box1->value.x1 = 710;
  head_box1->value.y1 = 90;
  head_box1->value.x2 = 790;
  head_box1->value.y2 = 200;

  auto head_box2 = std::make_shared<XStreamBBox>();
  head_box2->type_ = "BBox";
  head_box2->value.id = 13;
  head_box2->value.x1 = 90;
  head_box2->value.y1 = 110;
  head_box2->value.x2 = 270;
  head_box2->value.y2 = 200;

  head_box_list->datas_.emplace_back(BaseDataPtr(head_box1));
  head_box_list->datas_.emplace_back(BaseDataPtr(head_box2));

  auto face_track_id1 = std::make_shared<XStreamUint32>();
  face_track_id1->value = 1;
  auto face_track_id2 = std::make_shared<XStreamUint32>();
  face_track_id2->value = 2;
  disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(face_track_id1));
  disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(face_track_id2));

  auto head_track_id1 = std::make_shared<XStreamUint32>();
  head_track_id1->value = 3;
  auto head_track_id2 = std::make_shared<XStreamUint32>();
  head_track_id2->value = 4;

  disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(head_track_id1));
  disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(head_track_id2));

  auto face_landmarks1 = std::make_shared<XStreamLandmarks>();
  face_landmarks1->type_ = "Landmarks";
  face_landmarks1->value.values.resize(5);
  face_landmarks1->value.values[0].x = 10;
  face_landmarks1->value.values[0].y = 10;
  face_landmarks1->value.values[1].x = 15;
  face_landmarks1->value.values[1].y = 15;
  face_landmarks1->value.values[2].x = 20;
  face_landmarks1->value.values[2].y = 20;
  face_landmarks1->value.values[3].x = 25;
  face_landmarks1->value.values[3].y = 25;
  face_landmarks1->value.values[4].x = 30;
  face_landmarks1->value.values[4].y = 30;

  auto face_landmarks2 = std::make_shared<XStreamLandmarks>();
  face_landmarks2->type_ = "Landmarks";
  face_landmarks2->value.values.resize(5);
  face_landmarks2->value.values[0].x = 35;
  face_landmarks2->value.values[0].y = 35;
  face_landmarks2->value.values[1].x = 40;
  face_landmarks2->value.values[1].y = 40;
  face_landmarks2->value.values[2].x = 45;
  face_landmarks2->value.values[2].y = 45;
  face_landmarks2->value.values[3].x = 50;
  face_landmarks2->value.values[3].y = 50;
  face_landmarks2->value.values[4].x = 55;
  face_landmarks2->value.values[4].y = 55;

  rgb_lmk->datas_.emplace_back(BaseDataPtr(face_landmarks1));
  rgb_lmk->datas_.emplace_back(BaseDataPtr(face_landmarks2));

  auto head_landmarks1 = std::make_shared<XStreamLandmarks>();
  head_landmarks1->type_ = "Landmarks";
  head_landmarks1->value.values.resize(5);
  head_landmarks1->value.values[0].x = 10;
  head_landmarks1->value.values[0].y = 10;
  head_landmarks1->value.values[1].x = 15;
  head_landmarks1->value.values[1].y = 15;
  head_landmarks1->value.values[2].x = 20;
  head_landmarks1->value.values[2].y = 20;
  head_landmarks1->value.values[3].x = 25;
  head_landmarks1->value.values[3].y = 25;
  head_landmarks1->value.values[4].x = 30;
  head_landmarks1->value.values[4].y = 30;

  auto head_landmarks2 = std::make_shared<XStreamLandmarks>();
  head_landmarks2->type_ = "Landmarks";
  head_landmarks2->value.values.resize(5);
  head_landmarks2->value.values[0].x = 35;
  head_landmarks2->value.values[0].y = 35;
  head_landmarks2->value.values[1].x = 40;
  head_landmarks2->value.values[1].y = 40;
  head_landmarks2->value.values[2].x = 45;
  head_landmarks2->value.values[2].y = 45;
  head_landmarks2->value.values[3].x = 50;
  head_landmarks2->value.values[3].y = 50;
  head_landmarks2->value.values[4].x = 55;
  head_landmarks2->value.values[4].y = 55;

  nir_lmk->datas_.emplace_back(BaseDataPtr(head_landmarks1));
  nir_lmk->datas_.emplace_back(BaseDataPtr(head_landmarks2));

  auto out = flow->SyncPredict(input);

  EXPECT_EQ(out->error_code_, 0);
  EXPECT_EQ(out->datas_.size(), static_cast<size_t>(3));

  std::vector<int> face_id, head_id;
  for (int i = 0; i < 2; ++i) {
    auto &data = out->datas_[i];
    auto pdata = std::static_pointer_cast<BaseDataVector>(data);
    ASSERT_TRUE(!pdata->datas_.empty());
    for (const auto &element : pdata->datas_) {
      auto bbox = std::static_pointer_cast<XStreamBBox>(element);
      if (pdata->name_ == "merged_face_box") {
        face_id.emplace_back(bbox->value.id);
      } else if (pdata->name_ == "merged_head_box") {
        head_id.emplace_back(bbox->value.id);
      }
    }
  }
  EXPECT_EQ(face_id.size(), static_cast<size_t>(3));
  EXPECT_EQ(head_id.size(), static_cast<size_t>(2));
  // EXPECT_EQ(face_id[2], head_id[0]);
  // EXPECT_EQ(face_id[0], head_id[1]);
}

TEST_F(XStreamSDKTest, State_Check) {
  InputDataPtr input(new InputData());
  auto face_box_list = std::make_shared<BaseDataVector>();
  face_box_list->name_ = "face_box";
  input->datas_.push_back(BaseDataPtr(face_box_list));

  auto disappeared_face_id_list = std::make_shared<BaseDataVector>();
  disappeared_face_id_list->name_ = "disappeared_face_id";
  input->datas_.push_back(BaseDataPtr(disappeared_face_id_list));

  auto head_box_list = std::make_shared<BaseDataVector>();
  head_box_list->name_ = "head_box";
  input->datas_.push_back(BaseDataPtr(head_box_list));

  auto disappeared_head_id_list = std::make_shared<BaseDataVector>();
  disappeared_head_id_list->name_ = "disappeared_head_id";
  input->datas_.push_back(BaseDataPtr(disappeared_head_id_list));

  auto rgb_lmk = std::make_shared<BaseDataVector>();
  rgb_lmk->name_ = "rgb_lmk";
  input->datas_.push_back(BaseDataPtr(rgb_lmk));

  auto nir_lmk = std::make_shared<BaseDataVector>();
  nir_lmk->name_ = "nir_lmk";
  input->datas_.push_back(BaseDataPtr(nir_lmk));

  auto face_box1 = std::make_shared<XStreamBBox>();
  face_box1->type_ = "BBox";
  face_box1->value.id = 10;
  face_box1->value.x1 = 100;
  face_box1->value.y1 = 100;
  face_box1->value.x2 = 300;
  face_box1->value.y2 = 200;

  auto face_box2 = std::make_shared<XStreamBBox>();
  face_box2->state_ = xstream::DataState::FILTERED;
  face_box2->type_ = "BBox";
  face_box2->value.id = 12;
  face_box2->value.x1 = 400;
  face_box2->value.y1 = 100;
  face_box2->value.x2 = 500;
  face_box2->value.y2 = 200;

  auto face_box3 = std::make_shared<XStreamBBox>();
  face_box3->type_ = "BBox";
  face_box3->value.id = 14;
  face_box3->value.x1 = 700;
  face_box3->value.y1 = 100;
  face_box3->value.x2 = 800;
  face_box3->value.y2 = 200;

  face_box_list->datas_.emplace_back(BaseDataPtr(face_box1));
  face_box_list->datas_.emplace_back(BaseDataPtr(face_box2));
  face_box_list->datas_.emplace_back(BaseDataPtr(face_box3));

  auto head_box1 = std::make_shared<XStreamBBox>();
  head_box1->type_ = "BBox";
  head_box1->value.id = 11;
  head_box1->value.x1 = 710;
  head_box1->value.y1 = 90;
  head_box1->value.x2 = 790;
  head_box1->value.y2 = 200;

  auto head_box2 = std::make_shared<XStreamBBox>();
  head_box2->state_ = xstream::DataState::FILTERED;
  head_box2->type_ = "BBox";
  head_box2->value.id = 13;
  head_box2->value.x1 = 90;
  head_box2->value.y1 = 110;
  head_box2->value.x2 = 270;
  head_box2->value.y2 = 200;

  head_box_list->datas_.emplace_back(BaseDataPtr(head_box1));
  head_box_list->datas_.emplace_back(BaseDataPtr(head_box2));

  auto face_track_id1 = std::make_shared<XStreamUint32>();
  face_track_id1->value = 1;
  auto face_track_id2 = std::make_shared<XStreamUint32>();
  face_track_id2->value = 2;
  disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(face_track_id1));
  disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(face_track_id2));

  auto head_track_id1 = std::make_shared<XStreamUint32>();
  head_track_id1->value = 3;
  auto head_track_id2 = std::make_shared<XStreamUint32>();
  head_track_id2->value = 4;

  disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(head_track_id1));
  disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(head_track_id2));

  auto face_landmarks1 = std::make_shared<XStreamLandmarks>();
  face_landmarks1->type_ = "Landmarks";
  face_landmarks1->value.values.resize(5);
  face_landmarks1->value.values[0].x = 10;
  face_landmarks1->value.values[0].y = 10;
  face_landmarks1->value.values[1].x = 15;
  face_landmarks1->value.values[1].y = 15;
  face_landmarks1->value.values[2].x = 20;
  face_landmarks1->value.values[2].y = 20;
  face_landmarks1->value.values[3].x = 25;
  face_landmarks1->value.values[3].y = 25;
  face_landmarks1->value.values[4].x = 30;
  face_landmarks1->value.values[4].y = 30;

  auto face_landmarks2 = std::make_shared<XStreamLandmarks>();
  face_landmarks2->type_ = "Landmarks";
  face_landmarks2->value.values.resize(5);
  face_landmarks2->value.values[0].x = 35;
  face_landmarks2->value.values[0].y = 35;
  face_landmarks2->value.values[1].x = 40;
  face_landmarks2->value.values[1].y = 40;
  face_landmarks2->value.values[2].x = 45;
  face_landmarks2->value.values[2].y = 45;
  face_landmarks2->value.values[3].x = 50;
  face_landmarks2->value.values[3].y = 50;
  face_landmarks2->value.values[4].x = 55;
  face_landmarks2->value.values[4].y = 55;

  rgb_lmk->datas_.emplace_back(BaseDataPtr(face_landmarks1));
  rgb_lmk->datas_.emplace_back(BaseDataPtr(face_landmarks2));

  auto head_landmarks1 = std::make_shared<XStreamLandmarks>();
  head_landmarks1->type_ = "Landmarks";
  head_landmarks1->value.values.resize(5);
  head_landmarks1->value.values[0].x = 10;
  head_landmarks1->value.values[0].y = 10;
  head_landmarks1->value.values[1].x = 15;
  head_landmarks1->value.values[1].y = 15;
  head_landmarks1->value.values[2].x = 20;
  head_landmarks1->value.values[2].y = 20;
  head_landmarks1->value.values[3].x = 25;
  head_landmarks1->value.values[3].y = 25;
  head_landmarks1->value.values[4].x = 30;
  head_landmarks1->value.values[4].y = 30;

  auto head_landmarks2 = std::make_shared<XStreamLandmarks>();
  head_landmarks2->type_ = "Landmarks";
  head_landmarks2->value.values.resize(5);
  head_landmarks2->value.values[0].x = 35;
  head_landmarks2->value.values[0].y = 35;
  head_landmarks2->value.values[1].x = 40;
  head_landmarks2->value.values[1].y = 40;
  head_landmarks2->value.values[2].x = 45;
  head_landmarks2->value.values[2].y = 45;
  head_landmarks2->value.values[3].x = 50;
  head_landmarks2->value.values[3].y = 50;
  head_landmarks2->value.values[4].x = 55;
  head_landmarks2->value.values[4].y = 55;

  nir_lmk->datas_.emplace_back(BaseDataPtr(head_landmarks1));
  nir_lmk->datas_.emplace_back(BaseDataPtr(head_landmarks2));

  auto out = flow->SyncPredict(input);

  EXPECT_EQ(out->error_code_, 0);
  EXPECT_EQ(out->datas_.size(), static_cast<size_t>(3));

  std::vector<int> face_id, head_id;
  for (int i = 0; i < 2; ++i) {
    auto &data = out->datas_[i];
    auto pdata = std::static_pointer_cast<BaseDataVector>(data);
    ASSERT_TRUE(!pdata->datas_.empty());
    for (const auto &element : pdata->datas_) {
      auto bbox = std::static_pointer_cast<XStreamBBox>(element);
      if (pdata->name_ == "merged_face_box") {
        if (bbox->state_ == xstream::DataState::VALID) {
          face_id.emplace_back(bbox->value.id);
        } else {
          face_id.emplace_back(-1);
        }
      } else if (pdata->name_ == "merged_head_box") {
        if (bbox->state_ == xstream::DataState::VALID) {
          head_id.emplace_back(bbox->value.id);
        } else {
          head_id.emplace_back(-1);
        }
      }
    }
  }
  EXPECT_EQ(face_id.size(), static_cast<size_t>(3));
  EXPECT_EQ(head_id.size(), static_cast<size_t>(2));
  // EXPECT_EQ(face_id[2], head_id[0]);
  EXPECT_EQ(-1, head_id[1]);
  EXPECT_EQ(-1, face_id[1]);
}

class PassThroughDisableParam : public xstream::InputParam {
 public:
  explicit PassThroughDisableParam(const std::string &module_name)
      : xstream::InputParam(module_name) {}
  std::string Format() override { return "pass-through"; }
};

TEST_F(XStreamSDKTest, PassThrough) {
  InputDataPtr input(new InputData());
  auto face_box_list = std::make_shared<BaseDataVector>();
  face_box_list->name_ = "face_box";
  input->datas_.push_back(BaseDataPtr(face_box_list));

  auto disappeared_face_id_list = std::make_shared<BaseDataVector>();
  disappeared_face_id_list->name_ = "disappeared_face_id";
  input->datas_.push_back(BaseDataPtr(disappeared_face_id_list));

  auto head_box_list = std::make_shared<BaseData>();
  head_box_list->name_ = "head_box";
  head_box_list->state_ = xstream::DataState::INVALID;
  input->datas_.push_back(BaseDataPtr(head_box_list));

  auto disappeared_head_id_list = std::make_shared<BaseData>();
  disappeared_head_id_list->name_ = "disappeared_head_id";
  disappeared_head_id_list->state_ = xstream::DataState::INVALID;
  input->datas_.push_back(BaseDataPtr(disappeared_head_id_list));

  auto rgb_lmk = std::make_shared<BaseDataVector>();
  rgb_lmk->name_ = "rgb_lmk";
  input->datas_.push_back(BaseDataPtr(rgb_lmk));

  auto nir_lmk = std::make_shared<BaseDataVector>();
  nir_lmk->name_ = "nir_lmk";
  input->datas_.push_back(BaseDataPtr(nir_lmk));

  auto param = new PassThroughDisableParam("merge_method");
  input->params_.emplace_back(xstream::InputParamPtr(param));
  auto out = flow->SyncPredict(input);
  std::cout << out->error_detail_ << std::endl;
  EXPECT_EQ(out->error_code_, 0);
  EXPECT_EQ(out->datas_.size(), static_cast<size_t>(3));
  EXPECT_EQ(out->datas_[0].get(), face_box_list.get());
  EXPECT_EQ(out->datas_[2].get(), disappeared_face_id_list.get());
}

TEST_F(XStreamSDKTest, DISABLED_Cam_Calib_Test) {
  InputDataPtr input(new InputData());
  auto face_box_list = std::make_shared<BaseDataVector>();
  face_box_list->name_ = "face_box";
  input->datas_.push_back(BaseDataPtr(face_box_list));

  auto disappeared_face_id_list = std::make_shared<BaseDataVector>();
  disappeared_face_id_list->name_ = "disappeared_face_id";
  input->datas_.push_back(BaseDataPtr(disappeared_face_id_list));

  auto head_box_list = std::make_shared<BaseDataVector>();
  head_box_list->name_ = "head_box";
  input->datas_.push_back(BaseDataPtr(head_box_list));

  auto disappeared_head_id_list = std::make_shared<BaseDataVector>();
  disappeared_head_id_list->name_ = "disappeared_head_id";
  input->datas_.push_back(BaseDataPtr(disappeared_head_id_list));

  auto rgb_lmk = std::make_shared<BaseDataVector>();
  rgb_lmk->name_ = "rgb_lmk";
  input->datas_.push_back(BaseDataPtr(rgb_lmk));

  auto nir_lmk = std::make_shared<BaseDataVector>();
  nir_lmk->name_ = "nir_lmk";
  input->datas_.push_back(BaseDataPtr(nir_lmk));

  std::ifstream ifs("/userdata/merge_test.txt", std::istream::in);
  char rgb_image_name[256], nir_image_name[256];
  int target_num;
  std::vector<std::pair<XStreamBBox, XStreamBBox>> ground_truth_bbox;
  while (!ifs.eof()) {
    ifs >> rgb_image_name >> nir_image_name >> target_num;
    for (int i = 0; i < target_num; ++i) {
      float rgb_x1, rgb_y1, rgb_x2, rgb_y2;
      ifs >> rgb_x1 >> rgb_y1 >> rgb_x2 >> rgb_y2;
      auto rgb_face_box = std::make_shared<XStreamBBox>();
      rgb_face_box->type_ = "BBox";
      rgb_face_box->value.id = i;
      rgb_face_box->value.x1 = rgb_x1;
      rgb_face_box->value.y1 = rgb_y1;
      rgb_face_box->value.x2 = rgb_x2;
      rgb_face_box->value.y2 = rgb_y2;
      face_box_list->datas_.emplace_back(BaseDataPtr(rgb_face_box));
      float nir_x1, nir_y1, nir_x2, nir_y2;
      ifs >> nir_x1 >> nir_y1 >> nir_x2 >> nir_y2;
      auto nir_face_box = std::make_shared<XStreamBBox>();
      nir_face_box->type_ = "BBox";
      nir_face_box->value.id = i + 100;
      nir_face_box->value.x1 = nir_x1;
      nir_face_box->value.y1 = nir_y1;
      nir_face_box->value.x2 = nir_x2;
      nir_face_box->value.y2 = nir_y2;
      head_box_list->datas_.emplace_back(BaseDataPtr(nir_face_box));
      ground_truth_bbox.emplace_back(
          std::make_pair(*rgb_face_box, *nir_face_box));
    }
    auto face_track_id1 = std::make_shared<XStreamUint32>();
    face_track_id1->value = 1000;
    auto face_track_id2 = std::make_shared<XStreamUint32>();
    face_track_id2->value = 2000;
    disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(face_track_id1));
    disappeared_face_id_list->datas_.emplace_back(BaseDataPtr(face_track_id2));

    auto head_track_id1 = std::make_shared<XStreamUint32>();
    head_track_id1->value = 3000;
    auto head_track_id2 = std::make_shared<XStreamUint32>();
    head_track_id2->value = 4000;

    disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(head_track_id1));
    disappeared_head_id_list->datas_.emplace_back(BaseDataPtr(head_track_id2));

    auto face_landmarks1 = std::make_shared<XStreamLandmarks>();
    face_landmarks1->type_ = "Landmarks";
    face_landmarks1->value.values.resize(5);
    face_landmarks1->value.values[0].x = 10;
    face_landmarks1->value.values[0].y = 10;
    face_landmarks1->value.values[1].x = 15;
    face_landmarks1->value.values[1].y = 15;
    face_landmarks1->value.values[2].x = 20;
    face_landmarks1->value.values[2].y = 20;
    face_landmarks1->value.values[3].x = 25;
    face_landmarks1->value.values[3].y = 25;
    face_landmarks1->value.values[4].x = 30;
    face_landmarks1->value.values[4].y = 30;

    auto face_landmarks2 = std::make_shared<XStreamLandmarks>();
    face_landmarks2->type_ = "Landmarks";
    face_landmarks2->value.values.resize(5);
    face_landmarks2->value.values[0].x = 35;
    face_landmarks2->value.values[0].y = 35;
    face_landmarks2->value.values[1].x = 40;
    face_landmarks2->value.values[1].y = 40;
    face_landmarks2->value.values[2].x = 45;
    face_landmarks2->value.values[2].y = 45;
    face_landmarks2->value.values[3].x = 50;
    face_landmarks2->value.values[3].y = 50;
    face_landmarks2->value.values[4].x = 55;
    face_landmarks2->value.values[4].y = 55;

    rgb_lmk->datas_.emplace_back(BaseDataPtr(face_landmarks1));
    rgb_lmk->datas_.emplace_back(BaseDataPtr(face_landmarks2));

    auto head_landmarks1 = std::make_shared<XStreamLandmarks>();
    head_landmarks1->type_ = "Landmarks";
    head_landmarks1->value.values.resize(5);
    head_landmarks1->value.values[0].x = 10;
    head_landmarks1->value.values[0].y = 10;
    head_landmarks1->value.values[1].x = 15;
    head_landmarks1->value.values[1].y = 15;
    head_landmarks1->value.values[2].x = 20;
    head_landmarks1->value.values[2].y = 20;
    head_landmarks1->value.values[3].x = 25;
    head_landmarks1->value.values[3].y = 25;
    head_landmarks1->value.values[4].x = 30;
    head_landmarks1->value.values[4].y = 30;

    auto head_landmarks2 = std::make_shared<XStreamLandmarks>();
    head_landmarks2->type_ = "Landmarks";
    head_landmarks2->value.values.resize(5);
    head_landmarks2->value.values[0].x = 35;
    head_landmarks2->value.values[0].y = 35;
    head_landmarks2->value.values[1].x = 40;
    head_landmarks2->value.values[1].y = 40;
    head_landmarks2->value.values[2].x = 45;
    head_landmarks2->value.values[2].y = 45;
    head_landmarks2->value.values[3].x = 50;
    head_landmarks2->value.values[3].y = 50;
    head_landmarks2->value.values[4].x = 55;
    head_landmarks2->value.values[4].y = 55;

    nir_lmk->datas_.emplace_back(BaseDataPtr(head_landmarks1));
    nir_lmk->datas_.emplace_back(BaseDataPtr(head_landmarks2));

    auto out = flow->SyncPredict(input);

    EXPECT_EQ(out->error_code_, 0);
    EXPECT_EQ(out->datas_.size(), static_cast<size_t>(3));

    std::vector<XStreamBBox> face_box_s, head_box_s;
    for (int i = 0; i < 2; ++i) {
      auto &data = out->datas_[i];
      auto pdata = std::static_pointer_cast<BaseDataVector>(data);
      ASSERT_TRUE(!pdata->datas_.empty());
      for (const auto &element : pdata->datas_) {
        auto bbox = std::static_pointer_cast<XStreamBBox>(element);
        if (pdata->name_ == "merged_face_box") {
          face_box_s.emplace_back(*bbox);
        } else if (pdata->name_ == "merged_head_box") {
          head_box_s.emplace_back(*bbox);
        }
      }
    }
    auto ret = CheckResult(face_box_s, head_box_s, ground_truth_bbox);
    EXPECT_EQ(ret, 0);
  }
}
