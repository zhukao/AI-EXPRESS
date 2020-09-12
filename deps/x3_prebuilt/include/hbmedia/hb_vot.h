/*****************************************************************************
 * Copyright (c) 2019 Horizon Robotics.
 * Description: HAPI 视频输出模块头文件
 * Author: xiaolin.huang
 * Version:
 * Date: 2019-12-12 15:23:05
 * LastEditTime: 2019-12-23 17:35:21
 * History:
 *****************************************************************************/

#ifndef __HB_VOT_H__
#define __HB_VOT_H__

#include <stdlib.h>
#include "hb_common_vot.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// 设置视频输出公共属性
int HB_VOT_SetPubAttr(uint8_t dev, const VOT_PUB_ATTR_S *pstPubAttr);
// 获取视频输出公共属性
int HB_VOT_GetPubAttr(uint8_t dev, VOT_PUB_ATTR_S *pstPubAttr);
// 启用视频输出设备
int HB_VOT_Enable(uint8_t dev);
// 禁用视频输出设备
int HB_VOT_Disable(uint8_t dev);
// 设置LCD背光亮度
int HB_VOT_SetLcdBackLight(uint8_t dev, uint32_t backlight);
// 设置视频层属性
int HB_VOT_SetVideoLayerAttr(uint8_t layer,
                             const VOT_VIDEO_LAYER_ATTR_S *pstLayerAttr);
// 获取视频层属性
int HB_VOT_GetVideoLayerAttr(uint8_t layer,
                             VOT_VIDEO_LAYER_ATTR_S *pstLayerAttr);
// 使能视频层
int HB_VOT_EnableVideoLayer(uint8_t layer);
// 禁止视频层
int HB_VOT_DisableVideoLayer(uint8_t layer);
// 设置视频层输出图像效果
int HB_VOT_SetVideoLayerCSC(uint8_t layer, const VOT_CSC_S *pstVideoCSC);
// 获取视频层输出图像效果
int HB_VOT_GetVideoLayerCSC(uint8_t layer, VOT_CSC_S *pstVideoCSC);
// int HB_VOT_SetVideoLayerRotation(uint8_t layer, ROTATION_E enRotation);
// int HB_VOT_GetVideoLayerRotation(uint8_t layer, ROTATION_E *penRotation);
// 设置视频层放大属性
int HB_VOT_SetVideoLayerUpScale(uint8_t layer,
                                const VOT_UPSCALE_ATTR_S *pstUpScaleAttr);
// 获取视频层放大属性
int HB_VOT_GetVideoLayerUpScale(uint8_t layer,
                                VOT_UPSCALE_ATTR_S *pstUpScaleAttr);
// 视频层的通道设置属性的开始
int HB_VOT_BatchBegin(uint8_t layer);
// 视频层的通道设置属性的结束
int HB_VOT_BatchEnd(uint8_t layer);
int HB_VOT_GetScreenFrame(uint8_t layer, void *pstVFrame,
                          int millisec);
int HB_VOT_ReleaseScreenFrame(uint8_t layer,
                              const void *pstVFrame);
// 设置视频输出通道属性
int HB_VOT_SetChnAttr(uint8_t layer, uint8_t chn,
                      const VOT_CHN_ATTR_S *pstChnAttr);
// 获取视频输出通道属性
int HB_VOT_GetChnAttr(uint8_t layer, uint8_t chn, VOT_CHN_ATTR_S *pstChnAttr);
// 设置视频输出通道高级属性
int HB_VOT_SetChnAttrEx(uint8_t layer, uint8_t chn,
                        const VOT_CHN_ATTR_EX_S *pstChnAttrEx);
// 获取视频输出通道高级属性
int HB_VOT_GetChnAttrEx(uint8_t layer, uint8_t chn,
                        VOT_CHN_ATTR_EX_S *pstChnAttrEx);
// 启用视频输出通道
int HB_VOT_EnableChn(uint8_t layer, uint8_t chn);
// 禁用视频输出通道
int HB_VOT_DisableChn(uint8_t layer, uint8_t chn);
// 设置视频输出通道裁剪属性
int HB_VOT_SetChnCrop(uint8_t layer, uint8_t chn,
                      const VOT_CROP_INFO_S *pstCropInfo);
// 获取视频输出通道裁剪属性
int HB_VOT_GetChnCrop(uint8_t layer, uint8_t chn, VOT_CROP_INFO_S *pstCropInfo);
// 设置视频输出通道显示坐标
int HB_VOT_SetChnDisplayPosition(uint8_t layer, uint8_t chn,
                                 const POINT_S *pstDispPos);
// 获取视频输出通道显示坐标
int HB_VOT_GetChnDisplayPosition(uint8_t layer, uint8_t chn,
                                 POINT_S *pstDispPos);
// int HB_VOT_GetChnFrame(uint8_t layer, uint8_t chn, VIDEO_FRAME_INFO_S
// *pstFrame,
//                        int millisec);
// int HB_VOT_ReleaseChnFrame(uint8_t layer, uint8_t chn,
//                            const VIDEO_FRAME_INFO_S *pstFrame);
int HB_VOT_SetChnFrameRate(uint8_t layer, uint8_t chn, int frame_rate);
int HB_VOT_GetChnFrameRate(uint8_t layer, uint8_t chn, int *pframe_rate);
// 显示指定通道
int HB_VOT_ShowChn(uint8_t layer, uint8_t chn);
// 隐藏指定通道
int HB_VOT_HideChn(uint8_t layer, uint8_t chn);
int HB_VOT_SendFrame(uint8_t layer, uint8_t chn, void *pstVFrame,
                     int millisec);
int HB_VOT_ClearChnBuf(uint8_t layer, uint8_t chn, HB_BOOL bClrAll);

int HB_VOT_BindVps(uint8_t vpsGroup, uint8_t vpsChn, uint8_t layer, uint8_t chn);

int HB_VOT_EnableWB(VOT_WB votWb);
int HB_VOT_DisableWB(VOT_WB votWb);
int HB_VOT_GetWBAttr(VOT_WB votWb, VOT_WB_ATTR_S *pstWBAttr);
int HB_VOT_SetWBAttr(VOT_WB votWb, VOT_WB_ATTR_S *pstWBAttr);
int HB_VOT_GetWBFrame(VOT_WB votWb, void* pstVFrame, int millisec);
int HB_VOT_ReleaseWBFrame(VOT_WB votWb, void* pstVFrame);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HB_VOT_H__ */
