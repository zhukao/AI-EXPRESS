//===- hbdk_layout.h - Layout definition and helper function ------*- C -*-===//
//
//                     The HBDK Compiler Infrastructure
//
// This file is subject to the terms and conditions defined in file
// 'LICENSE.txt', which is part of this source code package.
//
//===----------------------------------------------------------------------===//
/// \file
/// The naive way to place a tensor on the memory storage is looping through its
/// NHWC dimension one by one, as shown by following C code.
///
///     for (int n = 0; n < N; ++n)
///       for (int h = 0; h < H; ++h)
///         for (int w = 0; w < W; ++w)
///           for (int c = 0; c < C; ++c)
///
/// In order to maximize MAC utilization, the tensor would be layout in a few
/// specific orders, based on what loop tiling and stride are choosed. The layout
/// could be represented by format string, X_Y_S_P.
///
///  - X means in which order we loop through NHWC.
///  - Y means the tiling size of each dimension, also means the order we loop
///    through them.
///  - S means the stride.
///  - P means the PE number and extend direction.
///
/// For example, NHCW_1N1H8W4C_S1 could be represented by C code below.
///
///     for (int n = 0; n < ceil(N/1); ++n)
///       for (int h = 0; h < ceil(H/1); ++h)
///         for (int c = 0; c < ceil(C/4); ++c)
///           for (int w = 0; w < ceil(W/8); ++w)
///             for (int nn = 0; nn < 1; ++nn)
///               for (int hh = 0; hh < 1; ++hh)
///                 for (int ww = 0; ww < 8; ++ww)
///                   for (int cc = 0; cc < 4; ++cc)
///
//===----------------------------------------------------------------------===//
#ifndef HBDK_LAYOUT_H_
#define HBDK_LAYOUT_H_
#pragma once

#include "hbdk_config.h"
#include "hbdk_type.h"

#ifdef __cplusplus
#include <cassert>
#include <cstdbool>
#include <cstdint>
#include <cstdlib>
#else
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  LAYOUT_ORDER_NHWC = 0,  ///< NHWC
  LAYOUT_ORDER_NHCW,      ///< HNCW
  LAYOUT_ORDER_NCHW,      ///< NCHW
  LAYOUT_ORDER_NCWH,      ///< NCWH
} hbrt_layout_order_t;

/**
 * This enum class represents the layout for (convolution) stride 2 (s2).
 * For target supporting s2, the layout could be one of the following,
 *
 *   - LAYOUT_STRIDE2_MODE_W_MAPPING
 *
 *   - LAYOUT_STRIDE2_MODE_DOUBLE_BLOCK
 */
typedef enum {
  LAYOUT_STRIDE2_MODE_W_MAPPING = 0,  ///< stride 2
  LAYOUT_STRIDE2_MODE_DOUBLE_BLOCK,   ///< stride 2
} hbrt_layout_stride2_mode_t;

typedef hbrt_layout_stride2_mode_t layout_stride2_mode_t HBRT_DEPRECATED_NAME(hbrt_layout_order_t,
                                                                              layout_stride2_mode_t, 3.7.1);
static const uint32_t HBRT_LAYOUT_NAME_MAX = 32;  ///< The maximum length of the name of layout

#ifndef SWIG
static const uint32_t LAYOUT_NAME_MAX HBRT_DEPRECATED_NAME(HBRT_LAYOUT_NAME_MAX, LAYOUT_NAME_MAX, 3.7.1) = 32;
#endif
/**
 * This enum class is corresponding to the layout format string.
 * Must be in the same order of hbdk_layout.def
 */
typedef enum hbrt_layout_type_t {
  LAYOUT_NHWC_NATIVE = 0,
  LAYOUT_NHCW_NATIVE,
  LAYOUT_NCHW_NATIVE,

  LAYOUT_NCHW_2N32C,
  LAYOUT_NCHW_2N32C_2PEN,
  LAYOUT_NCHW_2N32C_4PEN,

  LAYOUT_NHWC_4W4C_2PEC,
  LAYOUT_NHWC_16C_2PEC,
  LAYOUT_NHWC_4C_2PEC,
  LAYOUT_NHWC_4N4C_2PEN,
  LAYOUT_NHWC_8N_2PEN,
  LAYOUT_NHWC_4N16C_2PEN,
  LAYOUT_NHWC_4N_2PEN,

  LAYOUT_NHCW_32C,
  LAYOUT_NHCW_8W4C,
  LAYOUT_NHCW_8W4C_S2D,
  LAYOUT_NHCW_16W16C,
  LAYOUT_NHCW_16W16C_S2D,
  LAYOUT_NCHW_8C,
  LAYOUT_NCHW_8N4C,
  LAYOUT_NCHW_16N,
  LAYOUT_NCHW_8N32C,
  LAYOUT_NCHW_8N8W4C,
  LAYOUT_NHWC_4N,

  LAYOUT_NHCW_64W,
  LAYOUT_NHCW_64W_2PEN,
  LAYOUT_NHCW_64W_4PEN,
  LAYOUT_NHCW_64W_8PEN,
  LAYOUT_NHCW_32W2C,
  LAYOUT_NHCW_32W2C_2PEN,
  LAYOUT_NHCW_32W2C_4PEN,
  LAYOUT_NHCW_32W2C_8PEN,

  LAYOUT_NHCW_2H4W,
  LAYOUT_NHCW_2H4W_2PEW,
  LAYOUT_NHCW_2H4W_4PEW,
  LAYOUT_NHCW_2H4W_8PEW,
  LAYOUT_NHCW_2H4W_2PEN,
  LAYOUT_NHCW_2H4W_4PEN,
  LAYOUT_NHCW_2H4W_8PEN,

  LAYOUT_NHCW_2H2W2C,
  LAYOUT_NHCW_2H2W2C_2PEW,
  LAYOUT_NHCW_2H2W2C_4PEW,
  LAYOUT_NHCW_2H2W2C_8PEW,
  LAYOUT_NHCW_2H2W2C_2PEN,
  LAYOUT_NHCW_2H2W2C_4PEN,
  LAYOUT_NHCW_2H2W2C_8PEN,

  LAYOUT_NHCW_4H4W8C,
  LAYOUT_NHCW_4H4W8C_2PEN,
  LAYOUT_NHCW_4H4W8C_4PEN,
  LAYOUT_NHCW_4H4W8C_8PEN,
  LAYOUT_NHCW_4H4W8C_2PEW,
  LAYOUT_NHCW_4H4W8C_4PEW,
  LAYOUT_NHCW_4H4W8C_8PEW,
  LAYOUT_NHCW_4H4W8C_2PEC,
  LAYOUT_NHCW_4H4W8C_4PEC,
  LAYOUT_NHCW_4H4W8C_8PEC,

  LAYOUT_NHCW_32C_2PEC,
  LAYOUT_NHCW_32C_4PEC,
  LAYOUT_NHCW_32C_8PEC,
  LAYOUT_NHCW_32C_2PEN,
  LAYOUT_NHCW_32C_4PEN,
  LAYOUT_NHCW_32C_8PEN,

  LAYOUT_NHCW_4W8C,
  LAYOUT_NHCW_4W8C_2PEW,
  LAYOUT_NHCW_4W8C_4PEW,
  LAYOUT_NHCW_4W8C_8PEW,
  LAYOUT_NHCW_4W8C_2PEN,
  LAYOUT_NHCW_4W8C_4PEN,
  LAYOUT_NHCW_4W8C_8PEN,

  LAYOUT_NHCW_4W8C_2PEC,
  LAYOUT_NHCW_4W8C_4PEC,
  LAYOUT_NHCW_4W8C_8PEC,

  LAYOUT_NHCW_8W8C,
  LAYOUT_NHCW_8W8C_2PEC,
  LAYOUT_NHCW_8W8C_4PEC,
  LAYOUT_NHCW_8W8C_8PEC,
  LAYOUT_NHCW_8W8C_2PEN,
  LAYOUT_NHCW_8W8C_4PEN,
  LAYOUT_NHCW_8W8C_8PEN,

  LAYOUT_NCWH_ZHW_8N8C,
  LAYOUT_NCWH_ZHW_4N8C_2PEN,
  LAYOUT_NCWH_ZHW_2N8C_4PEN,
  LAYOUT_NCWH_ZHW_8C_8PEN,

  LAYOUT_NCWH_ZHW_64N,
  LAYOUT_NCWH_ZHW_32N_2PEN,
  LAYOUT_NCWH_ZHW_16N_4PEN,
  LAYOUT_NCWH_ZHW_8N_8PEN,

  LAYOUT_NCWH_ZHW_16N4C,
  LAYOUT_NCWH_ZHW_8N4C_2PEN,
  LAYOUT_NCWH_ZHW_4N4C_4PEN,
  LAYOUT_NCWH_ZHW_2N4C_8PEN,

  LAYOUT_NCHW_32N16C,
  LAYOUT_NCHW_32N16C_2PEN,
  LAYOUT_NCHW_32N16C_4PEN,
  LAYOUT_NCHW_32N16C_8PEN,

  LAYOUT_NCHW_32N2W8C,
  LAYOUT_NCHW_32N2W8C_2PEN,
  LAYOUT_NCHW_32N2W8C_4PEN,
  LAYOUT_NCHW_32N2W8C_8PEN,

  LAYOUT_NCHW_8N,

  LAYOUT_NCWH_ZHW_8N,
  LAYOUT_NCWH_ZHW_4N_2PEN,
  LAYOUT_NCWH_ZHW_2N_4PEN,
  LAYOUT_NCWH_ZHW_2C_8PEN,

  LAYOUT_NCHW_8W8C,

  LAYOUT_NHCW_8C,
  LAYOUT_NHCW_8C_2PEC,
  LAYOUT_NHCW_8C_4PEC,
  LAYOUT_NHCW_8C_8PEC,

  LAYOUT_NHCW_16C,
  LAYOUT_NHCW_16C_2PEC,
  LAYOUT_NHCW_16C_4PEC,
  LAYOUT_NHCW_16C_8PEC,

  LAYOUT_NHCW_4H4W2C,
  LAYOUT_NHCW_4H4W2C_2PEN,
  LAYOUT_NHCW_4H4W2C_4PEN,
  LAYOUT_NHCW_4H4W2C_8PEN,
  LAYOUT_NHCW_4H4W2C_2PEW,
  LAYOUT_NHCW_4H4W2C_4PEW,
  LAYOUT_NHCW_4H4W2C_8PEW,

  LAYOUT_NHCW_4H4W4C,
  LAYOUT_NHCW_4H4W4C_2PEN,
  LAYOUT_NHCW_4H4W4C_4PEN,
  LAYOUT_NHCW_4H4W4C_8PEN,
  LAYOUT_NHCW_4H4W4C_2PEW,
  LAYOUT_NHCW_4H4W4C_4PEW,
  LAYOUT_NHCW_4H4W4C_8PEW,

  LAYOUT_NHCW_4H4W4C_2PEC,
  LAYOUT_NHCW_4H4W4C_4PEC,
  LAYOUT_NHCW_4H4W4C_8PEC,

  LAYOUT_NHWC_32W2C,

  LAYOUT_NHCW_4C_8PEC,
  LAYOUT_NHCW_4C_4PEC,
  LAYOUT_NHCW_4C_2PEC,

  LAYOUT_NHWC_4W8C,
  LAYOUT_NHWC_4W16C,

  LAYOUT_NHWC_32C,

  LAYOUT_NHWC_8C_4PEW,
  LAYOUT_NHWC_2W8C_2PEW,

  LAYOUT_NHWC_4C,

  LAYOUT_NHCW_4C,
  LAYOUT_NHCW_2C,
  LAYOUT_NHCW_2C_2PEC,
  LAYOUT_NHCW_2C_4PEC,
  LAYOUT_NHCW_2C_8PEC,

  LAYOUT_NHWC_8C_4PEN,
  LAYOUT_NHWC_2W8C_2PEN,

  LAYOUT_NHCW_2C_2PEN,
  LAYOUT_NHCW_2C_4PEN,
  LAYOUT_NHCW_2C_8PEN,

  LAYOUT_NHCW_4C_2PEN,
  LAYOUT_NHCW_4C_4PEN,
  LAYOUT_NHCW_4C_8PEN,

  LAYOUT_NHCW_8C_2PEN,
  LAYOUT_NHCW_8C_4PEN,
  LAYOUT_NHCW_8C_8PEN,

  LAYOUT_NHWC_4W8C_2PEN,
  LAYOUT_NHWC_4W8C_4PEN,

  LAYOUT_NHWC_4W8C_2PEW,
  LAYOUT_NHWC_4W8C_4PEW,

  LAYOUT_NHWC_4096W8C,
  LAYOUT_NHWC_2048W8C_2PEW,
  LAYOUT_NHWC_1024W8C_4PEW,
  LAYOUT_NHWC_4096W8C_2PEN,
  LAYOUT_NHWC_4096W8C_4PEN,

  LAYOUT_NHCW_8C_2PEW,
  LAYOUT_NHCW_8C_4PEW,
  LAYOUT_NHCW_8C_8PEW,

  LAYOUT_NCHW_32N,
  LAYOUT_NCHW_32N_2PEN,
  LAYOUT_NCHW_32N_4PEN,
  LAYOUT_NCHW_32N_8PEN,

  LAYOUT_NHCW_16W32C,

  LAYOUT_NHCW_4H4W16C,
  LAYOUT_NHCW_4H4W16C_2PEN,
  LAYOUT_NHCW_4H4W16C_4PEN,
  LAYOUT_NHCW_4H4W16C_8PEN,
  LAYOUT_NHCW_4H4W16C_2PEW,
  LAYOUT_NHCW_4H4W16C_4PEW,
  LAYOUT_NHCW_4H4W16C_8PEW,

  LAYOUT_NCWH_ZHW_8N16C,
  LAYOUT_NCWH_ZHW_4N16C_2PEN,
  LAYOUT_NCWH_ZHW_2N16C_4PEN,
  LAYOUT_NCWH_ZHW_16C_8PEN,

  LAYOUT_NHWC_16W2C,
  LAYOUT_NHCW_32W,

  LAYOUT_NCHW_32W,
  LAYOUT_NCHW_8W,
  LAYOUT_NHWC_8N32C,
  LAYOUT_NCHW_32C,

  LAYOUT_NUM,

} hbrt_layout_type_t;

#ifndef SWIG
static const hbrt_layout_type_t LAYOUT_NHCW_32W1C HBRT_DEPRECATED_NAME(LAYOUT_NHCW_32W, LAYOUT_NHCW_32W1C,
                                                                       3.9.1) = LAYOUT_NHCW_32W;
#endif

/**
 * Get layout name from enum
 * @param name name of the layout
 * @param layout layout enum
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout
 */
HBDK_PUBLIC extern hbrt_error_t hbrtGetLayoutName(const char **name, hbrt_layout_type_t layout);

/**
 * Convert data layout
 * @param to_data the converted data will be written to this address
 * @param to_layout_type target layout type
 * @param from_data the address of source data
 * @param from_layout_type source layout type
 * @param element_type element type of the data
 * @param aligned_dim the dimensions of source data. should be 4-element uint32 array.
 * @param convert_endianness if true, the endianness of the data will also be converted.
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType
 * @note aligned_dim should take padding into account.
 */
HBDK_PUBLIC extern hbrt_error_t hbrtConvertLayout(void *to_data, hbrt_layout_type_t to_layout_type,
                                                  const void *from_data, hbrt_layout_type_t from_layout_type,
                                                  hbrt_element_type_t element_type, hbrt_dimension_t aligned_dim,
                                                  bool convert_endianness);

/**
 * Similar to hbrtConvertLayout, but only data in the ROI will be converted
 * @param to_data the converted data will be written to this address
 * @param to_layout_type target layout type
 * @param from_data the address of source data
 * @param from_layout_type source layout type
 * @param element_type element type of the data
 * @param aligned_dim the dimensions of source data. should be 4-element uint32 array.
 * @param convert_endianness if true, the endianness of the data will also be converted.
 * @param roi_coord the coordinates of the start point of roi. inclusive
 * @param roi_size the size of the roi. exclusive
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType
 * @note aligned_dim should take padding into account
 */
HBDK_PUBLIC extern hbrt_error_t hbrtConvertLayoutRoi(void *to_data, hbrt_layout_type_t to_layout_type,
                                                     const void *from_data, hbrt_layout_type_t from_layout_type,
                                                     hbrt_element_type_t element_type, hbrt_dimension_t aligned_dim,
                                                     bool convert_endianness, hbrt_roi_t roi);

/**
 * Similar to hbrtConvertLayout, but only data in the ROI ({n_index, 0, 0, c_index}, {1, H, W, 1}) will be converted
 * @param to_data the converted data will be written to this address
 * @param to_layout_type target layout type
 * @param from_data the address of source data
 * @param from_layout_type source layout type
 * @param element_type element type of the data
 * @param aligned_dim the dimensions of source data. should be 4-element uint32 array.
 * @param convert_endianness if true, the endianness of the data will also be converted.
 * @param n_index index of N of data to convert
 * @param c_index index of C of data to convert
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType
 * @note aligned_dim should take padding into account
 */
HBDK_PUBLIC extern hbrt_error_t hbrtConvertLayoutToNative1HW1(void *to_data, const void *from_data,
                                                              hbrt_layout_type_t from_layout_type,
                                                              hbrt_element_type_t element_type,
                                                              hbrt_dimension_t aligned_dim, bool convert_endianness,
                                                              uint32_t n_index, uint32_t c_index);

/**
 * Similar to hbrtConvertLayout, but only data in the ROI ({n_index, h_index, w_index, 0}, {1, 1, 1, C}) will be
 * converted
 * @param to_data the converted data will be written to this address
 * @param to_layout_type target layout type
 * @param from_data the address of source data
 * @param from_layout_type source layout type
 * @param element_type element type of the data
 * @param aligned_dim the dimensions of source data. should be 4-element uint32 array.
 * @param convert_endianness if true, the endianness of the data will also be converted.
 * @param n_index index of N of data to convert
 * @param h_index index of H of data to convert
 * @param w_index index of W of data to convert
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType
 * @note aligned_dim should take padding into account
 */
HBDK_PUBLIC extern hbrt_error_t hbrtConvertLayoutToNative111C(void *to_data, const void *from_data,
                                                              hbrt_layout_type_t from_layout_type,
                                                              hbrt_element_type_t element_type,
                                                              hbrt_dimension_t aligned_dim, bool convert_endianness,
                                                              uint32_t n_index, uint32_t h_index, uint32_t w_index);

/**
 * Similar to hbrtConvertLayout, but only one point will be converted
 * @param to_data the converted data will be written to this address
 * @param to_layout_type target layout type
 * @param from_data the address of source data
 * @param from_layout_type source layout type
 * @param element_type element type of the data
 * @param aligned_dim the dimensions of source data. should be 4-element uint32 array.
 * @param convert_endianness if true, the endianness of the data will also be converted.
 * @param n_index index of N of data to convert
 * @param h_index index of H of data to convert
 * @param w_index index of W of data to convert
 * @param c_index index of C of data to convert
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType
 * @note aligned_dim should take padding into account
 */
HBDK_PUBLIC extern hbrt_error_t hbrtConvertLayoutToNative1111(void *to_data, const void *from_data,
                                                              hbrt_layout_type_t from_layout_type,
                                                              hbrt_element_type_t element_type,
                                                              hbrt_dimension_t aligned_dim, bool convert_endianness,
                                                              hbrt_dimension_t coord);

/**
 * Add padding to data
 * @param data_with_padding data with padding will be written to this address
 * @param dim_with_padding dimensions of data with padding.  should be 4-element uint32 array.
 * @param data_wo_padding source data without padding
 * @param dim_wo_padding dimensions of data without padding.  should be 4-element uint32 array.
 * @param element_type data element type
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType, ::hbrtErrorMemoryOverflow
 */
HBDK_PUBLIC extern hbrt_error_t hbrtAddPadding(void *data_with_padding, hbrt_dimension_t dim_with_padding,
                                               const void *data_wo_padding, hbrt_dimension_t dim_wo_padding,
                                               hbrt_element_type_t element_type);

/**
 * Remove padding from data
 * @param data_wo_padding data without padding will be written to this address
 * @param dim_wo_padding dimensions of data without padding.  should be 4-element uint32 array.
 * @param data_with_padding source data with padding
 * @param dim_with_padding dimensions of data with padding.  should be 4-element uint32 array.
 * @param element_type data element type
 * @return ::hbrtSuccess, ::hbrtErrorIllegalLayout, ::hbrtErrorIllegalElementType, ::hbrtErrorMemoryOverflow
 */
HBDK_PUBLIC extern hbrt_error_t hbrtRemovePadding(void *data_wo_padding, hbrt_dimension_t dim_wo_padding,
                                                  const void *data_with_padding, hbrt_dimension_t dim_with_padding,
                                                  hbrt_element_type_t element_type);

/**
 * Convert the endianss in [input, input+size) and store in output.
 * @param output the result will be written to this address
 * @param input source data address
 * @param size byte size of source data
 * @return ::hbrtSuccess, ::hbrtErrorMemoryOverflow
 * @note Input and output cannot have overlap, unless they are the same address.
 */
HBDK_PUBLIC extern hbrt_error_t hbrtConvertEndianness(void *output, const void *input, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // HBDK_LAYOUT_H_
