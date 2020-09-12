/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_VEHICLE_SNAP_STRATEGY_HPP_
#define VEHICLE_SNAP_STRATEGY_VEHICLE_SNAP_STRATEGY_HPP_

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include "horizon/vision_type/vision_type.hpp"
#include "vehicle_snap_strategy/vehicle_snap_strategy_api.hpp"
#include "vehicle_snap_strategy/snap_controller.hpp"
#include "vehicle_snap_strategy/target_builder.hpp"
#include "vehicle_snap_strategy/parse_config_from_json.hpp"
#include "vehicle_snap_strategy/config_params_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief The class implement the snap strategy.
 */
class VehicleSnapStrategy : public VehicleSnapStrategyAPI {
 public:
  /*
   * @brief Constructor.
   * @param functional handle to crop image.
   * @param config_file_name, the name of config file.
   */
  VehicleSnapStrategy(const std::string &config_file_name,
                      const CropImage &crop_image);
  ~VehicleSnapStrategy() = default;

  /*
   * @brief The processing function that does the snapshot.
   * @param frame, the video frame.
   * @param target_list, the vehicle list in the frame.
   * @param disappeared_track_ids, the list of disappeared vehicle track ids
   * @return the snapped result.
   */
  VehicleListPtr Process(const ImageFramePtr &frame,
                         VehicleListPtr &target_list,
                         const TrackIdListPtr &disappeared_track_ids) override;
  /*
   * @brief Set the snap mode
   * @param snap_mode, Mode of snap
   */
  Status SetSnapMode(const SnapMode &snap_mode) override;

  /*
   * @brief Set the black area.
   * @param black_area
   */
  Status SetBlackArea(const std::vector<vision::BBox> &black_area) override;

  /*
   * @brief Set the line which are used in cross line mode.
   * @param the line.
   */
  Status SetLine(const std::pair<PointInt, PointInt> &line) override;

  /*
   * @brief Update snap params
   */
  Status UpdateSnapParams(const std::string &param_str) override;

 private:
  /*
   * @brief Score selective controlled vehicle snap process
   */
  VehicleListPtr ScoreSelectiveControlledProcess(
      const ImageFramePtr &frame, VehicleListPtr &target_list,
      const TrackIdListPtr &disappeared_track_ids);
  /*
   * @brief Cross line controlled vehicle snap process
   */
  VehicleListPtr CrossLineControlledProcess(
      const ImageFramePtr &frame, VehicleListPtr &target_list,
      const TrackIdListPtr &disappeared_track_ids);

  /*
   * @brief Init the members.
   */
  void Init();

 private:
  SnapControlParam snap_post_control_params_;
  WholeScoreParam snap_score_params_;

  std::unordered_map<uint64_t, SnapControllerPtr> controllers_;

  PtrTargetBuilder ptr_target_builder_ = nullptr;

  ParseJsonSnapPtr parser_ptr_ = nullptr;

  CropImage crop_image_;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_VEHICLE_SNAP_STRATEGY_HPP_
