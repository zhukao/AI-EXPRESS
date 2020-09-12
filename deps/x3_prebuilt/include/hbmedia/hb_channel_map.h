/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.12.16
 */

#ifndef _HB_CHANNEL_MAP_H__
#define _HB_CHANNEL_MAP_H__

#include "./hb_media_codec.h"
#include "./hb_type.h"
// #include "vdi_osal.h"


#define HB_ERR_CHANNEL_MAP_NULL_PTR -1
#define HB_ERR_CHANNEL_MAP_NOT_PERM -2
#define HB_ERR_CHANNEL_MAP_INVALID_CHNID -3
#define HB_ERR_CHANNEL_MAP_EXIST -4
#define HB_ERR_CHANNEL_MAP_UNEXIST -5


typedef struct HB_CHANNEL_CTX_MAP {
  int32_t s32ChannelId;
  media_codec_context_t* Context;
} CHANNEL_CTX_MAP_S;

typedef struct HB_VECHN_MAP_MGR {
  HB_BOOL bInitFlag;
  int32_t u32ChnNum;
  int32_t u32ChnMaxNum;
  CHANNEL_CTX_MAP_S *stChannalContextMap;

  int32_t (*InitChnMap)(struct HB_VECHN_MAP_MGR* pstChnCtxMapMgr);

  int32_t (*BindChnMap)(int32_t s32ChannelId, media_codec_context_t* ctx,
                          struct HB_VECHN_MAP_MGR* pstChnCtxMapMgr);

  int32_t (*FindCtxByChn)(int32_t s32ChannelId, media_codec_context_t** ctx,
                            struct HB_VECHN_MAP_MGR* pstChnCtxMapMgr);

  int32_t (*UnBindChnCtxMap)(int32_t s32ChannelId,
                               struct HB_VECHN_MAP_MGR* pstChnCtxMapMgr);

  int32_t (*UnBindChnMap)(struct HB_VECHN_MAP_MGR* pstChnCtxMapMgr);

  video_mutex_t mutexLock;
} CHANNEL_CTX_MAP_MGR;


extern CHANNEL_CTX_MAP_MGR g_VencChnMapMgr;

extern CHANNEL_CTX_MAP_MGR g_VdecChnMapMgr;

#endif   //  _HB_CHANNEL_MAP_H__
