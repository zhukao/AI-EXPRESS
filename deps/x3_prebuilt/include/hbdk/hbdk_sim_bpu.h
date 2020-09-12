//
// Created by zhangyinan on 11/4/19.
//

#ifndef HBDK_SIM_BPU_H_
#define HBDK_SIM_BPU_H_
#pragma once

#include "hbdk_error.h"
#include "hbdk_march.h"
#include "hbdk_type.h"

#ifdef __cplusplus
extern "C" {
#endif

HBDK_PUBLIC hbrt_error_t hbrtSimBpu(uint32_t* ret_interrupt_num, const void* funccall, hbrt_march_t march);

#ifdef __cplusplus
}
#endif

#endif  // HBDK_SIM_BPU_H_
