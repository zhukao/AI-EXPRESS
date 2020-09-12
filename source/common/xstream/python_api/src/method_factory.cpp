/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      method_factory.cpp
 * @brief     main function
 * @author    Zhuoran Rong (zhuoran.rong@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include "hobotxstream/method_factory.h"  // NOLINT
#include "method/bbox_filter/b_box_filter.h"
#include "MOTMethod/MOTMethod.h"
#include "FasterRCNNMethod/FasterRCNNMethod.h"
#include "GradingMethod/GradingMethod.h"
#include "SnapShotMethod/SnapShotMethod.h"
#include "CNNMethod/CNNMethod.h"
#include "MergeMethod/MergeMethod.h"
#include "FilterSkipFrameMethod/FilterSkipFrameMethod.h"
#include "plate_vote_method/plate_vote_method.h"
#include "VehiclePlateMatchMethod/VehiclePlateMatchMethod.h"
#include "vote_method/vote_method.h"
#include "VehicleRoadRelationshipMethod/vehicle_road_relationship_method.h"

namespace xstream {
namespace method_factory {

MethodPtr CreateMethod(const std::string &method_name) {
  if ("BBoxFilter" == method_name) {
    return MethodPtr(new BBoxFilter());
  } else if ("MOTMethod" == method_name) {
    return MethodPtr(new MOTMethod());
  } else if ("FasterRCNNMethod" == method_name) {
    return MethodPtr(new FasterRCNNMethod());
  } else if ("GradingMethod" == method_name) {
    return MethodPtr(new GradingMethod());
  } else if ("SnapShotMethod" == method_name) {
    return MethodPtr(new SnapShotMethod());
  } else if ("CNNMethod" == method_name) {
    return MethodPtr(new CNNMethod());
  } else if ("MergeMethod" == method_name) {
    return MethodPtr(new MergeMethod());
  } else if ("FilterSkipFrameMethod" == method_name) {
    return MethodPtr(new FilterSkipFrameMethod());
  } else if ("VehiclePlateMatchMethod" == method_name) {
    return MethodPtr(new VehiclePlateMatchMethod());
  } else if ("VoteMethod" == method_name) {
    return MethodPtr(new VoteMethod());
  } else if ("PlateVoteMethod" == method_name) {
    return MethodPtr(new PlateVoteMethod());
  } else if ("VehicleRoadRelationshipMethod" == method_name) {
    return MethodPtr(new VehicleRoadRelationshipMethod());
  } else {
    return MethodPtr();
  }
}

}  // namespace method_factory
}  // namespace xstream
