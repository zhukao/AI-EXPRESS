/*
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief: plate vote method
 * @author : tangji.sun
 * @email : tangji.sun@horizon.ai
 * @date: 2019-11-08
 */

#include "plate_vote_method/plate_vote_method.h"
#include <assert.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <numeric>
#include <queue>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"
using hobot::vision::BBox;

#define InvalidType -1

namespace xstream {

int PlateVoteMethod::Init(const std::string &config_file_path) {
  LOGI << "Init " << config_file_path << std::endl;
  std::ifstream config_ifs(config_file_path);
  if (!config_ifs.good()) {
    LOGF << "open config file failed.";
  }
  Json::Value config_jv;
  config_ifs >> config_jv;
  if (config_jv.isMember("max_slide_window_size") &&
      config_jv["max_slide_window_size"].isNumeric()) {
    max_slide_window_size_ = config_jv["max_slide_window_size"].asInt();
  } else {
    max_slide_window_size_ = 100;
  }
  HOBOT_CHECK(max_slide_window_size_ > 0);

  method_param_ = nullptr;

  return 0;
}

std::vector<std::vector<BaseDataPtr>> PlateVoteMethod::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  LOGI << "Run PlateVoteMethod";
  LOGD << "input's size: " << input.size();

  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    const auto &frame_input = input[i];
    auto &frame_output = output[i];
    RunSingleFrame(frame_input, frame_output);
  }
  return output;
}

void PlateVoteMethod::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> &frame_output) {
  HOBOT_CHECK(frame_input.size() == 3);  // BBox, disappeard track_id and type.
  frame_output.resize(1);                // type

  auto out_infos = std::make_shared<BaseDataVector>();

  frame_output[0] = out_infos;

  std::vector<BaseDataPtr> boxes;
  if (frame_input[0]) {
    boxes = std::static_pointer_cast<BaseDataVector>(frame_input[0])->datas_;
  }

  std::vector<BaseDataPtr> disappeared_track_ids;
  if (frame_input[1]) {
    disappeared_track_ids =
        std::static_pointer_cast<BaseDataVector>(frame_input[1])->datas_;
  }

  std::vector<BaseDataPtr> vote_infos;
  if (frame_input[2]) {
    vote_infos =
        std::static_pointer_cast<BaseDataVector>(frame_input[2])->datas_;
  }

  HOBOT_CHECK(boxes.size() == vote_infos.size());
  LOGI << "box num: " << boxes.size();
  for (size_t i = 0; i < boxes.size(); ++i) {
    const auto &box =
        std::static_pointer_cast<XStreamData<BBox>>(boxes[i])->value;

    assert(box.id >= 0);
    uint32_t track_id = static_cast<uint32_t>(box.id);

    XStreamData<std::vector<int>> vote_info;
    if (vote_infos[i]->state_ == DataState::VALID) {
      auto info =
          std::static_pointer_cast<XStreamData<std::vector<int>>>(vote_infos[i])
              ->value;
      if (info.empty()) {
        auto iter = slide_window_map.find(track_id);
        if (iter == slide_window_map.end()) {
          vote_info.value.clear();
        } else {
          vote_info.value = slide_window_map[track_id].back().value;
        }
      } else {
        vote_info.value = info;
      }

    } else {
      auto iter = slide_window_map.find(track_id);
      if (iter == slide_window_map.end()) {
        vote_info.value.clear();
      } else {
        vote_info.value = slide_window_map[track_id].back().value;
      }
    }
    if (!vote_info.value.empty()) {
      auto iter = slide_window_map.find(track_id);
      if (iter == slide_window_map.end()) {
        slide_window_map[track_id].push_back(vote_info);

      } else {
        int queue_size = slide_window_map[track_id].size();
        if (queue_size < max_slide_window_size_) {
          slide_window_map[track_id].push_back(vote_info);
        } else if (queue_size == max_slide_window_size_) {
          slide_window_map[track_id].pop_front();
          assert(slide_window_map[track_id].size()
            == static_cast<std::size_t>(queue_size - 1));
          slide_window_map[track_id].push_back(vote_info);
        } else {
          HOBOT_CHECK(0) << "impossible.";
        }
      }
    }

    std::shared_ptr<XStreamData<std::vector<int>>> vote_info_ptr(
        new XStreamData<std::vector<int>>());
    std::vector<std::string> vote_queue;
    auto iter = slide_window_map.find(track_id);
    if (iter != slide_window_map.end()) {
      auto &slide_window = slide_window_map[track_id];

      for (auto &plate : slide_window) {
        std::string str;
        for (auto s : plate.value) {
          str += s;
        }
        vote_queue.push_back(str);
      }
    }

    std::string result;
    if (vote_queue.size() == 1) {
      result = vote_queue.front();
    } else {
      result = Rover(vote_queue);
    }
    std::vector<int> vec;
    for (auto s : result) {
      vec.push_back(static_cast<int>(s));
    }
    vote_info_ptr->value = vec;

    out_infos->datas_.push_back(vote_info_ptr);
    if (vote_info_ptr->value.empty()) {
      out_infos->datas_.back()->state_ = DataState::INVALID;
    }
  }

  for (const auto &data : disappeared_track_ids) {
    // LOGD << "disappeared_track_ids loops";
    auto disappeared_track_id =
        std::static_pointer_cast<XStreamData<uint32_t>>(data)->value;
    auto iter = slide_window_map.find(disappeared_track_id);
    if (iter != slide_window_map.end()) {
      slide_window_map.erase(iter);
    }
  }

  // LOGD << "track_id2slide_window's size: " << slide_window_map.size();
}
std::vector<std::vector<int>> PlateVoteMethod::EditDistance(
    const std::string &src, const std::string &dest) {
  auto len1 = src.length(), len2 = dest.length();

  std::vector<std::vector<int>> edit(len1 + 1);
  for (auto &v : edit) {
    v.resize(len2 + 1);
  }

  for (std::size_t i = 0; i <= len1; ++i) {
    edit[i][0] = i;
  }
  for (std::size_t j = 0; j <= len2; ++j) {
    edit[0][j] = j;
  }

  for (std::size_t i = 1; i <= len1; ++i) {
    for (std::size_t j = 1; j <= len2; ++j) {
      if (src[i - 1] == dest[j - 1]) {
        edit[i][j] = edit[i - 1][j - 1];
      } else {
        int x = 1 + edit[i - 1][j];
        int y = 1 + edit[i][j - 1];
        int z = 1 + edit[i - 1][j - 1];
        edit[i][j] = std::min(x, std::min(y, z));
      }
    }
  }
  return edit;
}

// align the two strings
std::pair<std::string, std::string> PlateVoteMethod::Reconstruct(
    const std::string &src, const std::string &dest,
    const std::vector<std::vector<int>> &edit) {
  std::vector<char> s_star, t_star;
  int m = src.length() - 1, n = dest.length() - 1;

  std::vector<std::function<std::pair<int, int>(int, int)>> options;
  options.emplace_back([](int x, int y) {
    return std::pair<int, int>({x - 1, y - 1});
  });
  options.emplace_back([](int x, int y) {
    return std::pair<int, int>({x - 1, y});
  });
  options.emplace_back([](int x, int y) {
    return std::pair<int, int>({x, y - 1});
  });

  auto edit_value = [&edit](int i, int j) {
    return (i < 0 || j < 0) ? std::numeric_limits<int>::max() : edit[i][j];
  };
  while (m >= 0 && n >= 0) {
    std::vector<int> values{edit_value(m - 1, n - 1), edit_value(m - 1, n),
                            edit_value(m, n - 1)};
    auto min_ele = std::min_element(values.begin(), values.end());
    auto min_idx = std::distance(values.begin(), min_ele);
    auto f = options[min_idx];
    if (min_idx == 0) {
      s_star.emplace_back(src[m]);
      t_star.emplace_back(dest[n]);
    } else if (min_idx == 1) {
      s_star.emplace_back(src[m]);
      t_star.emplace_back('~');
    } else {
      s_star.emplace_back('~');
      t_star.emplace_back(dest[n]);
    }
    auto next = f(m, n);
    m = next.first;
    n = next.second;
  }
  std::string new_src;
  std::string new_dest;
  for (auto i = s_star.size(); i > 0; --i) {
    new_src = new_src + s_star[i - 1];
  }
  for (auto i = t_star.size(); i > 0; --i) {
    new_dest = new_dest + t_star[i - 1];
  }
  return {new_src, new_dest};
}

std::string PlateVoteMethod::Rover(const std::vector<std::string> &strs) {
  if (strs.empty()) {
    return "";
  }
  auto str_guard = strs[0];
  std::vector<std::string> aligned_strs;
  for (std::size_t i = 1; i < strs.size(); ++i) {
    auto edit = EditDistance(str_guard, strs[i]);
    auto aligned = Reconstruct(str_guard, strs[i], edit);
    str_guard = aligned.first;
    aligned_strs.emplace_back(aligned.second);
  }

  std::string voted_str = "";
  for (std::size_t i = 0; i < str_guard.length(); ++i) {
    std::vector<char> chars;
    for (std::size_t j = 0; j < aligned_strs.size(); ++j) {
      chars.emplace_back(aligned_strs[j][i]);
    }
    std::map<char, int> max_dict = {};
    int cnt = 0;
    char itm = ' ';
    for (std::size_t j = 0; j < aligned_strs.size(); ++j) {
      auto item = aligned_strs[j][i];
      max_dict[item] = max_dict.emplace(item, 0).first->second + 1;
      if (max_dict[item] >= cnt) {
        std::tie(cnt, itm) = std::tie(max_dict[item], item);
      }
    }
    if (itm != '~') {
      voted_str = voted_str + itm;
    }
  }
  return voted_str;
}

}  // namespace xstream
