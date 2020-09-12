
/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.12.16
 */

#ifndef  __HB_VDEC_H__
#define  __HB_VDEC_H__

//#include "hb_common.h"
//#include "hb_comm_video.h"
//#include "hb_comm_vb.h"
#include "hb_comm_vdec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

int32_t HB_VDEC_Module_Init(void);
int32_t HB_VDEC_Module_Uninit(void);

/*****************************************************************************
 * 函数描述:创建视频解码通道
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *    pstAttr:解码通道属性指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_CreateChn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);

/*****************************************************************************
 * 函数描述:销毁视频解码通道
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_DestroyChn(VDEC_CHN VdChn);

/*****************************************************************************
 * 函数描述:获取视频解码通道参数
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstAttr:解码通道属性指针。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_GetChnAttr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr);

/*****************************************************************************
 * 函数描述:设置视频解码通道参数
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *    pstAttr:解码通道属性指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_SetChnAttr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);


/*****************************************************************************
 * 函数描述:解码器开始接收用户发送的码流
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_StartRecvStream(VDEC_CHN VdChn);


/*****************************************************************************
 * 函数描述:解码器停止接收用户发送的码流
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_StopRecvStream(VDEC_CHN VdChn);


/*****************************************************************************
 * 函数描述:复位视频解码通道
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_ResetChn(VDEC_CHN VdChn);

/*****************************************************************************
 * 函数描述:设置解码通道参数
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *    pstParam:解码通道参数指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
//int32_t HB_VDEC_SetChnParam(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S* pstParam);


/*****************************************************************************
 * 函数描述:获取解码通道参数
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstParam:解码通道参数指针。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
//int32_t HB_VDEC_GetChnParam(VDEC_CHN VdChn, VDEC_CHN_PARAM_S* pstParam);

/*****************************************************************************
 * 函数描述:向视频解码通道发送码流数据。
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *    pstStream:解码码流数据指针。
 *    s32MilliSec:送码流超时时间，取值范围：[-1,+ ∞ )
 *                    -1：阻塞。
 *                     0：非阻塞。
 *                大于 0：超时时间。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_SendStream(VDEC_CHN VdChn, const VIDEO_STREAM_S *pstStream,
                           int32_t s32MilliSec);

/*****************************************************************************
 * 函数描述:获取视频解码通道的解码图像。
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *    pstFrameInfo:获取的解码图像信息。
 *    s32MilliSec:获取图像超时时间，取值范围：[-1,+ ∞ )
 *                    -1：阻塞。
 *                     0：非阻塞。
 *                大于 0：超时时间。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_GetFrame(VDEC_CHN VdChn, VIDEO_FRAME_S *pstFrameInfo,
                         int32_t s32MilliSec);


/*****************************************************************************
 * 函数描述:释放视频解码通道的图像
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *    pstFrameInfo:解码后的图像信息指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_ReleaseFrame(VDEC_CHN VdChn, const VIDEO_FRAME_S *pstFrameInfo);


/*****************************************************************************
 * 函数描述:获取解码通道对应的设备文件句柄。
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 * 输出参数:
 *     fd:返回解码通道文件句柄。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_GetFd(VDEC_CHN VdChn, int32_t *fd);


/*****************************************************************************
 * 函数描述:关闭解码通道对应的设备文件句柄。
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)。
 *     fd:返回解码通道文件句柄。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_CloseFd(VDEC_CHN VdChn, int32_t fd);

/*****************************************************************************
 * 函数描述:查询解码通道状态
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)
 * 输出参数:
 *     pstStatus:解码通道的状态指针
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_QueryStatus(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus);

/*****************************************************************************
 * 函数描述:获取视频解码通道的用户数据
 * 输入参数:
 *    VdChn:视频通道号，[0, VDEC_MAX_CHN_NUM)
 *    s32MilliSec:获取用户数据方式标志，取值范围：
 *    -1：阻塞
 *     0：非阻塞
 *     正值：超时时间，没有上限值，以ms为单位动态属性
 * 输出参数:
 *     pstUserData:获取的解码用户数据
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_GetUserData(VDEC_CHN VdChn, VDEC_USERDATA_S *pstUserData,
        int32_t s32MilliSec);

/*****************************************************************************
 * 函数描述:释放用户数据
 * 输入参数:
 *    VdChn:视频解码通道号，[0, VDEC_MAX_CHN_NUM)
 * 输出参数:
 *     pstUserData:解码后的用户数据指针
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VDEC_ReleaseUserData(VDEC_CHN VdChn, VDEC_USERDATA_S *pstUserData);

//int32_t HB_VDEC_SetProtocolParam(VDEC_CHN VdChn, const VDEC_PRTCL_PARAM_S *pstParam);
//int32_t HB_VDEC_GetProtocolParam(VDEC_CHN VdChn,VDEC_PRTCL_PARAM_S *pstParam);

// int32_t HB_VDEC_SetUserPic(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstUsrPic);
// int32_t HB_VDEC_EnableUserPic(VDEC_CHN VdChn, HI_BOOL bInstant);
// int32_t HB_VDEC_DisableUserPic(VDEC_CHN VdChn);
//
// int32_t HB_VDEC_SetDisplayMode(VDEC_CHN VdChn, VIDEO_DISPLAY_MODE_E enDisplayMode);
// int32_t HB_VDEC_GetDisplayMode(VDEC_CHN VdChn, VIDEO_DISPLAY_MODE_E *penDisplayMode);
//
// int32_t HB_VDEC_SetRotation(VDEC_CHN VdChn, ROTATION_E enRotation);
// int32_t HB_VDEC_GetRotation(VDEC_CHN VdChn, ROTATION_E *penRotation);

// int32_t HB_VDEC_AttachVbPool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool);
// int32_t HB_VDEC_DetachVbPool(VDEC_CHN VdChn);
//
// int32_t HB_VDEC_SetModParam(const VDEC_MOD_PARAM_S *pstModParam);
// int32_t HB_VDEC_GetModParam(VDEC_MOD_PARAM_S *pstModParam);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __HB_VDEC_H__ */

