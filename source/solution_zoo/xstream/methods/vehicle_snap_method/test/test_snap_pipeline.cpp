/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.29
 */

#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include "hobotlog/hobotlog.hpp"
#include "gtest/gtest.h"
#include "json/json.h"
#include "horizon/vision_type/vision_type.hpp"
#include "utils_test.hpp"
#include "vehicle_match_strategy/pairs_match_api.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy_api.hpp"
#include "vehicle_snap_strategy_namespace.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

bool CheckFileOpen(const std::ifstream &ifs, const std::string &file_name) {
  if (ifs.is_open()) {
    return true;
  } else {
    LOGI << "Can not open file: " << file_name;
    return false;
  }
}

bool CheckFileOpen(const std::ofstream &ofs, const std::string &file_name) {
  if (ofs.is_open()) {
    return true;
  } else {
    LOGI << "Can not open file: " << file_name;
    return false;
  }
}

void ParsingInputFile(Json::Value &input_line, VehicleListPtr &vehicle_list,
                      PlateListPtr &plate_list) {
  Json::Value &vehicle_data = input_line["vehicle"];
  Json::Value &plate_data = input_line["plate_box"];
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
    veh_tmp.bbox->score = vehicle_data[i]["attrs"]["score"].asFloat();
    auto &veh_type = vehicle_data[i]["attrs"]["type"];
    if (!veh_type.isNull()) {
      std::string type_str = veh_type.asString();
      if (type_str == "BigTruck")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::BigTruck);
      if (type_str == "Bus")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::Bus);
      if (type_str == "Lorry")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::Lorry);
      if (type_str == "MPV")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::MPV);
      if (type_str == "MiniVan")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::MiniVan);
      if (type_str == "Minibus")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::Minibus);
      if (type_str == "SUV")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::SUV);
      if (type_str == "Scooter")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::Scooter);
      if (type_str == "Sedan_Car")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::Sedan_Car);
      if (type_str == "Special_vehicle")
        veh_tmp.model_type =
            static_cast<int>(VehicleModelType::Special_vehicle);
      if (type_str == "Three_Wheeled_Truck")
        veh_tmp.model_type =
            static_cast<int>(VehicleModelType::Three_Wheeled_Truck);
      if (type_str == "other")
        veh_tmp.model_type = static_cast<int>(VehicleModelType::other);
    }
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
    plate_tmp.bbox->score = plate_data[i]["attrs"]["score"].asFloat();

    std::string is_double_tmp = plate_data[i]["attrs"]["double"].asString();

    if (is_double_tmp == "no")
      plate_tmp.is_double = 0;
    else if (is_double_tmp == "yes")
      plate_tmp.is_double = 1;
    else
      LOGD << "Error, plate box type is not recognized.";

    plate_list->push_back(std::move(plate_tmp));
  }
  LOGD << "Parsing data done...";
}

void ParsingDisappearedTrackIdsFile(Json::Value &input_line,
                                    TrackIdListPtr &disappeared_track_ids) {
  for (int i = 0; i < static_cast<int>(input_line["track_ids"].size()); ++i) {
    disappeared_track_ids->push_back(input_line["track_ids"][i].asUInt64());
  }
  LOGD << "Parsing input disappeared track ids list file done.";
}

void SaveMatchingResults(Json::Value &input_line, VehicleListPtr &vehicle_list,
                         int count, std::ofstream &match_output_fs) {
  Json::Value results;
  results["frame_id"] = count;
  results["image_key"] = input_line["image_key"];
  results["width"] = input_line["width"];
  results["height"] = input_line["height"];
  Json::Value res_veh(Json::arrayValue);
  for (const auto &itr : *vehicle_list) {
    Json::Value veh;
    veh["data"] = Json::Value(Json::arrayValue);

    veh["data"].append(static_cast<int>(itr.bbox->x1));
    veh["data"].append(static_cast<int>(itr.bbox->y1));
    veh["data"].append(static_cast<int>(itr.bbox->x2));
    veh["data"].append(static_cast<int>(itr.bbox->y2));
    veh["track_id"] = itr.bbox->id;

    if (itr.plate != nullptr) {
      veh["plate_box"] = Json::Value();
      veh["plate_box"]["data"] = Json::Value(Json::arrayValue);

      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->x1));
      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->y1));
      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->x2));
      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->y2));

      veh["plate_box"]["attrs"] = Json::Value();
      if (itr.plate->is_double == 1)
        veh["plate_box"]["attrs"]["double"] = "yes";
      else if (itr.plate->is_double == 0)
        veh["plate_box"]["attrs"]["double"] = "no";
    }
    res_veh.append(veh);
  }
  results["vehicle"] = res_veh;
  LOGD << "Results parsing done.";
  Json::StreamWriterBuilder writerBuilder;
  writerBuilder["commentStyle"] = "None";
  writerBuilder["indentation"] = "";
  std::shared_ptr<Json::StreamWriter> jsonWriter(
      writerBuilder.newStreamWriter());
  jsonWriter->write(results, &match_output_fs);
  match_output_fs << '\n';

  LOGD << "Writing matching results done.";
}

void SaveSnapResult(const std::string img_save_prefix, Json::Value &input_line,
                    VehicleListPtr &snap_res, std::ofstream &snap_output_fs,
                    const std::string &video_name, int count) {
  Json::Value results;
  results["snap_frame_id"] = count;
  Json::Value res_veh(Json::arrayValue);
  for (const auto &itr : *snap_res) {
    Json::Value veh;
    veh["data"] = Json::Value(Json::arrayValue);

    veh["data"].append(static_cast<int>(itr.bbox->x1));
    veh["data"].append(static_cast<int>(itr.bbox->y1));
    veh["data"].append(static_cast<int>(itr.bbox->x2));
    veh["data"].append(static_cast<int>(itr.bbox->y2));
    veh["track_id"] = itr.bbox->id;
    veh["snap_score"] = itr.snap_score;

    uint64_t frame_id = itr.time_stamp / 40 + 1;
    // image key of the snapped frame
    std::stringstream image_key_ss;
    image_key_ss << std::setfill('0') << std::setw(6) << frame_id;
    std::string image_key = video_name + "_" + image_key_ss.str() + ".jpg";
    veh["image_key"] = image_key;

    // Save sub img of vehicle.
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(6) << itr.bbox->id;
    std::string img_save_name(img_save_prefix + "_" + ss.str() + ".jpg");

    // cv::imwrite(img_save_name, itr.sub_img->img);

    if (itr.plate != nullptr) {
      veh["plate_box"] = Json::Value();
      veh["plate_box"]["data"] = Json::Value(Json::arrayValue);

      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->x1));
      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->y1));
      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->x2));
      veh["plate_box"]["data"].append(static_cast<int>(itr.plate->bbox->y2));

      veh["plate_box"]["attrs"] = Json::Value();
      if (itr.plate->is_double == 1)
        veh["plate_box"]["attrs"]["double"] = "yes";
      else if (itr.plate->is_double == 0)
        veh["plate_box"]["attrs"]["double"] = "no";
    }
    res_veh.append(veh);
  }
  results["vehicle"] = res_veh;
  LOGD << "Snap results parsing done.";
  Json::StreamWriterBuilder writerBuilder;
  writerBuilder["commentStyle"] = "None";
  writerBuilder["indentation"] = "";
  std::shared_ptr<Json::StreamWriter> jsonWriter(
      writerBuilder.newStreamWriter());
  jsonWriter->write(results, &snap_output_fs);
  snap_output_fs << '\n';
}

TEST(VehicleSnapPipeline, MatchAndSnap) {
  // SetLogLevel(HOBOT_LOG_DEBUG);
  SetLogLevel(HOBOT_LOG_INFO);

  std::string match_config_file("./config/config_match.json");
  std::string snap_config_file("./config/config_snap.json");

  std::ofstream match_output_fs("./output/snap_match_result.json");
  CheckFileOpen(match_output_fs, "./output/snap_match_result.json");

  std::ofstream snap_output_fs("./output/snap_result.json");
  CheckFileOpen(snap_output_fs, "./output/snap_result.json");

  std::shared_ptr<PairsMatchAPI> matcher =
      PairsMatchAPI::NewPairsMatchAPI(match_config_file);
  std::shared_ptr<VehicleSnapStrategyAPI> snaper =
      VehicleSnapStrategyAPI::NewVehicleSnapStrategyAPI(snap_config_file,
                                                        crop_image);

  Json::CharReaderBuilder reader;
  std::string errs;

  std::string line;
  std::string input_file_root("./config/data/ZGCsihuan/");
  std::string video_name("11_40_08");
  std::string input_file_name =
      input_file_root + "merge_res/" + video_name + "_car.json";
  std::ifstream input_fs(input_file_name);
  CheckFileOpen(input_fs, input_file_name);
  Json::Value input_line;

  std::ifstream input_disappeared_track_ids_fs(
      input_file_root + "track_res/" + video_name +
      "_car_disappeared_trackids.json");
  CheckFileOpen(input_disappeared_track_ids_fs,
                video_name + "_car_disappeared_list.json");

  int count = 0;

  std::string img_path_root(input_file_root + "images/" + video_name + "/" +
                            video_name + "_");
  std::string img_save_root("./output/snapped_subimgs/");

  while (std::getline(input_fs, line)) {
    LOGD << "Processing " << count << " frames";
    std::istringstream line_ss(line);
    // bool open_status =
    //    Json::parseFromStream(reader, line_ss, &input_line, &errs);
    VehicleListPtr vehicle_list = std::make_shared<VehicleList>();
    PlateListPtr plate_list = std::make_shared<PlateList>();
    TrackIdListPtr disappeared_track_ids = std::make_shared<TrackIdList>();

    // Parsing input data
    ParsingInputFile(input_line, vehicle_list, plate_list);

    // Matching between vehicles and plates
    matcher->Process(vehicle_list, plate_list);
    LOGD << "Matching vehicle and plate done...";

    // Save matching results.
    SaveMatchingResults(input_line, vehicle_list, count, match_output_fs);

    // Parsing input disappeared track ids list file
    std::getline(input_disappeared_track_ids_fs, line);
    std::istringstream line_ss_diseppear(line);
    // open_status =
    //    Json::parseFromStream(reader, line_ss_diseppear, &input_line, &errs);
    ParsingDisappearedTrackIdsFile(input_line, disappeared_track_ids);

    // Reading image
    std::stringstream string_ss;
    string_ss << std::setfill('0') << std::setw(6) << count + 1;
    std::string img_file_name = img_path_root + string_ss.str() + ".jpg";
    auto img = cv::imread(img_file_name);
    auto img_frame = std::make_shared<hobot::vision::CVImageFrame>();
    img_frame->img = img;
    img_frame->pixel_format =
        HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawRGB;
    img_frame->time_stamp = count * 40;
    img_frame->frame_id = count;

    // Doing the snap
    auto snap_res =
        snaper->Process(img_frame, vehicle_list, disappeared_track_ids);
    LOGD << "Get the snap results done.";

    // Save snap results
    if (snap_res->size() > 0)
      SaveSnapResult(img_save_root + video_name, input_line, snap_res,
                     snap_output_fs, video_name, count);

    ++count;
  }

  match_output_fs.close();
  snap_output_fs.close();
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
