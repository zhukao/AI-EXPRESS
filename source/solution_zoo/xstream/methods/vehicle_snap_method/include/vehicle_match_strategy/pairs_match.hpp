/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#ifndef VEHICLE_MATCH_STRATEGY_PAIRS_MATCH_HPP_
#define VEHICLE_MATCH_STRATEGY_PAIRS_MATCH_HPP_

#include <json/json.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_match_strategy/config_params_type.hpp"
#include "vehicle_match_strategy/pairs_match_api.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief Match between plate boxes and vehicle boxes
 */
class PairsMatch : public PairsMatchAPI {
 public:
  PairsMatch();
  ~PairsMatch() = default;

  /*
   * @brief Consttructor
   * @params config file path
   */
  explicit PairsMatch(const std::string &config_file);

  /*
   * @brief Constructor
   * @params algorithm config parameters.
   */
  explicit PairsMatch(const Params &params);

  /*
   * @brief Main process function.
   * @params vehicle_list: pointer to a vector of VehicleType.
   * @params plate_list: pointer to a vector of PlateType.
   * @return the matched plate will be writed to corresponding
   * vehicle.
   */
  void Process(VehicleListPtr &vehicle_list,
               const PlateListPtr &plate_list) override;

 private:
  /*
   * @brief Perform common init process.
   */
  void Init();

  /*
   * @brief Init three dimensional vector.
   */
  TrebleVec InitThreeDimVector();

  /*
   * @brief Compute relative position of large bbox and
   * small bbox.
   * @params large_bbox: large object, i.e., vehicle bbox
   * @params small_bbox: small object, i.e., plate bbox
   */
  std::vector<int> GetRelativePos(const vision::BBox &large_bbox,
                                  const vision::BBox &small_bbox) const;

  /*
   * @brief Compute distance between a vehicle and a plate bbox
   * @param large_bbox
   * @param small_bbox
   * @return the distance
   */
  float ComputePairDistance(const vision::BBox &large_bbox,
                            const vision::BBox &small_bbox) const;

  /*
   * @brief Update the statis matrix
   * @param vehicel_bbox
   * @param plate_bbox
   */
  void UpdateStatMat(const vision::BBox &vehicle_bbox,
                     const vision::BBox &plate_bbox);

  /*
   * @biref Internal function that prepare match ids.
   */
  void PrepareMatchIDs(const VehicleListPtr &vehicle_list,
                       const PlateListPtr &plate_list);
  /*
   * @biref Internal function that process one to one object match.
   */
  void ProcessOneToOneMatch(VehicleListPtr &vehicle_list,
                            const PlateListPtr &plate_list);

  /*
   * @brief Check whether statis matrix is ready.
   */
  bool CheckIfStatReady();

  /*
   * @biref Internal function that process multi to multi objects match.
   */
  void ProcessMultiToMultiMatch(VehicleListPtr &vehicle_list,
                                const PlateListPtr &plate_list);

 private:
  ParamsPtr params_;

  int count_ = 0;
  bool is_working_ = false;
  int working_mat_ = 0;
  int img_cell_width_;
  int img_cell_height_;
  int space_dim_;

  IntToVecMap vehicle_match_ids_;
  IntToVecMap plate_match_ids_;

  std::vector<TrebleVec> stat_matrix_;
};

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_MATCH_STRATEGY_PAIRS_MATCH_HPP_
