/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved
 * @Author  jianglei.huang
 * @version 0.0.1
 * @date  2019.10.16
 */

#include "vehicle_match_strategy/pairs_match.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>
#include "hobotlog/hobotlog.hpp"
#include "json/json.h"
#include "vehicle_common_utility/data_type.hpp"
#include "vehicle_match_strategy/parse_config_from_json.hpp"
#include "vehicle_snap_strategy_namespace.hpp"

VEHICLE_SNAP_STRATEGY_NAMESPACE

PairsMatch::PairsMatch() {
  params_ = std::make_shared<Params>();
  Init();
}

PairsMatch::PairsMatch(const std::string &config_file) {
  auto parser = ParseJsonMatch(config_file);
  params_ = std::make_shared<Params>(std::move(parser.params_));
  Init();
}

PairsMatch::PairsMatch(const Params &params) {
  params_ = std::make_shared<Params>(params);
  Init();
}

TrebleVec PairsMatch::InitThreeDimVector() {
  return TrebleVec(
      space_dim_,
      DoubleVec(params_->bbox_cells, std::vector<int>(params_->bbox_cells, 0)));
}

void PairsMatch::Init() {
  img_cell_width_ = params_->img_width / params_->img_cells;
  img_cell_height_ = params_->img_height / params_->img_cells;
  space_dim_ = params_->img_cells * params_->img_cells;

  stat_matrix_ = std::vector<TrebleVec>(2, InitThreeDimVector());
}

std::vector<int> PairsMatch::GetRelativePos(
    const vision::BBox &large_bbox, const vision::BBox &small_bbox) const {
  float large_cx = BBoxCenterX(large_bbox);
  float large_cy = BBoxCenterY(large_bbox);
  float large_w = large_bbox.x2 - large_bbox.x1;
  float large_h = large_bbox.y2 - large_bbox.y1;

  float small_cx = std::max(0.f, BBoxCenterX(small_bbox) - large_bbox.x1);
  float small_cy = std::max(0.f, BBoxCenterY(small_bbox) - large_bbox.y1);

  // Relative position of large bbox in image
  int col_in_img = static_cast<int>(large_cx / img_cell_width_);
  int row_in_img = static_cast<int>(large_cy / img_cell_height_);
  int row_in_stat = row_in_img * params_->img_cells + col_in_img;

  // Relative position of small bbox in large bbox
  float large_cell_w = large_w / params_->bbox_cells;
  float large_cell_h = large_h / params_->bbox_cells;
  int col_in_large_bbox = small_cx / large_cell_w;
  int row_in_large_bbox = small_cy / large_cell_h;

  return std::vector<int>{row_in_stat, row_in_large_bbox, col_in_large_bbox};
}

float PairsMatch::ComputePairDistance(const vision::BBox &large_bbox,
                                      const vision::BBox &small_bbox) const {
  std::vector<int> relative_pos = GetRelativePos(large_bbox, small_bbox);
  std::vector<int> max_index_of_2dmat =
      ArgMax(stat_matrix_[working_mat_][relative_pos[0]]);

  float pair_dis;
  if (stat_matrix_[working_mat_][relative_pos[0]][max_index_of_2dmat[0]]
                  [max_index_of_2dmat[1]] < params_->min_stat_value) {
    std::vector<int> max_index_of_3dmat = ArgMax(stat_matrix_[working_mat_]);
    if (stat_matrix_[working_mat_][max_index_of_3dmat[0]][max_index_of_3dmat[1]]
                    [max_index_of_3dmat[2]] < params_->min_stat_value) {
      pair_dis = params_->max_pair_dis + 10;
    } else {
      std::vector<int> shape{params_->img_cells, params_->img_cells};
      std::vector<int> max_stat_index =
          UnravelIndex(max_index_of_3dmat[0], shape);
      std::vector<int> this_stat_index = UnravelIndex(relative_pos[0], shape);
      pair_dis = abs(max_stat_index[0] - this_stat_index[0]) +
                 abs(max_stat_index[1] - this_stat_index[1]) +
                 abs(relative_pos[1] - max_index_of_3dmat[1]) +
                 abs(relative_pos[2] - max_index_of_3dmat[2]);
    }
  } else {
    pair_dis = abs(relative_pos[1] - max_index_of_2dmat[0]) +
               abs(relative_pos[2] - max_index_of_2dmat[1]);
  }
  return pair_dis;
}

void PairsMatch::UpdateStatMat(const vision::BBox &vehicle_bbox,
                               const vision::BBox &plate_bbox) {
  std::vector<int> pos_index = GetRelativePos(vehicle_bbox, plate_bbox);
  stat_matrix_[0][pos_index[0]][pos_index[1]][pos_index[2]]++;
  stat_matrix_[1][pos_index[0]][pos_index[1]][pos_index[2]]++;
}

void PairsMatch::PrepareMatchIDs(const VehicleListPtr &vehicle_list,
                                 const PlateListPtr &plate_list) {
  for (std::size_t i = 0; i < vehicle_list->size(); ++i) {
    vision::BBox vehicle_bbox((*(*vehicle_list)[i].bbox));
    for (std::size_t j = 0; j < plate_list->size(); ++j) {
      vision::BBox plate_bbox((*(*plate_list)[j].bbox));
      float in_ratio_value = BBoxInRatio(plate_bbox, vehicle_bbox);
      if (in_ratio_value > params_->overlap_th) {
        if (plate_match_ids_.find(j) == plate_match_ids_.end())
          plate_match_ids_.insert({j, std::vector<int>{static_cast<int>(i)}});
        else
          plate_match_ids_[j].push_back(i);
        if (vehicle_match_ids_.find(i) == vehicle_match_ids_.end())
          vehicle_match_ids_.insert({i, std::vector<int>{static_cast<int>(j)}});
        else
          vehicle_match_ids_[i].push_back(j);
      }
    }
  }
}

void PairsMatch::ProcessOneToOneMatch(VehicleListPtr &vehicle_list,
                                      const PlateListPtr &plate_list) {
  for (size_t i = 0; i < vehicle_list->size(); ++i) {
    if (vehicle_match_ids_.find(i) == vehicle_match_ids_.end()) continue;
    if (vehicle_match_ids_[i].size() == 1 &&
        plate_match_ids_[vehicle_match_ids_[i][0]].size() == 1) {
      vision::BBox vehicle_bbox((*(*vehicle_list)[i].bbox));
      auto plate = (*plate_list)[vehicle_match_ids_[i][0]];
      vision::BBox plate_bbox(*plate.bbox);
      (*vehicle_list)[i].plate = std::make_shared<Plate>(plate);

      UpdateStatMat(vehicle_bbox, plate_bbox);
      plate_match_ids_.erase(vehicle_match_ids_[i][0]);
      vehicle_match_ids_.erase(i);
    }
  }
}

bool PairsMatch::CheckIfStatReady() {
  ++count_;
  if (!is_working_ && count_ < params_->min_statis_frames) {
    return false;
  } else {
    is_working_ = true;
    return true;
  }
}

void PairsMatch::ProcessMultiToMultiMatch(VehicleListPtr &vehicle_list,
                                          const PlateListPtr &plate_list) {
  std::vector<Triple> match_pre;
  for (const auto &itr1 : vehicle_match_ids_) {
    vision::BBox vehicle_bbox((*(*vehicle_list)[itr1.first].bbox));
    for (const auto &itr2 : itr1.second) {
      vision::BBox plate_bbox((*(*plate_list)[itr2].bbox));
      float pair_dis = ComputePairDistance(vehicle_bbox, plate_bbox);
      if (pair_dis < params_->max_pair_dis) {
        match_pre.push_back(std::make_tuple(itr1.first, itr2, pair_dis));
      }
    }
  }
  std::sort(match_pre.begin(), match_pre.end(),
            [](const Triple &a,
               const Triple &b) { return std::get<2>(a) < std::get<2>(b); });

  for (size_t i = 0; i < match_pre.size(); ++i) {
    if (vehicle_match_ids_.find(std::get<0>(match_pre[i])) !=
            vehicle_match_ids_.end() &&
        plate_match_ids_.find(std::get<1>(match_pre[i])) !=
            plate_match_ids_.end()) {
      auto plate = (*plate_list)[std::get<1>(match_pre[i])];
      (*vehicle_list)[std::get<0>(match_pre[i])].plate =
          std::make_shared<Plate>(plate);

      vehicle_match_ids_.erase(std::get<0>(match_pre[i]));
      plate_match_ids_.erase(std::get<1>(match_pre[i]));
    }
  }
}

void PairsMatch::Process(VehicleListPtr &vehicle_list,
                         const PlateListPtr &plate_list) {
  PrepareMatchIDs(vehicle_list, plate_list);
  ProcessOneToOneMatch(vehicle_list, plate_list);
  if (CheckIfStatReady()) {
    ProcessMultiToMultiMatch(vehicle_list, plate_list);
  }

  if (count_ > params_->work_frames) {
    stat_matrix_[working_mat_] = InitThreeDimVector();
    working_mat_ = (working_mat_+1) % 2;
    count_ = 0;
  }

  plate_match_ids_.clear();
  vehicle_match_ids_.clear();

  return;
}

VEHICLE_SNAP_STRATEGY_NAMESPACE_END
