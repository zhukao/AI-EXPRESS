/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     iou_test
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2019.7.25
 */

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "callback.hpp"

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<uint32_t> XStreamUint32;

class XStreamIOUMotMethodTest : public ::testing::Test {
 public:
  XStreamIOUMotMethodTest() = default;

 protected:
  void SetUp() override {}
  void TearDown() override {}
  xstream::XStreamSDK* flow_ = nullptr;
};

class MotMethodParam : public xstream::InputParam {
 public:
  MotMethodParam(const std::string &method_name,
      const std::string &json_config) : InputParam(method_name) {
    content_ = json_config;
    is_json_format_ = true;
  }
  std::string Format() override { return content_; };
 private:
  std::string content_ = "";
};

TEST_F(XStreamIOUMotMethodTest, ConfigUpdate) {
  // init the SDK
  flow_ = xstream::XStreamSDK::CreateSDK();
  TestCallback callback;
  flow_->SetConfig("config_file", "./config/iou_mot.json");
  flow_->Init();
  auto version = flow_->GetVersion("IOU_example");
  EXPECT_EQ(version, "0.0.27");
  auto ret = flow_->GetConfig("IOU_example");
  EXPECT_EQ(ret->is_json_format_, true);
  auto config_content = ret->Format();
  auto config_content_gt = "{\n"
                           "\t\"device\" : \"X2\",\n"
                           "\t\"max_det_target_num\" : 512,\n"
                           "\t\"max_track_target_num\" : 512,\n"
                           "\t\"need_check_merge\" : false,\n"
                           "\t\"original_bbox\" : true,\n"
                           "\t\"support_hungarian\" : false,\n"
                           "\t\"time_gap\" : 40,\n"
                           "\t\"tracker_type\" : \"IOU\",\n"
                           "\t\"update_no_target_predict\" : false,\n"
                           "\t\"vanish_frame_count\" : 30\n}"
                           "\n";
  EXPECT_EQ(config_content, config_content_gt);

  xstream::InputParamPtr method_param(
      new MotMethodParam("IOU_example",
                         "{\n"
                         "\"max_det_target_num\" : 256,\n"
                         "\"max_track_target_num\" : 256,\n"
                         "\"need_check_merge\" : true,\n"
                         "\"original_bbox\" : false,\n"
                         "\"support_hungarian\" : true,\n"
                         "\"time_gap\" : 33,\n"
                         "\"device\" : \"X1\",\n"
                         "\"tracker_type\" : \"IOU\",\n"
                         "\"update_no_target_predict\" : true,\n"
                         "\"vanish_frame_count\" : 50\n}"));
  flow_->UpdateConfig("IOU_example", method_param);

  ret = flow_->GetConfig("IOU_example");
  EXPECT_EQ(ret->is_json_format_, true);
  config_content = ret->Format();
  config_content_gt = "{\n"
                      "\t\"device\" : \"X1\",\n"
                      "\t\"max_det_target_num\" : 256,\n"
                      "\t\"max_track_target_num\" : 256,\n"
                      "\t\"need_check_merge\" : true,\n"
                      "\t\"original_bbox\" : false,\n"
                      "\t\"support_hungarian\" : true,\n"
                      "\t\"time_gap\" : 33,\n"
                      "\t\"tracker_type\" : \"IOU\",\n"
                      "\t\"update_no_target_predict\" : true,\n"
                      "\t\"vanish_frame_count\" : 50\n}"
                      "\n";
  EXPECT_EQ(config_content, config_content_gt);
  delete flow_;
}

TEST_F(XStreamIOUMotMethodTest, SyncSimpleTest) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  flow_ = xstream::XStreamSDK::CreateSDK();
  flow_->SetConfig("config_file", "./config/iou_mot.json");
  flow_->Init();

  InputDataPtr inputdata(new InputData());

  std::shared_ptr<BaseDataVector> face_head_box(new BaseDataVector);
  std::shared_ptr<XStreamBBox> bbox1(new XStreamBBox());
  bbox1->type_ = "BBox";
  bbox1->value.x1 = 0;
  bbox1->value.y1 = 0;
  bbox1->value.x2 = 1000;
  bbox1->value.y2 = 1000;
  bbox1->value.score = 0.3;

  std::shared_ptr<XStreamBBox> bbox2(new XStreamBBox());
  bbox2->type_ = "BBox";
  bbox2->value.x1 = 0;
  bbox2->value.y1 = 0;
  bbox2->value.x2 = 10;
  bbox2->value.y2 = 10;
  bbox2->value.score = 0.5;
  face_head_box->datas_.push_back(BaseDataPtr(bbox1));
  face_head_box->datas_.push_back(BaseDataPtr(bbox2));

  face_head_box->name_ = "face_head_box_list";
  inputdata->datas_.push_back(BaseDataPtr(face_head_box));

  auto output = flow_->SyncPredict(inputdata);

  EXPECT_EQ(output->sequence_id_, 0);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_STREQ(output->error_detail_.data(), "");
  EXPECT_EQ(output->datas_.size(), uint(2));

  TestCallback callback;
  callback.IsSameBBox(inputdata->datas_, output->datas_, true);

  delete flow_;
  flow_ = nullptr;
}

TEST_F(XStreamIOUMotMethodTest, Switch_IOU_PassThrough) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  flow_ = xstream::XStreamSDK::CreateSDK();
  flow_->SetConfig("config_file", "./config/iou_mot.json");
  flow_->Init();

  xstream::InputParamPtr iou_param(
      new MotMethodParam("IOU_example", "MOT"));
  xstream::InputParamPtr pass_through_param(
      new MotMethodParam("IOU_example", "pass-through"));

  InputDataPtr inputdata(new InputData());
  std::shared_ptr<BaseDataVector> face_head_box(new BaseDataVector);
  std::shared_ptr<XStreamBBox> bbox1(new XStreamBBox());
  bbox1->type_ = "BBox";
  bbox1->value.x1 = 0;
  bbox1->value.y1 = 0;
  bbox1->value.x2 = 1000;
  bbox1->value.y2 = 1000;
  bbox1->value.score = 0.3;
  std::shared_ptr<XStreamBBox> bbox2(new XStreamBBox());
  bbox2->type_ = "BBox";
  bbox2->value.x1 = 0;
  bbox2->value.y1 = 0;
  bbox2->value.x2 = 10;
  bbox2->value.y2 = 10;
  bbox2->value.score = 0.5;
  face_head_box->datas_.push_back(BaseDataPtr(bbox1));
  face_head_box->datas_.push_back(BaseDataPtr(bbox2));
  face_head_box->name_ = "face_head_box_list";
  inputdata->datas_.push_back(BaseDataPtr(face_head_box));

  inputdata->params_.clear();
  inputdata->params_.push_back(iou_param);
  auto output = flow_->SyncPredict(inputdata);
  EXPECT_EQ(output->sequence_id_, 0);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_STREQ(output->error_detail_.data(), "");
  EXPECT_EQ(output->datas_.size(), uint(2));
  TestCallback callback;
  callback.IsSameBBox(inputdata->datas_, output->datas_, true);

  inputdata->params_.clear();
  inputdata->params_.push_back(pass_through_param);
  output = flow_->SyncPredict(inputdata);
  EXPECT_EQ(output->sequence_id_, 1);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_STREQ(output->error_detail_.data(), "");
  EXPECT_EQ(output->datas_.size(), uint(2));
  callback.IsSameBBox(inputdata->datas_, output->datas_, false);

  inputdata->params_.clear();
  inputdata->params_.push_back(iou_param);
  output = flow_->SyncPredict(inputdata);
  EXPECT_EQ(output->sequence_id_, 2);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_STREQ(output->error_detail_.data(), "");
  EXPECT_EQ(output->datas_.size(), uint(2));
  callback.IsSameBBox(inputdata->datas_, output->datas_, true);

  delete flow_;
  flow_ = nullptr;
}

TEST_F(XStreamIOUMotMethodTest, X1_Verification) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  const std::string postv_path = "./test/test_support/ipc_gk8c_postv.txt";
  std::ifstream postv_fn(postv_path, std::ios::binary);
  if (postv_fn.fail()) {
    std::cout << "Open postv_result failed" << std::endl;
  }

  // init the SDK
  flow_ = xstream::XStreamSDK::CreateSDK();
  flow_->SetConfig("config_file", "./config/iou_mot.json");
  flow_->Init();
  xstream::InputParamPtr method_param(
      new MotMethodParam("IOU_example",
                         "{\n"
                         "\"device\" : \"X1\",\n"
                         "\"max_det_target_num\" : 256,\n"
                         "\"max_track_target_num\" : 256,\n"
                         "\"need_check_merge\" : true,\n"
                         "\"original_bbox\" : false,\n"
                         "\"support_hungarian\" : false,\n"
                         "\"time_gap\" : 33,\n"
                         "\"tracker_type\" : \"IOU\",\n"
                         "\"update_no_target_predict\" : false,\n"
                         "\"vanish_frame_count\" : 50\n}"));
  flow_->UpdateConfig("IOU_example", method_param);

  std::string postv_str;
  std::string track_str;
  TestCallback callback;

  unsigned frame_id = 1;
  while (getline(postv_fn, postv_str)) {
    InputDataPtr inputdata(new InputData());

    uint32_t timestamp;
    std::istringstream ss(postv_str);
    ss >> timestamp;
    callback.GetLog() += (std::to_string(timestamp) + " ");

    auto *face_head_box(new BaseDataVector);
    int32_t x = 0, y = 0, w = 0, h = 0, score = 0, md_id = -1;
    while (ss >> md_id) {
      ss >> x;
      ss >> y;
      ss >> w;
      ss >> h;
      ss >> score;
      if (md_id == 0) {
        std::shared_ptr<XStreamBBox> bbox(new XStreamBBox());
        bbox->type_ = "BBox";
        bbox->value.x1 = x;
        bbox->value.y1 = y;
        bbox->value.x2 = x + w;
        bbox->value.y2 = y + h;
        bbox->value.score = score;
        face_head_box->datas_.push_back(bbox);
      } else if (md_id == 1) { }
    }

    face_head_box->name_ = "face_head_box_list";
    inputdata->datas_.push_back(BaseDataPtr(face_head_box));

    auto out = flow_->SyncPredict(inputdata);
    callback.SaveOutputLog(out);
    frame_id++;
  }

  const std::string track_path =
      "./test/test_support/ipc_forward1_track.txt";
  std::ifstream track_fn(track_path, std::ios::binary);
  if (track_fn.fail()) {
    std::cout << "Open track_result failed" << std::endl;
  }
  std::string track_gt((std::istreambuf_iterator<char>(track_fn)),
                       std::istreambuf_iterator<char>());
  std::ofstream ofs("./kalman_result.txt");
  ofs << callback.GetLog().data();
//  EXPECT_STREQ(track_gt.data(), callback.GetLog().data());

  delete flow_;
  flow_ = nullptr;
}

TEST_F(XStreamIOUMotMethodTest, Original_BBox) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  const std::string postv_path = "./test/test_support/ipc_gk8c_postv.txt";
  std::ifstream postv_fn(postv_path, std::ios::binary);
  if (postv_fn.fail()) {
    std::cout << "Open postv_result failed" << std::endl;
  }

  // init the SDK
  flow_ = xstream::XStreamSDK::CreateSDK();
  flow_->SetConfig("config_file", "./config/iou_mot.json");
  flow_->Init();
  xstream::InputParamPtr method_param(
      new MotMethodParam("IOU_example",
                         "{\n"
                         "\"device\" : \"X2\",\n"
                         "\"max_det_target_num\" : 256,\n"
                         "\"max_track_target_num\" : 256,\n"
                         "\"bool need_check_merge\" : false,\n"
                         "\"bool original_bbox\" : true,\n"
                         "\"bool support_hungarian\" : false,\n"
                         "\"time_gap\" : 40,\n"
                         "\"tracker_type\" : \"IOU\",\n"
                         "\"update_no_target_predict\" : false,\n"
                         "\"vanish_frame_count\" : 30\n}"));
  flow_->UpdateConfig("IOU_example", method_param);

  std::string postv_str;
  std::string track_str;
  TestCallback callback;

  unsigned frame_id = 1;
  while (getline(postv_fn, postv_str)) {
    InputDataPtr inputdata(new InputData());
    uint32_t timestamp;
    std::istringstream ss(postv_str);
    ss >> timestamp;

    auto *face_head_box(new BaseDataVector);
    int32_t x = 0, y = 0, w = 0, h = 0, score = 0, md_id = -1;
    while (ss >> md_id) {
      ss >> x;
      ss >> y;
      ss >> w;
      ss >> h;
      ss >> score;
      if (md_id == 0) {
        std::shared_ptr<XStreamBBox> bbox(new XStreamBBox());
        bbox->type_ = "BBox";
        bbox->value.x1 = x;
        bbox->value.y1 = y;
        bbox->value.x2 = x + w;
        bbox->value.y2 = y + h;
        bbox->value.score = score;
        face_head_box->datas_.push_back(bbox);
      } else if (md_id == 1) { }
    }

    face_head_box->name_ = "face_head_box_list";
    inputdata->datas_.push_back(BaseDataPtr(face_head_box));
    auto out = flow_->SyncPredict(inputdata);
    callback.IsSameBBox(inputdata->datas_, out->datas_, true);
    frame_id++;
  }

  delete flow_;
  flow_ = nullptr;
}

TEST_F(XStreamIOUMotMethodTest, config_reset) {
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  // init the SDK
  flow_ = xstream::XStreamSDK::CreateSDK();
  flow_->SetConfig("config_file", "./config/iou_mot.json");
  flow_->Init();
  xstream::InputParamPtr method_param(
      new MotMethodParam("IOU_example", "Reset"));
  method_param->is_json_format_ = false;
  flow_->UpdateConfig("IOU_example", method_param);

  delete flow_;
  flow_ = nullptr;
}

