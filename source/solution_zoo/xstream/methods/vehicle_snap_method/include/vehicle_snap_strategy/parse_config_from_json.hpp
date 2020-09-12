/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.24
 */

#ifndef VEHICLE_SNAP_STRATEGY_PARSE_CONFIG_FROM_JSON_HPP_
#define VEHICLE_SNAP_STRATEGY_PARSE_CONFIG_FROM_JSON_HPP_

#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "json/json.h"
#include "vehicle_common_utility/parse_json.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief Parse configs for snap related configurations.
 */
class ParseJsonSnap : protected ParseJson {
 public:
  ParseJsonSnap() = default;

  /*
   * @brief Constructor.
   * @param config_file, config file name.
   */
  explicit ParseJsonSnap(const std::string &config_file) { Init(config_file); }

  /*
   * @brief Parse snap params from string.
   * @param A string containing the config params
   */
  void ParseSnapParamsFromString(const std::string &param_str);

 public:
  SnapControlParam snap_post_control_params_;
  WholeScoreParam snap_score_params_;

  std::vector<vision::BBox> black_area_;

 private:
  /*
   * @brief Actually pasing configs from input file strram.
   * @param is, input config file stream.
   */
  void DoParsing(std::ifstream &is) override;

  /*
   * @brief Overloaded pasing method parsing params from str.
   * @param is, input string stream.
   */
  void DoParsing(std::istringstream &is) override;

  /*
   * @brief Parsing score
   */
  void ParseScoreSelectiveSnapControlParams(const Json::Value &config);

  /*
   * @brief Parsing cross line control related params.
   * @param config, json field.
   */
  void ParseCrossLineSnapParams(const Json::Value &config);

  /*
   * @brief Parsing score selective control related params.
   * @param config, json field.
   */
  void ParseScoreSelectiveSnapParams(const Json::Value &config);

  /*
   * @brief Parsing snap mode
   * @param config, json field
   */
  void ParseSnapMode(const Json::Value &config);

  /*
   * @brief Parsing specific snap score related params.
   * @param config, json field.
   */
  void ParseSnapScoreParams(const Json::Value &config);

  /*
   * @brief Parsing params that are common in different snap score factors.
   * @param json field and corresponding param struct data.
   */
  void ParseCommonScoreParams(const Json::Value &factor_config,
                              const Json::Value &config,
                              FactorScoreParamPtr param);

  /*
   * @brief Parse black area from config
   * @param Corresponding json fields.
   */
  void ParseBlackArea(const Json::Value &config);

  /*
   * @beirf Parse computing size score required params for
   * different kind of catgory vehicels
   * @param Corresponding json fields.
   */
  void ParseSizeRelatedParam(const Json::Value &config,
                             SizeScoreParamPtr &param);
};

typedef std::shared_ptr<ParseJsonSnap> ParseJsonSnapPtr;

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_PARSE_CONFIG_FROM_JSON_HPP_
