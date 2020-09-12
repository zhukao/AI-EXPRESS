/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file profiler.h
 * @brief
 * @author ruoting.ding
 * @email ruoting.ding@horizon.ai
 * @date 2019/4/15
 */

#ifndef HOBOTXSTREAM_PROFILER_H_
#define HOBOTXSTREAM_PROFILER_H_

#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


inline std::int64_t getMicroSecond() {
  auto time_now = std::chrono::system_clock::now();
  auto duration_in_ms = std::chrono::duration_cast<std::chrono::microseconds>(
      time_now.time_since_epoch());
  return duration_in_ms.count();
}

struct FpsStatisticInfo {
  /// the interval of Statistical result output
  static int cycle_ms;
  /// FPS start time
  std::int64_t pre_time;
  /// current total count
  std::atomic_int cnt;

  FpsStatisticInfo() { cnt = 0; }
};

struct TimeStatisticInfo {
  /// the interval of Statistical result output
  static int cycle_ms;
  /// process start time
  std::int64_t pre_time;

  TimeStatisticInfo() { pre_time = 0; }
};

#if 0
struct StatisticInfo {
  /// the interval of Statistical result output
  static int cycle_num;
  /// the interval of Statistical result output
  static int cycle_ms;

  /// start time
  int64_t pre_time;
  /// current total count
  std::atomic_int cnt;

  StatisticInfo() {
    cnt = 0;
    // pre_time = getMicroSecond();
  }
};
#endif

class ProfilerScope;
class Profiler;
typedef std::shared_ptr<Profiler> ProfilerPtr;

class Profiler : public std::enable_shared_from_this<Profiler> {
 public:
  Profiler() = default;
  ~Profiler();

  enum class State { kNotRunning = 0, kRunning };

  static ProfilerPtr Get();
  static ProfilerPtr Release() {
    if (instance_) {
      instance_ = nullptr;
    }
    return instance_;
  }
  inline bool IsRunning() const { return state_ == State::kRunning; }

  void Log(const std::stringstream &ss);

  inline void Start() { SetState(State::kRunning); }
  inline void Stop() { SetState(State::kNotRunning); }

  bool SetOutputFile(const std::string &file);

  void SetIntervalForTimeStat(int cycle_ms);
  void SetIntervalForFPSStat(int cycle_ms);

  enum class Type { kFps, kProcessTime };
  std::unique_ptr<ProfilerScope> CreateScope(const std::string &name,
                                             const std::string &tag);

  std::string name_ = "";

 private:
  Profiler(const Profiler &) = delete;
  void SetState(Profiler::State state);
  static ProfilerPtr instance_;
  State state_ = State::kNotRunning;

  std::unordered_map<std::string, std::shared_ptr<FpsStatisticInfo>> fps_stats_;
  std::unordered_map<std::string, std::shared_ptr<TimeStatisticInfo>>
      time_stats_;

  std::fstream foi_;
  std::mutex lock_;
};

class ProfilerScope {
 public:
  ProfilerScope(ProfilerPtr profiler, const std::string &name,
                const std::string tag)
      : profiler_(profiler), name_(name), tag_(tag) {}

  virtual ~ProfilerScope() {}

 protected:
  ProfilerPtr profiler_;
  std::string name_;
  std::string tag_;
};

class ScopeTime : public ProfilerScope {
 public:
  ScopeTime(ProfilerPtr profiler, const std::string &name,
            const std::string tag, std::shared_ptr<TimeStatisticInfo> stat)
      : ProfilerScope(profiler, name, tag), stat_(stat) {
    if (stat_->pre_time == 0) {
      stat_->pre_time = getMicroSecond();
    }
  }

  ~ScopeTime() {
    auto cur_time = getMicroSecond();
    auto diff = cur_time - stat_->pre_time;
    if (diff > stat_->cycle_ms) {
      std::stringstream ss;
      ss << name_ << " | " << std::this_thread::get_id() << " | "
         << "X"
         << " | " << std::to_string(stat_->pre_time) << " | "
         << std::to_string(diff) << "\n";
      profiler_->Log(ss);
    }
    stat_->pre_time = 0;
  }

 private:
  std::shared_ptr<TimeStatisticInfo> stat_;
};

class ScopeFps : public ProfilerScope {
 public:
  ScopeFps(ProfilerPtr profiler, const std::string &name, const std::string tag,
           std::shared_ptr<FpsStatisticInfo> stat)
      : ProfilerScope(profiler, name, tag), stat_(stat) {
    if (stat_->cnt == 0) {
      stat_->pre_time = getMicroSecond();
    }
    stat_->cnt++;
  }

  ~ScopeFps() {
    auto cur_time = getMicroSecond();
    auto diff = cur_time - stat_->pre_time;
    if (diff > stat_->cycle_ms) {
      std::stringstream ss;
      ss << name_ << " | " << std::this_thread::get_id() << " | "
         << "C"
         << " | " << std::to_string(stat_->pre_time) << " | "
         << stat_->cnt / (diff / 1000.0 / 1000) << "\n";
      ss << name_ << " | " << std::this_thread::get_id() << " | "
         << "C"
         << " | " << std::to_string(cur_time) << " | "
         << "0"
         << "\n";
      profiler_->Log(ss);
      stat_->cnt = 0;
    }
  }

 private:
  std::shared_ptr<FpsStatisticInfo> stat_;
};

#define RUN_FPS_PROFILER_WITH_PROFILER(profiler, name) \
  std::unique_ptr<ProfilerScope> scope_fps;            \
  if (profiler->IsRunning()) {                         \
    scope_fps = profiler->CreateScope(name, "FPS");    \
  }

#define RUN_TIME_PROFILER_WITH_PROFILER(profiler, name) \
  std::unique_ptr<ProfilerScope> scope_time;            \
  if (profiler->IsRunning()) {                          \
    scope_time = profiler->CreateScope(name, "TIME");   \
  }

#define RUN_FPS_PROFILER(name) \
  RUN_FPS_PROFILER_WITH_PROFILER(Profiler::Get(), name)

// TODO(zhe.sun) SnapshotMethod
#define RUN_PROCESS_TIME_PROFILER(name) \
  RUN_TIME_PROFILER_WITH_PROFILER(Profiler::Get(), name)

#endif  // HOBOTXSTREAM_PROFILER_H_
