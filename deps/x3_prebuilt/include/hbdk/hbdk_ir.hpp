/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2016-2019 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/

#ifndef HBDK_IR_HPP_
#define HBDK_IR_HPP_
#pragma once

#include <hbdk_march.h>
#include <hbdk_type.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace hbdk {
namespace hbir {

using Coord = std::vector<int32_t>;           // for tensor ROI coord
using Shape = std::vector<uint32_t>;          // for tensor dimension, ROI size
using Shape2D = std::pair<int32_t, int32_t>;  // for stride, padding, etc

struct int1_t {
  int8_t v = 0;
};
struct int2_t {
  int8_t v = 0;
};
struct int4_t {
  int8_t v = 0;
};
struct uint1_t {
  uint8_t v = 0;
};
struct uint2_t {
  uint8_t v = 0;
};
struct uint4_t {
  uint8_t v = 0;
};

template <typename T>
struct ElementType;

template <typename T>
void CheckVersion(const T *, std::uint32_t version) {
  if (version != cereal::detail::Version<T>::version) {
    std::cerr << "The class " << cereal::detail::binding_name<T>::name() << " version is "
              << cereal::detail::Version<T>::version << ", but the loaded version is " << version << std::endl;
    std::abort();
  }
}

template <typename T>
void AbortOnVersion(const T *, std::uint32_t version) {
  if (version != cereal::detail::Version<T>::version) {
    std::cerr << "The class " << cereal::detail::binding_name<T>::name() << " with version "
              << cereal::detail::Version<T>::version << " cannot deserialize incompatible version " << version
              << std::endl;
  } else {
    std::cerr << "The latest version has not been processed, serialize error in "
              << cereal::detail::binding_name<T>::name() << std::endl;
  }
  std::abort();
}

/**
 * Tensor used by Layer inputs and outputs
 */
struct Tensor {
 public:
  enum class element_type_t {
    UNKNOWN,
    S8,
    S16,
    S32,
    S64,
    U8,
    U16,
    U32,
    U64,
    F32,
    F64,
    S1,
    S2,
    S4,
    U1,
    U2,
    U4,
  };

  Tensor() = default;
  Tensor(const Tensor &) = default;
  virtual ~Tensor() noexcept = default;
  /**
   * Construct a Tensor,
   * different type tensor may has different shape dimension, element type, number of shift value,
   * see Layer description
   *
   * @param name tensor name (globally unique)
   * @param shape tensor shape
   * @param element_type
   * @param shift tensor data shift values
   * @param is_model_output mark the tensor as model output
   */
  Tensor(std::string name, Shape shape, element_type_t element_type = element_type_t::S8,
         std::vector<int32_t> shift = {}, bool is_model_output = false)
      : name(std::move(name)),
        shape(std::move(shape)),
        shift(std::move(shift)),
        is_model_output(is_model_output),
        element_type(element_type) {}

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(CEREAL_NVP(name), CEREAL_NVP(shape), CEREAL_NVP(element_type), CEREAL_NVP(shift), CEREAL_NVP(is_model_output),
         CEREAL_NVP(data));
    } else if (version == 2) {
      ar(CEREAL_NVP(name), CEREAL_NVP(shape), CEREAL_NVP(element_type), CEREAL_NVP(shift), CEREAL_NVP(scale),
         CEREAL_NVP(is_model_output), CEREAL_NVP(data));
    } else {
      AbortOnVersion(this, version);
    }
  }

 public:
  std::string name;
  Shape shape;
  std::vector<int32_t> shift;
  std::vector<float> scale;
  bool is_model_output = false;

  // ------------------------------------------------
  // ---------- tensor data & element type ----------

  element_type_t GetElementType() const { return element_type; }
  void SetElementType(element_type_t type) { element_type = type; }

  template <typename T>
  void SetData(const T *ptr, size_t size) {
    if (ElementType<T>::value != element_type) {
      std::cerr << "cannot set data with element type " << static_cast<uint32_t>(ElementType<T>::value) << " to tensor "
                << name << " with element type " << static_cast<uint32_t>(element_type) << std::endl;
    }
    data.assign(reinterpret_cast<const char *>(ptr), reinterpret_cast<const char *>(ptr) + size * sizeof(T));
  }

  template <typename T>
  uint32_t GetDataSize() const {
    if (ElementType<T>::value != element_type) {
      std::cerr << "cannot get size of data with element type " << static_cast<uint32_t>(ElementType<T>::value)
                << " from tensor " << name << " with element type " << static_cast<uint32_t>(element_type) << std::endl;
    }
    return data.size() / sizeof(T);
  }

  template <typename T>
  const T *GetData() const {
    if (ElementType<T>::value != element_type) {
      std::cerr << "cannot get data with element type " << static_cast<uint32_t>(ElementType<T>::value)
                << " from tensor " << name << " with element type " << static_cast<uint32_t>(element_type) << std::endl;
    }
    return reinterpret_cast<const T *>(data.data());
  }

  bool HasData() const { return !data.empty(); }

 private:
  element_type_t element_type = element_type_t::UNKNOWN;
  std::vector<char> data;

  // ---------- tensor data & element type ----------
  // ------------------------------------------------
};

/**
 * Layer used to describe the model
 */
struct Layer {
 public:
  enum class layer_type_t {
    UNKNOWN,
    QUANTI_INPUT,
    CONVOLUTION,
    POOLING_AVG,
    POOLING_MAX,
    POOLING_AVG_GLOBAL,
    POOLING_MAX_GLOBAL,
    RELU,
    ELEMENTWISE_MUL,
    ELEMENTWISE_ADD,
    ELEMENTWISE_MULADD,  // deprecated.
    SPLIT,
    CONCAT,
    CHANNEL_MAX,
    SOFTMAX,
    WARPING,
    DETECTION_POST_PROCESS,
    RCNN_POST_PROCESS,
    ROI_ALIGN,
    ROI_RESIZE,
    LUT,
    CHANNEL_SUM,
    FILTER,
    SLICE,
    ELEMENTWISE_SUB,
    RESHAPE,
    QUANTI_FLATTEN,
    DECONVOLUTION,
    BPUCONVOLUTION,
    BPUDECONVOLUTION,
    SCONVOLUTION,
    SCALE_RELU,  // represent Leaky ReLU, PReLU, RReLU
    LINEAR_POLYNOMIAL,
    RELU_X,
    NEAREST_UPSAMPLE,
    SHUFFLE,
    SDECONVOLUTION,
    SELEMENTWISE_ADD,  // elementwise add two tensor that quantized by scale
    SQUANTI_INPUT,
    OPTICAL_PYRAMID,
    PAD,
    CORRELATION,
    SELEMENTWISE_MUL,  // elementwise mul two tensor that quantized by scale
    STEPWISE_FIT,
    CONVERT_BETWEEN_INT8_AND_UINT8,
    DILATE,
    ELEMENTWISE_MIN,
    ELEMENTWISE_MAX,
  };

  Layer() = default;
  virtual ~Layer() noexcept = default;

  /**
   * Layer base, cannot be constructed
   *
   * @param name unique layer name
   * @param input_tensors input tensors of the layer
   * @param output_tensors output tensors of the layer
   */
  Layer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
        std::vector<std::shared_ptr<Tensor>> output_tensors)
      : name(std::move(name)), input_tensors(std::move(input_tensors)), output_tensors(std::move(output_tensors)) {}
  virtual layer_type_t GetLayerType() const = 0;

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(CEREAL_NVP(name), CEREAL_NVP(input_tensors), CEREAL_NVP(output_tensors), CEREAL_NVP(desc));
    } else {
      AbortOnVersion(this, version);
    }
  }

  std::string name;
  std::vector<std::shared_ptr<Tensor>> input_tensors;
  std::vector<std::shared_ptr<Tensor>> output_tensors;
  std::string desc;  // user-defined information, e.g. how to use the output of this layer.
  // the compiler does not parse it
};

/**
 * Quantized Input layer
 *
 * has 1 input (fake) and 1 output (int8).
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 *
 * NOTE: the input and output should has same shape.
 *       the compiler just use output as model input, the input will be ignored.
 *
 */
struct QuantiInputLayer : public Layer {
 public:
  using Layer::Layer;
  ~QuantiInputLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::QUANTI_INPUT; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Similar to QUANTI_INPUT
 *
 * different quantization method
 *
 */
struct SQuantiInputLayer : public Layer {
 public:
  using Layer::Layer;
  ~SQuantiInputLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SQUANTI_INPUT; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Convolution Layer
 *
 * inputs [feature, weight, bias, sumin]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 * 2. weight shape is [N, H, W, C], has 1 or N shift values
 * 3. bias shape is [N], has 1 or N shift values, can be nullptr
 * 4. sumin shape is [N, H, W, C], has 1 or C shift values, can be nullptr
 *
 * output
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 *
 * output = feature * weight + bias + sumin
 */
struct ConvolutionLayer : public Layer {
 public:
  ConvolutionLayer() = default;  // for windows build
  ConvolutionLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                   std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D stride, Shape2D padding)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        stride(std::move(stride)),
        padding(std::move(padding)) {}
  ~ConvolutionLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::CONVOLUTION; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(align_output_shift));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(dilation),
         CEREAL_NVP(align_output_shift));
    } else if (version == 4) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(dilation),
         CEREAL_NVP(align_output_shift), CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }

  Shape2D stride;                   // [H, W]
  Shape2D padding;                  // [H, W]
  Shape2D dilation = {1, 1};        // [H, W]
  bool align_output_shift = false;  // deprecated, align_output_shift = true is unsupported.
  bool enable_rounding = false;
};

/**
 * SAlphaPlusConvolution Layer
 *
 * inputs [feature, weight, bias, sumin]
 * 1. feature shape is [N, H, W, C], no shift values
 * 2. weight shape is [N, H, W, C], no shift values, int8
 * 3. bias shape is [N], no shift values, can be nullptr
 * 4. sumin shape is [N, H, W, C], no shift values, can be nullptr
 *
 * output
 * 1. output shape is [N, H, W, C], no shift values
 *
 * quanti: output = (((feature * weight + (bias << bias_shift) + (sumin << sumin_shift) * sumin_scale) >> accu_shift) *
 * scale) >> output_shift
 *
 * unquanti: output = feature * weight + (bias << bias_shift) + (sumin << sumin_shift) * sumin_scale
 */
struct SConvolutionLayer : public Layer {
 public:
  SConvolutionLayer() = default;
  SConvolutionLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                    std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D stride, Shape2D padding)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        stride(std::move(stride)),
        padding(std::move(padding)) {}
  SConvolutionLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                    std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D stride, Shape2D padding,
                    std::vector<int8_t> bias_left_shift, std::vector<int32_t> output_scale,
                    std::vector<int8_t> accu_right_shift, std::vector<int8_t> output_right_shift,
                    std::vector<int8_t> sumin_left_shift, std::vector<int32_t> sumin_scale)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        stride(std::move(stride)),
        padding(std::move(padding)),
        bias_left_shift(std::move(bias_left_shift)),
        output_scale(std::move(output_scale)),
        accu_right_shift(std::move(accu_right_shift)),
        output_right_shift(std::move(output_right_shift)),
        sumin_left_shift(std::move(sumin_left_shift)),
        sumin_scale(std::move(sumin_scale)) {}
  ~SConvolutionLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SCONVOLUTION; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(bias_left_shift),
         CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift), CEREAL_NVP(output_right_shift),
         CEREAL_NVP(sumin_left_shift), CEREAL_NVP(sumin_scale));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(dilation),
         CEREAL_NVP(bias_left_shift), CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift),
         CEREAL_NVP(output_right_shift), CEREAL_NVP(sumin_left_shift), CEREAL_NVP(sumin_scale));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(dilation),
         CEREAL_NVP(bias_left_shift), CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift),
         CEREAL_NVP(output_right_shift), CEREAL_NVP(sumin_left_shift), CEREAL_NVP(sumin_scale),
         CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }

  Shape2D stride;             // [H, W]
  Shape2D padding;            // [H, W]
  Shape2D dilation = {1, 1};  // [H, W]

  std::vector<int8_t> bias_left_shift;
  std::vector<int32_t> output_scale;
  std::vector<int8_t> accu_right_shift;
  std::vector<int8_t> output_right_shift;
  std::vector<int8_t> sumin_left_shift;
  std::vector<int32_t> sumin_scale;

  bool enable_rounding = false;
};

/**
 * Deconvolution Layer
 *
 * inputs [feature, weight, bias, sumin]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 * 2. weight shape is [N, H, W, C], has 1 or N shift values
 * 3. bias shape is [N], has 1 or N shift values, can be nullptr
 * 4. sumin is at present not supported
 *
 * output
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 *
 * output = feature * weight + bias + sumin
 */
struct DeconvolutionLayer : public Layer {
 public:
  DeconvolutionLayer() = default;
  DeconvolutionLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                     std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D stride, Shape2D padding,
                     Shape2D output_padding)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        stride(std::move(stride)),
        padding(std::move(padding)),
        output_padding(std::move(output_padding)) {}
  ~DeconvolutionLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::DECONVOLUTION; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(output_padding));
    } else {
      AbortOnVersion(this, version);
    }
  }
  Shape2D stride;
  Shape2D padding;
  Shape2D output_padding;
};

struct SDeconvolutionLayer : public Layer {
 public:
  SDeconvolutionLayer() = default;
  SDeconvolutionLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                      std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D stride, Shape2D padding,
                      Shape2D output_padding)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        stride(std::move(stride)),
        padding(std::move(padding)),
        output_padding(std::move(output_padding)) {}

  SDeconvolutionLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                      std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D stride, Shape2D padding,
                      Shape2D output_padding, std::vector<int8_t> bias_left_shift, std::vector<int32_t> output_scale,
                      std::vector<int8_t> accu_right_shift, std::vector<int8_t> output_right_shift,
                      std::vector<int8_t> sumin_left_shift, std::vector<int32_t> sumin_scale)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        stride(std::move(stride)),
        padding(std::move(padding)),
        output_padding(std::move(output_padding)),
        bias_left_shift(std::move(bias_left_shift)),
        output_scale(std::move(output_scale)),
        accu_right_shift(std::move(accu_right_shift)),
        output_right_shift(std::move(output_right_shift)),
        sumin_left_shift(std::move(sumin_left_shift)),
        sumin_scale(std::move(sumin_scale)) {}
  ~SDeconvolutionLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SDECONVOLUTION; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(stride), CEREAL_NVP(padding), CEREAL_NVP(output_padding),
         CEREAL_NVP(bias_left_shift), CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift),
         CEREAL_NVP(output_right_shift), CEREAL_NVP(sumin_left_shift), CEREAL_NVP(sumin_scale));
    } else {
      AbortOnVersion(this, version);
    }
  }

  Shape2D stride;
  Shape2D padding;
  Shape2D output_padding;

  std::vector<int8_t> bias_left_shift;
  std::vector<int32_t> output_scale;
  std::vector<int8_t> accu_right_shift;
  std::vector<int8_t> output_right_shift;
  std::vector<int8_t> sumin_left_shift;
  std::vector<int32_t> sumin_scale;
};

/**
 * Average Pooling Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct AvgPoolingLayer : public Layer {
 public:
  AvgPoolingLayer() = default;
  AvgPoolingLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                  std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D kernel, Shape2D stride, Shape2D padding)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        kernel(kernel),
        stride(stride),
        padding(padding) {}
  ~AvgPoolingLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::POOLING_AVG; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(kernel), CEREAL_NVP(stride), CEREAL_NVP(padding),
         CEREAL_NVP(result_right_shift));
    } else {
      AbortOnVersion(this, version);
    }
  }

  Shape2D kernel;                   // [H, W]
  Shape2D stride;                   // [H, W]
  Shape2D padding;                  // [H, W]
  uint32_t result_right_shift = 9;  // result sum will right shift this
};

/**
 * Max Pooling Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct MaxPoolingLayer : public Layer {
 public:
  MaxPoolingLayer() = default;
  MaxPoolingLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                  std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D kernel, Shape2D stride, Shape2D padding)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        kernel(kernel),
        stride(stride),
        padding(padding) {}
  ~MaxPoolingLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::POOLING_MAX; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(kernel), CEREAL_NVP(stride), CEREAL_NVP(padding),
         CEREAL_NVP(result_right_shift));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(kernel), CEREAL_NVP(stride), CEREAL_NVP(padding),
         CEREAL_NVP(result_right_shift), CEREAL_NVP(pad_mode));
    } else {
      AbortOnVersion(this, version);
    }
  }

  Shape2D kernel;
  Shape2D stride;
  Shape2D padding;
  uint32_t result_right_shift = 9;  // result sum will right shift this
  enum class PadMode { CONSTANT, BOUNDARY } pad_mode = PadMode::CONSTANT;
};

/**
 * Global Average Pooling Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct GlobalAvgPoolingLayer : public Layer {
 public:
  using Layer::Layer;
  ~GlobalAvgPoolingLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::POOLING_AVG_GLOBAL; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(result_right_shift), CEREAL_NVP(sum_right_shift));
    } else {
      AbortOnVersion(this, version);
    }
  }

  uint32_t result_right_shift = 9;  // result sum will right shift this
  uint32_t sum_right_shift = 0;     // partial sum will right shift this
};

/**
 * Global Max Pooling Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct GlobalMaxPoolingLayer : public Layer {
 public:
  using Layer::Layer;
  ~GlobalMaxPoolingLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::POOLING_MAX_GLOBAL; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

///**
// * Upscaling Layer
// *
// * input [feature]
// * 1. feature shape is [N, H, W, C], has 1 or C shift values
// *
// * output [output]
// * 1. output shape is [N, H, W, C], has 1 or C shift values
// */
// struct UpscalingLayer : public Layer {
//  using Layer::Layer;
//  ~UpscalingLayer() noexcept override = default;
//  layer_type_t GetLayerType() const override { return layer_type_t::UPSCALING; }
//
//  template <class Archive>
//  void serialize(Archive &ar, std::int32_t const version) {
//    CheckVersion(this, version);
//    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
//    ar(cereal::base_class<Layer>(this));
//  }
//};

/**
 * RoiResize Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 */
struct RoiResizeLayer : public Layer {
  using Layer::Layer;
  struct Roi;
  enum class PadMode;
  enum class AlignMode;
  RoiResizeLayer() = default;
  RoiResizeLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                 std::vector<std::shared_ptr<Tensor>> output_tensors, RoiResizeLayer::Roi roi,
                 RoiResizeLayer::PadMode pad_mode, AlignMode align_mode)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        roi_(roi),
        pad_mode_(pad_mode),
        align_mode_(align_mode) {
    if (roi_.left == 0 && roi_.right == 0 && roi_.top == 0 && roi_.bottom == 0) {
      roi_.right = this->input_tensors.at(0)->shape[2];
      roi_.bottom = this->input_tensors.at(0)->shape[1];
    }
  }
  ~RoiResizeLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ROI_RESIZE; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_), CEREAL_NVP(pad_mode_));
    } else if (version == 4) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_.left), CEREAL_NVP(roi_.top), CEREAL_NVP(roi_.right),
         CEREAL_NVP(roi_.bottom), CEREAL_NVP(pad_mode_), CEREAL_NVP(align_mode_));
    } else if (version == 5) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_.left), CEREAL_NVP(roi_.top), CEREAL_NVP(roi_.right),
         CEREAL_NVP(roi_.bottom), CEREAL_NVP(pad_mode_), CEREAL_NVP(align_mode_), CEREAL_NVP(is_interpolate_twice));
    } else if (version == 6) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_.left), CEREAL_NVP(roi_.top), CEREAL_NVP(roi_.right),
         CEREAL_NVP(roi_.bottom), CEREAL_NVP(pad_mode_), CEREAL_NVP(align_mode_), CEREAL_NVP(is_interpolate_twice),
         CEREAL_NVP(precision_bit_num_));
    } else if (version == 7) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_.left), CEREAL_NVP(roi_.top), CEREAL_NVP(roi_.right),
         CEREAL_NVP(roi_.bottom), CEREAL_NVP(pad_mode_), CEREAL_NVP(align_mode_), CEREAL_NVP(is_interpolate_twice),
         CEREAL_NVP(precision_bit_num_), CEREAL_NVP(start_offset_));
    } else if (version == 8) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_.left), CEREAL_NVP(roi_.top), CEREAL_NVP(roi_.right),
         CEREAL_NVP(roi_.bottom), CEREAL_NVP(pad_mode_), CEREAL_NVP(align_mode_), CEREAL_NVP(is_interpolate_twice),
         CEREAL_NVP(precision_bit_num_), CEREAL_NVP(start_offset_), CEREAL_NVP(step_));
    } else if (version == 9) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(roi_.left), CEREAL_NVP(roi_.top), CEREAL_NVP(roi_.right),
         CEREAL_NVP(roi_.bottom), CEREAL_NVP(pad_mode_), CEREAL_NVP(align_mode_), CEREAL_NVP(is_interpolate_twice),
         CEREAL_NVP(precision_bit_num_), CEREAL_NVP(start_offset_), CEREAL_NVP(step_),
         CEREAL_NVP(calibrate_step_block_size_));
    } else {
      AbortOnVersion(this, version);
    }
  }

  struct Roi {
    int32_t left = 0;
    int32_t top = 0;
    int32_t right = 0;   // exclusive
    int32_t bottom = 0;  // exclusive
    template <class Archive>
    void serialize(Archive &ar) {
      // DO NOT CHANGE THIS CODE, IT IS USED BY VERSION 2,3
      ar(CEREAL_NVP(left), CEREAL_NVP(top), CEREAL_NVP(right), CEREAL_NVP(bottom));
    }
  } roi_;

  enum class PadMode { ZERO, BOUNDARY } pad_mode_ = PadMode::ZERO;
  enum class AlignMode { ORIGIN, CENTER } align_mode_ = AlignMode::ORIGIN;
  bool is_interpolate_twice = true;
  std::pair<uint32_t, uint32_t> precision_bit_num_ = {8U, 8U};
  /*
   * this field should be added to ROI top/left.
   * It contains decimal part (with precision_bit_num as decimal_bit_num)
   * {0, 0} and {INT32_MIN, INT32_MIN} both have no effect.
   */
  std::pair<int32_t, int32_t> start_offset_;

  /*
   * if the step value is not calculated by standard method, the value can be specified directly.
   * the step value should be 8bit integer and 8bit decimal.
   */
  std::pair<uint32_t, uint32_t> step_ = {UINT32_MAX, UINT32_MAX};

  /*
   * On X2/X3, the step precision is 8bit. In some cases, this may result in poor scaling accuracy.
   * You can use 16bit steps to calibrate the interpolation point coordinates per calibrate_step_block_size_ points.
   * -1 indicates that calibration is not enabled.
   */
  int32_t calibrate_step_block_size_ = -1;
};

/**
 * Relu Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct ReluLayer : public Layer {
  using Layer::Layer;
  ~ReluLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::RELU; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Elementwise Multiply Layer
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], has 1 or C shift values
 * 1. feature2 shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct ElementwiseMul : public Layer {
  using Layer::Layer;
  ~ElementwiseMul() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ELEMENTWISE_MUL; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }

  bool enable_rounding = false;
};

/**
 * SElementwise Multiply Layer
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], has 1 or C shift values
 * 1. feature2 shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct SElementwiseMul : public Layer {
  using Layer::Layer;
  ~SElementwiseMul() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SELEMENTWISE_MUL; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(bias_left_shift), CEREAL_NVP(output_scale),
         CEREAL_NVP(accu_right_shift), CEREAL_NVP(output_right_shift), CEREAL_NVP(sumin_left_shift),
         CEREAL_NVP(sumin_scale));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift),
         CEREAL_NVP(output_right_shift), CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }

  std::vector<int32_t> output_scale;
  std::vector<int8_t> accu_right_shift;
  std::vector<int8_t> output_right_shift;

  bool enable_rounding = false;

  // deprecated parameters, plan to delete on version 3.12.x
  std::vector<int32_t> sumin_scale;
  std::vector<int8_t> bias_left_shift;
  std::vector<int8_t> sumin_left_shift;
};

/**
 * Elementwise Add Layer
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], has 1 or C shift values
 * 1. feature2 shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct ElementwiseAdd : public Layer {
  using Layer::Layer;
  ~ElementwiseAdd() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ELEMENTWISE_ADD; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }
  bool enable_rounding = false;
};

/**
 * SElementwise Add Layer
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], no shift values
 * 2. feature2 shape is [N, H, W, C], no shift values
 *    (feature2 is sumin)
 *
 * output [output]
 * 1. output shape is [N, H, W, C], no shift values
 *
 * output = (saturate16((feature1 << input_left_shift + (sumin * sumin_scale) >> accu_shift) * scale) >> output_shift
 */
struct SElementwiseAdd : public Layer {
  using Layer::Layer;
  ~SElementwiseAdd() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SELEMENTWISE_ADD; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift),
         CEREAL_NVP(output_right_shift), CEREAL_NVP(sumin_scale), CEREAL_NVP(sumin_left_shift),
         CEREAL_NVP(input_left_shift), CEREAL_NVP(enable_rounding));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(output_scale), CEREAL_NVP(accu_right_shift),
         CEREAL_NVP(output_right_shift), CEREAL_NVP(sumin_scale), CEREAL_NVP(sumin_left_shift),
         CEREAL_NVP(input_left_shift), CEREAL_NVP(input_scale), CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }

  std::vector<int32_t> output_scale;
  std::vector<int8_t> accu_right_shift;
  std::vector<int8_t> output_right_shift;

  std::vector<int32_t> input_scale;  // only used by x2ad
  std::vector<int8_t> input_left_shift;

  std::vector<int32_t> sumin_scale;
  std::vector<int8_t> sumin_left_shift;

  bool enable_rounding = false;
};

/* ===============================================
 * NOTE:
 * This is no ElementwiseMulAdd layer.
 * To simulate an ElementwiseMulAdd layer,
 * you need to create two layers. One ElementwiseMul and One ElementwiseAdd.
 * The output of ElementwiseMul should be used by ElementwiseAdd, and not the model output.
 * The shift of ElementwiseMul output should be the sum of its inputs shifts.
 * The element size of ElementwiseMul output should be >= the sum of its input element size.
 * (Otherwise overflow could occur and we cannot fuse them).
 * ===============================================
 */

/**
 * Elementwise Subtract Layer
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], has 1 or C shift values
 * 1. feature2 shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct ElementwiseSub : public Layer {
  using Layer::Layer;
  ~ElementwiseSub() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ELEMENTWISE_SUB; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(enable_rounding));
    } else {
      AbortOnVersion(this, version);
    }
  }
  bool enable_rounding = false;
};

/**
 * Split Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output1, output2, ...]
 * 1~n. output shape is [N, H, W, C], has 1 or C shift values
 *
 * outputs can be split in any number instead of split equally
 */
struct SplitLayer : public Layer {
  using Layer::Layer;
  ~SplitLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SPLIT; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Concat Layer
 *
 * input [feature1, feature2, ...]
 * 1~n. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 */
struct ConcatLayer : public Layer {
  using Layer::Layer;
  ~ConcatLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::CONCAT; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Channel Max Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N,H,W,1 or 2], has 1 or 2 shift values
 *
 * NOTE: when keep_score is true: channel max output 2 channels, the first is the index in feature channel, the second
 * is the max value, when keep_score is false: channel max output 1 channel, keep index
 */
struct ChannelMaxLayer : public Layer {
  ChannelMaxLayer() = default;
  ChannelMaxLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                  std::vector<std::shared_ptr<Tensor>> output_tensors, bool keep_score, bool run_length_encoding,
                  int32_t class_offset, int32_t group_number)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        keep_score(keep_score),
        run_length_encoding(run_length_encoding),
        class_offset(class_offset),
        group_number(group_number) {}
  ~ChannelMaxLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::CHANNEL_MAX; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(keep_score), CEREAL_NVP(run_length_encoding),
         CEREAL_NVP(class_offset), CEREAL_NVP(group_number));
    } else {
      AbortOnVersion(this, version);
    }
  }

  bool keep_score;
  bool run_length_encoding;
  int32_t class_offset;
  int32_t group_number;
};

/**
 * Channel Sum Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, 1], has 1 shift value
 */
struct ChannelSumLayer : public Layer {
  using Layer::Layer;
  ~ChannelSumLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::CHANNEL_SUM; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Filter Layer
 *
 * input [?]
 *
 * output [?]
 */
struct FilterLayer : public Layer {
  FilterLayer() = default;
  FilterLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
              std::vector<std::shared_ptr<Tensor>> output_tensors, int32_t anchor_num, int32_t thresh,
              uint32_t max_box_n, uint32_t start_c, uint32_t end_c)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        anchor_dim(anchor_num),
        threshold(thresh),
        max_box_num(max_box_n),
        start_channel(start_c),
        end_channel(end_c) {}
  ~FilterLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::FILTER; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(anchor_dim), CEREAL_NVP(threshold), CEREAL_NVP(max_box_num),
         CEREAL_NVP(start_channel), CEREAL_NVP(end_channel));
    } else {
      AbortOnVersion(this, version);
    }
  }

  int32_t anchor_dim;
  int32_t threshold;
  uint32_t max_box_num;
  uint32_t start_channel;
  uint32_t end_channel;
};

/**
 * Softmax Layer
 *
 * input [?]
 *
 * output [?]
 */
struct SoftmaxLayer : public Layer {
  using Layer::Layer;
  SoftmaxLayer() = default;
  SoftmaxLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
               std::vector<std::shared_ptr<Tensor>> output_tensors, bool preserve_shape)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        preserve_shape(std::move(preserve_shape)) {}
  ~SoftmaxLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SOFTMAX; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(preserve_shape));
    } else {
      AbortOnVersion(this, version);
    }
  }

  bool preserve_shape = false;
};

/**
 * Warping Layer
 *
 * input [?]
 *
 * output [?]
 */
struct WarpingLayer : public Layer {
  using Layer::Layer;
  ~WarpingLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::WARPING; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(mode), CEREAL_NVP(stride), CEREAL_NVP(mapping_offset),
         CEREAL_NVP(kernel_shape), CEREAL_NVP(padding_value), CEREAL_NVP(is_mapping_y_then_x));
      is_uint8_mode = false;
    } else if (version == 4) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(mode), CEREAL_NVP(stride), CEREAL_NVP(mapping_offset),
         CEREAL_NVP(kernel_shape), CEREAL_NVP(padding_value), CEREAL_NVP(is_mapping_y_then_x),
         CEREAL_NVP(is_uint8_mode));
    } else {
      AbortOnVersion(this, version);
    }
  }

  enum class warping_mode_t { NORMAL, DFCONV, YUV, UV };
  warping_mode_t mode = warping_mode_t::NORMAL;
  Shape2D stride;
  Shape2D mapping_offset;
  Shape2D kernel_shape;
  int32_t padding_value = 0;
  bool is_mapping_y_then_x = false;  // motion vector channel 0 is y, and channel 1 is x ?
  bool is_uint8_mode = false;        // 8-bit input/output/padding_value are interpreted as uint8
};

/**
 * Detection Post Process Layer
 *
 * input [feature1, feature2, ..., anchor_table, exp_table, im_info]
 * 1~n. feature shape is [N, H, W, C], has 1 or C shift values, type is int8
 * n+1. anchor_table shape is [num_anchor, 4], no shift values, type is int32
 *      one anchor has 4 indexes in 4D shape
 * n+2. exp_table shape is [256], has 1 shift value, type is int32
 * n+3. im_info shape is [N, 1, 1, 2], no shift values, type is int32, can be nullptr
 *
 * output [bbox_tensor]
 * 1. bbox_tensor shape is [num_batch, num_bbox, 6], no shift, type is int32
 *    one bbox has 6 numbers [x1, y1, x2, y2, score, class_index]
 */
struct DetectionPostProcessLayer : public Layer {
  struct Attr {
    int32_t padding_value = -1;   // padding value for invalid boxes
    int32_t nms_threshold = 128;  // The overlap threshold that nms uses to suppress boxes (0-1, 8bit quantized)
    // The final number of output boxes, pad if it's fewer than it.
    int32_t nms_output_bbox_num = 300;
    // The score margin to prevent boxes to be suppressed in nms
    int32_t nms_margin = 0;
    // The shift number of the regression box_deltas and score. Usually 5/6 is a good choice.
    // This attribute is called "input_shift" in MXNET.
    int32_t score_shift = 6;
    // The threshold used to filter out low-score boxes (quantized by the input_shift)
    int32_t box_filter_threshold = 0;
    int32_t image_h = 1024;  // The original input image height.
    int32_t image_w = 1024;  // The original input image width.
    // The initial seed for the LRSF random number generator for boxes overflowing replacement.
    int32_t initial_seed = 1;
    // Whether clip the result box into [0, image_h or w - 1]
    std::vector<uint8_t> use_clipping_list;  // 0 or 1
    // The anchor box start address for each branch in the 1-D input data: anchor"
    std::vector<int32_t> anchor_start_addr;
    // The number of classes to be classified for each branch. Used for InferShape checking.
    std::vector<int32_t> class_num_list;
    // The offset of classes that handled by one branch in the total number of classes
    std::vector<int32_t> class_offset_list;
    // Class indexes with custom thresholds, in corespondent with the class_threshold_values
    std::vector<int32_t> class_threshold_indexes;
    // Class thresholds, in corespondent with the class_threshold_indexes
    std::vector<int32_t> class_threshold_values;
    // The stride_w of the RPN layer. Usually 16 in Faster RCNN.
    std::vector<int32_t> feature_stride_h;
    // The stride_h of the RPN layer. Usually 16 in Faster RCNN.
    std::vector<int32_t> feature_stride_w;
    // The number of anchors used for each branch.
    std::vector<int32_t> num_anchors;
    // how to slicing feature map in H dimension
    std::vector<int32_t> __h_block_size_list__ = {-1, -1, -1, -1, -1};
    // how to slicing feature map in W dimension
    std::vector<int32_t> __w_block_size_list__ = {-1, -1, -1, -1, -1};
    // whether to skip nms in the final dpp output
    bool skip_nms = false;
    // Whether to do stable or unstable sort
    bool stable_sort = false;
    // Whether the image size is fixed. If not fixed, an extra input im_info needs to be provided.
    bool image_size_fixed = true;
    // Whether to use special sort, which effectively a stable sort. This option is imcompatible with stable_sort = True
    bool use_special_sort = false;

    template <class Archive>
    void serialize(Archive &ar) {
      // DO NOT CHANGE THIS CODE, IT IS USED BY VERSION 3
      ar(CEREAL_NVP(padding_value), CEREAL_NVP(nms_threshold), CEREAL_NVP(nms_output_bbox_num), CEREAL_NVP(nms_margin),
         CEREAL_NVP(score_shift), CEREAL_NVP(box_filter_threshold), CEREAL_NVP(image_h), CEREAL_NVP(image_w),
         CEREAL_NVP(initial_seed), CEREAL_NVP(use_clipping_list), CEREAL_NVP(anchor_start_addr),
         CEREAL_NVP(class_num_list), CEREAL_NVP(class_offset_list), CEREAL_NVP(class_threshold_indexes),
         CEREAL_NVP(class_threshold_values), CEREAL_NVP(feature_stride_h), CEREAL_NVP(feature_stride_w),
         CEREAL_NVP(num_anchors), CEREAL_NVP(__h_block_size_list__), CEREAL_NVP(__w_block_size_list__),
         CEREAL_NVP(skip_nms), CEREAL_NVP(stable_sort), CEREAL_NVP(image_size_fixed));
    }
  };

  DetectionPostProcessLayer() = default;
  DetectionPostProcessLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                            std::vector<std::shared_ptr<Tensor>> output_tensors, DetectionPostProcessLayer::Attr attr)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)), attr(std::move(attr)) {}
  ~DetectionPostProcessLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::DETECTION_POST_PROCESS; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr));
    } else if (version == 4) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr.padding_value), CEREAL_NVP(attr.nms_threshold),
         CEREAL_NVP(attr.nms_output_bbox_num), CEREAL_NVP(attr.nms_margin), CEREAL_NVP(attr.score_shift),
         CEREAL_NVP(attr.box_filter_threshold), CEREAL_NVP(attr.image_h), CEREAL_NVP(attr.image_w),
         CEREAL_NVP(attr.initial_seed), CEREAL_NVP(attr.use_clipping_list), CEREAL_NVP(attr.anchor_start_addr),
         CEREAL_NVP(attr.class_num_list), CEREAL_NVP(attr.class_offset_list), CEREAL_NVP(attr.class_threshold_indexes),
         CEREAL_NVP(attr.class_threshold_values), CEREAL_NVP(attr.feature_stride_h), CEREAL_NVP(attr.feature_stride_w),
         CEREAL_NVP(attr.num_anchors), CEREAL_NVP(attr.__h_block_size_list__), CEREAL_NVP(attr.__w_block_size_list__),
         CEREAL_NVP(attr.skip_nms), CEREAL_NVP(attr.stable_sort), CEREAL_NVP(attr.image_size_fixed));
    } else if (version == 5) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr.padding_value), CEREAL_NVP(attr.nms_threshold),
         CEREAL_NVP(attr.nms_output_bbox_num), CEREAL_NVP(attr.nms_margin), CEREAL_NVP(attr.score_shift),
         CEREAL_NVP(attr.box_filter_threshold), CEREAL_NVP(attr.image_h), CEREAL_NVP(attr.image_w),
         CEREAL_NVP(attr.initial_seed), CEREAL_NVP(attr.use_clipping_list), CEREAL_NVP(attr.anchor_start_addr),
         CEREAL_NVP(attr.class_num_list), CEREAL_NVP(attr.class_offset_list), CEREAL_NVP(attr.class_threshold_indexes),
         CEREAL_NVP(attr.class_threshold_values), CEREAL_NVP(attr.feature_stride_h), CEREAL_NVP(attr.feature_stride_w),
         CEREAL_NVP(attr.num_anchors), CEREAL_NVP(attr.__h_block_size_list__), CEREAL_NVP(attr.__w_block_size_list__),
         CEREAL_NVP(attr.skip_nms), CEREAL_NVP(attr.stable_sort), CEREAL_NVP(attr.image_size_fixed),
         CEREAL_NVP(attr.use_special_sort));
    } else {
      AbortOnVersion(this, version);
    }
  }

  DetectionPostProcessLayer::Attr attr;
};

/**
 * RCNN Post Process Layer
 *
 * input [bbox_tensor, bbox_score, bbox_deltas]
 * 1. bbox_tensor shape is [num_batch, num_bbox, 6], no shift, type is int32
 *    one bbox has 6 numbers [x1, y1, x2, y2, score, class_index]
 * 2. bbox_score shape is [num_batch * num_bbox, 1, 1, (num_class + 1)], no shift, type is float32
 * 3. bbox_deltas shape is [num_batch * num_bbox, 1, 1, (num_class + 1) * 4], no shift, type is float32
 * 4. im_info shape is [N, 1, 1, 2], no shift values, type is int32, can be nullptr
 *
 * output [output0, output1]
 * 1. output0 shape is [num_batch, nms_top_n, 6], no shift, type is int32
 *    one bbox has 6 numbers [x1, y1, x2, y2, score, class_index]
 * 2. output1 shape is [num_batch, nms_top_n, 6], no shift, type is float32
 *    one bbox has 6 numbers [x1, y1, x2, y2, score, class_index]
 */
struct RcnnPostProcessLayer : public Layer {
  struct Attr;
  RcnnPostProcessLayer() = default;
  RcnnPostProcessLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                       std::vector<std::shared_ptr<Tensor>> output_tensors, Attr attr)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)), attr(std::move(attr)) {}
  ~RcnnPostProcessLayer() override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::RCNN_POST_PROCESS; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr.original_img_h), CEREAL_NVP(attr.original_img_w),
         CEREAL_NVP(attr.nms_threshold), CEREAL_NVP(attr.class_number), CEREAL_NVP(attr.nms_top_n),
         CEREAL_NVP(attr.score_threshold), CEREAL_NVP(attr.bbox_delta_mean), CEREAL_NVP(attr.bbox_delta_std),
         CEREAL_NVP(attr.image_size_fixed));
    } else {
      AbortOnVersion(this, version);
    }
  }

  struct Attr {
    uint32_t original_img_h = 0;
    uint32_t original_img_w = 0;
    float nms_threshold = 0.0;
    uint32_t class_number = 0;
    uint32_t nms_top_n = 0;
    float score_threshold = 0.0;
    std::vector<float> bbox_delta_mean;  // array[4]
    std::vector<float> bbox_delta_std;   // array[4]
    // Whether the image size is fixed. If not fixed, an extra input im_info needs to be provided.
    bool image_size_fixed = true;

    template <class Archive>
    void serialize(Archive &ar) {
      // DO NOT CHANGE THIS CODE, IT IS USED BY VERSION 2
      ar(CEREAL_NVP(original_img_h), CEREAL_NVP(original_img_w), CEREAL_NVP(nms_threshold), CEREAL_NVP(class_number),
         CEREAL_NVP(nms_top_n), CEREAL_NVP(score_threshold), CEREAL_NVP(bbox_delta_mean), CEREAL_NVP(bbox_delta_std),
         CEREAL_NVP(image_size_fixed));
    }
  } attr;
};

/**
 * ROI Align Layer
 *
 * ========== roi align mode ==========
 *
 * input [feature1, feature2, ..., bbox_tensor]
 * 1~n. feature shape is [N, H, W, C], has 1 or C shift values, type is int8
 * n+1. bbox_tensor shape is [1, num_bbox, 6], no shift, type is int32
 *      one bbox has 6 numbers [left, top, right, bottom, score, class_index]
 *
 * output [output]
 * 1. output shape is [num_bbox, H, W, C], has 1 or C shift values, type is int8
 */
struct ROIAlignLayer : public Layer {
  struct Attr;
  ROIAlignLayer() = default;
  ROIAlignLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                std::vector<std::shared_ptr<Tensor>> output_tensors, Attr attr)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)), attr(std::move(attr)) {}
  ~ROIAlignLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ROI_ALIGN; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr.output_shape), CEREAL_NVP(attr.feature_stride_h),
         CEREAL_NVP(attr.feature_stride_w), CEREAL_NVP(attr.base_image_scale), CEREAL_NVP(attr.middle_layer_id),
         CEREAL_NVP(attr.ignore_score_value), CEREAL_NVP(attr.num_pooling), CEREAL_NVP(attr.max_num_pooling),
         CEREAL_NVP(attr.pool_method), CEREAL_NVP(attr.padding_mode), CEREAL_NVP(attr.area_index),
         CEREAL_NVP(attr.clip_box), CEREAL_NVP(attr.box_augmentation), CEREAL_NVP(attr.box_augmentation_params));
    } else {
      AbortOnVersion(this, version);
    }
  }

  struct Attr {
    Shape2D output_shape;  // ROI output shape (h,w), it usually varies from different tasks, like (7, 7) for detection

    std::vector<uint16_t> feature_stride_h = {1u};  // The corresponding stride_h for data1-5
    std::vector<uint16_t> feature_stride_w = {1u};  // The corresponding stride_w for data1-5

    // box_layer_id = floor(middle_layer_id + log2(sqrt(box_w * box_h) / base_image_scale))
    int32_t base_image_scale = 224;
    int32_t middle_layer_id = 0;  // It can usually be calculated as 4 - log2(min(stride_list))

    float ignore_score_value = -128;  // the score value for ignored boxes

    // when num_pooling >= 0, this Op will always does certain number of pooling,
    // and the crop_resize target size is also fixed
    int32_t num_pooling = -1;
    int32_t max_num_pooling = -1;  // when it >= 0, this Op will have at most `max_num_pooling` times of poolings
    enum class PoolMethod { AVG, MAX } pool_method = PoolMethod::AVG;  // the pooling method used after crop_and_resize

    // - shrink: make sure the step size is small enough to avoid the border
    // - zero: pad with 0
    // - nearest: pad with border value
    // - border: ?
    enum class PaddingMode { SHRINK, ZERO, NEAREST, BORDER } padding_mode = PaddingMode::SHRINK;

    int32_t area_index = -1;  // if area_index >= 0, it will get the area data from input, instead of calculation

    bool clip_box = false;          // when true, the box will be clipped when use_box_augmentation is true.
    bool box_augmentation = false;  // when true, the box will be augmented when cropping (mostly for skeleton)
    std::vector<float> box_augmentation_params = {1.0, 0, 0, 0, 0};  // array<5>, representing a, r1, r2, r3, r4

    template <class Archive>
    void serialize(Archive &ar) {
      // DO NOT CHANGE THIS CODE, IT IS USED BY VERSION 2
      ar(CEREAL_NVP(output_shape), CEREAL_NVP(feature_stride_h), CEREAL_NVP(feature_stride_w),
         CEREAL_NVP(base_image_scale), CEREAL_NVP(middle_layer_id), CEREAL_NVP(ignore_score_value),
         CEREAL_NVP(num_pooling), CEREAL_NVP(max_num_pooling), CEREAL_NVP(pool_method), CEREAL_NVP(padding_mode),
         CEREAL_NVP(area_index), CEREAL_NVP(clip_box), CEREAL_NVP(box_augmentation),
         CEREAL_NVP(box_augmentation_params));
    }
  } attr;
};

/**
 * Slice Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C], has 1 or C shift values,
 *
 * output [feature]
 * 1. output shape is [N, H, W, C], has 1 or C shift values,
 */
struct SliceLayer : public Layer {
  struct Attr;
  SliceLayer() = default;
  SliceLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
             std::vector<std::shared_ptr<Tensor>> output_tensors, std::vector<uint32_t> begin,
             std::vector<uint32_t> end, std::vector<uint32_t> step)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        begin(std::move(begin)),
        end(std::move(end)),
        step(std::move(step)) {}
  ~SliceLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SLICE; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(begin), CEREAL_NVP(step), CEREAL_NVP(end));
    } else {
      AbortOnVersion(this, version);
    }
  }

  std::vector<uint32_t> begin;  // can be [N] or [N, H] or [N, H, W] or [N, H, W, C]
  std::vector<uint32_t> end;    // can be [N] or [N, H] or [N, H, W] or [N, H, W, C]
  std::vector<uint32_t> step;   // can be [N] or [N, H] or [N, H, W] or [N, H, W, C]
};

/**
 * Lookup Table Layer
 *
 * input [?]
 *
 * output [?]
 */
struct LutLayer : public Layer {
  struct Attr;
  LutLayer() = default;
  LutLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
           std::vector<std::shared_ptr<Tensor>> output_tensors, Attr attr)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)), attr(std::move(attr)) {}
  ~LutLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::LUT; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr.dense_shift), CEREAL_NVP(attr.dense_scale),
         CEREAL_NVP(attr.dense_beta), CEREAL_NVP(attr.dense_min), CEREAL_NVP(attr.dense_max),
         CEREAL_NVP(attr.dense_table), CEREAL_NVP(attr.sparse_shift), CEREAL_NVP(attr.sparse_scale),
         CEREAL_NVP(attr.sparse_beta), CEREAL_NVP(attr.sparse_min), CEREAL_NVP(attr.sparse_max),
         CEREAL_NVP(attr.sparse_table), CEREAL_NVP(attr.left_shift), CEREAL_NVP(attr.left_scale),
         CEREAL_NVP(attr.left_beta), CEREAL_NVP(attr.right_shift), CEREAL_NVP(attr.right_scale),
         CEREAL_NVP(attr.right_beta), CEREAL_NVP(attr.x_min), CEREAL_NVP(attr.x_max), CEREAL_NVP(attr.enable_symmetry),
         CEREAL_NVP(attr.symmetry_k), CEREAL_NVP(attr.symmetry_b), CEREAL_NVP(attr.idx_bits));
      attr.share_table = false;
      attr.pointwise_shift = false;
      attr.dense_min_float = 0;
      attr.dense_max_float = 0;
      attr.sparse_min_float = 0;
      attr.sparse_max_float = 0;
      attr.x_min_float = 0;
      attr.x_max_float = 0;
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(attr.dense_shift), CEREAL_NVP(attr.dense_scale),
         CEREAL_NVP(attr.dense_beta), CEREAL_NVP(attr.dense_min), CEREAL_NVP(attr.dense_max),
         CEREAL_NVP(attr.dense_table), CEREAL_NVP(attr.sparse_shift), CEREAL_NVP(attr.sparse_scale),
         CEREAL_NVP(attr.sparse_beta), CEREAL_NVP(attr.sparse_min), CEREAL_NVP(attr.sparse_max),
         CEREAL_NVP(attr.sparse_table), CEREAL_NVP(attr.left_shift), CEREAL_NVP(attr.left_scale),
         CEREAL_NVP(attr.left_beta), CEREAL_NVP(attr.right_shift), CEREAL_NVP(attr.right_scale),
         CEREAL_NVP(attr.right_beta), CEREAL_NVP(attr.x_min), CEREAL_NVP(attr.x_max), CEREAL_NVP(attr.enable_symmetry),
         CEREAL_NVP(attr.symmetry_k), CEREAL_NVP(attr.symmetry_b), CEREAL_NVP(attr.idx_bits),
         CEREAL_NVP(attr.share_table), CEREAL_NVP(attr.pointwise_shift), CEREAL_NVP(attr.dense_min_float),
         CEREAL_NVP(attr.dense_max_float), CEREAL_NVP(attr.sparse_min_float), CEREAL_NVP(attr.sparse_max_float),
         CEREAL_NVP(attr.x_min_float), CEREAL_NVP(attr.x_max_float));
    } else {
      AbortOnVersion(this, version);
    }
  }

  enum class LutType {
    SIGMOID = 0,
    TANH = 1,
    EXP = 2,
    LOG = 3,
  };

  struct Attr {
    int32_t dense_shift;
    int32_t dense_scale;
    int32_t dense_beta;
    int32_t dense_min;  // Not used, use float version instead
    int32_t dense_max;  // Not used, use float version instead
    std::vector<int32_t> dense_table;
    int32_t sparse_shift;
    int32_t sparse_scale;
    int32_t sparse_beta;
    int32_t sparse_min;  // Not used, use float version instead
    int32_t sparse_max;  // Not used, use float version instead
    std::vector<int32_t> sparse_table;
    int32_t left_shift;
    int32_t left_scale;
    int32_t left_beta;
    int32_t right_shift;
    int32_t right_scale;
    int32_t right_beta;
    int32_t x_min;  // Not used, use float version instead
    int32_t x_max;  // Not used, use float version instead
    bool enable_symmetry;
    int32_t symmetry_k;
    int32_t symmetry_b;
    int32_t idx_bits;

    bool share_table;
    bool pointwise_shift;
    float dense_min_float;
    float dense_max_float;
    float sparse_min_float;
    float sparse_max_float;
    float x_min_float;
    float x_max_float;

    template <class Archive>
    void serialize(Archive &ar) {
      // DO NOT CHANGE THIS CODE, IT IS USED BY VERSION 1
      ar(CEREAL_NVP(dense_shift), CEREAL_NVP(dense_scale), CEREAL_NVP(dense_beta), CEREAL_NVP(dense_min),
         CEREAL_NVP(dense_max), CEREAL_NVP(dense_table), CEREAL_NVP(sparse_shift), CEREAL_NVP(sparse_scale),
         CEREAL_NVP(sparse_beta), CEREAL_NVP(sparse_min), CEREAL_NVP(sparse_max), CEREAL_NVP(sparse_table),
         CEREAL_NVP(left_shift), CEREAL_NVP(left_scale), CEREAL_NVP(left_beta), CEREAL_NVP(right_shift),
         CEREAL_NVP(right_scale), CEREAL_NVP(right_beta), CEREAL_NVP(x_min), CEREAL_NVP(x_max),
         CEREAL_NVP(enable_symmetry), CEREAL_NVP(symmetry_k), CEREAL_NVP(symmetry_b), CEREAL_NVP(idx_bits));
    }
  } attr;
};

/**
 * Reshape Layer
 *
 * input
 * 1. feature shape is [N, H, W, C]
 *
 * output
 * 1. feature shape is [N, H, W, C]
 */
struct ReshapeLayer : public Layer {
  ReshapeLayer() = default;
  ReshapeLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
               std::vector<std::shared_ptr<Tensor>> output_tensors)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)) {}
  ~ReshapeLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::RESHAPE; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      uint8_t tmp;
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(mode), CEREAL_NVP(tmp));
      factor.first = factor.second = tmp;
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(mode), CEREAL_NVP(factor));
    } else if (version == 3) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(mode), CEREAL_NVP(factor), CEREAL_NVP(output_shape));
    } else {
      AbortOnVersion(this, version);
    }
  }

  enum class reshape_mode_t {
    STACK_NEIGHBOR = 0,
    REORDER_UPSCALE_CONSECUTIVE,  // channels are 00000111112222233333 before reorder_upscale
    REORDER_UPSCALE_UNFOLD,       // channels are 01230123012301230123 before reorder_upscale
    NHWC_REINTERPRET_CAST,        // reshape do not move any data
  };

  reshape_mode_t mode;
  std::pair<uint8_t, uint8_t> factor;

  /**
   * output shape of the reshape, only one dim can be inferred by set -1
   */
  Shape output_shape;
};

/**
 * Quanti Flatten Layer
 *
 * input [feature]
 * 1. feature shape is [N, H, W, C]
 *
 * output [output]
 * 1. output shape is [N, 1, 1, H*W*C]
 */
struct QuantiFlatten : public Layer {
  using Layer::Layer;
  ~QuantiFlatten() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::QUANTI_FLATTEN; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Represent Leaky ReLU, PReLU, RReLU
 *
 * input [feature, scale]
 * 1. feature shape is [N, H, W, C]
 *
 * output [feature]
 * 1. output shape is [N, H, W, C]
 */
struct ScaleRelu : public Layer {
  using Layer::Layer;
  ~ScaleRelu() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SCALE_RELU; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(scales), CEREAL_NVP(shifts));
    } else {
      AbortOnVersion(this, version);
    }
  }

  std::vector<int32_t> scales;
  std::vector<int8_t> shifts;
};

/**
 * Linear Polynomial
 *
 * Inputs: 4 inputs (feature, weight, bias) and 1 output
 *         X: input feature, w: weight, b: bias, Y: output
 *
 * Param:
 *   precision: the precision of weight data {8bit, 14bit}. default: 8bit
 *   per_channel_mode: default: false
 *     Normal mode: Y = w1X1 + w2X2 + ... + wnXn + b
 *     Per channel mode: Yi = wiXi + bi
 *
 * Output shift: the same as the input shifts and all shifts are the same.
 */
struct LinearPolynomial : public Layer {
  using Layer::Layer;
  ~LinearPolynomial() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::LINEAR_POLYNOMIAL; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(precision), CEREAL_NVP(per_channel_mode));
    } else {
      AbortOnVersion(this, version);
    }
  }

  int32_t precision = 8;
  bool per_channel_mode = false;
};

/**
 * Nearest Upsample layer.
 *
 * input
 * 1. feature shape is [N, H, W, C]
 *
 * output
 * 1. feature shape is [N, factor*H, factor*W, C]
 */
struct NearestUpsample : public Layer {
  using Layer::Layer;

  ~NearestUpsample() noexcept override = default;

  layer_type_t GetLayerType() const override { return layer_type_t::NEAREST_UPSAMPLE; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(factor));
    } else {
      AbortOnVersion(this, version);
    }
  }
  uint8_t factor;
};

/**
 * Feature after ReluX, the values all in [0, x]
 * when x=(6 << shift), it is ReLU6, the shift is feature's shift
 *
 * NOTE: input's shift should equal to output's shift
 *
 * input [feature, scale]
 * 1. feature shape is [N, H, W, C]
 *
 * output [feature]
 * 1. output shape is [N, H, W, C]
 */
struct ReluX : public Layer {
  using Layer::Layer;
  layer_type_t GetLayerType() const override { return layer_type_t::RELU_X; }
  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(x));
    } else {
      AbortOnVersion(this, version);
    }
  }

  uint32_t x = INT8_MAX;  // x is quantized value, the shift is same to input's shift.
};

struct ShuffleLayer : public Layer {
  using Layer::Layer;
  ShuffleLayer() = default;
  ShuffleLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
               std::vector<std::shared_ptr<Tensor>> output_tensors, std::vector<int32_t> shuffle_index)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        shuffle_index(std::move(shuffle_index)) {}
  ~ShuffleLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::SHUFFLE; }
  std::vector<int32_t> shuffle_index;

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(shuffle_index));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Optical Pyramid Layer
 *
 * input: original feature(scalar)
 * output: original layer gradient xy and DownSample feature and Gradient x and y
 *
 * output number is decided by pyramid layer
 */
struct OpticalPyramidLayer : public Layer {
  using Layer::Layer;
  enum class BorderMode;
  OpticalPyramidLayer() = default;
  OpticalPyramidLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
                      std::vector<std::shared_ptr<Tensor>> output_tensors, std::vector<int32_t> scalar_outputs,
                      std::vector<int32_t> gradient_outputs, int32_t pyramid_level, int32_t padding_pixel,
                      BorderMode border_mode, std::string scalar_element_type)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        scalar_outputs_(std::move(scalar_outputs)),
        grad_outputs_(std::move(gradient_outputs)),
        pyramid_level_(pyramid_level),
        padding_pixel_(padding_pixel),
        border_mode_(border_mode),
        scalar_element_type_(scalar_element_type) {}
  ~OpticalPyramidLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::OPTICAL_PYRAMID; }
  std::vector<std::pair<bool, bool>> pyramid_config_;
  // NOTE do not change the sequence, if you modify it, do so in optical_pyramid.hpp as well
  enum class BorderMode { BORDER_REPLICATE, BORDER_REFLECT, BORDER_REFLECT_101, BORDER_WRAP, BORDER_CONSTANT };

  std::vector<int32_t> scalar_outputs_;
  std::vector<int32_t> grad_outputs_;
  int32_t pyramid_level_;
  int32_t padding_pixel_;
  BorderMode border_mode_;
  std::string scalar_element_type_;

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(pyramid_config_), CEREAL_NVP(border_mode_),
         CEREAL_NVP(pyramid_level_), CEREAL_NVP(scalar_outputs_), CEREAL_NVP(grad_outputs_));
    } else if (version == 2) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(pyramid_config_), CEREAL_NVP(border_mode_),
         CEREAL_NVP(pyramid_level_), CEREAL_NVP(scalar_outputs_), CEREAL_NVP(grad_outputs_), CEREAL_NVP(padding_pixel_),
         CEREAL_NVP(scalar_element_type_));
    } else {
      AbortOnVersion(this, version);
    }
  }
};
/**
 * Pad Layer
 *
 * input [feature]
 * 1. feature1 shape is [N, H, W, C]
 *
 * output [output]
 * 1. output shape is [N', H', W', C']
 */
struct PadLayer : public Layer {
  using Layer::Layer;
  ~PadLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::PAD; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(mode), CEREAL_NVP(constant_value), CEREAL_NVP(pad_before),
         CEREAL_NVP(pad_after));
    } else {
      AbortOnVersion(this, version);
    }
  }

  enum class Mode { CONSTANT, EDGE };
  Mode mode = Mode::CONSTANT;
  int32_t constant_value = 0;

  std::vector<uint32_t> pad_before = {0, 0, 0, 0};  // pad size before of each dim
  std::vector<uint32_t> pad_after = {0, 0, 0, 0};   // pad size after of each dim
};

/**
 * Correlation Layer
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C]
 * 2. feature2 shape is [N, H, W, C]
 *    (feature2 shape equal to feature1 shape)
 *
 * output [output]
 * 1. output shape is [N, H, W, C]
 *
 */
struct Correlation : public Layer {
  using Layer::Layer;
  ~Correlation() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::CORRELATION; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(kernel_size), CEREAL_NVP(max_displacement), CEREAL_NVP(stride1),
         CEREAL_NVP(stride2), CEREAL_NVP(pad_size), CEREAL_NVP(is_multiply));
    } else {
      AbortOnVersion(this, version);
    }
  }

  uint32_t kernel_size = 1u;
  uint32_t max_displacement = 1u;
  uint32_t stride1 = 1u;
  uint32_t stride2 = 1u;
  uint32_t pad_size = 0u;
  bool is_multiply = true;  // mul or sub
};

/**
 * StepwiseFit Layer
 *
 * Use elementwise lookup table to fit any curve, input is int8 or int32, output is int8
 *
 * if input data type is int8:
 * output = output_table[input + 128]
 * if input data type is int32:
 * output = output_table[min_arg_i{boundary_table[i] >= input}]
 *
 * boundary_table should be a vector with 255 elements in strict increasing order
 * output_table should be a vector with 256 elements
 */
struct StepwiseFit : public Layer {
  using Layer::Layer;
  ~StepwiseFit() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::STEPWISE_FIT; }
  // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(boundary_table), CEREAL_NVP(output_table));
    } else {
      AbortOnVersion(this, version);
    }
  }

  std::vector<int32_t> boundary_table;
  std::vector<int8_t> output_table;
};

/**
 * ConvertBetweenInt8AndUint8 Layer:
 *
 * See the all the data as int8, and elementwise subtract 128(for int8, plus 128 as well)
 *   int8_t a = -1;
 *   int8_t b = -1 - 128 = 127
 *   int8_t c = -1 + 128 = 127
 *
 * So this layer can convert uint8 image to int8 and to calculate on BPU, and then convert to uint8 image
 */
struct ConvertBetweenInt8AndUint8Layer : public Layer {
  using Layer::Layer;
  ~ConvertBetweenInt8AndUint8Layer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::CONVERT_BETWEEN_INT8_AND_UINT8; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/***
 * Dilate Inst, same with cv.dilate
 *
 *  dst(x, y) =          max             src(x + x', y + y')
 *             (x',y'):element(x',y') not 0
 *
 *  x' and y' are the kernel size
 *
 */
struct DilateLayer : public Layer {
  using Layer::Layer;
  DilateLayer() = default;
  DilateLayer(std::string name, std::vector<std::shared_ptr<Tensor>> input_tensors,
              std::vector<std::shared_ptr<Tensor>> output_tensors, Shape2D kernel, std::string element_type)
      : Layer(std::move(name), std::move(input_tensors), std::move(output_tensors)),
        kernel(kernel),
        element_type_(std::move(element_type)) {}
  ~DilateLayer() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::DILATE; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this), CEREAL_NVP(kernel), CEREAL_NVP(element_type_));
    } else {
      AbortOnVersion(this, version);
    }
  }

  Shape2D kernel;
  std::string element_type_;
};

/**
 * Elementwise min layer, F0 = min(F1, F2)
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], has 1 or C shift values
 * 2. feature2 shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 *
 * NODE: input1.shifts == input2.shifts == output.shifts
 */
struct ElementwiseMin : public Layer {
  using Layer::Layer;
  ~ElementwiseMin() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ELEMENTWISE_MIN; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

/**
 * Elementwise max layer, F0 = max(F1, F2)
 *
 * input [feature1, feature2]
 * 1. feature1 shape is [N, H, W, C], has 1 or C shift values
 * 2. feature2 shape is [N, H, W, C], has 1 or C shift values
 *
 * output [output]
 * 1. output shape is [N, H, W, C], has 1 or C shift values
 *
 * NODE: input1.shifts == input2.shifts == output.shifts
 */
struct ElementwiseMax : public Layer {
  using Layer::Layer;
  ~ElementwiseMax() noexcept override = default;
  layer_type_t GetLayerType() const override { return layer_type_t::ELEMENTWISE_MAX; }

  template <class Archive>
  void serialize(Archive &ar, std::int32_t const version) {
    // Don't forget to update the version number using HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION in the following
    if (version == 1) {
      ar(cereal::base_class<Layer>(this));
    } else {
      AbortOnVersion(this, version);
    }
  }
};

}  // namespace hbir
}  // namespace hbdk

#ifndef SWIG
#define HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(type, version) \
  CEREAL_REGISTER_TYPE(type);                                      \
  CEREAL_CLASS_VERSION(type, version);

// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::Tensor, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::Layer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::QuantiInputLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ConvolutionLayer, 4);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SConvolutionLayer, 3);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::AvgPoolingLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::MaxPoolingLayer, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::GlobalAvgPoolingLayer, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::GlobalMaxPoolingLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::RoiResizeLayer, 9);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ReluLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ElementwiseMul, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SElementwiseMul, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ElementwiseAdd, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ElementwiseSub, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SElementwiseAdd, 3);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SplitLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ConcatLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ChannelMaxLayer, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SoftmaxLayer, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::WarpingLayer, 4);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::DetectionPostProcessLayer, 5);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::RcnnPostProcessLayer, 3)
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ROIAlignLayer, 3);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ChannelSumLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::FilterLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SliceLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::LutLayer, 3);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ReshapeLayer, 3);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::QuantiFlatten, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::DeconvolutionLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SDeconvolutionLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ScaleRelu, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::LinearPolynomial, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ReluX, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::NearestUpsample, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ShuffleLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::SQuantiInputLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::OpticalPyramidLayer, 2);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::PadLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::Correlation, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::StepwiseFit, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ConvertBetweenInt8AndUint8Layer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::DilateLayer, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ElementwiseMin, 1);
// NOLINTNEXTLINE
HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION(hbdk::hbir::ElementwiseMax, 1);
#undef HBDK_HBIR_CEREAL_REGISTER_TYPE_WITH_VERSION

#endif  // SWIG

namespace hbdk {
namespace hbir {

struct Serializer {
  template <typename... T>
  static void ToJsonFile(const std::string &filename, T &... objects) {
    std::ofstream ofs(filename);
    cereal::JSONOutputArchive oarchive(ofs);
    oarchive(std::forward<T>(objects)...);
  }

  template <typename... T>
  static void FromJsonFile(const std::string &filename, T &... objects) {
    std::ifstream ifs(filename);
    cereal::JSONInputArchive iarchive(ifs);
    iarchive(std::forward<T>(objects)...);
  }

  template <typename... T>
  static void ToBinaryFile(const std::string &filename, T &... objects) {
    std::ofstream ofs(filename, std::ios::binary);
    cereal::BinaryOutputArchive oarchive(ofs);
    oarchive(std::forward<T>(objects)...);
  }

  template <typename... T>
  static void FromBinaryFile(const std::string &filename, T &... objects) {
    std::ifstream ifs(filename, std::ios::binary);
    cereal::BinaryInputArchive iarchive(ifs);
    iarchive(std::forward<T>(objects)...);
  }
};

template <>
struct ElementType<int1_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S1;
};
template <>
struct ElementType<int2_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S2;
};
template <>
struct ElementType<int4_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S4;
};
template <>
struct ElementType<int8_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S8;
};
template <>
struct ElementType<int16_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S16;
};
template <>
struct ElementType<int32_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S32;
};
template <>
struct ElementType<int64_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::S64;
};
template <>
struct ElementType<uint1_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U1;
};
template <>
struct ElementType<uint2_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U2;
};
template <>
struct ElementType<uint4_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U4;
};
template <>
struct ElementType<uint8_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U8;
};
template <>
struct ElementType<uint16_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U16;
};
template <>
struct ElementType<uint32_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U32;
};
template <>
struct ElementType<uint64_t> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::U64;
};
template <>
struct ElementType<float> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::F32;
};
template <>
struct ElementType<double> {
  static constexpr Tensor::element_type_t value = Tensor::element_type_t::F64;
};

#define HBIR_TENSOR_ELEMENT_TYPE_SWITCH(element_type, DType, ...) \
  switch (element_type) {                                         \
    case hbir::Tensor::element_type_t::S1: {                      \
      using DType = hbir::int1_t;                                 \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::S2: {                      \
      using DType = hbir::int2_t;                                 \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::S4: {                      \
      using DType = hbir::int4_t;                                 \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::S8: {                      \
      using DType = int8_t;                                       \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::S16: {                     \
      using DType = int16_t;                                      \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::S32: {                     \
      using DType = int32_t;                                      \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::S64: {                     \
      using DType = int64_t;                                      \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U1: {                      \
      using DType = hbir::uint1_t;                                \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U2: {                      \
      using DType = hbir::uint2_t;                                \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U4: {                      \
      using DType = hbir::uint4_t;                                \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U8: {                      \
      using DType = uint8_t;                                      \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U16: {                     \
      using DType = uint16_t;                                     \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U32: {                     \
      using DType = uint32_t;                                     \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::U64: {                     \
      using DType = uint64_t;                                     \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::F32: {                     \
      using DType = float;                                        \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    case hbir::Tensor::element_type_t::F64: {                     \
      using DType = double;                                       \
      { __VA_ARGS__ }                                             \
    } break;                                                      \
    default:                                                      \
      std::cerr << "unknown hbir tensor element type";            \
  }

extern "C" {
HBDK_PUBLIC int CheckOps(const char *serialized_std_vector_of_layers, size_t byte_size, hbrt_march_t march,
                         bool print_err_msg);
}

}  // namespace hbir
}  // namespace hbdk

#endif  // HBDK_IR_HPP_
