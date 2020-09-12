/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.28
 */

#ifndef VEHICLE_SNAP_STRATEGY_VEHICLE_SNAP_STRATEGY_API_HPP_
#define VEHICLE_SNAP_STRATEGY_VEHICLE_SNAP_STRATEGY_API_HPP_

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include "horizon/vision_type/vision_type.hpp"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

using CropImage = std::function<
    ImageFramePtr(const ImageFramePtr &img_ptr, const vision::BBox &crop_rect,
                  const uint32_t &output_width, const uint32_t &output_height,
                  const bool &need_resize)>;

/*
 * @brief The API class
 */
class VehicleSnapStrategyAPI {
 public:
  virtual ~VehicleSnapStrategyAPI() = default;

  /*
   * @brief The function constructs the snap API.
   * @param config_file_name, the name of the config file.
   * @param crop_image, functional handle to crop image.
   * @return the constructed vehicle snap strategy API.
   */
  static std::shared_ptr<VehicleSnapStrategyAPI> NewVehicleSnapStrategyAPI(
      const std::string &config_file_name, const CropImage &crop_image);

  /*
   * @brief The processing function that does the snapshot.
   * @param frame, the video frame.
   * @param target_list, the vehicle list in the frame.
   * @param disappeared_track_ids, the list of disappeared vehicle track ids
   * @return the snapped result.
   */
  virtual VehicleListPtr Process(const ImageFramePtr &frame,
                                 VehicleListPtr &target_list,
                                 const TrackIdListPtr &disappeared_track_ids) {
    return nullptr;
  }

  /*
   * @brief Set the snap mode
   * @param snap_mode, Mode of snap
   * @return `True` if update successfully else `False`
   */
  virtual Status SetSnapMode(const SnapMode &snap_mode) = 0;

  /*
   * @brief Set the black area. This will be only working in score selective
   * mode
   * @param black_area
   * @return `True` if update successfully else `False`
   */
  virtual Status SetBlackArea(const std::vector<vision::BBox> &black_area) = 0;

  /*
   * @brief Set the line which are used in cross line snap  mode.
   * Warning, this method will be only be effective in cross line snap mode
   * @param the line.
   * @return `True` if update successfully else `False`
   */
  virtual Status SetLine(const std::pair<PointInt, PointInt> &line) = 0;

  /*
   * @brief Update snap params
   * @return `True` if update successfully else `False`
   */
  virtual Status UpdateSnapParams(const std::string &param_str) = 0;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_SNAP_STRATEGY_VEHICLE_SNAP_STRATEGY_API_HPP_
