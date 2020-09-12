/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.22
 */

#include "vehicle_match_strategy/parse_config_from_json.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

void ParseJsonMatch::DoParsing(std::ifstream &ifs) {
  Json::Value config;
  Json::CharReaderBuilder reader;
  std::string errs;
  bool open_status = Json::parseFromStream(reader, ifs, &config, &errs);
  if (open_status) {
    if (!config["match"].isNull()) {
      const Json::Value sub_conf = config["match"];
      if (!sub_conf["image_width"].isNull()) {
        params_.img_width = sub_conf["image_width"].asInt();
      }
      if (!sub_conf["image_height"].isNull()) {
        params_.img_height = sub_conf["image_height"].asInt();
      }
      if (!sub_conf["nums_cells_bbox"].isNull()) {
        params_.bbox_cells = sub_conf["nums_cells_bbox"].asInt();
      }
      if (!sub_conf["nums_cells_image"].isNull()) {
        params_.img_cells = sub_conf["nums_cells_image"].asInt();
      }
      if (!sub_conf["overlap_threshold"].isNull()) {
        params_.overlap_th = sub_conf["overlap_threshold"].asFloat();
      }
      if (!sub_conf["min_statis_frames"].isNull()) {
        params_.min_statis_frames = sub_conf["min_statis_frames"].asInt();
      }
      if (!sub_conf["work_frames"].isNull()) {
        params_.work_frames = sub_conf["work_frames"].asInt();
      }
      if (!sub_conf["max_pair_dis"].isNull()) {
        params_.max_pair_dis = sub_conf["max_pair_dis"].asFloat();
      }
      if (!sub_conf["min_stat_value"].isNull()) {
        params_.min_stat_value = sub_conf["min_stat_value"].asInt();
      }
    }
  }
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
