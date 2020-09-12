/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#ifndef VEHICLE_COMMON_UTILITY_UTILS_HPP_
#define VEHICLE_COMMON_UTILITY_UTILS_HPP_

#include <vector>
#include <utility>
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

/*
 * @brief compute center x of a bbox
 */
float BBoxCenterX(const vision::BBox &bbox);

/*
 * @brief compute center y of a bbox
 */
float BBoxCenterY(const vision::BBox &bbox);

/*
 * @brief compute intersection area of two boxes
 */
float IntersectionArea(const vision::BBox &bbox1, const vision::BBox &bbox2);

/*
 * @brief compute area of the box
 */
float BBoxArea(const vision::BBox &bbox);

/*
 * @brief compute overlap ratio
 */
float BBoxOverlapRatio(const vision::BBox &bbox1, const vision::BBox &bbox2);

/*
 * @brief compute the ratio of intersection to the area of first bbox
 * i.e., intersec(bbox1, bbox2) / area(bbox1)
 */
float BBoxInRatio(const vision::BBox &bbox1, const vision::BBox &bbox2);

/*
 * @brief compute distance between two bboxes
 */
float BBoxDistance(const vision::BBox &bbox1, const vision::BBox &bbox2);

/*
 * @brief Compute the index of a mat.
 * @params Three dimensional matrix.
 */
std::vector<int> ArgMax(const TrebleVec &mat);

/*
 * @brief Compute the index of a mat.
 * @params Two dimensional matrix.
 */
std::vector<int> ArgMax(const DoubleVec &mat);

/*
 * @brief Compute index list from a single value index.
 * @param index, single value index
 * @param shape, shape of original matrix
 * @return index list of original matrix.
 */
std::vector<int> UnravelIndex(const int &index, const std::vector<int> &shape);

/*
 * @brief Sigmoid function, i.e., 1.0/(1+exp(-x))
 * @param The value
 * @return Sigmoid function maped value.
 */
float SigmoidFunction(float score);

/*
 * @brief Piecewise linear function.
 * @param The value, min inflection point, and the max inflection point.
 * @return Piecewise linear maped value.
 */
float LinearFunction(float score, float min_value, float max_value);

/*
 * @brief The function call specific map functions specificed by fun_type.
 * @param score, the value
 * @param fun_type, the norm function type
 * @param min_value, max_value, inflection points.
 */
float MapFunction(float score, NormFunType fun_type, float min_value,
                  float max_value);

/*
 * @brief Compute the distance of a point to a straight line
 * @param point
 * @param line, the straight line.
 * @return the distance
 */
float DistanceToLine(const PointInt &point,
                     std::pair<PointInt, PointInt> &line);

/*
 * @brief Get the size catgory of a vehicle with a special type
 * @param model_type, the type of the vehicle
 */
SizeCatgory GetVehicleSizeCatgory(const VehicleModelType &model_type);

VEHICLE_SNAP_STRATEGY_NAMESPACE_END

#endif  // VEHICLE_COMMON_UTILITY_UTILS_HPP_
