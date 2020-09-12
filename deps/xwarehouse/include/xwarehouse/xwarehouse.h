/**
 * Copyright (c) 2017 Horizon Robotics. All rights reserved.
 * @file      xwarehouse.h
 * @brief     xwarehouse sdk interface！
 * @author    youliang.fei
 * @email     youliang.fei@horizon.ai
 * @version   2.0.0
 * @date      2018.9.26
 */

#ifndef INCLUDE_XWAREHOUSE_XWAREHOUSE_H_
#define INCLUDE_XWAREHOUSE_XWAREHOUSE_H_

#include "xwarehouse/xwarehouse_data.h"
#ifdef _WIN32
#ifdef XWAREHOUSE_WIN_VER  // build
#define WAREHOUSE_EXPORT __declspec(dllexport)
#else // interface
#define WAREHOUSE_EXPORT __declspec(dllimport)
#endif //XWAREHOUSE_WIN_VER
#elif defined(__GNUC__) && __GNUC__ >= 4
#define WAREHOUSE_EXPORT __attribute__((visibility("default")))
#else
#define WAREHOUSE_EXPORT
#endif

#ifdef __cplusplus
#define HOBOT_XWAREHOUSE_API extern "C" WAREHOUSE_EXPORT
#else
#define HOBOT_XWAREHOUSE_API WAREHOUSE_EXPORT
#endif

WAREHOUSE_NAMESPACE

typedef int HobotXWHStatus;

/**
 * @warning 该SDK支持数据库加载底库并进行检索
 */

/**
 * @brief
 * @param data_source_info
 * @return
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHInit(const HobotXWHDataSourceInfo *data_source_info);

/**
 * @brief 创建一个底库
 * @param set_name
 * @param model_version
 * @param feature_size ：特征向量的大小
 * @param data_source_info
 * ：分库元信息配置,data_source_info的type要和HobotXWHInit里面的type一致
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHCreateSet(
    const char *set_name, const char *model_version, const int feature_size,
    const HobotXWHDataSourceInfo *data_source_info);

/**
 * @brief 删除一个底库，该接口会删除数据库中跟该底库相关的信息。
 * @param set_name
 * @param model_version
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHDropSet(const char *set_name, const char *model_version);

/**
 * @brief 列出当前加载的所有底库信息。
 * @param result
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHListSet(HobotXWHListSetResult **result);

/**
 * @brief 释放HobotXWHListSetResult指针
 * @param result
 * @return
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHReleaseSetResult(HobotXWHListSetResult **result);

/**
 * @brief 新增一条记录
 * @param set_name ： 底库名称
 * @param model_version ：底库模型版本
 * @param record ：记录
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHAddRecord(const char *set_name,
                                 const char *model_version,
                                 const HobotXWHRecord *record);

/**
 * @brief 新增某个记录中的特征
 * @param set_name ：底库名称
 * @param model_version ：底库模型版本
 * @param id ：记录id
 * @param feature ：特征
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHAddFeature(const char *set_name,
                                  const char *model_version, const char *id,
                                  const HobotXWHFeature *feature);

/**
 * @brief 更新一条记录
 * @param set_name ：底库名称
 * @param model_version ：底库模型版本
 * @param record ： 记录
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHUpdateRecord(const char *set_name,
                                    const char *model_version,
                                    const HobotXWHRecord *record);

/**
 * @brief 更新某个特征
 * @param set_name ：底库名称
 * @param model_version ：底库模型版本
 * @param id ：待更新的id
 * @param feature ： 待更新的特征
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHUpdateFeature(const char *set_name,
                                     const char *model_version, const char *id,
                                     const HobotXWHFeature *feature);

/**
 * @brief 删除某条记录
 * @param set_name ：底库名称
 * @param model_version ： 底库模型版本
 * @param id ：待删除的id
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHDeleteRecord(const char *set_name,
                                    const char *model_version, const char *id);

/**
 * @brief 删除某条记录中的某个特征
 * @param set_name ： 底库名称
 * @param model_version ：底库模型版本
 * @param id ：记录id
 * @param feature_uri ： 待删除的特征
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHDeleteFeature(const char *set_name,
                                     const char *model_version, const char *id,
                                     const char *feature_uri);

/**
 * @brief 列出某个底库中所有的记录。
 * @param set_name : 底库名称
 * @param model_version ： 底库模型版本
 * @param list_result
 * ：调用完需要调用HobotXWHReleaseListRecordResult释放list_result内存
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHListRecord(const char *set_name,
                                  const char *model_version,
                                  HobotXWHListRecordResult **list_result);

/**
 * @brief 释放HobotXWHListRecordResult，每次调用完HoboXWHtListRecord都要去释放
 * @param result
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHReleaseListRecordResult(
    HobotXWHListRecordResult **result);

/**
 * @brief 列出某个底库中某条记录的下的所有特征信息。
 * @param set_name ：底库名称
 * @param model_version ： 底库模型版本
 * @param id ：记录id
 * @param list_result
 * ：调用完需要调用HobotXWHReleaseListFeatureResult释放list_result内存
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HoboXWHListFeature(const char *set_name,
                                  const char *model_version, const char *id,
                                  HobotXWHListFeatureResult **list_result);

/**
 * @brief 释放HobotXWHListFeatureResult，每次调用完HobotXWHListFeature都要去释放
 * @param result
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHReleaseListFeatureResult(
    HobotXWHListFeatureResult **result);

/**
 * @brief 设置某个分库的阈值信息
 * @param set_name ： 底库名称
 * @param model_version ： 底库模型版本
 * @param distance_threshold ：距离阈值
 * @param similar_threshold ：相似度计算参数
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHSetThreshold(const char *set_name,
                                    const char *model_version,
                                    const float distance_threshold,
                                    const float similar_threshold);

/**
 * @brief 特征检索
 * @param set_name ：分库名称
 * @param model_version ：分库模型版本
 * @param search_param ：检索参数，详细见HobotXWHSearchParam结构体
 * @param search_result
 * ：检索结果，通过HobotXWHAllocSearchResult申请内存，通过HobotXWHReleaseSearchResult释放内存
 * @return
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHSearch(const char *set_name, const char *model_version,
                              const HobotXWHSearchParam *search_param,
                              HobotXWHSearchResult *search_result);

/**
 * @brief HobotXWHAllocSearchResult
 * 通过此函数申请获取HobotXWHSearchResult结构体指针，
 * 使用完后必须调用HobotReleaseSearchResult()释放资源
 * @return ：HobotXWHSearchResult指针
 */
HOBOT_XWAREHOUSE_API
HobotXWHSearchResult *HobotXWHAllocSearchResult();

/**
 * @brief 释放HobotXWHSearchResult
 * @param result
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHReleaseSearchResult(HobotXWHSearchResult **result);

/**
 * @brief 1：1特征比对
 * @param record1 ：记录中的id和特征的url可以为空
 * @param record2 ：记录中的id和特征的url可以为空
 * @param distance_threshold ：比对的阈值
 * @param similar_threshold ：相似度计算参数
 * @param compare_result ：比对的结果
 * @return
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHFeatureCompare1V1(const HobotXWHRecord *record1,
                                         const HobotXWHRecord *record2,
                                         const float distance_threshold,
                                         const float similar_threshold,
                                         HobotXWHCompareResult *compare_result);

/**
 * @brief 关闭SDK，释放资源。
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHClose();

/**
 * @brief 获取SDK版本
 * @return ：SDK版本信息
 */
HOBOT_XWAREHOUSE_API
const char *HobotXWHGetVersion();

/**
 * @brief 判断是否需要重新注册特征值
 * @param current_model_version
 * @param last_model_version
 * @param is_update_db
 * @return ：0：成功，失败返回错误码
 */
HOBOT_XWAREHOUSE_API
HobotXWHStatus HobotXWHCheckModelVersion(const char *current_model_version,
                                         const char *last_model_version,
                                         bool *is_update_db);
/**
 * @brief
 * @param in : license_path
 * @return ：license信息
 */
HOBOT_XWAREHOUSE_API
const char *HobotXWHGetLicense(const char *license_path);

WAREHOUSE_NAMESPACE_END

#endif  // INCLUDE_XWAREHOUSE_XWAREHOUSE_H_
