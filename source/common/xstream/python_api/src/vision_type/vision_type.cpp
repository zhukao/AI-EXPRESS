/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      bbox_type.cpp
 * @brief     main function
 * @author    Zhuoran Rong (zhuoran.rong@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include "pybind11/pybind11.h"

#include "hobotxsdk/xstream_data.h"
#include <cassert>  // NOLINT
#include <iostream> // NOLINT

#include "base_data_warp.h" // NOLINT

namespace xstream {

extern BaseDataWrapperPtr bbox_builder(float x1, float y1, float x2, float y2);
extern void bbox_dump(BaseDataWrapper *bbox);
// extern void imageframe_dump(BaseDataWrapper *bbox);

namespace py = pybind11;

PYBIND11_MODULE(vision_type, m) {
  // optional module docstring
  m.doc() = "xstream sdk vision data type";

  m.def("bbox", &bbox_builder, "A function which make bbox");
  m.def("bbox_dump", &bbox_dump, "dump bbox fields to python");
  // m.def("imageframe_dump", &imageframe_dump);
}

}  // namespace xstream
