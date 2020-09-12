/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2016-2018 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/
#ifndef __HB_VP_H
#define __HB_VP_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

// #include <stdint.h>
#include "ion.h"

#define VP_MAX_PUB_POOLS 16 

#define VP_INVALID_BLOCKID	-1
#define VP_INVALID_POOLID	-2
#define HB_ERR_VP_NOT_PERM	-3
#define HB_ERR_VP_UNEXIST	-4
#define HB_ERR_VP_BUSY		-5
#define HB_ERR_SYS_BUSY		-6
#define HB_ERR_SYS_ILLEGAL_PARAM	-7
#define HB_ERR_SYS_NOMEM	-8
#define HB_ERR_VP_ILLEGAL_PARAM		-9

typedef struct HB_VP_POOL_CONFIG_S {
    uint64_t u64BlkSize;
    uint32_t u32BlkCnt;
    uint32_t cacheEnable;
} VP_POOL_CONFIG_S;

typedef struct HB_VP_CONFIG_S {
    uint32_t u32MaxPoolCnt;
    VP_POOL_CONFIG_S pubPool[VP_MAX_PUB_POOLS];
} VP_CONFIG_S;

typedef struct HB_VP_AUXILIARY_CONFIG_S {
    int u32AuxiliaryConfig;
} VP_AUXILIARY_CONFIG_S;

int HB_VP_SetConfig(VP_CONFIG_S *VpConfig);
int HB_VP_GetConfig(VP_CONFIG_S *VpConfig);
int HB_VP_Init(void);
int HB_VP_Exit(void);
uint32_t HB_VP_CreatePool(VP_POOL_CONFIG_S *VpPoolCfg);
int HB_VP_DestroyPool(uint32_t Pool);
uint32_t HB_VP_GetBlock(uint32_t Pool, uint64_t u64BlkSize);
int HB_VP_ReleaseBlock(uint32_t Block);
uint32_t HB_VP_PhysAddr2Block(uint64_t u64PhyAddr);
uint64_t HB_VP_Block2PhysAddr(uint32_t Block);
uint32_t HB_VP_Block2PoolId(uint32_t Block);
int HB_VP_MmapPool(uint32_t Pool);
int HB_VP_MunmapPool(uint32_t Pool);
int HB_VP_GetBlockVirAddr(uint32_t Pool, uint64_t u64PhyAddr,
                            void **ppVirAddr);
int HB_VP_InquireUserCnt(uint32_t Block);
int HB_VP_SetAuxiliaryConfig
                    (const VP_AUXILIARY_CONFIG_S *pstAuxiliaryConfig);
int HB_VP_DmaCopy(void *dstPaddr, void *srcPaddr, uint32_t len);
int HB_SYS_Alloc(uint64_t *pu64PhyAddr, void **ppVirAddr, uint32_t u32Len);
int HB_SYS_AllocCached(uint64_t *pu64PhyAddr, void **ppVirAddr, uint32_t u32Len);
int HB_SYS_Free(uint64_t u64PhyAddr, void *pVirAddr);
int HB_SYS_CacheInvalidate(uint64_t pu64PhyAddr, void *pVirAddr, uint32_t u32Len);
int HB_SYS_CacheFlush(uint64_t pu64PhyAddr, void *pVirAddr, uint32_t u32Len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // __HB_VP_H
