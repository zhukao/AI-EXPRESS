/*
 * Copyright (c) 2019 Horizon Robotics
 * @author sunshuhuan
 * @file vision_type_util.cpp
 * @date 2019.03.26
 */

#include "horizon/vision_type/vision_type_util.h"

#include <cstdlib>
#include <cstring>

#include "horizon/vision_type/vision_msg.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_error.h"

#define CHECK_NULL(p)                                                          \
  if (nullptr == p)                                                            \
    return kHorizonVisionErrorParam;

#define CHECK_SUCCESS(code)                                                    \
  if (kHorizonVisionSuccess != code)                                           \
    return code;

char *HorizonVisionStrDup(const char *str) {
  if (str) {
    auto str_len = std::strlen(str) + 1;
    auto new_str = static_cast<char *>(std::malloc(str_len));
    if (new_str) {
      std::memcpy(new_str, str, str_len);
    }
    return new_str;
  }
  return nullptr;
}

char *HorizonVisionMemDup(const char *src_addr, size_t size) {
  if (src_addr) {
    auto new_addr = static_cast<char *>(std::malloc(size));
    if (new_addr) {
      std::memcpy(new_addr, src_addr, size);
    }
    return new_addr;
  }
  return nullptr;
}

int HorizonVisionAllocImage(HorizonVisionImage **pimg) {
  CHECK_NULL(pimg)
  *pimg = static_cast<HorizonVisionImage *>(
      std::calloc(1, sizeof(HorizonVisionImage)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyImage(HorizonVisionImage *image,
                           bool copy_image_data,
                           HorizonVisionImage *new_image) {
  CHECK_NULL(image)
  CHECK_NULL(new_image)
  *new_image = *image;
  if (image->data && copy_image_data && image->data_size != 0) {
    new_image->data = static_cast<uint8_t *>(
        std::calloc(image->data_size, sizeof(uint8_t)));
    memcpy(new_image->data, image->data, image->data_size);
  } else {
    new_image->data = nullptr;
    new_image->data_size = 0;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupImage(HorizonVisionImage *image,
                          bool dup_image_data,
                          HorizonVisionImage **pnew_image) {
  CHECK_NULL(image)
  CHECK_SUCCESS(HorizonVisionAllocImage(pnew_image))
  CHECK_SUCCESS(HorizonVisionCopyImage(image, dup_image_data, *pnew_image))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanImage(HorizonVisionImage *img) {
  CHECK_NULL(img)
  if (img->data) {
    std::free(img->data);
    img->data = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanImageWithoutData(HorizonVisionImage *img) {
  CHECK_NULL(img)
  if (img->data) {
    img->data = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeImage(HorizonVisionImage *img) {
  CHECK_NULL(img)
  HorizonVisionCleanImage(img);
  std::free(img);
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeImageWithoutData(HorizonVisionImage *img) {
  CHECK_NULL(img)
  img->data = nullptr;
  std::free(img);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocImageFrame(HorizonVisionImageFrame **pimage) {
  CHECK_NULL(pimage)
  *pimage = static_cast<HorizonVisionImageFrame *>(
      std::calloc(1, sizeof(HorizonVisionImageFrame)));
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocImageFrames(HorizonVisionImageFrame ***pimage,
                                  uint32_t frame_num) {
  CHECK_NULL(pimage)
  auto image_frames = static_cast<HorizonVisionImageFrame **>(
      std::calloc(frame_num, sizeof(HorizonVisionImageFrame *)));
  *pimage = image_frames;
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyImageFrame(HorizonVisionImageFrame *image_frame,
                                bool copy_image_data,
                                HorizonVisionImageFrame *new_image_frame) {
  CHECK_NULL(image_frame)
  CHECK_NULL(new_image_frame)
  *new_image_frame = *image_frame;
  CHECK_SUCCESS(HorizonVisionCopyImage(&image_frame->image,
                                       copy_image_data,
                                       &new_image_frame->image))
  return kHorizonVisionSuccess;
}

int HorizonVisionDupImageFrame(HorizonVisionImageFrame *image_frame,
                               bool dup_image_data,
                               HorizonVisionImageFrame **pnew_image_frame) {
  CHECK_NULL(image_frame)
  CHECK_SUCCESS(HorizonVisionAllocImageFrame(pnew_image_frame))
  CHECK_SUCCESS(HorizonVisionCopyImageFrame(image_frame,
                                            dup_image_data,
                                            *pnew_image_frame))
  return kHorizonVisionSuccess;
}

static int DupImageFrames(HorizonVisionImageFrame **image_frame,
                     uint32_t image_num,
                     HorizonVisionImageFrame ***pnew_image_frame,
                     bool dup_image_data) {
  CHECK_NULL(image_frame)
  HorizonVisionImageFrame **new_frames = nullptr;
  auto ret = HorizonVisionAllocImageFrames(&new_frames, image_num);
  if (ret != kHorizonVisionSuccess) {
    return ret;
  }
  for (uint i = 0; i < image_num; ++i) {
    HorizonVisionDupImageFrame(image_frame[i], dup_image_data, &new_frames[i]);
  }
  *pnew_image_frame = new_frames;
  return kHorizonVisionSuccess;
}

int HorizonVisionDupImageFrames(HorizonVisionImageFrame **image_frame,
                                uint32_t image_num,
                                HorizonVisionImageFrame ***pnew_image_frame) {
  return DupImageFrames(image_frame, image_num, pnew_image_frame, true);
}

int HorizonVisionDupImageFramesWithoutData(
    HorizonVisionImageFrame **image_frame,
    uint32_t image_num,
    HorizonVisionImageFrame ***pnew_image_frame) {
  return DupImageFrames(image_frame, image_num, pnew_image_frame, false);
}

int HorizonVisionFreeImageFrame(HorizonVisionImageFrame *image) {
  CHECK_NULL(image)
  HorizonVisionCleanImage(&image->image);
  std::free(image);
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeImageFrameWithoutData(HorizonVisionImageFrame *image) {
  CHECK_NULL(image)
  HorizonVisionCleanImageWithoutData(&image->image);
  std::free(image);
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeImageFrames(HorizonVisionImageFrame **images,
                                 uint32_t image_num) {
  CHECK_NULL(images)
  for (uint32_t i = 0; i < image_num; ++i) {
    auto ret = HorizonVisionFreeImageFrame(images[i]);
    if (ret != kHorizonVisionSuccess) {
      return ret;
    }
  }
  std::free(images);
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeImageFramesWithoutData(HorizonVisionImageFrame **images,
                                            uint32_t image_num) {
  CHECK_NULL(images)
  for (uint32_t i = 0; i < image_num; ++i) {
    auto ret = HorizonVisionFreeImageFrameWithoutData(images[i]);
    if (ret != kHorizonVisionSuccess) {
      return ret;
    }
  }
  std::free(images);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocLandmarks(HorizonVisionLandmarks **pnew_lmks) {
  CHECK_NULL(pnew_lmks)
  *pnew_lmks = static_cast<HorizonVisionLandmarks *>(
      std::calloc(1, sizeof(HorizonVisionLandmarks)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyLandmarks(HorizonVisionLandmarks *lmks,
                               HorizonVisionLandmarks *new_lmks) {
  CHECK_NULL(lmks)
  CHECK_NULL(new_lmks)
  *new_lmks = *lmks;
  if (lmks->num == 0) {
    return kHorizonVisionSuccess;
  }
  auto points = static_cast<HorizonVisionPoint *>(
      std::calloc(lmks->num, sizeof(HorizonVisionPoint)));
  if (nullptr == points)
    return kHorizonVisionErrorNoMem;
  std::memcpy(points, lmks->points, sizeof(HorizonVisionPoint) * lmks->num);
  new_lmks->points = points;
  return kHorizonVisionSuccess;
}

int HorizonVisionDupLandmarks(HorizonVisionLandmarks *lmks,
                              HorizonVisionLandmarks **pnew_lmks) {
  CHECK_NULL(lmks)
  CHECK_SUCCESS(HorizonVisionAllocLandmarks(pnew_lmks))
  CHECK_SUCCESS(HorizonVisionCopyLandmarks(lmks, *pnew_lmks))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanLandmarks(HorizonVisionLandmarks *lmks) {
  CHECK_NULL(lmks)
  lmks->num = 0;
  if (lmks->points) {
    std::free(lmks->points);
    lmks->points = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeLandmarks(HorizonVisionLandmarks *lmks) {
  CHECK_NULL(lmks)
  HorizonVisionCleanLandmarks(lmks);
  std::free(lmks);
  return kHorizonVisionSuccess;
}
// char array
int HorizonVisionAllocCharArray(HorizonVisionCharArray **pnew_farray) {
  CHECK_NULL(pnew_farray)
  *pnew_farray = static_cast<HorizonVisionCharArray *>(
      std::calloc(1, sizeof(HorizonVisionCharArray)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyCharArray(HorizonVisionCharArray *farray,
                               HorizonVisionCharArray *new_farray) {
  CHECK_NULL(farray)
  if (farray->num == 0) {
    return kHorizonVisionSuccess;
  }
  char *values = nullptr;
  if (new_farray->num != farray->num) {
    std::free(new_farray->values);
    new_farray->num = 0;
    values = static_cast<char *>(std::calloc(farray->num, sizeof(char)));
    if (nullptr == values)
      return kHorizonVisionErrorNoMem;
  } else {
    values = new_farray->values;
  }
  std::memcpy(values, farray->values, sizeof(char) * farray->num);
  *new_farray = *farray;
  new_farray->values = values;
  return kHorizonVisionSuccess;
}

int HorizonVisionDupCharArray(HorizonVisionCharArray *farray,
                              HorizonVisionCharArray **pnew_farray) {
  CHECK_NULL(farray)
  CHECK_SUCCESS(HorizonVisionAllocCharArray(pnew_farray))
  CHECK_SUCCESS(HorizonVisionCopyCharArray(farray, *pnew_farray))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanCharArray(HorizonVisionCharArray *farray) {
  CHECK_NULL(farray)
  farray->num = 0;
  if (farray->values) {
    std::free(farray->values);
    farray->values = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeCharArray(HorizonVisionCharArray *farray) {
  CHECK_NULL(farray)
  HorizonVisionCleanCharArray(farray);
  std::free(farray);
  return kHorizonVisionSuccess;
}

// float array
int HorizonVisionAllocFloatArray(HorizonVisionFloatArray **pnew_farray) {
  CHECK_NULL(pnew_farray)
  *pnew_farray = static_cast<HorizonVisionFloatArray *>(
      std::calloc(1, sizeof(HorizonVisionFloatArray)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyFloatArray(HorizonVisionFloatArray *farray,
                                HorizonVisionFloatArray *new_farray) {
  CHECK_NULL(farray)
  if (farray->num == 0) {
    return kHorizonVisionSuccess;
  }
  float *values = nullptr;
  if (new_farray->num != farray->num) {
    std::free(new_farray->values);
    new_farray->num = 0;
    values = static_cast<float *>(std::calloc(farray->num, sizeof(float)));
    if (nullptr == values)
      return kHorizonVisionErrorNoMem;
  } else {
    values = new_farray->values;
  }
  std::memcpy(values, farray->values, sizeof(float) * farray->num);
  *new_farray = *farray;
  new_farray->values = values;
  return kHorizonVisionSuccess;
}

int HorizonVisionDupFloatArray(HorizonVisionFloatArray *farray,
                               HorizonVisionFloatArray **pnew_farray) {
  CHECK_NULL(farray)
  CHECK_SUCCESS(HorizonVisionAllocFloatArray(pnew_farray))
  CHECK_SUCCESS(HorizonVisionCopyFloatArray(farray, *pnew_farray))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanFloatArray(HorizonVisionFloatArray *farray) {
  CHECK_NULL(farray)
  farray->num = 0;
  if (farray->values) {
    std::free(farray->values);
    farray->values = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeFloatArray(HorizonVisionFloatArray *farray) {
  CHECK_NULL(farray)
  HorizonVisionCleanFloatArray(farray);
  std::free(farray);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocSegmentation(HorizonVisionSegmentation **pnew_farray) {
  CHECK_NULL(pnew_farray)
  *pnew_farray = static_cast<HorizonVisionSegmentation *>(
      std::calloc(1, sizeof(HorizonVisionSegmentation)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopySegmentation(HorizonVisionSegmentation *farray,
                                  HorizonVisionSegmentation *new_farray) {
  CHECK_NULL(farray)
  if (farray->num == 0) {
    return kHorizonVisionSuccess;
  }
  float *values = nullptr;
  if (new_farray->num != farray->num) {
    std::free(new_farray->values);
    new_farray->num = 0;
    values = static_cast<float *>(std::calloc(farray->num, sizeof(float)));
    if (nullptr == values)
      return kHorizonVisionErrorNoMem;
  } else {
    values = new_farray->values;
  }
  std::memcpy(values, farray->values, sizeof(float) * farray->num);
  *new_farray = *farray;
  new_farray->values = values;
  return kHorizonVisionSuccess;
}

int HorizonVisionDupSegmentation(HorizonVisionSegmentation *farray,
                                 HorizonVisionSegmentation **pnew_farray) {
  CHECK_NULL(farray)
  CHECK_SUCCESS(HorizonVisionAllocSegmentation(pnew_farray))
  CHECK_SUCCESS(HorizonVisionCopySegmentation(farray, *pnew_farray))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanSegmentation(HorizonVisionSegmentation *farray) {
  CHECK_NULL(farray)
  farray->num = 0;
  if (farray->values) {
    std::free(farray->values);
    farray->values = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeSegmentation(HorizonVisionSegmentation *farray) {
  CHECK_NULL(farray)
  HorizonVisionCleanSegmentation(farray);
  std::free(farray);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocFaceSmartData(HorizonVisionFaceSmartData **psmart) {
  CHECK_NULL(psmart)
  *psmart = static_cast<HorizonVisionFaceSmartData *>(
      std::calloc(1, sizeof(HorizonVisionFaceSmartData)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyFaceSmartData(HorizonVisionFaceSmartData *smart_data,
                                   HorizonVisionFaceSmartData *new_smart) {
  CHECK_NULL(smart_data)
  CHECK_NULL(new_smart)
  *new_smart = *smart_data;
  if (smart_data->landmarks) {
    CHECK_SUCCESS(
        HorizonVisionDupLandmarks(smart_data->landmarks, &new_smart->landmarks))
  }
  if (smart_data->feature) {
    CHECK_SUCCESS(
        HorizonVisionDupFloatArray(smart_data->feature, &new_smart->feature))
  }
  if (smart_data->encrypted_feature) {
    CHECK_SUCCESS(
        HorizonVisionDupCharArray(smart_data->encrypted_feature,
                                  &new_smart->encrypted_feature))
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupFaceSmartData(HorizonVisionFaceSmartData *smart_data,
                                  HorizonVisionFaceSmartData **pnew_smart) {
  CHECK_NULL(smart_data)
  CHECK_SUCCESS(HorizonVisionAllocFaceSmartData(pnew_smart))
  CHECK_SUCCESS(HorizonVisionCopyFaceSmartData(smart_data, *pnew_smart))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanFaceSmartData(HorizonVisionFaceSmartData *smart) {
  CHECK_NULL(smart)
  if (smart->landmarks) {
    HorizonVisionFreeLandmarks(smart->landmarks);
  }
  if (smart->feature) {
    HorizonVisionFreeFloatArray(smart->feature);
  }
  if (smart->encrypted_feature) {
    HorizonVisionFreeCharArray(smart->encrypted_feature);
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeFaceSmartData(HorizonVisionFaceSmartData *smart) {
  CHECK_NULL(smart)
  HorizonVisionCleanFaceSmartData(smart);
  std::free(smart);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocBodySmartData(HorizonVisionBodySmartData **psmart) {
  CHECK_NULL(psmart)
  *psmart = static_cast<HorizonVisionBodySmartData *>(
      std::calloc(1, sizeof(HorizonVisionBodySmartData)));
  return kHorizonVisionSuccess;
}

int HorizonVisionCopyBodySmartData(HorizonVisionBodySmartData *smart_data,
                                   HorizonVisionBodySmartData *new_smart) {
  CHECK_NULL(smart_data)
  CHECK_NULL(new_smart)
  *new_smart = *smart_data;
  if (smart_data->segmentation) {
    CHECK_SUCCESS(HorizonVisionDupSegmentation(smart_data->segmentation,
                                               &new_smart->segmentation))
  }
  if (smart_data->skeleton) {
    CHECK_SUCCESS(
        HorizonVisionDupLandmarks(smart_data->skeleton, &new_smart->skeleton))
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupBodySmartData(HorizonVisionBodySmartData *smart_data,
                                  HorizonVisionBodySmartData **pnew_smart) {
  CHECK_NULL(smart_data)
  CHECK_SUCCESS(HorizonVisionAllocBodySmartData(pnew_smart))
  CHECK_SUCCESS(HorizonVisionCopyBodySmartData(smart_data, *pnew_smart))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanBodySmartData(HorizonVisionBodySmartData *smart) {
  CHECK_NULL(smart)
  if (smart->skeleton) {
    HorizonVisionFreeLandmarks(smart->skeleton);
  }
  if (smart->segmentation) {
    HorizonVisionFreeSegmentation(smart->segmentation);
  }

  return kHorizonVisionSuccess;
}

int HorizonVisionFreeBodySmartData(HorizonVisionBodySmartData *smart) {
  CHECK_NULL(smart)
  HorizonVisionCleanBodySmartData(smart);
  std::free(smart);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocSmartData(HorizonVisionSmartData **psmart, int num) {
  CHECK_NULL(psmart)
  if (num == 0) {
    *psmart = nullptr;
    return kHorizonVisionSuccess;
  }
  auto smart_data = static_cast<HorizonVisionSmartData *>(
      std::calloc(num, sizeof(HorizonVisionSmartData)));
  for (auto i = 0; i < num; ++i) {
    smart_data[i].face = nullptr;
    smart_data[i].body = nullptr;
    smart_data[i].face_extra = nullptr;
  }
  *psmart = smart_data;
  return kHorizonVisionSuccess;
}

int HorizonVisionCopySmartData(HorizonVisionSmartData *smart_data,
                               HorizonVisionSmartData *new_smart) {
  CHECK_NULL(smart_data)
  CHECK_NULL(new_smart)
  *new_smart = *smart_data;
  new_smart->face = nullptr;
  new_smart->body = nullptr;
  // @note: copy extra_info pointer directly!
  if (smart_data->face) {
    CHECK_SUCCESS(
        HorizonVisionDupFaceSmartData(smart_data->face, &new_smart->face))
  }
  if (smart_data->body) {
    CHECK_SUCCESS(
        HorizonVisionDupBodySmartData(smart_data->body, &new_smart->body))
  }
  if (smart_data->face_extra) {
    new_smart->face_extra = reinterpret_cast<HorizonVisionFaceExtraInfo *>
    (calloc(1, sizeof(HorizonVisionFaceExtraInfo)));
    *new_smart->face_extra = *smart_data->face_extra;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupSmartData(HorizonVisionSmartData *smart_data,
                              HorizonVisionSmartData **pnew_smart) {
  CHECK_NULL(smart_data)
  CHECK_SUCCESS(HorizonVisionAllocSmartData(pnew_smart, 1))
  CHECK_SUCCESS(HorizonVisionCopySmartData(smart_data, *pnew_smart))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanSmartData(HorizonVisionSmartData *smart) {
  CHECK_NULL(smart)
  HorizonVisionFreeFaceSmartData(smart->face);
  smart->face = nullptr;
  HorizonVisionFreeBodySmartData(smart->body);
  smart->body = nullptr;
  if (smart->face_extra) {
    std::free(smart->face_extra);
  }
  smart->face_extra = nullptr;
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeSmartData(HorizonVisionSmartData *smart, int num) {
  CHECK_NULL(smart)
  for (auto i = 0; i < num; ++i) {
    HorizonVisionCleanSmartData(&smart[i]);
  }
  std::free(smart);

  return kHorizonVisionSuccess;
}

int HorizonVisionAllocSmartFrame(HorizonVisionSmartFrame **psmart) {
  CHECK_NULL(psmart)
  *psmart = static_cast<HorizonVisionSmartFrame *>(
      std::calloc(1, sizeof(HorizonVisionSmartFrame)));
  (*psmart)->image_num = 0;
  (*psmart)->image_frame = nullptr;
  (*psmart)->smart_data_list_num = 0;
  (*psmart)->smart_data_list = nullptr;
  return kHorizonVisionSuccess;
}

static int CopySmartFrame(HorizonVisionSmartFrame *smart_frame,
                     HorizonVisionSmartFrame *new_smart_frame,
                     bool deep_copy_img_data) {
  CHECK_NULL(smart_frame)
  CHECK_NULL(new_smart_frame)
  *new_smart_frame = *smart_frame;
  new_smart_frame->smart_data_list = nullptr;

  auto &image_num = smart_frame->image_num;
  if (image_num != 0) {
    new_smart_frame->image_frame = static_cast<HorizonVisionImageFrame **>(
        std::calloc(image_num, sizeof(HorizonVisionImageFrame *)));
  }
  for (uint32_t i = 0; i < image_num; i++) {
    auto &cp_image_frame = new_smart_frame->image_frame[i];
    auto *image_frame = smart_frame->image_frame[i];
    CHECK_SUCCESS(HorizonVisionDupImageFrame(image_frame,
                                             deep_copy_img_data,
                                             &cp_image_frame))
  }

  auto &data_num = smart_frame->smart_data_list_num;
  CHECK_SUCCESS(HorizonVisionAllocSmartData(
      &new_smart_frame->smart_data_list, data_num))
  for (uint32_t i = 0; i < data_num; i++) {
    auto &new_smart_data = new_smart_frame->smart_data_list[i];
    auto &old_smart_data = smart_frame->smart_data_list[i];
    CHECK_SUCCESS(HorizonVisionCopySmartData(&old_smart_data, &new_smart_data))
  }

  return kHorizonVisionSuccess;
}

int HorizonVisionCopySmartFrame(HorizonVisionSmartFrame *smart_frame,
                                HorizonVisionSmartFrame *new_smart_frame) {
  return CopySmartFrame(smart_frame, new_smart_frame, true);
}

int HorizonVisionCopySmartFrameWithoutData(
    HorizonVisionSmartFrame *smart_frame,
    HorizonVisionSmartFrame *new_smart_frame) {
  return CopySmartFrame(smart_frame, new_smart_frame, false);
}

static int dup_smart_frame(HorizonVisionSmartFrame *smart_frame,
                    HorizonVisionSmartFrame **pnew_smart_frame,
                    bool dup_img_data) {
  CHECK_NULL(smart_frame)
  CHECK_SUCCESS(HorizonVisionAllocSmartFrame(pnew_smart_frame))
  if (dup_img_data) {
    CHECK_SUCCESS(HorizonVisionCopySmartFrame(smart_frame,
                                              *pnew_smart_frame))
  } else {
    CHECK_SUCCESS(HorizonVisionCopySmartFrameWithoutData(smart_frame,
                                                         *pnew_smart_frame))
  }

  return kHorizonVisionSuccess;
}

int HorizonVisionDupSmartFrame(HorizonVisionSmartFrame *smart_frame,
                               HorizonVisionSmartFrame **pnew_smart_frame) {
  return dup_smart_frame(smart_frame, pnew_smart_frame, true);
}

int HorizonVisionDupSmartFrameWithoutData(
    HorizonVisionSmartFrame *smart_frame,
    HorizonVisionSmartFrame **pnew_smart_frame) {
  return dup_smart_frame(smart_frame, pnew_smart_frame, false);
}

static int FreeSmartFrame(HorizonVisionSmartFrame *smart,
                     bool free_img_data) {
  CHECK_NULL(smart)
  for (uint i = 0; i < smart->image_num; ++i) {
    if (free_img_data) {
      HorizonVisionFreeImageFrame(smart->image_frame[i]);
    } else {
      HorizonVisionFreeImageFrameWithoutData(smart->image_frame[i]);
    }

    smart->image_frame[i] = nullptr;
  }
  if (smart->image_frame) {
    std::free(smart->image_frame);
    smart->image_frame = nullptr;
    smart->image_num = 0;
  }
  for (size_t i = 0; i < smart->smart_data_list_num; i++) {
    HorizonVisionCleanSmartData(&smart->smart_data_list[i]);
  }
  if (smart->smart_data_list) {
    std::free(smart->smart_data_list);
    smart->smart_data_list = nullptr;
  }
  std::free(smart);
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeSmartFrame(HorizonVisionSmartFrame *smart) {
  return FreeSmartFrame(smart, true);
}

int HorizonVisionFreeSmartFrameWithoutData(HorizonVisionSmartFrame *smart) {
  return FreeSmartFrame(smart, false);
}

int HorizonVisionAllocSnapshot(HorizonVisionSnapshot **psnaps, int num) {
  CHECK_NULL(psnaps)
  if (num == 0) {
    *psnaps = nullptr;
    return kHorizonVisionSuccess;
  }
  auto snapshots = static_cast<HorizonVisionSnapshot *>(
      std::calloc(num, sizeof(HorizonVisionSnapshot)));
  for (auto i = 0; i < num; ++i) {
    snapshots[i].smart_data = nullptr;
    snapshots[i].croped_image = nullptr;
  }
  *psnaps = snapshots;
  return kHorizonVisionSuccess;
}

int HorizonVisionCopySnapshot(HorizonVisionSnapshot *snapshot,
                              HorizonVisionSnapshot *new_snapshot) {
  CHECK_NULL(snapshot)
  CHECK_NULL(new_snapshot)
  *new_snapshot = *snapshot;
  if (snapshot->smart_data) {
    CHECK_SUCCESS(HorizonVisionDupSmartData(
        snapshot->smart_data, &new_snapshot->smart_data))
  }
  if (snapshot->croped_image) {
    CHECK_SUCCESS(HorizonVisionDupImage(
        snapshot->croped_image, true, &new_snapshot->croped_image))
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupSnapshot(HorizonVisionSnapshot *snapshot,
                             HorizonVisionSnapshot **pnew_snapshot) {
  CHECK_NULL(snapshot)
  CHECK_SUCCESS(HorizonVisionAllocSnapshot(pnew_snapshot, 1))
  CHECK_SUCCESS(HorizonVisionCopySnapshot(snapshot, *pnew_snapshot))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanSnapshot(HorizonVisionSnapshot *snap) {
  CHECK_NULL(snap)

  if (snap->smart_data) {
    HorizonVisionFreeSmartData(snap->smart_data, 1);
  }
  if (snap->croped_image) {
    HorizonVisionFreeImage(snap->croped_image);
  }

  return kHorizonVisionSuccess;
}

int HorizonVisionFreeSnapshot(HorizonVisionSnapshot *snap, int num) {
  CHECK_NULL(snap)
  for (auto i = 0; i < num; ++i) {
    HorizonVisionCleanSnapshot(&snap[i]);
  }
  std::free(snap);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocSnapshotTarget(HorizonVisionSnapshotTarget **ptargets,
                                     int num) {
  CHECK_NULL(ptargets)
  if (num == 0) {
    *ptargets = nullptr;
    return kHorizonVisionSuccess;
  }
  auto targets = static_cast<HorizonVisionSnapshotTarget *>(
      std::calloc(num, sizeof(HorizonVisionSnapshotTarget)));
  for (auto i = 0; i < num; ++i) {
    targets[i].snapshots_num = 0;
    targets[i].snapshots = nullptr;
  }
  *ptargets = targets;
  return kHorizonVisionSuccess;
}

int HorizonVisionCopySnapshotTarget(HorizonVisionSnapshotTarget *target,
                                    HorizonVisionSnapshotTarget *new_target) {
  CHECK_NULL(target)
  CHECK_NULL(new_target)
  *new_target = *target;
  CHECK_SUCCESS(HorizonVisionAllocSnapshot(
      &new_target->snapshots, new_target->snapshots_num))
  for (uint32_t i = 0; i < target->snapshots_num; i++) {
    auto &new_snapshot = new_target->snapshots[i];
    auto &old_snapshot = target->snapshots[i];
    CHECK_SUCCESS(HorizonVisionCopySnapshot(&old_snapshot, &new_snapshot))
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupSnapshotTarget(HorizonVisionSnapshotTarget *target,
                                   HorizonVisionSnapshotTarget **pnew_target) {
  CHECK_NULL(target)
  CHECK_SUCCESS(HorizonVisionAllocSnapshotTarget(pnew_target, 1))
  CHECK_SUCCESS(HorizonVisionCopySnapshotTarget(target, *pnew_target))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanSnapshotTarget(HorizonVisionSnapshotTarget *targets) {
  CHECK_NULL(targets)
  for (uint32_t i = 0; i < targets->snapshots_num; ++i) {
    HorizonVisionCleanSnapshot(&targets->snapshots[i]);
  }
  if (targets->snapshots) {
    std::free(targets->snapshots);
    targets->snapshots = nullptr;
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionFreeSnapshotTarget(HorizonVisionSnapshotTarget *targets,
                                    int num) {
  CHECK_NULL(targets)
  for (auto i = 0; i < num; ++i) {
    HorizonVisionCleanSnapshotTarget(&targets[i]);
  }
  std::free(targets);
  return kHorizonVisionSuccess;
}

int HorizonVisionAllocSnapshotFrame(HorizonVisionSnapshotFrame **psnapshots) {
  CHECK_NULL(psnapshots)
  *psnapshots = static_cast<HorizonVisionSnapshotFrame *>(
      std::calloc(1, sizeof(HorizonVisionSnapshotFrame)));
  (*psnapshots)->targets_num = 0;
  (*psnapshots)->targets = nullptr;
  return kHorizonVisionSuccess;
}

int HorizonVisionCopySnapshotFrame(HorizonVisionSnapshotFrame *snapshots,
                                   HorizonVisionSnapshotFrame *new_snapshots) {
  CHECK_NULL(snapshots)
  CHECK_NULL(new_snapshots)
  *new_snapshots = *snapshots;
  CHECK_SUCCESS(HorizonVisionAllocSnapshotTarget(
      &new_snapshots->targets, new_snapshots->targets_num))
  for (uint32_t i = 0; i < snapshots->targets_num; i++) {
    auto &old_target = snapshots->targets[i];
    auto &new_target = new_snapshots->targets[i];
    CHECK_SUCCESS(HorizonVisionCopySnapshotTarget(&old_target, &new_target))
  }
  return kHorizonVisionSuccess;
}

int HorizonVisionDupSnapshotFrame(HorizonVisionSnapshotFrame *snapshots,
                                  HorizonVisionSnapshotFrame **pnew_snapshots) {
  CHECK_NULL(snapshots)
  CHECK_SUCCESS(HorizonVisionAllocSnapshotFrame(pnew_snapshots))
  CHECK_SUCCESS(HorizonVisionCopySnapshotFrame(snapshots, *pnew_snapshots))
  return kHorizonVisionSuccess;
}

int HorizonVisionCleanSnapshotFrame(HorizonVisionSnapshotFrame *snapshots) {
  CHECK_NULL(snapshots)
  int ret = HorizonVisionFreeSnapshotTarget(
      snapshots->targets, snapshots->targets_num);
  return ret;
}

int HorizonVisionFreeSnapshotFrame(HorizonVisionSnapshotFrame *snapshots) {
  CHECK_NULL(snapshots)
  HorizonVisionCleanSnapshotFrame(snapshots);
  std::free(snapshots);
  return kHorizonVisionSuccess;
}
