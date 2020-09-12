/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.23
 */

#ifndef METHOD_VEHICLE_PLATE_MATCH_METHOD_H_
#define METHOD_VEHICLE_PLATE_MATCH_METHOD_H_

#include <atomic>
#include <string>
#include <vector>
#include <memory>
// #include "hobotxstream/data_types/filter_param.h"
#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"

#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_common_utility/utils.hpp"
#include "vehicle_match_strategy/config_params_type.hpp"
#include "vehicle_match_strategy/pairs_match_api.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

namespace xstream {

class VehiclePlateMatchMethod : public Method {
 public:
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) override;

  void Finalize() override;

  int UpdateParameter(InputParamPtr ptr) override;

  InputParamPtr GetParameter() const override;

  std::string GetVersion() const override;
  void OnProfilerChanged(bool on) override;

 private:
  std::shared_ptr<hobot::vehicle_snap_strategy::PairsMatchAPI> matcher;
};

}  // namespace xstream


#endif  // METHOD_VEHICLE_MATCH_H_
