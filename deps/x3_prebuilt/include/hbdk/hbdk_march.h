
// clang-format off
// @formatter:off
//===----- hbdk_march.h - Runtime definition and helper function ---*- C -*-===//
// Automatically generated. DO NOT EDIT
//
//                     The HBDK Compiler Infrastructure
//
// This file is subject to the terms and conditions defined in file
// 'LICENSE.txt', which is part of this source code package.
//
//===-----------------------------------------------------------------------===//

#ifndef HBDK_MARCH_H_
#define HBDK_MARCH_H_
#pragma once

#include "hbdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HBRT_MARCH_NAME_LENGTH (3U)

#define MARCH_NAME_LENGTH \
HBRT_PRAGMA_WARN( \
"MARCH_NAME_LENGTH has been superseded by HBRT_MARCH_NAME_LENGTH since 3.6.1, and will be removed in a future release")\
HBRT_MARCH_NAME_LENGTH

#define HBRT_MAGIC_HEADER_SIZE (16U)

#define MAGIC_HEADER_SIZE \
HBRT_PRAGMA_WARN( \
"MAGIC_HEADER_SIZE has been superseded by HBRT_MAGIC_HEADER_SIZE since 3.6.1, and will be removed in a future release")\
HBRT_MAGIC_HEADER_SIZE

typedef enum {
  HBRT_MARCH_UNKNOWN = ('?' << 0) + ('?' << 8) + ('?' << 16),
  HBRT_MARCH_BERNOULLI = ('X' << 0) + ('2' << 8) + (' ' << 16),
  HBRT_MARCH_BERNOULLI2 = ('X' << 0) + ('2' << 8) + ('A' << 16),
} hbrt_march_t;

typedef hbrt_march_t MARCH HBRT_DEPRECATED_NAME(hbrt_march_t, MARCH, 3.7.1);

/* Following definition is aim to keep compatible with 3.5.6 or earlier version */
#define MARCH_X2 \
HBRT_PRAGMA_WARN( \
"MARCH_X2 has been superseded by HBRT_MARCH_BERNOULLI since 3.6.1, and will be removed in a future release")\
HBRT_MARCH_BERNOULLI

#define MARCH_X2A \
HBRT_PRAGMA_WARN( \
"MARCH_X2A has been superseded by HBRT_MARCH_BERNOULLI2 since 3.6.1, and will be removed in a future release")\
HBRT_MARCH_BERNOULLI2



#ifdef __cplusplus
}
#endif

#endif // HBDK_MARCH_H_
