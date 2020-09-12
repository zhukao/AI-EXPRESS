//
// Created by yaoyao.sun on 2019-05-14.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <gtest/gtest.h>

#include "VehiclePlateMatchMethod/VehiclePlateMatchMethod.h"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"


using hobot::vision::BBox;
using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;
using xstream::InputParamPtr;
using xstream::VehiclePlateMatchMethod;

typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<hobot::vision::Landmarks> XStreamLandmarks;
typedef xstream::XStreamData<uint32_t> XStreamUint32;
typedef xstream::XStreamData<hobot::vision::Attribute<int>>
    XStreamAttribute;

TEST(VehiclePlateMatchMethod_TEST, Basic) {
  int rv;
  std::string str;
  VehiclePlateMatchMethod vpm_method;
  std::string config_fname("./config/config_match.json");

  rv = vpm_method.Init(config_fname);
  EXPECT_EQ(rv, 0);

  str = vpm_method.GetVersion();
  vpm_method.OnProfilerChanged(true);

  InputParamPtr param_in;
  rv = vpm_method.UpdateParameter(param_in);
  EXPECT_EQ(rv, 0);

  param_in  = vpm_method.GetParameter();
  EXPECT_EQ(param_in, nullptr);

    // 1. input
    // 1.1 vehicle_bbox_list
    XStreamBBox *bbox1(new XStreamBBox(hobot::vision::BBox(0, 0, 100, 100)));
    bbox1->type_ = "BBox";
    auto data_vbox = std::make_shared<BaseDataVector>();
    data_vbox->name_ = "vehicle_bbox_list";
    data_vbox->datas_.push_back(BaseDataPtr(bbox1));
    // 1.2 vehicle_lmk
    XStreamLandmarks *lmks1 = new XStreamLandmarks();
    lmks1->value.values.push_back(hobot::vision::Point(0, 0, 0));
    auto data_vlmk = std::make_shared<BaseDataVector>();
    data_vlmk->name_ = "vehicle_lmk";
    data_vlmk->datas_.push_back(BaseDataPtr(lmks1));
    // 1.3 plate_bbox_list
    XStreamBBox *pbbox1(new XStreamBBox(hobot::vision::BBox(30, 60, 90, 90)));
    pbbox1->type_ = "BBox";
    auto data_pbox = std::make_shared<BaseDataVector>();
    data_pbox->name_ = "plate_bbox_list";
    data_pbox->datas_.push_back(BaseDataPtr(pbbox1));
    // 1.4 plate_lmk
    XStreamLandmarks *plmks1 = new XStreamLandmarks();
    plmks1->value.values.push_back(hobot::vision::Point(0, 0, 0));
    auto data_plmk = std::make_shared<BaseDataVector>();
    data_plmk->name_ = "plate_lmk";
    data_plmk->datas_.push_back(BaseDataPtr(plmks1));
    // 1.5 plate_type
    XStreamAttribute *ptype = new XStreamAttribute();
    ptype->value.value = 1;
    auto data_ptype = std::make_shared<BaseDataVector>();
    data_ptype->name_ = "plate_type";
    data_ptype->datas_.push_back(BaseDataPtr(ptype));
    // 1.6 plate_color
    XStreamAttribute *pcolor = new XStreamAttribute();
    pcolor->value.value = 1;
    auto data_pcolor = std::make_shared<BaseDataVector>();
    data_pcolor->name_ = "plate_color";
    data_pcolor->datas_.push_back(BaseDataPtr(pcolor));

  std::vector<std::vector<BaseDataPtr>> input;
  input.reserve(1);
  input.push_back(std::vector<BaseDataPtr>());
  input[0].push_back(data_vbox);
  input[0].push_back(data_pbox);
  input[0].push_back(data_ptype);
  input[0].push_back(data_pcolor);
  input[0].push_back(data_vlmk);  // adjust order
  input[0].push_back(data_plmk);

  // 2. build param
  std::vector<xstream::InputParamPtr> param;
  param.reserve(1);

  // 3. run
  auto xstream_output = vpm_method.DoProcess(input, param);

  // 4. parse output
  // ASSERT_EQ(xstream_output.size(), static_cast<std::size_t>(1));

  // 5. end
  vpm_method.Finalize();
  return;
}
