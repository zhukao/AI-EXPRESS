/**
 * @file gtest_util.cpp
 * @author Jet.Sun (shuhuan.sun@horizon.ai)
 * @brief gtest cases for util functions
 * @version 0.1
 * @date 2019-12-04
 *
 * @Horizon Robotics Copyright (c) 2019
 *
 */

#include <locale>
#include <string>

#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type_util.h"

#include "gtest/gtest.h"

TEST(HorizonVisionMemDup, check_data) {
  const char *src_mem = "012345678";
  const size_t mem_len = 8;
  char *mem_addr = HorizonVisionMemDup(src_mem, mem_len);
  EXPECT_NE(nullptr, mem_addr);
  {
    auto &f = std::use_facet<std::collate<char>>(std::locale());
    EXPECT_EQ(
        0, f.compare(src_mem, src_mem + mem_len, mem_addr, mem_addr + mem_len));
  }
  std::free(mem_addr);
}
