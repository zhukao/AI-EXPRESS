/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.24
 */

#include <cmath>
#include <exception>
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "vehicle_snap_strategy/parse_config_from_json.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

void ParseJsonSnap::ParseSnapParamsFromString(const std::string &param_str) {
  std::istringstream in_ss(param_str);
  try {
    DoParsing(in_ss);
  }
  catch (const std::exception &e) {
    LOGE << " a standard exception was caught, with message '" << e.what()
         << "'";
  }
}

void ParseJsonSnap::DoParsing(std::ifstream &is) {
  Json::Value config;
  Json::CharReaderBuilder reader;
  std::string errs;
  bool open_status = Json::parseFromStream(reader, is, &config, &errs);
  if (open_status) {
    ParseSnapMode(config["snap_mode"]);
    if (!config["snap"].isNull()) {
      ParseScoreSelectiveSnapParams(config["snap"]["score_selective"]);
      ParseCrossLineSnapParams(config["snap"]["cross_line"]);
    } else {
      throw std::invalid_argument("'snap' field is not in config file!");
    }
  } else {
    throw std::runtime_error("Json parsing string file stream failed!");
  }
}

void ParseJsonSnap::DoParsing(std::istringstream &is) {
  Json::Value config;
  Json::CharReaderBuilder reader;
  std::string errs;
  bool open_status = Json::parseFromStream(reader, is, &config, &errs);
  if (open_status) {
    ParseSnapMode(config["snap_mode"]);
    if (!config["snap"].isNull()) {
      ParseScoreSelectiveSnapParams(config["snap"]["score_selective"]);
      ParseCrossLineSnapParams(config["snap"]["cross_line"]);
    } else {
      throw std::invalid_argument("'snap' field is not in config file!");
    }
  } else {
    throw std::runtime_error("Json parsing from string stream failed!");
  }
}

void ParseJsonSnap::ParseSnapMode(const Json::Value &config) {
  if (!config.isNull()) {
    std::string snap_mode = config.asString();
    if (snap_mode == "cross_line")
      snap_post_control_params_.snap_mode = SnapMode::CrossLineSnap;
    else if (snap_mode == "score_selective")
      snap_post_control_params_.snap_mode = SnapMode::ScoreSelectiveSnap;
  } else {
    throw std::invalid_argument(
        "'snap_mode' field is not in config file or string!");
  }
}

void ParseJsonSnap::ParseCrossLineSnapParams(const Json::Value &config) {
  if (!config.isNull()) {
    auto &params = snap_post_control_params_.cross_line_snap_params;
    if (!config["line_width"].isNull())
      params.line_width = config["line_width"].asInt();
    else
      throw std::invalid_argument(
          "'line_width' field is not in the config file or string");
    if (!config["line"].isNull()) {
      PointInt point_a(config["line"][0].asInt(), config["line"][1].asInt());
      PointInt point_b(config["line"][2].asInt(), config["line"][3].asInt());
      params.line = {point_a, point_b};
    } else {
      throw std::invalid_argument(
          "'line' field is not in the config file or string");
    }
  }
}

void ParseJsonSnap::ParseScoreSelectiveSnapParams(const Json::Value &config) {
  if (!config.isNull()) {
    if (!config["post_control"].isNull())
      ParseScoreSelectiveSnapControlParams(config["post_control"]);
    else
      throw std::invalid_argument(
          "'post_control' field is not in the config file or string");
    if (!config["factors"].isNull())
      ParseSnapScoreParams(config["factors"]);
    else
      throw std::invalid_argument(
          "'factors' field is not in the config file or string");
  } else {
    throw std::invalid_argument(
        "'score_selective' field is not in the config file or string");
  }
}

void ParseJsonSnap::ParseScoreSelectiveSnapControlParams(
    const Json::Value &config) {
  auto &params = snap_post_control_params_.score_selective_snap_params;
  if (!config["ignore_overlap_ratio"].isNull())
    params.ignore_overlap_ratio = config["ignore_overlap_ratio"].asFloat();
  if (!config["post_frame_threshold"].isNull())
    params.post_frame_threshold = config["post_frame_threshold"].asInt();
  if (!config["min_tracklet_len"].isNull())
    params.min_tracklet_len = config["min_tracklet_len"].asInt();
  if (!config["snap_min_score"].isNull())
    params.snap_min_score = config["snap_min_score"].asFloat();
  if (!config["growth_ladder"].isNull())
    params.growth_ladder = config["growth_ladder"].asFloat();
  if (!config["black_area"].isNull()) ParseBlackArea(config["black_area"]);
}

void ParseJsonSnap::ParseSnapScoreParams(const Json::Value &config) {
  if (!config["vehicle_size"].isNull()) {
    snap_score_params_.vehicle_size_params = std::make_shared<SizeScoreParam>();
    ParseCommonScoreParams(config["vehicle_size"], config,
                           snap_score_params_.vehicle_size_params);
    ParseSizeRelatedParam(config["vehicle_size"],
                          snap_score_params_.vehicle_size_params);
  }

  if (!config["plate_size"].isNull()) {
    snap_score_params_.plate_size_params = std::make_shared<SizeScoreParam>();
    ParseCommonScoreParams(config["plate_size"], config,
                           snap_score_params_.plate_size_params);
    ParseSizeRelatedParam(config["plate_size"],
                          snap_score_params_.plate_size_params);
  }

  if (!config["vehicle_det_score"].isNull()) {
    snap_score_params_.vehicle_det_params =
        std::make_shared<FactorScoreParam>();
    ParseCommonScoreParams(config["vehicle_det_score"], config,
                           snap_score_params_.vehicle_det_params);
  }

  if (!config["plate_det_score"].isNull()) {
    snap_score_params_.plate_det_params = std::make_shared<FactorScoreParam>();
    ParseCommonScoreParams(config["plate_det_score"], config,
                           snap_score_params_.plate_det_params);
  }

  if (!config["vehicle_visibility"].isNull()) {
    snap_score_params_.visibility_params = std::make_shared<FactorScoreParam>();
    ParseCommonScoreParams(config["vehicle_visibility"], config,
                           snap_score_params_.visibility_params);
  }

  if (!config["vehicle_integrity"].isNull()) {
    snap_score_params_.integrity_params =
        std::make_shared<IntegrityScoreParam>();
    ParseCommonScoreParams(config["vehicle_integrity"], config,
                           snap_score_params_.integrity_params);
    snap_score_params_.integrity_params->dis_th =
        config["vehicle_integrity"]["dis_th"].asInt();
  }
}

void ParseJsonSnap::ParseCommonScoreParams(const Json::Value &factor_config,
                                           const Json::Value &config,
                                           FactorScoreParamPtr param) {
  if (!factor_config["weight"].isNull())
    param->weight = factor_config["weight"].asFloat();
  if (!factor_config["norm_fun"].isNull()) {
    std::string tmp(factor_config["norm_fun"].asString());
    if (tmp == "linear")
      param->norm_fun = NormFunType::Linear;
    else if (tmp == "sigmoid")
      param->norm_fun = NormFunType::Sigmoid;
  }
  if (!factor_config["min"].isNull())
    param->min_value = factor_config["min"].asFloat();
  if (!factor_config["max"].isNull())
    param->max_value = factor_config["max"].asFloat();

  if (!config["norm"].isNull()) {
    if (!config["norm"]["min"].isNull())
      param->norm_min = config["norm"]["min"].asFloat();
    if (!config["norm"]["max"].isNull())
      param->norm_max = config["norm"]["max"].asFloat();
  }
}

void ParseJsonSnap::ParseBlackArea(const Json::Value &config) {
  for (int i = 0; i < static_cast<int>(config.size()); ++i) {
    if (config[i].size() == 4) {
      vision::BBox tmp_bbox;
      tmp_bbox.x1 = config[i][0].asFloat();
      tmp_bbox.y1 = config[i][1].asFloat();
      tmp_bbox.x2 = config[i][2].asFloat();
      tmp_bbox.y2 = config[i][3].asFloat();
      black_area_.push_back(tmp_bbox);
    }
  }
}

void ParseJsonSnap::ParseSizeRelatedParam(const Json::Value &config,
                                          SizeScoreParamPtr &param) {
  if (!config["valid_min_w"].isNull())
    param->valid_min_w = config["valid_min_w"].asInt();
  if (!config["valid_min_h"].isNull())
    param->valid_min_h = config["valid_min_h"].asInt();
  if (!config["size_max"].isNull()) {
    auto sub_config = config["size_max"];
    if (!sub_config["default"].isNull())
      param->size_max.insert(
          {SizeCatgory::Unknown_Size, sub_config["default"].asFloat()});
    if (!sub_config["large"].isNull())
      param->size_max.insert(
          {SizeCatgory::Large, sub_config["large"].asFloat()});
    if (!sub_config["median"].isNull())
      param->size_max.insert(
          {SizeCatgory::Median, sub_config["median"].asFloat()});
    if (!sub_config["small"].isNull())
      param->size_max.insert(
          {SizeCatgory::Small, sub_config["small"].asFloat()});
  }
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
