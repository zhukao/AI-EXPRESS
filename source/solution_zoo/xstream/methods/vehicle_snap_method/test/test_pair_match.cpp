/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "json/json.h"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_match_strategy/pairs_match_api.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

TEST(VehicleMatchModule, InitWithParams) {
  Params params;
  std::shared_ptr<PairsMatchAPI> matcher =
      PairsMatchAPI::NewPairsMatchAPI(params);
  EXPECT_TRUE(matcher != nullptr);
}

TEST(VehicleMatchModule, PairMatch) {
  // SetLogLevel(HOBOT_LOG_DEBUG);
  SetLogLevel(HOBOT_LOG_INFO);

  std::string config_file("./config/config_match.json");

  std::ofstream output_fs("./config/match_result.json",
      std::ios::out | std::ios::binary);
  if (!output_fs.is_open()) {
    std::cout << "open output file failed" << std::endl;
    return;
  }
  output_fs << "Matching results."
            << "\n";

  Json::CharReaderBuilder reader;
  std::string errs;

  std::shared_ptr<PairsMatchAPI> matcher =
      PairsMatchAPI::NewPairsMatchAPI(config_file);

  std::string line;
  std::ifstream input_fs("./config/data/CAM050.json");
  if (!input_fs.is_open()) {
    std::cout<< "no test data" << std::endl;
    return;
  }

  Json::Value input_line;
  int count = 0;
  while (std::getline(input_fs, line)) {
    if (count > 100) {
      break;
    }
    LOGD << "Processing " << count << " frames";
    std::istringstream line_ss(line);
    // bool open_status =
    //    Json::parseFromStream(reader, line_ss, &input_line, &errs);
    Json::Value &vehicle_data = input_line["vehicle"];
    Json::Value &plate_data = input_line["plate_box"];
    VehicleListPtr vehicle_list = std::make_shared<VehicleList>();
    PlateListPtr plate_list = std::make_shared<PlateList>();

    for (int i = 0; i < static_cast<int>(vehicle_data.size()); ++i) {
      std::vector<float> tmp(4, 0.f);
      tmp[0] = vehicle_data[i]["data"][0].asFloat();
      tmp[1] = vehicle_data[i]["data"][1].asFloat();
      tmp[2] = vehicle_data[i]["data"][2].asFloat();
      tmp[3] = vehicle_data[i]["data"][3].asFloat();
      BBoxPtr bbox =
          std::make_shared<vision::BBox>(tmp[0], tmp[1], tmp[2], tmp[3]);
      Vehicle veh_tmp;
      veh_tmp.bbox = bbox;
      veh_tmp.bbox->id = vehicle_data[i]["track_id"].asUInt64();
      vehicle_list->push_back(std::move(veh_tmp));
    }

    for (int i = 0; i < static_cast<int>(plate_data.size()); ++i) {
      std::vector<float> tmp(4, 0.f);
      tmp[0] = plate_data[i]["data"][0].asFloat();
      tmp[1] = plate_data[i]["data"][1].asFloat();
      tmp[2] = plate_data[i]["data"][2].asFloat();
      tmp[3] = plate_data[i]["data"][3].asFloat();

      BBoxPtr bbox =
          std::make_shared<vision::BBox>(tmp[0], tmp[1], tmp[2], tmp[3]);
      Plate plate_tmp;
      plate_tmp.bbox = bbox;

      std::string is_double_tmp = plate_data[i]["attrs"]["double"].asString();

      if (is_double_tmp == "no")
        plate_tmp.is_double = 0;
      else if (is_double_tmp == "yes")
        plate_tmp.is_double = 1;
      else
        LOGI << "Error, plate box type is not recognized.";

      plate_list->push_back(std::move(plate_tmp));
    }
    LOGD << "Parsing data done...";

    matcher->Process(vehicle_list, plate_list);

    LOGD << "Matching vehicle and plate done...";

    Json::Value results;
    results["frame_id"] = count;
    results["image_key"] = input_line["image_key"];
    results["width"] = input_line["width"];
    results["height"] = input_line["height"];
    Json::Value res_veh(Json::arrayValue);
    for (const auto &itr : *vehicle_list) {
      Json::Value veh;
      veh["bbox"] = Json::Value(Json::arrayValue);

      veh["bbox"].append(static_cast<int>(itr.bbox->x1));
      veh["bbox"].append(static_cast<int>(itr.bbox->y1));
      veh["bbox"].append(static_cast<int>(itr.bbox->x2));
      veh["bbox"].append(static_cast<int>(itr.bbox->y2));
      veh["track_id"] = itr.bbox->id;

      if (itr.plate != nullptr) {
        veh["plate_box"] = Json::Value();
        veh["plate_box"]["bbox"] = Json::Value(Json::arrayValue);

        veh["plate_box"]["bbox"].append(static_cast<int>(itr.plate->bbox->x1));
        veh["plate_box"]["bbox"].append(static_cast<int>(itr.plate->bbox->y1));
        veh["plate_box"]["bbox"].append(static_cast<int>(itr.plate->bbox->x2));
        veh["plate_box"]["bbox"].append(static_cast<int>(itr.plate->bbox->y2));

        veh["plate_box"]["attrs"] = Json::Value();
        if (itr.plate->is_double == 1)
          veh["plate_box"]["attrs"]["double"] = "yes";
        else if (itr.plate->is_double == 0)
          veh["plate_box"]["attrs"]["double"] = "no";
      }
      res_veh.append(veh);
    }
    results["persons"] = res_veh;
    LOGD << "Results parsing done.";
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["commentStyle"] = "None";
    writerBuilder["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> jsonWriter(
        writerBuilder.newStreamWriter());
    jsonWriter->write(results, &output_fs);
    output_fs << '\n';

    LOGD << "Writing results done.";

    ++count;
  }
  input_fs.close();
  output_fs.close();
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
