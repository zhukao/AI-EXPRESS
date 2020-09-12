/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.12.10
 */

#ifndef __HB_VENC_H__
#define __HB_VENC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

    
 //#include "hb_common.h"
// #include "hb_comm_video.h"
#include "hb_comm_venc.h"
// #include "hb_comm_vb.h"


typedef int32_t VENC_CHN;


 int32_t HB_VENC_Module_Init(void);
 int32_t HB_VENC_Module_Uninit(void);

/*****************************************************************************
 * 函数描述:创建视频编码通道
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstAttr:编码通道属性指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr);

/*****************************************************************************
 * 函数描述:销毁编码通道。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_DestroyChn(VENC_CHN VeChn);

/*****************************************************************************
 * 函数描述:复位通道。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_ResetChn(VENC_CHN VeChn);

/*****************************************************************************
 * 函数描述:开启编码通道接收输入图像，允许指定接收帧数，超出指定的帧数后自动停止接收图像。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstAttr:接收图像参数结构体指针，用于指定需要接收的图像帧数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_StartRecvFrame(VENC_CHN VeChn,
                               const VENC_RECV_PIC_PARAM_S *pstRecvParam);

/*****************************************************************************
 * 函数描述:停止编码通道接收输入图像。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_StopRecvFrame(VENC_CHN VeChn);

/*****************************************************************************
 * 函数描述:
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstAttr:编码通道属性指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetChnAttr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr);

/*****************************************************************************
 * 函数描述:
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstAttr:编码通道属性指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetChnAttr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr);

/*****************************************************************************
 * 函数描述:获取编码的码流。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstStream:码流结构体指针。
 *    s32MilliSec:获取码流超时时间，取值范围：[-1,+ ∞ )
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
int32_t HB_VENC_GetStream(VENC_CHN VeChn, VIDEO_STREAM_S *pstStream,
                          int32_t s32MilliSec);

/*****************************************************************************
 * 函数描述:释放码流缓存。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstStream:码流结构体指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_ReleaseStream(VENC_CHN VeChn, VIDEO_STREAM_S *pstStream);

/*****************************************************************************
 * 函数描述:用户发送原始图像进行编码。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstFrame:原始图像信息结构指针。
 *    s32MilliSec:发送图像超时时间，取值范围：[-1,+ ∞ )
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
int32_t HB_VENC_SendFrame(VENC_CHN VeChn, VIDEO_FRAME_S *pstFrame,
                          int32_t s32MilliSec);

/*****************************************************************************
 * 函数描述:请求 IDR 帧。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_RequestIDR(VENC_CHN VeChn);

/*****************************************************************************
 * 函数描述:获取编码通道对应的设备文件句柄。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *     fd:返回编码通道文件句柄。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetFd(VENC_CHN VeChn,int32_t *fd);

/*****************************************************************************
 * 函数描述:关闭编码通道对应的设备文件句柄。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *     fd:返回编码通道文件句柄。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_CloseFd(VENC_CHN VeChn, int32_t fd);

/*****************************************************************************
 * 函数描述:设置 H.264/H.265 通道的 Roi 配置属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstRoiAttr:对应 ROI 区域的配置。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetRoiAttr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr);

/*****************************************************************************
 * 函数描述:获取 H.264/H.265 通道的 Roi 配置属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstRoiAttr:对应 ROI 区域的配置。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetRoiAttr(VENC_CHN VeChn,
                           VENC_ROI_ATTR_S *pstRoiAttr);

/*****************************************************************************
 * 函数描述:设置 H.264 通道的 slice 分割属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstSliceSplit:H.264 码流 slice 分割参数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH264SliceSplit(VENC_CHN VeChn,
                                  const VENC_H264_SLICE_SPLIT_S *pstSliceSplit);

/*****************************************************************************
 * 函数描述:获取 H.264 通道的 slice 分割属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstSliceSplit:H.264 码流 slice 分割参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH264SliceSplit(VENC_CHN VeChn,
                                  VENC_H264_SLICE_SPLIT_S *pstSliceSplit);

/*****************************************************************************
 * 函数描述:设置 H.264 协议编码通道的帧内预测属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH264IntraPred:H.264 协议编码通道的帧内预测参数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH264IntraPred(
    VENC_CHN VeChn, const VENC_H264_INTRA_PRED_S *pstH264IntraPred);

/*****************************************************************************
 * 函数描述:获取 H.264 协议编码通道的帧内预测属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH264IntraPred:H.264 协议编码通道的帧内预测参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH264IntraPred(VENC_CHN VeChn,
                                 VENC_H264_INTRA_PRED_S *pstH264IntraPred);

/*****************************************************************************
 * 函数描述:设置 H.264 协议编码通道的变换、量化属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH264Trans:H.264 协议编码通道的变换、量化参数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH264Trans(VENC_CHN VeChn,
                             const VENC_H264_TRANS_S *pstH264Trans);

/*****************************************************************************
 * 函数描述:获取 H.264 协议编码通道的变换、量化属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH264Trans:H.264 协议编码通道的变换、量化参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH264Trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans);

/*****************************************************************************
 * 函数描述:设置 H.264 协议编码通道的熵编码属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH264EntropyEnc:H.264 协议编码通道的熵编码属性。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH264Entropy(VENC_CHN VeChn,
                               const VENC_H264_ENTROPY_S *pstH264EntropyEnc);

/*****************************************************************************
 * 函数描述:获取 H.264 协议编码通道的熵编码属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH264EntropyEnc:H.264 协议编码通道的熵编码属性。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH264Entropy(VENC_CHN VeChn,
                               VENC_H264_ENTROPY_S *pstH264EntropyEnc);

/*****************************************************************************
 * 函数描述:设置 H.264 协议编码通道的 dblk 类型。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH264Dblk:H.264 协议编码通道的 dblk 属性。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH264Dblk(VENC_CHN VeChn,
                            const VENC_H264_DBLK_S *pstH264Dblk);

/*****************************************************************************
 * 函数描述:获取 H.264 协议编码通道的 dblk 类型。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH264Dblk:H.264 协议编码通道的 dblk 属性。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH264Dblk(VENC_CHN VeChn, VENC_H264_DBLK_S *pstH264Dblk);

/*****************************************************************************
 * 函数描述:设置 H.264 协议编码通道的 VUI 配置。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH264Vui:H.264 协议编码通道的 Vui 属性。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui);

/*****************************************************************************
 * 函数描述:获取 H.264 协议编码通道的 VUI 配置。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH264Vui:H.264 协议编码通道的 Vui 属性。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH264Vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui);

/*****************************************************************************
 * 函数描述:设置 H.265 协议编码通道的 VUI 配置。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH265Vui:H.265 协议编码通道的 Vui 属性。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH265Vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui);

/*****************************************************************************
 * 函数描述:获取 H.265 协议编码通道的 VUI 配置。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH265Vui:H.265 协议编码通道的 Vui 属性。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH265Vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui);

/*****************************************************************************
 * 函数描述:获取通道码率控制高级参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstRcParam:通道码率控制参数指针。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetRcParam(VENC_CHN VeChn, VENC_RC_ATTR_S *pstRcParam);

/*****************************************************************************
 * 函数描述:设置通道码率控制高级参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstRcParam:通道码率控制参数指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetRcParam(VENC_CHN VeChn, const VENC_RC_ATTR_S *pstRcParam);

/*****************************************************************************
 * 函数描述:设置 H.264/H.265 编码通道高级跳帧参考参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstRefParam:H.264/H.265 编码通道高级跳帧参考参数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetRefParam(VENC_CHN VeChn,
                            const VENC_REF_PARAM_S *pstRefParam);

/*****************************************************************************
 * 函数描述:获取 H.264/H.265 编码通道高级跳帧参考参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstRefParam:H.264/H.265 编码通道高级跳帧参考参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetRefParam(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam);

/*****************************************************************************
 * 函数描述:设置 H265 通道的 slice 分割属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstSliceSplit:H.265 码流 slice 分割参数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH265SliceSplit(VENC_CHN VeChn,
                                  const VENC_H265_SLICE_SPLIT_S *pstSliceSplit);

/*****************************************************************************
 * 函数描述:获取 H265 通道的 slice 分割属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstSliceSplit:H.265 码流 slice 分割参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH265SliceSplit(VENC_CHN VeChn,
                                  VENC_H265_SLICE_SPLIT_S *pstSliceSplit);

/*****************************************************************************
 * 函数描述:设置 H265 通道的 PU 属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstPredUnit:H.265 协议编码通道的 PU 配置。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH265PredUnit(VENC_CHN VeChn,
                                const VENC_H265_PU_S *pstPredUnit);

/*****************************************************************************
 * 函数描述:获取 H265 通道的 PU 属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstPredUnit:H.265 协议编码通道的 PU 配置。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH265PredUnit(VENC_CHN VeChn, VENC_H265_PU_S *pstPredUnit);

/*****************************************************************************
 * 函数描述:设置 H265 通道的变换量化属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH265Trans:H.265 协议编码通道的变换量化配置。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH265Trans(VENC_CHN VeChn,
                             const VENC_H265_TRANS_S *pstH265Trans);

/*****************************************************************************
 * 函数描述:获取 H265 通道的变换量化属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH265Trans:H.265 协议编码通道的变换量化配置。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH265Trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans);

/*****************************************************************************
 * 函数描述:设置 H.265 通道的 Deblocking 属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH265Dblk:H.265 协议编码通道的 Deblocking 配置。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH265Dblk(VENC_CHN VeChn,
                            const VENC_H265_DBLK_S *pstH265Dblk);

/*****************************************************************************
 * 函数描述:获取 H.265 通道的 Deblocking 属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH265Dblk:H.265 协议编码通道的 Deblocking 配置。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH265Dblk(VENC_CHN VeChn, VENC_H265_DBLK_S *pstH265Dblk);

/*****************************************************************************
 * 函数描述:设置 H.265 通道的 Sao 属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstH265Sao:H.265 协议编码通道的 Sao 配置。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetH265Sao(VENC_CHN VeChn, const VENC_H265_SAO_S *pstH265Sao);

/*****************************************************************************
 * 函数描述:获取 H.265 通道的 Sao 属性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstH265Sao:H.265 协议编码通道的 Sao 配置。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetH265Sao(VENC_CHN VeChn, VENC_H265_SAO_S *pstH265Sao);

/*****************************************************************************
 * 函数描述:设置 P 帧刷 Islice 的参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstIntraRefresh:刷 Islice 参数。。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetIntraRefresh(VENC_CHN VeChn,
                                const VENC_INTRA_REFRESH_S *pstIntraRefresh);

/*****************************************************************************
 * 函数描述:获取 P 帧刷 Islice 的参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstIntraRefresh:刷 Islice 参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetIntraRefresh(VENC_CHN VeChn,
                                VENC_INTRA_REFRESH_S *pstIntraRefresh);

/*****************************************************************************
 * 函数描述:设置通道参数
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstChnParam:VeChn编码通道参数指针。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetChnParam(VENC_CHN VeChn,
                            const VENC_CHN_PARAM_S *pstChnParam);

/*****************************************************************************
 * 函数描述:获取通道参数
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstChnParam:VeChn编码通道参数指针。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetChnParam(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam);

/*****************************************************************************
 * 函数描述:设置 CU 模式选择的倾向性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstCuPrediction:CU 模式选择的倾向性参数。
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetCuPrediction(VENC_CHN VeChn,
                                const VENC_CU_PREDICTION_S *pstCuPrediction);

/*****************************************************************************
 * 函数描述:获取 CU 模式选择的倾向性。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstCuPrediction:CU 模式选择的倾向性参数。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetCuPrediction(VENC_CHN VeChn,
                                VENC_CU_PREDICTION_S *pstCuPrediction);
/*****************************************************************************
 * 函数描述:设置JPEG 协议编码通道的高级参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstJpegParam:JPEG 协议编码通道的高级参数集合。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetJpegParam(VENC_CHN VeChn, const VENC_JPEG_PARAM_S
*pstJpegParam);

/*****************************************************************************
 * 函数描述:获取JPEG 协议编码通道的高级参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstJpegParam:JPEG 协议编码通道的高级参数集合。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetJpegParam(VENC_CHN VeChn, VENC_JPEG_PARAM_S
*pstJpegParam);

/*****************************************************************************
 * 函数描述:设置JPEG 协议编码通道的高级参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstMjpegParam:MJPEG 协议编码通道的高级参数集合。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetMjpegParam(VENC_CHN VeChn, const VENC_MJPEG_PARAM_S
*pstMjpegParam);

/*****************************************************************************
 * 函数描述:获取MJPEG 协议编码通道的高级参数。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 * 输出参数:
 *    pstMjpegParam:MJPEG 协议编码通道的高级参数集合。
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetMjpegParam(VENC_CHN VeChn,
VENC_MJPEG_PARAM_S *pstMjpegParam);

/*****************************************************************************
 * 函数描述:查询编码通道状态
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)
 * 输出参数:
 *     pstStatus:编码通道的状态指针
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_QueryStatus(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus);
/*****************************************************************************
 * 函数描述:插入用户数据
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)
 * 输入参数:
 *     pu8Data:用户数据指针
 * 输入参数:
 *     u32Len:用户数据长度
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_InserUserData(VENC_CHN VeChn, uint8_t *pu8Data,
                            uint32_t u32Len);
/*****************************************************************************
 * 函数描述:支持用户发送原始图像及该图的QpMap表进行编码。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    pstFrame:原始图像信息结构指针。
 *    s32MilliSec:发送图像超时时间，取值范围：[-1,+ ∞ )
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
int32_t HB_VENC_SendFrameEx(VENC_CHN VeChn, const USER_FRAME_INFO_S *pstFrame,
                          int32_t s32MilliSec);

/*****************************************************************************
 * 函数描述: enable/disable idr frame。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    bEnableIDR: true enable idr frame, false disable idr frame
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_EnableIDR(VENC_CHN VeChn, HB_BOOL bEnableIDR);

/*****************************************************************************
 * 函数描述:。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_SetJpegEncodeMode(VENC_CHN VeChn,
    const HB_VENC_JPEG_ENCODE_MODE_E enJpegEncodeMode);

/*****************************************************************************
 * 函数描述:。
 * 输入参数:
 *    VeChn:视频编码通道号，[0, VENC_MAX_CHN_NUM)。
 *    
 * 输出参数:
 *     无
 * 返回值：
 *     0：成功
 *   非0：失败，参照错误码
 * 说明:
 *****************************************************************************/
int32_t HB_VENC_GetJpegEncodeMode(VENC_CHN VeChn,
    HB_VENC_JPEG_ENCODE_MODE_E *enJpegEncodeMode);

// int32_t HB_VENC_GetRoiAttrEx(VENC_CHN VeChn, HB_U32 u32Index,
// VENC_ROI_ATTR_EX_S *pstRoiAttrEx);

// int32_t HB_VENC_SetRoiAttrEx(VENC_CHN VeChn,
// const VENC_ROI_ATTR_EX_S *pstRoiAttrEx);

// int32_t HB_VENC_SetRoiBgFrameRate(VENC_CHN VeChn, const
// VENC_ROIBG_FRAME_RATE_S *pstRoiBgFrmRate);

// int32_t HB_VENC_GetRoiBgFrameRate(VENC_CHN VeChn, VENC_ROIBG_FRAME_RATE_S
// *pstRoiBgFrmRate);

// int32_t HB_VENC_SetH265Entropy(VENC_CHN VeChn, const VENC_H265_ENTROPY_S
// *pstH265Entropy);

// int32_t HB_VENC_GetH265Entropy(VENC_CHN VeChn,
// VENC_H265_ENTROPY_S *pstH265Entropy);

// int32_t HB_VENC_SetFrameLostStrategy(VENC_CHN VeChn, const VENC_FRAMELOST_S
// *pstFrmLostParam);

// int32_t HB_VENC_GetFrameLostStrategy(VENC_CHN VeChn,
// VENC_FRAMELOST_S *pstFrmLostParam);

// int32_t HB_VENC_SetSuperFrameStrategy(VENC_CHN VeChn, const
// VENC_SUPERFRAME_CFG_S *pstSuperFrmParam);

// int32_t HB_VENC_GetSuperFrameStrategy(VENC_CHN VeChn,VENC_SUPERFRAME_CFG_S
// *pstSuperFrmParam);

// int32_t HB_VENC_GetSSERegion(VENC_CHN VeChn, HB_U32 u32Index,VENC_SSE_CFG_S
// *pstSSECfg);

// int32_t HB_VENC_SetSSERegion(VENC_CHN VeChn, const VENC_SSE_CFG_S
// * pstSSECfg);

// int32_t HB_VENC_SetModParam(const VENC_PARAM_MOD_S *pstModParam);
// int32_t HB_VENC_GetModParam(VENC_PARAM_MOD_S *pstModParam);

// int32_t HB_VENC_GetForegroundProtect(VENC_CHN VeChn,VENC_FOREGROUND_PROTECT_S
// *pstForegroundProtect);

// int32_t HB_VENC_SetForegroundProtect(VENC_CHN
// VeChn,const VENC_FOREGROUND_PROTECT_S *pstForegroundProtect);

// int32_t HB_VENC_SetSceneMode(VENC_CHN VeChn, const VENC_SCENE_MODE_E
// enSceneMode);

// int32_t HB_VENC_GetSceneMode(VENC_CHN VeChn, VENC_SCENE_MODE_E
// *penSceneMode);

// int32_t HB_VENC_AttachVbPool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool);
// int32_t HB_VENC_DetachVbPool(VENC_CHN VeChn);

// int32_t HB_VENC_SetJpegEncodeMode(VENC_CHN VeChn, const
// VENC_JPEG_ENCODE_MODE_E enJpegEncodeMode);

// int32_t HB_VENC_GetJpegEncodeMode(VENC_CHN VeChn, VENC_JPEG_ENCODE_MODE_E
// *penJpegEncodeMode);

// int32_t HB_VENC_GetStreamBufInfo(VENC_CHN VeChn, VENC_STREAM_BUF_INFO_S
// *pstStreamBufInfo);

// int32_t HB_VENC_SetSkipBias(VENC_CHN VeChn, const  VENC_SKIP_BIAS_S *
// pstSkipBias);

// int32_t HB_VENC_GetSkipBias(VENC_CHN VeChn, VENC_SKIP_BIAS_S *
// pstSkipBias);

// int32_t HB_VENC_SetDeBreathEffect(VENC_CHN VeChn, const VENC_DEBREATHEFFECT_S
// * pstDeBreathEffect);

// int32_t HB_VENC_GetDeBreathEffect(VENC_CHN VeChn,
// VENC_DEBREATHEFFECT_S * pstDeBreathEffect);

// int32_t HB_VENC_SetHierarchicalQp(VENC_CHN VeChn, const
// VENC_HIERARCHICAL_QP_S
// * pstHierarchicalQp);

// int32_t HB_VENC_GetHierarchicalQp(VENC_CHN VeChn,
// VENC_HIERARCHICAL_QP_S * pstHierarchicalQp);

// int32_t HB_VENC_SetRcAdvParam(VENC_CHN VeChn, const VENC_RC_ADVPARAM_S
// *pstRcAdvParam);

// int32_t HB_VENC_GetRcAdvParam(VENC_CHN VeChn,
// VENC_RC_ADVPARAM_S *pstRcAdvParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HB_VENC_H__ */
