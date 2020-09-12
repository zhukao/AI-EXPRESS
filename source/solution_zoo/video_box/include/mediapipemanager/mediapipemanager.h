#ifndef INCLUDE_MEDIAPIPEMANAGER_H_
#define INCLUDE_MEDIAPIPEMANAGER_H_

#include <mutex>
#include <vector>

#include "mediapipemanager/meidapipeline.h"

namespace horizon {
namespace vision {

class MediaPipeManager {
public:
  static MediaPipeManager &GetInstance();
  ~MediaPipeManager() = default;
  int Init(int max_pool_count = 32);
  int AddPipeLine(std::shared_ptr<MediaPipeLine> pipeline);
  const std::vector<std::shared_ptr<MediaPipeLine>> &GetPipeLine();

protected:
private:
  MediaPipeManager();
  MediaPipeManager(const MediaPipeManager &);
  MediaPipeManager &operator=(const MediaPipeManager &);

  static MediaPipeManager *instance_;
  std::mutex manager_mutex_;
  bool initialized_;
  uint32_t vp_max_pool_count_;
  std::vector<std::shared_ptr<MediaPipeLine>> media_pipelines_;
  // Manager Group ID
  // Manager Pipelines
};

}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_MEDIAPIPEMANAGER_H_