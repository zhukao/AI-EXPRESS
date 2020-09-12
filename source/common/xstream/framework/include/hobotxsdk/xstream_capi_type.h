/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef HOBOTXSDK_XSTREAM_CAPI_TYPE_H_
#define HOBOTXSDK_XSTREAM_CAPI_TYPE_H_

#include "stddef.h"
#include "stdint.h"

#if !defined(HOBOT_EXPORT)
#ifdef HR_WIN
#ifdef HOBOT_FACEPOST_DLL_EXPORTS
#ifdef HOBOT_EXPORTS
#define HOBOT_EXPORT __declspec(dllexport)
#else
#define HOBOT_EXPORT __declspec(dllimport)
#endif  // #ifdef HOBOT_EXPORTS
#else
#define HOBOT_EXPORT
#endif  // #ifdef HOBOT_FACEPOST_DLL_EXPORTS
#else
#define HOBOT_EXPORT
#endif  // #ifdef HR_WIN
#endif  // #if !defined(HOBOT_EXPORT)

#ifdef __cplusplus
extern "C" {
#endif

#define DECLEAR_DEFINE_CAPI_FREE_PARENT(type)                                 \
  HOBOT_EXPORT                                                                \
  inline void HobotXStreamCapi##type##FreeParent(                             \
      HobotXStreamCapiData** parent) {                                        \
    HobotXStreamCapi##type##Free((HobotXStreamCapiBaseDataVector**)(parent)); \
  }

typedef struct HobotXStreamCapiBaseData_ {
  const char* type_;
  const char* name_;
  int error_code_;
  const char* error_detail_;
  void* context_;  //< 用来存储SDK内部上下文，在free HobotXStreamCapiData
                   // 时通知SDK释放context_指向的资源
} HobotXStreamCapiBaseData;

typedef HobotXStreamCapiBaseData HobotXStreamCapiData;

/**
 * @brief 释放DataList
 *
 * @param datalist [in, out] 释放datalist并把指针置空
 */
HOBOT_EXPORT
void HobotXStreamCapiDataFree(HobotXStreamCapiData** data);

/**
 * @brief HobotXStreamCapiData数组结构
 *
 */
typedef struct HobotXStreamCapiDataList_ {
  size_t datas_size_;
  HobotXStreamCapiData* datas_[];
} HobotXStreamCapiDataList;

typedef HobotXStreamCapiDataList HobotXStreamCapiInputList;
typedef HobotXStreamCapiDataList HobotXStreamCapiOutputList;

/**
 * @brief HobotXStreamCapiBaseDataVector数组结构
 *
 */
typedef struct HobotXStreamCapiBaseDataVector_ {
  HobotXStreamCapiData parent_;
  HobotXStreamCapiDataList* datas_;
} HobotXStreamCapiBaseDataVector;

/**
 * @brief HobotXStreamCapiBaseDataVector构造
 *
 * @param length
 * @return HobotXStreamCapiBaseDataVector*
 */
HOBOT_EXPORT
HobotXStreamCapiBaseDataVector* HobotXStreamCapiBaseDataVectorAlloc(
    size_t length);

/**
 * @brief HobotXStreamCapiBaseDataVector释放
 *
 * @param data_vector
 */
HOBOT_EXPORT
void HobotXStreamCapiBaseDataVectorFree(
    HobotXStreamCapiBaseDataVector** data_vector);

/**
 * @brief 定义一个根据BaseDataVector.parent释放BaseDataVector的函数
 *
 */
DECLEAR_DEFINE_CAPI_FREE_PARENT(BaseDataVector);

/**
 * @brief 申请DataList
 *
 */
HOBOT_EXPORT
HobotXStreamCapiDataList* HobotXStreamCapiDataListAlloc(size_t length);

/**
 * @brief 释放DataList
 *
 * @param datalist [in, out] 释放datalist并把指针置空
 */
HOBOT_EXPORT
void HobotXStreamCapiDataListFree(HobotXStreamCapiDataList** datalist);

/**
 * @brief 异步callback返回的的数据结构定义
 *
 */
typedef struct HobotXStreamCapiCallbackData_ {
  int64_t sequence_id_;
  int error_code_;
  char* error_detail_;  //< 释放callbackdata时会被释放
  HobotXStreamCapiInputList* inputs_;  //< 是异步输入的数据指针，SDK不会主动释放
  HobotXStreamCapiDataList*
      outputs_;  //< SDK不会主动释放，需要用户使用完后主动调用
                 // HobotXStreamCapiDataListFree
} HobotXStreamCapiCallbackData;

#ifdef __cplusplus
}
#endif

#endif  // HOBOTXSDK_XSTREAM_CAPI_TYPE_H_
