//
// Created by yaoyao.sun on 2019-05-24.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>

#include "vote_method/vote_method.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;

using xstream::VoteMethod;
using hobot::vision::BBox;

int TestAntiSpfCalculate(int argc, char **argv) {
  VoteMethod anti_spf_calculate_method;
  std::string config_file = "./config/vote_method.json";
  anti_spf_calculate_method.Init(config_file);

  std::shared_ptr<BaseDataVector> face_rects_ptr(new BaseDataVector());
  std::shared_ptr<XStreamData<BBox>> box1(new XStreamData<BBox>());
  box1->value.id = 1;
  std::shared_ptr<XStreamData<BBox>> box2(new XStreamData<BBox>());
  box2->value.id = 2;
  // std::shared_ptr<XStreamData<BBox>> box3(new XStreamData<BBox>());
  // box3->value.id = 3;
  // std::shared_ptr<XStreamData<BBox>> box4(new XStreamData<BBox>());
  // box4->value.id = 4;
  face_rects_ptr->datas_.push_back(box1);
  face_rects_ptr->datas_.push_back(box2);
  // face_rects_ptr->datas_.push_back(box3);
  // face_rects_ptr->datas_.push_back(box4);

  std::shared_ptr<BaseDataVector> liveness_ptr(new BaseDataVector);
  std::shared_ptr<XStreamData<int>> liveness1(new XStreamData<int>());
  liveness1->value = 1;
  std::shared_ptr<XStreamData<int>> liveness2(new XStreamData<int>());
  liveness2->value = 2;
  // std::shared_ptr<XStreamData<int>> liveness3(new XStreamData<int>());
  // liveness3->value = 3;
  // std::shared_ptr<XStreamData<int>> liveness4(new XStreamData<int>());
  // liveness4->value = 4;
  liveness_ptr->datas_.push_back(liveness1);
  liveness_ptr->datas_.push_back(liveness2);
  // liveness_ptr->datas_.push_back(liveness3);
  // liveness_ptr->datas_.push_back(liveness4);

  std::shared_ptr<BaseDataVector> disappeared_track_ids_ptr(new BaseDataVector);
  std::shared_ptr<XStreamData<uint32_t>> disappeard_track_id1(
      new XStreamData<uint32_t>());
  disappeard_track_id1->value = 1;
  // std::shared_ptr<XStreamData<uint32_t>> disappeard_track_id2(
  //     new XStreamData<uint32_t>());
  // disappeard_track_id2->value = 2;
  disappeared_track_ids_ptr->datas_.push_back(disappeard_track_id1);
  // disappeared_track_ids_ptr->datas_.push_back(disappeard_track_id2);

  std::vector<std::vector<BaseDataPtr>> input;
  std::vector<xstream::InputParamPtr> param;
  int batch_size = 5;
  input.resize(batch_size);
  for (int i = 0; i < batch_size - 1; ++i) {
    input[i].push_back(face_rects_ptr);
    input[i].push_back(nullptr);
    input[i].push_back(liveness_ptr);
  }
  // auto ptr = static_pointer_cast<XStreamData<int>>(
  //     liveness_ptr->datas_[batch_size - 1]);
  // ptr->value = 5;
  // static_pointer_cast<XStreamData<int>>(liveness_ptr->datas_[batch_size -
  // 1])->value = 5;
  std::shared_ptr<BaseDataVector> face_rects_ptr2(new BaseDataVector());
  std::shared_ptr<XStreamData<BBox>> box12(new XStreamData<BBox>());
  box12->value.id = 1;
  std::shared_ptr<XStreamData<BBox>> box22(new XStreamData<BBox>());
  box22->value.id = 2;
  // std::shared_ptr<XStreamData<BBox>> box3(new XStreamData<BBox>());
  // box3->value.id = 3;
  // std::shared_ptr<XStreamData<BBox>> box4(new XStreamData<BBox>());
  // box4->value.id = 4;
  face_rects_ptr2->datas_.push_back(box12);
  face_rects_ptr2->datas_.push_back(box22);
  // face_rects_ptr->datas_.push_back(box3);
  // face_rects_ptr->datas_.push_back(box4);

  std::shared_ptr<BaseDataVector> liveness_ptr2(new BaseDataVector);
  std::shared_ptr<XStreamData<int>> liveness12(new XStreamData<int>());
  liveness12->value = 2;
  std::shared_ptr<XStreamData<int>> liveness22(new XStreamData<int>());
  liveness22->value = 3;
  // std::shared_ptr<XStreamData<int>> liveness3(new XStreamData<int>());
  // liveness3->value = 3;
  // std::shared_ptr<XStreamData<int>> liveness4(new XStreamData<int>());
  // liveness4->value = 4;
  liveness_ptr2->datas_.push_back(liveness12);
  liveness_ptr2->datas_.push_back(liveness22);
  // liveness_ptr->datas_.push_back(liveness3);
  // liveness_ptr->datas_.push_back(liveness4);

  std::shared_ptr<BaseDataVector> disappeared_track_ids_ptr2(
      new BaseDataVector);
  std::shared_ptr<XStreamData<uint32_t>> disappeard_track_id12(
      new XStreamData<uint32_t>());
  disappeard_track_id12->value = 1;
  // std::shared_ptr<XStreamData<uint32_t>> disappeard_track_id2(
  //     new XStreamData<uint32_t>());
  // disappeard_track_id2->value = 2;
  disappeared_track_ids_ptr2->datas_.push_back(disappeard_track_id12);
  // disappeared_track_ids_ptr->datas_.push_back(disappeard_track_id2);
  input[batch_size - 1].push_back(face_rects_ptr2);
  input[batch_size - 1].push_back(disappeared_track_ids_ptr2);
  input[batch_size - 1].push_back(liveness_ptr2);

  std::vector<std::vector<BaseDataPtr>> xstream_output =
      anti_spf_calculate_method.DoProcess(input, param);
  HOBOT_CHECK(static_cast<int>(xstream_output.size()) == batch_size);
  for (int i = 0; i < batch_size; ++i) {
    auto one_frame_out = xstream_output[i];
    HOBOT_CHECK(one_frame_out.size() == 2);
    auto out_track_ids_ptr = one_frame_out[0];
    auto out_liveness_ptr = one_frame_out[1];
    auto out_track_ids =
        std::static_pointer_cast<BaseDataVector>(out_track_ids_ptr)->datas_;
    auto out_liveness =
        std::static_pointer_cast<BaseDataVector>(out_liveness_ptr)->datas_;
    HOBOT_CHECK(out_liveness.size() == out_track_ids.size());
    std::cout << "frame id: " << i << std::endl;
    for (size_t i = 0; i < out_liveness.size(); ++i) {
      auto track_id =
          std::static_pointer_cast<XStreamData<uint32_t>>(
              out_track_ids[i])->value;
      auto liveness =
          std::static_pointer_cast<XStreamData<int>>(out_liveness[i])->value;
      std::cout << "track_id: " << track_id << ", liveness: " << liveness
                << std::endl;
    }
  }

  anti_spf_calculate_method.Finalize();
  return 0;
}
