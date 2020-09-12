/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.24
 */

#ifndef VEHICLE_COMMON_UTILITY_PARSE_JSON_HPP_
#define VEHICLE_COMMON_UTILITY_PARSE_JSON_HPP_

#include <fstream>
#include <string>
#include "hobotlog/hobotlog.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief Virtual class to be inheritted
 */
class ParseJson {
 public:
  ParseJson() = default;

 protected:
  /*
   * @brief Init class members
   * @param config_file
   */
  void Init(const std::string &config_file) {
    std::ifstream config_fs(config_file);
    if (config_fs.is_open())
      DoParsing(config_fs);
    else
      LOGI << config_file << ": open file failed, using default.";

    config_fs.close();
  }

  /*
   * @biref Actually init class members by parsing config.
   * Must be realized in child class.
   * @param ifs, config file input stream
   */
  virtual void DoParsing(std::ifstream &is) = 0;

  /*
   * @biref Overloaded function.
   * Actually init class members by parsing config.
   * Must be realized in child class.
   * @param ifs, string input stream.
   */
  virtual void DoParsing(std::istringstream &is) = 0;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_COMMON_UTILITY_PARSE_JSON_HPP_
