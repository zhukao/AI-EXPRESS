/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     图像处理接口实现
 * @author    hangjun.yang
 * @email     hangjun.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.13
 */

#include "hobotxstream/image_tools.h"
#include "hobotxstream/imagetools/decoder.h"
#include "hobotxstream/imagetools/convertor.h"
#include "hobotxstream/imagetools/resizer.h"
#include "hobotxstream/imagetools/cropper.h"
#include "hobotxstream/imagetools/padding.h"
#include "hobotxstream/imagetools/rotater.h"
#include "horizon/vision_type/vision_type.hpp"

static void SetImageFrame(xstream::ImageToolsFormatData &frame,
                          const uint8_t *data,
                          const int data_size,
                          const int width,
                          const int height,
                          const int first_stride,
                          const int second_stride,
                          const HobotXStreamImageToolsPixelFormat format) {
  frame.array_type_ = xstream::FotmatDataArrayType::kContinueType;
  frame.data_[0] = const_cast<uint8_t *>(data);
  frame.data_size_[0] = data_size;
  frame.format_ = format;
  frame.width_ = width;
  frame.height_ = height;
  frame.first_stride_ = first_stride;
  frame.second_stride_ = second_stride;
}

static void SetYuvFrame(xstream::ImageToolsFormatData &frame,
                        const uint8_t *data[3],
                        const int data_size[3],
                        const int width,
                        const int height,
                        const int first_stride,
                        const int second_stride,
                        const HobotXStreamImageToolsPixelFormat format) {
  frame.array_type_ = xstream::FotmatDataArrayType::kContinueType;
  for (int i = 0; i < 3; ++i) {
    frame.data_[i] = const_cast<uint8_t *>(data[i]);
    frame.data_size_[i] = data_size[i];
  }
  frame.array_type_ = xstream::FotmatDataArrayType::kSeperateType;
  frame.format_ = format;
  frame.width_ = width;
  frame.height_ = height;
  frame.first_stride_ = first_stride;
  frame.second_stride_ = second_stride;
}

XSTREAM_API int HobotXStreamAllocImage(const int image_size,
                                 uint8_t** output_image) {
  if (nullptr == output_image || image_size <= 0) {
    return -1;
  }
  *output_image = static_cast<uint8_t *>(std::calloc(image_size,
                                                     sizeof(uint8_t)));
  if (nullptr == (*output_image)) {
    return -1;
  } else {
    return 0;
  }
}

XSTREAM_API int HobotXStreamFreeImage(const uint8_t* input) {
  if (nullptr != input) {
    std::free(const_cast<uint8_t*>(input));
  }
  return 0;
}

XSTREAM_API int HobotXStreamDecodeImage(\
                         const uint8_t *input,
                         const int input_size,
                         const HobotXStreamImageToolsPixelFormat dst_fmt,
                         uint8_t **output,
                         int *output_size,
                         int *width,
                         int *height,
                         int *first_stride,
                         int *second_stride) {
  if (nullptr == input
      || nullptr == output
      || nullptr == output_size
      || nullptr == width
      || nullptr == height
      || input_size <= 0
      || dst_fmt > IMAGE_TOOLS_RAW_YUV_I420) {
    return -1;
  }
  xstream::ImageDecoder decoder;
  xstream::ImageToolsFormatData decode_output;
  bool ret = decoder.Decode(input, input_size, dst_fmt, decode_output);
  if (!ret) {
    return -1;
  }
  *output = decode_output.data_[0];
  *output_size = decode_output.data_size_[0];
  *width = decode_output.width_;
  *height = decode_output.height_;
  if (first_stride != nullptr) {
    *first_stride = decode_output.first_stride_;
  }
  if (second_stride != nullptr) {
    *second_stride = decode_output.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamConvertImage(\
                      const uint8_t *input,
                      const int input_size,
                      const int width,
                      const int height,
                      const int input_first_stride,
                      const int input_second_stride,
                      const enum HobotXStreamImageToolsPixelFormat input_fmt,
                      const enum HobotXStreamImageToolsPixelFormat output_fmt,
                      uint8_t **output,
                      int *output_size,
                      int *output_first_stride,
                      int *output_second_stride) {
  if (nullptr == input
      || input_size <= 0
      || width <= 0
      || height <= 0
      || nullptr == output
      || nullptr == output_size) {
    return -1;
  }
  xstream::ImageConvertor convertor;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetImageFrame(input_frame,
                input,
                input_size,
                width,
                height,
                input_first_stride,
                input_second_stride,
                input_fmt);
  bool ret = convertor.Convert(input_frame, output_fmt, output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamConvertYuvImage(\
                      const uint8_t *input_yuv_data[3],
                      const int input_yuv_size[3],
                      const int width,
                      const int height,
                      const int input_first_stride,
                      const int input_second_stride,
                      const enum HobotXStreamImageToolsPixelFormat input_fmt,
                      const enum HobotXStreamImageToolsPixelFormat output_fmt,
                      uint8_t **output,
                      int *output_size,
                      int *output_first_stride,
                      int *output_second_stride) {
  if (width <= 0
      || height <= 0
      || nullptr == output
      || nullptr == output_size) {
    return -1;
  }
  int yuv_max_index = 0;
  if (IMAGE_TOOLS_RAW_YUV_I420 == input_fmt) {
    yuv_max_index = 3;
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == input_fmt
             || IMAGE_TOOLS_RAW_YUV_NV12 == input_fmt) {
    yuv_max_index = 2;
  } else {
    return -1;
  }
  for (int i = 0; i < yuv_max_index; ++i) {
    if (input_yuv_data[i] == nullptr || input_yuv_size[i] <= 0) {
      return -1;
    }
  }
  xstream::ImageConvertor convertor;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetYuvFrame(input_frame,
              input_yuv_data,
              input_yuv_size,
              width,
              height,
              input_first_stride,
              input_second_stride,
              input_fmt);

  bool ret = convertor.Convert(input_frame, output_fmt, output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamResizeImage(\
                         const uint8_t *input,
                         const int input_size,
                         const int input_width,
                         const int input_height,
                         const int input_first_stride,
                         const int input_second_stride,
                         const enum HobotXStreamImageToolsPixelFormat format,
                         const int fix_aspect_ratio,
                         const int dst_width,
                         const int dst_height,
                         uint8_t **output,
                         int *output_size,
                         int *output_first_stride,
                         int *output_second_stride,
                         struct HobotXStreamImageToolsResizeInfo *resize_info) {
  if (nullptr == input
      || input_size <= 0
      || input_width <= 0
      || input_height <= 0
      || dst_width <= 0
      || dst_height <= 0
      || nullptr == output
      || nullptr == output_size) {
    return -1;
  }
  xstream::ImageResizer resizer;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetImageFrame(input_frame,
                input,
                input_size,
                input_width,
                input_height,
                input_first_stride,
                input_second_stride,
                format);
  bool ret = resizer.Resize(input_frame,
                            dst_width,
                            dst_height,
                            fix_aspect_ratio,
                            output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  if (resize_info != nullptr) {
    *resize_info = resizer.GetResizeInfo();
  }
  return 0;
}

XSTREAM_API int HobotXStreamResizeImageByRatio(\
                         const uint8_t *input,
                         const int input_size,
                         const int input_width,
                         const int input_height,
                         const int input_first_stride,
                         const int input_second_stride,
                         const enum HobotXStreamImageToolsPixelFormat format,
                         const int fix_aspect_ratio,
                         const float width_ratio,
                         const float height_ratio,
                         uint8_t **output,
                         int *output_size,
                         int *output_first_stride,
                         int *output_second_stride,
                         struct HobotXStreamImageToolsResizeInfo *resize_info) {
  int dst_width = 0;
  int dst_height = 0;
  if (!fix_aspect_ratio) {
    dst_width = input_width * width_ratio + 0.5;
    dst_height = input_height * height_ratio + 0.5;
  } else {
    // 正常情况，应该是width_ratio = height_ratio
    // 不等时，选择缩放因子大的缩放
    float ratio = width_ratio;
    if (ratio < height_ratio) {
      ratio = height_ratio;
    }
    dst_width = input_width * ratio + 0.5;
    dst_height = input_height * ratio + 0.5;
  }
  if (IMAGE_TOOLS_RAW_YUV_NV21 == format
     || IMAGE_TOOLS_RAW_YUV_NV12 == format
     || IMAGE_TOOLS_RAW_YUV_I420 == format) {
    dst_width += (dst_width & 1);
    dst_height += (dst_height & 1);
  }

  return HobotXStreamResizeImage(input,
                              input_size,
                              input_width,
                              input_height,
                              input_first_stride,
                              input_second_stride,
                              format,
                              fix_aspect_ratio,
                              dst_width,
                              dst_height,
                              output,
                              output_size,
                              output_first_stride,
                              output_second_stride,
                              resize_info);
}

XSTREAM_API int HobotXStreamCropImage(\
                       const uint8_t *input,
                       const int input_size,
                       const int input_width,
                       const int input_height,
                       const int input_first_stride,
                       const int input_second_stride,
                       const enum HobotXStreamImageToolsPixelFormat format,
                       const int top_left_x,
                       const int top_left_y,
                       const int bottom_right_x,
                       const int bottom_right_y,
                       uint8_t **output,
                       int *output_size,
                       int *output_width,
                       int *output_height,
                       int *output_first_stride,
                       int *output_second_stride) {
  if (nullptr == input
      || input_size <= 0
      || input_width <= 0
      || input_height <= 0
      || top_left_x < 0
      || top_left_y < 0
      || bottom_right_x < 0
      || bottom_right_y < 0
      || nullptr == output
      || nullptr == output_size
      || nullptr == output_width
      || nullptr == output_height) {
    return -1;
  }
  xstream::ImageCropper cropper;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetImageFrame(input_frame,
                input,
                input_size,
                input_width,
                input_height,
                input_first_stride,
                input_second_stride,
                format);
  bool ret = cropper.Crop(input_frame,
                          top_left_x,
                          top_left_y,
                          bottom_right_x,
                          bottom_right_y,
                          output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  *output_width = output_frame.width_;
  *output_height = output_frame.height_;
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamCropYUVImage(\
                       const uint8_t *input_yuv_data[3],
                       const int input_yuv_size[3],
                       const int input_width,
                       const int input_height,
                       const int input_first_stride,
                       const int input_second_stride,
                       const enum HobotXStreamImageToolsPixelFormat format,
                       const int top_left_x,
                       const int top_left_y,
                       const int bottom_right_x,
                       const int bottom_right_y,
                       uint8_t **output,
                       int *output_size,
                       int *output_width,
                       int *output_height,
                       int *output_first_stride,
                       int *output_second_stride) {
  if (input_width <= 0
      || input_height <= 0
      || top_left_x < 0
      || top_left_y < 0
      || bottom_right_x < 0
      || bottom_right_y < 0
      || nullptr == output
      || nullptr == output_size
      || nullptr == output_width
      || nullptr == output_height) {
    return -1;
  }
  int yuv_max_index = 0;
  if (IMAGE_TOOLS_RAW_YUV_I420 == format) {
    yuv_max_index = 3;
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == format
             || IMAGE_TOOLS_RAW_YUV_NV12 == format) {
    yuv_max_index = 2;
  } else {
    return -1;
  }
  for (int i = 0; i < yuv_max_index; ++i) {
    if (input_yuv_data[i] == nullptr || input_yuv_size[i] <= 0) {
      return -1;
    }
  }
  xstream::ImageCropper cropper;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetYuvFrame(input_frame,
              input_yuv_data,
              input_yuv_size,
              input_width,
              input_height,
              input_first_stride,
              input_second_stride,
              format);
  bool ret = cropper.Crop(input_frame,
                          top_left_x,
                          top_left_y,
                          bottom_right_x,
                          bottom_right_y,
                          output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  *output_width = output_frame.width_;
  *output_height = output_frame.height_;
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamCropImageWithPaddingBlack(\
                       const uint8_t *input,
                       const int input_size,
                       const int input_width,
                       const int input_height,
                       const int input_first_stride,
                       const int input_second_stride,
                       const enum HobotXStreamImageToolsPixelFormat format,
                       const int top_left_x,
                       const int top_left_y,
                       const int bottom_right_x,
                       const int bottom_right_y,
                       uint8_t **output,
                       int *output_size,
                       int *output_width,
                       int *output_height,
                       int *output_first_stride,
                       int *output_second_stride) {
  if (nullptr == input
      || input_size <= 0
      || input_width <= 0
      || input_height <= 0
      || bottom_right_x < top_left_x
      || bottom_right_y < top_left_y
      || nullptr == output
      || nullptr == output_size
      || nullptr == output_width
      || nullptr == output_height) {
    return -1;
  }
  xstream::ImageCropper cropper;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetImageFrame(input_frame,
                input,
                input_size,
                input_width,
                input_height,
                input_first_stride,
                input_second_stride,
                format);
  bool ret = cropper.CropWithPadBlack(input_frame,
                                      top_left_x,
                                      top_left_y,
                                      bottom_right_x,
                                      bottom_right_y,
                                      output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  *output_width = output_frame.width_;
  *output_height = output_frame.height_;
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamCropYuvImageWithPaddingBlack(\
                       const uint8_t *input_yuv_data[3],
                       const int input_yuv_size[3],
                       const int input_width,
                       const int input_height,
                       const int input_first_stride,
                       const int input_second_stride,
                       const enum HobotXStreamImageToolsPixelFormat format,
                       const int top_left_x,
                       const int top_left_y,
                       const int bottom_right_x,
                       const int bottom_right_y,
                       uint8_t **output,
                       int *output_size,
                       int *output_width,
                       int *output_height,
                       int *output_first_stride,
                       int *output_second_stride) {
  if (input_width <= 0
      || input_height <= 0
      || bottom_right_x < top_left_x
      || bottom_right_y < top_left_y
      || nullptr == output
      || nullptr == output_size
      || nullptr == output_width
      || nullptr == output_height) {
    return -1;
  }
  int yuv_max_index = 0;
  if (IMAGE_TOOLS_RAW_YUV_I420 == format) {
    yuv_max_index = 3;
  } else if (IMAGE_TOOLS_RAW_YUV_NV21 == format
             || IMAGE_TOOLS_RAW_YUV_NV12 == format) {
    yuv_max_index = 2;
  } else {
    return -1;
  }
  for (int i = 0; i < yuv_max_index; ++i) {
    if (input_yuv_data[i] == nullptr || input_yuv_size[i] <= 0) {
      return -1;
    }
  }
  xstream::ImageCropper cropper;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetYuvFrame(input_frame,
              input_yuv_data,
              input_yuv_size,
              input_width,
              input_height,
              input_first_stride,
              input_second_stride,
              format);
  bool ret = cropper.CropWithPadBlack(input_frame,
                                      top_left_x,
                                      top_left_y,
                                      bottom_right_x,
                                      bottom_right_y,
                                      output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  *output_width = output_frame.width_;
  *output_height = output_frame.height_;
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamCropImageFrameWithPaddingBlack(\
                       void *input,
                       const int top_left_x,
                       const int top_left_y,
                       const int bottom_right_x,
                       const int bottom_right_y,
                       enum HobotXStreamImageToolsPixelFormat *output_format,
                       uint8_t **output,
                       int *output_size,
                       int *output_width,
                       int *output_height,
                       int *output_first_stride,
                       int *output_second_stride) {
  if (nullptr == input
      || bottom_right_x < top_left_x
      || bottom_right_y < top_left_y
      || nullptr == output_format
      || nullptr == output
      || nullptr == output_size
      || nullptr == output_width
      || nullptr == output_height) {
    return -1;
  }

  hobot::vision::ImageFrame *image_frame =
              static_cast<hobot::vision::ImageFrame *>(input);
  HobotXStreamImageToolsPixelFormat input_format = IMAGE_TOOLS_RAW_NONE;
  xstream::FotmatDataArrayType data_array_type =
              xstream::FotmatDataArrayType::kContinueType;
  switch (image_frame->pixel_format) {
    case kHorizonVisionPixelFormatRawRGB: {
      input_format = IMAGE_TOOLS_RAW_RGB;
      break;
    }
    case kHorizonVisionPixelFormatRawBGR: {
      input_format = IMAGE_TOOLS_RAW_BGR;
      break;
    }
    case kHorizonVisionPixelFormatRawGRAY: {
      input_format = IMAGE_TOOLS_RAW_GRAY;
      break;
    }
    case kHorizonVisionPixelFormatRawNV21: {
      input_format = IMAGE_TOOLS_RAW_YUV_NV21;
      break;
    }
    case kHorizonVisionPixelFormatRawNV12: {
      input_format = IMAGE_TOOLS_RAW_YUV_NV12;
      break;
    }
    case kHorizonVisionPixelFormatRawI420: {
      input_format = IMAGE_TOOLS_RAW_YUV_I420;
      break;
    }
    case kHorizonVisionPixelFormatX2PYM: {
#if defined(X2)
      input_format = IMAGE_TOOLS_RAW_YUV_NV12;
      data_array_type = xstream::FotmatDataArrayType::kSeperateType;
      break;
#else
      return -1;
#endif
    }
    case kHorizonVisionPixelFormatX2SRC: {
#if defined(X2)
      input_format = IMAGE_TOOLS_RAW_YUV_NV12;
      data_array_type = xstream::FotmatDataArrayType::kSeperateType;
      break;
#else
      return -1;
#endif
    }
    case kHorizonVisionPixelFormatX3PYM: {
#if defined(X3)
    input_format = IMAGE_TOOLS_RAW_YUV_NV12;
    data_array_type = xstream::FotmatDataArrayType::kSeperateType;
    break;
#else
      return -1;
#endif
    }
    case kHorizonVisionPixelFormatX3SRC: {
#if defined(X3)
      input_format = IMAGE_TOOLS_RAW_YUV_NV12;
      data_array_type = xstream::FotmatDataArrayType::kSeperateType;
      break;
#else
      return -1;
#endif
    }
    case kHorizonVisionPixelFormatPYM: {
      input_format = IMAGE_TOOLS_RAW_YUV_NV12;
      data_array_type = xstream::FotmatDataArrayType::kSeperateType;
      break;
    }
    default:
      return -1;
  }
  *output_format = input_format;
  if (xstream::FotmatDataArrayType::kContinueType == data_array_type) {
    return HobotXStreamCropImageWithPaddingBlack(
                        reinterpret_cast<uint8_t *>(image_frame->Data()),
                        static_cast<int>(image_frame->DataSize()),
                        static_cast<int>(image_frame->Width()),
                        static_cast<int>(image_frame->Height()),
                        static_cast<int>(image_frame->Stride()),
                        static_cast<int>(image_frame->StrideUV()),
                        input_format,
                        top_left_x,
                        top_left_y,
                        bottom_right_x,
                        bottom_right_y,
                        output,
                        output_size,
                        output_width,
                        output_height,
                        output_first_stride,
                        output_second_stride);
  } else {
    const uint8_t *input_yuv_data[3] = {
        reinterpret_cast<uint8_t *>(image_frame->Data()),
        reinterpret_cast<uint8_t *>(image_frame->DataUV()), 0};
    const int input_yuv_size[3] = {\
                static_cast<int>(image_frame->DataSize()),
                static_cast<int>(image_frame->DataUVSize()),
                0};
    return HobotXStreamCropYuvImageWithPaddingBlack(\
                        input_yuv_data,
                        input_yuv_size,
                        static_cast<int>(image_frame->Width()),
                        static_cast<int>(image_frame->Height()),
                        static_cast<int>(image_frame->Stride()),
                        static_cast<int>(image_frame->StrideUV()),
                        input_format,
                        top_left_x,
                        top_left_y,
                        bottom_right_x,
                        bottom_right_y,
                        output,
                        output_size,
                        output_width,
                        output_height,
                        output_first_stride,
                        output_second_stride);
  }
  return -1;
}

XSTREAM_API int HobotXStreamPadImage(\
                      const uint8_t *input,
                      const int input_size,
                      const int input_width,
                      const int input_height,
                      const int input_first_stride,
                      const int input_second_stride,
                      const enum HobotXStreamImageToolsPixelFormat format,
                      const int padding_left_width,
                      const int padding_right_width,
                      const int padding_top_height,
                      const int padding_bottom_height,
                      const uint8_t padding_value[3],
                      uint8_t **output,
                      int *output_size,
                      int *output_width,
                      int *output_height,
                      int *output_first_stride,
                      int *output_second_stride) {
    if (nullptr == input
      || input_size <= 0
      || input_width <= 0
      || input_height <= 0
      || padding_left_width < 0
      || padding_right_width < 0
      || padding_top_height < 0
      || nullptr == padding_value
      || nullptr == output
      || nullptr == output_size
      || nullptr == output_width
      || nullptr == output_height) {
    return -1;
  }
  xstream::ImagePadding padding;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetImageFrame(input_frame,
                input,
                input_size,
                input_width,
                input_height,
                input_first_stride,
                input_second_stride,
                format);
  bool ret = padding.Pad(input_frame,
                         padding_left_width,
                         padding_right_width,
                         padding_top_height,
                         padding_bottom_height,
                         padding_value,
                         output_frame);
  if (!ret) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  *output_width = output_frame.width_;
  *output_height = output_frame.height_;
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }
  return 0;
}

XSTREAM_API int HobotXStreamRotateImage(
                         const uint8_t *input,
                         const int input_size,
                         const int input_width,
                         const int input_height,
                         const int input_first_stride,
                         const int input_second_stride,
                         const enum HobotXStreamImageToolsPixelFormat format,
                         const int degree,
                         uint8_t **output,
                         int *output_size,
                         int *output_width,
                         int *output_height,
                         int *output_first_stride,
                         int *output_second_stride) {
  if (!input || input_size <= 0 || input_width <= 0 || input_height <= 0 ||
      input_first_stride < 0 || input_second_stride < 0 || !output ||
      !output_size || !output_width || !output_height) {
    return -1;
  }

  xstream::ImageRotater rotater;
  xstream::ImageToolsFormatData input_frame;
  xstream::ImageToolsFormatData output_frame;
  SetImageFrame(input_frame,
                input,
                input_size,
                input_width,
                input_height,
                input_first_stride,
                input_second_stride,
                format);
  if (!rotater.Rotate(input_frame, degree, output_frame)) {
    return -1;
  }
  *output = output_frame.data_[0];
  *output_size = output_frame.data_size_[0];
  *output_width = output_frame.width_;
  *output_height = output_frame.height_;
  if (output_first_stride != nullptr) {
    *output_first_stride = output_frame.first_stride_;
  }
  if (output_second_stride != nullptr) {
    *output_second_stride = output_frame.second_stride_;
  }

  return 0;
}
