/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef HOBOTXSDK_XSTREAM_CAPI_H_
#define HOBOTXSDK_XSTREAM_CAPI_H_

#include "xstream_capi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *HobotXStreamCapiHandle;

/**
 * @brief Get version information
 * @return version字符串
 */
HOBOT_EXPORT
const char *HobotXStreamCapiGetVersion();

/**
 * @brief Set license path
 *
 * @param path [in] 存放license.txt的文件夹路径
 */
HOBOT_EXPORT
void HobotXStreamCapiSetLicensePath(const char *path);

/**
 * @brief Get license information
 * @return 授权信息描述
 */
HOBOT_EXPORT
const char *HobotXStreamCapiGetLicenseInfo();

/**
 * @brief 更新配置
 *
 * @param cfg_name [in] 配置名称
 * @param value [in] 属性值
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiSetGlobalConfig(const char *cfg_name, const char *value);

/**
 * @brief 初始化 SDK
 *
 * @param handle [out] 返回sdk句柄
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiInit(HobotXStreamCapiHandle *handle);

/**
 * @brief 结束sdk并释放所有资源
 *
 * @param handle [in] sdk句柄
 * @return int 错误码: 成功返回0, 否则返回负数
 */

HOBOT_EXPORT
int HobotXStreamCapiFinalize(HobotXStreamCapiHandle handle);

/**
 * @brief 同步处理单帧输入
 *
 * @param handle [in] sdk句柄
 * @param inputs [in] 指向输入帧结构体的指针
 * @param outputs [out]
 * 用来存储计算结果指针的指针，用完须调用HobotXStreamCapiDataListFree()释放
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiProcessSync(HobotXStreamCapiHandle handle,
                                const HobotXStreamCapiInputList *inputs,
                                HobotXStreamCapiDataList **outputs);

/**
 * @brief 异步接收HobotXStreamCapiProcessAsync()处理结果的回调函数类型
 *
 * @param handle [in] sdk句柄
 * @param result [in] SDK输出结果, 该指针将在回调结束时回收
 * @param pUserData [in] 用户数据指针，回调函数会原样返回
 */
typedef void (*HobotXStreamCapiCallback)(HobotXStreamCapiHandle handle,
                                         HobotXStreamCapiCallbackData *result,
                                         void *pUserData);

/**
 *
 * @brief 设置callback异步接收HobotXStreamCapiProcessAsync()处理结果
 *
 * @param handle [in] sdk句柄
 * @param callback [in] 回调函数
 * @param pUserData [in] 用户数据指针，回调函数会原样返回
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiSetCallback(HobotXStreamCapiHandle handle,
                                HobotXStreamCapiCallback callback,
                                void *pUserData);

/**
 * @brief 功能同HobotXStreamCapiProcessAsync，异步发送数据给SDK
 *
 * @param handle [in] sdk句柄
 * @param inputs [in]
 * 输入的数据，通过HobotXStreamCapiDataListAlloc()函数申请内存，并在
 * HobotXStreamCapiSmartListCallback() 回调方法中HobotXStreamCapiDataListFree()
 *
 *
 * @return int64_t 负数则为错误码，>=0则为sequence id
 */
HOBOT_EXPORT
int64_t HobotXStreamCapiProcessAsync(HobotXStreamCapiHandle handle,
                                     const HobotXStreamCapiInputList *inputs);

/**
 * @brief 配置设置项
 *
 * @param handle [in] sdk句柄
 * @param cfg_name [in] 配置名称
 * @param value [in] 属性值
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiSetConfig(HobotXStreamCapiHandle handle,
                              const char *cfg_name, const char *value);

/**
 * @brief 更新配置
 *
 * @param handle [in] sdk句柄
 * @param cfg_name [in] 配置名称
 * @param value [in] 属性值
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiUpdateConfig(HobotXStreamCapiHandle handle,
                                 const char *cfg_name, const char *value);

/**
 * @brief 查询配置
 *
 * @param handle [in] sdk句柄
 * @param cfg_name [in] 配置名称
 * @param pvalue [out]
 * 属性值输出地址，属性值字符串长度不会超过256个字节，所以请分配256字节内存空间
 * @return int 错误码: 成功返回0, 否则返回负数
 */
HOBOT_EXPORT
int HobotXStreamCapiGetConfig(HobotXStreamCapiHandle handle,
                              const char *cfg_name, char *pvalue,
                              uint32_t *length);

#ifdef __cplusplus
}
#endif

#endif  // HOBOTXSDK_XSTREAM_CAPI_H_
