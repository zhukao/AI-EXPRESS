/**
 * Copyright (c) 2017 Horizon Robotics. All rights reserved.
 * @file      xwarehouse_data.h
 * @brief
 * @author    youliang.fei
 * @email     youliang.fei@horizon.ai
 * @version   2.0.0
 * @date      2018.9.26
 */

#ifndef INCLUDE_XWAREHOUSE_XWAREHOUSE_DATA_H_
#define INCLUDE_XWAREHOUSE_XWAREHOUSE_DATA_H_
#include <stdint.h>

#define HOBOT_ID_LENGTH (48)
#define HOBOT_URI_LENGTH (256)
#define HOBOT_FILENAME_LENGTH (128)
#define HOBOT_VERSION_LENGTH (256)

#ifdef __cplusplus
#define WAREHOUSE_NAMESPACE  namespace hobot { namespace warehouse {
#define WAREHOUSE_NAMESPACE_END }}
#else
#define WAREHOUSE_NAMESPACE
#define WAREHOUSE_NAMESPACE_END
#include <stdbool.h>
#endif

WAREHOUSE_NAMESPACE

/**
 * @brief 错误码
 */
typedef enum {
  Errcode_Ok = 0,
  Errcode_Param_Error = 1000,
  Errcode_Init_Failed = 1001,
  Errcode_Insert_Set_Failed = 1002,
  Errcode_Delete_Set_Failed = 1003,
  Errcode_Update_Threshold_Failed = 1004,
  Errcode_List_Set_Failed = 1005,
  Errcode_Load_Set_Failed = 1006,
  Errcode_SourceType_Not_Matched = 1007,
  Errcode_Set_Init_Failed = 1010,
  Errcode_Set_Is_Existed = 1011,
  Errcode_Set_Is_Not_Existed = 1012,
  Errcode_Feature_Size_Not_Match = 1013,
  Errcode_Add_Record_Failed = 1014,
  Errcode_Delete_Record_Failed = 1015,
  Errcode_Update_Record_Failed = 1016,
  Errcode_Add_Feature_Failed = 1017,
  Errcode_Delete_Feature_Failed = 1018,
  Errcode_Update_Feature_Failed = 1019,
  Errcode_Id_is_Existed = 1020,
  Errcode_Id_is_not_Existed = 1021,
  Errcode_Decrypt_Failed = 1022,
} HobotXWHErrCode;

/**
 * @brief 底库加载的数据源类型
 */
typedef enum {
  // FILE = 0,  // 文件(预留，本期不支持)
  SQLITE = 1,  // 嵌入式数据库,特征数据保存到数据库和内存
  // MONGODB = 2,  // mongodb,特征数据保存到数据库和内存
  MEMORY = 3,  // 只加载到内存
  // FILE_GENERATE = 4, // 底库生成
} HobotXWHDataSourceType;

/**
 * @brief 底库从文件加载，文件内容序列化的方式
 */
typedef enum {
  CVS = 0,  // 普通的CVS规范格式文件
  PROTO,    // protobuf序列化化后存储的文件
} HobotXWHSerializeType;

typedef struct {
  // 存储为文件需要指定文件的格式
  HobotXWHSerializeType serialize_type_;
  char file_name_[HOBOT_FILENAME_LENGTH];
} HobotXWHLibraryFileInfo;

/**
 * @brief 数据源配置
 */
typedef struct {
  // 数据源类型
  HobotXWHDataSourceType type_;
  // sqlite数据库文件存储的目录!!!!!
  char db_file_[HOBOT_FILENAME_LENGTH];
  // 从文件加载的底库文件
  HobotXWHLibraryFileInfo zip_file_;
  // 是否校验属性(目前只校验人种)，0:不校验，1:校验人种属性
  int is_check_attr_;
} HobotXWHDataSourceInfo;

/**
 * @brief 特征
 */
typedef struct {
  // 特征对应的key
  // 检索的时候这个值可以为空
  char img_uri_[HOBOT_URI_LENGTH];
  // 特征属性，bit0等于0为定点，等于1为浮点，bit1~bit5表示shift值,
  // bit6等于0表示加密，等于1表示非加密，bit7~bit31 reserved
  int feature_attr_;
  // 特征向量大小
  int size_;
  // 特征值
  void *feature_;
} HobotXWHFeature;

/**
 * @brief 记录
 */
typedef struct {
  // 记录id,不为空
  char id_[HOBOT_ID_LENGTH];
  // 特征数组大小
  int size_;
  // 特征数组指针
  HobotXWHFeature *features_;
} HobotXWHRecord;

/**
 * @brief 检索的条件
 */
typedef struct {
  int top_n_;
  float distance_threshold_;
  float similar_threshold_;
  // 待检索的特征数组大小
  int size_;
  // 待检索的特征数组指针
  HobotXWHFeature *features_;
} HobotXWHSearchParam;

/**
 * @brief 检索的结果信息
 */
typedef struct {
  char id_[HOBOT_ID_LENGTH];
  // 距离
  float distance_;
  // 相似度
  float similar_;
} HobotXWHIdScore;

/**
 * @brief feature_set 信息
 */
typedef struct {
  char set_name_[HOBOT_ID_LENGTH];
  // 模型版本和加密key版本
  char model_version_[HOBOT_VERSION_LENGTH];
  int feature_size_;
  float distance_threshold_;
  float similar_threshold_;
  char db_file_[HOBOT_FILENAME_LENGTH];
  int is_check_attr_;
} HobotXWHSetInfo;

/**
 * @brief set信息
 */
typedef struct {
  // 分库个数
  int num_;
  // 分库数据指针
  HobotXWHSetInfo *sets_;
} HobotXWHListSetResult;

/**
 * @brief 获取所有的记录结果
 */
typedef struct {
  // 返回的记录数组大小
  int num_;
  // 记录数组指针
  HobotXWHRecord *record_;
} HobotXWHListRecordResult;

/**
 * @brief 获取一个记录下所有的特征
 */
typedef struct {
  // 单条记录
  HobotXWHRecord record_;
} HobotXWHListFeatureResult;

/**
 * @brief 检索的结果
 */
typedef struct {
  // 是否匹配到
  bool is_matched_;
  // 接口返回的结果数组大小
  int num_;
  // id_score数组指针
  HobotXWHIdScore *id_score_;
} HobotXWHSearchResult;

/**
 * @brief 1:1比对结果
 */
typedef struct {
  // 是否匹配
  bool is_matched_;
  // 距离
  float distance_;
  // 相似度
  float similar_;
} HobotXWHCompareResult;

WAREHOUSE_NAMESPACE_END

#endif  // INCLUDE_XWAREHOUSE_XWAREHOUSE_DATA_H_
