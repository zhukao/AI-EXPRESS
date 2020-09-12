/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:yilong.liang
 * @email:
 * @date:
 */

#ifndef RGN_HB_RGN_H_
#define RGN_HB_RGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <hb_errno.h>
#include <hb_sys.h>

#include "hb_vio_interface.h"
#include "logging.h"


#define rgn_err(format, ...) pr_err(format"\n", ##__VA_ARGS__)
#define rgn_log(format, ...) pr_info(format"\n", ##__VA_ARGS__)
#define rgn_dbg(format, ...) pr_debug(format"\n", ##__VA_ARGS__)

#define ALIGN_NUM 8

#ifdef WIN32
#define ATTRIBUTE
#else
#define ATTRIBUTE __attribute__((aligned (ALIGN_NUM)))
#endif

#define RGN_HANDLE_MAX 108
#define PIPELINE_MAX_NUM 6
#define RGN_GROUP_MAX 8
#define RGN_BATCHHANDLE_MAX 18

#define IMAGE_MAX_WIDTH 4096
#define IMAGE_MAX_HEIGHT 4096
#define RGN_MIN_WIDTH 32
#define RGN_MIN_HEIGHT 2
#define STA_MIN_WIDTH 2
#define STA_MIN_HEIGHT 2
#define STA_MAX_WIDTH 255
#define STA_MAX_HEIGHT 255

typedef enum {
	EN_INVALID_CHNID = 1,
    EN_ILLEGAL_PARAM,
    EN_RGN_EXIST,
    EN_RGN_UNEXIST,
    EN_NULL_PTR,
    EN_NOMEM,
    EN_OPEN_FILE_FAIL,
    EN_INVALID_OPERATION,
    EN_PROCESS_FAIL
} EN_RGN_ERR_CODE_E;

#define HB_ERR_RGN_INVALID_CHNID		HB_DEF_ERR(HB_ID_RGN, EN_INVALID_CHNID)
#define HB_ERR_RGN_ILLEGAL_PARAM        HB_DEF_ERR(HB_ID_RGN, EN_ILLEGAL_PARAM)
#define HB_ERR_RGN_EXIST                HB_DEF_ERR(HB_ID_RGN, EN_RGN_EXIST)
#define HB_ERR_RGN_UNEXIST              HB_DEF_ERR(HB_ID_RGN, EN_RGN_UNEXIST)
#define HB_ERR_RGN_NULL_PTR             HB_DEF_ERR(HB_ID_RGN, EN_NULL_PTR)
#define HB_ERR_RGN_NOMEM                HB_DEF_ERR(HB_ID_RGN, EN_NOMEM)
#define HB_ERR_RGN_OPEN_FILE_FAIL       HB_DEF_ERR(HB_ID_RGN, EN_OPEN_FILE_FAIL)
#define HB_ERR_RGN_INVALID_OPERATION    \
                                HB_DEF_ERR(HB_ID_RGN, EN_INVALID_OPERATION)
#define HB_ERR_RGN_PROCESS_FAIL         HB_DEF_ERR(HB_ID_RGN, EN_PROCESS_FAIL)

typedef enum HB_RGN_TYPE_PARAM_E        /*region type*/
{
    OVERLAY_RGN,
    COVER_RGN
} RGN_TYPE_E;

typedef enum HB_RGN_CHN_ID_ATTR_E      /*ipu channel region attached to*/
{
	CHN_US,
	CHN_DS0,
    CHN_DS1,
    CHN_DS2,
    CHN_DS3,
    CHN_DS4,
    CHANNEL_MAX_NUM
}RGN_CHN_ID_E;

typedef enum HB_RGN_FONT_SIZE_ATTR_E    /*font size*/
{
	FONT_SIZE_SMALL = 1,
	FONT_SIZE_MEDIUM,
    FONT_SIZE_LARGE,
	FONT_SIZE_EXTRA_LARGE
}RGN_FONT_SIZE_E;

typedef enum HB_RGN_FONT_COLOR_ATTR_E   /*font color*/
{
	FONT_COLOR_WHITE = 1,
	FONT_COLOR_BLACK,
    FONT_COLOR_GREY,
    FONT_COLOR_BLUE,
    FONT_COLOR_GREEN,
    FONT_COLOR_YELLOW,
    FONT_COLOR_BROWN,
    FONT_COLOR_ORANGE,
    FONT_COLOR_PURPLE,
    FONT_COLOR_PINK,
    FONT_COLOR_RED,
    FONT_COLOR_CYAN,
    FONT_COLOR_DARKBLUE,
    FONT_COLOR_DARKGREEN,
    FONT_COLOR_DARKRED,
    FONT_KEY_COLOR = 16
}RGN_FONT_COLOR_E;

typedef enum HB_PIXEL_FORMAT_ATTR_E     /*pixel format*/
{
    PIXEL_FORMAT_VGA_4
} RGN_PIXEL_FORMAT_E;

typedef int32_t RGN_HANDLE;            /*region handle*/

typedef int32_t RGN_HANDLEGROUP;       /*region group handle*/

typedef struct HB_RGN_SIZE_ATTR_S       /*size attribute*/
{
    uint32_t u32Width;                /*width of size*/
    uint32_t u32Height;               /*height of size*/
} RGN_SIZE_S;

typedef struct HB_RGN_POINT_ATTR_S      /*point attribute*/
{
    uint32_t u32X;                    /*x-coordinate of point*/
    uint32_t u32Y;                    /*y-coordinate of point*/
}RGN_POINT_S;

typedef struct HB_RGN_RECT_ATTR_S       /*rectangle attribute*/
{
    uint32_t u32X;                    /*x-coordinate of rectangle*/
    uint32_t u32Y;                    /*y-coordinate of rectangle*/
    uint32_t u32Width;                /*width of rectangle*/
    uint32_t u32Height;               /*height of rectangle*/
} RGN_RECT_S;

typedef struct HB_RGN_OVERLAY_ATTR_S    /*overlay region attribute*/
{
    RGN_PIXEL_FORMAT_E enPixelFmt;    /*pixel format of overlay*/
    RGN_FONT_COLOR_E enBgColor;               /*background color of bitmap*/
    RGN_SIZE_S stSize;                /*size of overlay*/
}RGN_OVERLAY_S;

typedef struct HB_RGN_ATTR_S            /*region attribute*/
{
    RGN_TYPE_E enType;                /*type of region(cover, overlay)*/
    RGN_OVERLAY_S stOverlayAttr;      /*attribute of overlay*/
} RGN_ATTR_S;

typedef struct HB_RGN_CHN_S             /*channel struct*/
{
    uint32_t s32PipelineId;            /*pipeline id*/
    int32_t enChnId;             /*channel id*/
} RGN_CHN_S;

typedef struct HB_RGN_OVERLAY_CHN_ATTR_S         // channel overlay region
                                                // display attribute
{
    RGN_POINT_S stPoint;                      /*point of region*/
}RGN_OVERLAY_CHN_S;

typedef struct HB_RGN_COVER_CHN_ATTR_S          // channel cover region
                                                // display attribute
{
    RGN_RECT_S stRect;                        /*rectangle of cover region*/
    uint32_t u32Color;                        /*color of cover region*/
}RGN_COVER_CHN_S;

typedef union HB_RGN_CHN_ATTR_U                 /*channel region attribute*/
{
    RGN_OVERLAY_CHN_S stOverlayChn;           /*overlay region attribute*/
    RGN_COVER_CHN_S stCoverChn;               /*cover region attribute*/
} RGN_CHN_U;

typedef struct HB_RGN_CANVAS_INFO_S             /*canvas info*/
{
    RGN_SIZE_S stSize;                        /*size of canvas*/
    RGN_PIXEL_FORMAT_E enPixelFmt;            /*pixel format of canvas*/
    void *pAddr;                      /*address of canvas*/
}RGN_CANVAS_S;

typedef struct HB_RGN_CHN_ATTR_S                /*channel display attribute*/
{
    bool bShow;                               /*whether region display*/
    bool bInvertEn;                           /*enable invert*/
    RGN_CHN_U unChnAttr;                      /*region display attribute*/
} RGN_CHN_ATTR_S;

typedef struct HB_RGN_BITMAP_ATTR_S           /*bitmap attribute*/
{
    RGN_PIXEL_FORMAT_E enPixelFormat;         /*pixel of bitmap*/
    RGN_SIZE_S stSize;                        /*size of bitmap*/
    void *pAddr;                              /*address of bitmap*/
}RGN_BITMAP_S;

typedef struct HB_RGN_DRAW_WORD_PARAM_S         /*param for drawing words*/
{
    void *pAddr;
    RGN_SIZE_S stSize;                        /*size of address*/
    RGN_POINT_S stPoint;                      /*point of string in bitmap*/
    uint8_t *pu8Str;                          /*string of drawwing*/
    RGN_FONT_COLOR_E enFontColor;             /*font color of bitmap*/
    RGN_FONT_SIZE_E enFontSize;               /*font size of bitmap*/
    bool bFlushEn;
}RGN_DRAW_WORD_S;

//定义画线操作的相关配置。
typedef struct HB_RGN_DRAW_LINE_PARAM_S {       /*param for drawing lines*/
    void *pAddr;
    RGN_SIZE_S stSize;                        /*size of address*/
    RGN_POINT_S stStartPoint;                     /* Line start point */
    RGN_POINT_S stEndPoint;                       /* Line end point */
    uint32_t u32Thick;                            /* Width of line */
    uint32_t u32Color;                            /* Color of line */
    bool bFlushEn;
} RGN_DRAW_LINE_S;

typedef struct HB_RGN_STA_ATTR_S {
	uint8_t u8StaEn;
	uint16_t u16StartX;
	uint16_t u16StartY;
	uint16_t u16Width;
	uint16_t u16Height;
} RGN_STA_S;

/*create a new region*/
int32_t HB_RGN_Create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion);

/*destory a existential region*/
int32_t HB_RGN_Destroy(RGN_HANDLE Handle);

/*get the attribute of region*/
int32_t HB_RGN_GetAttr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion);

/*set the attribute of region*/
int32_t HB_RGN_SetAttr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion);

/*set bitmap for region*/
int32_t HB_RGN_SetBitMap(RGN_HANDLE Handle, const RGN_BITMAP_S *pstBitmapAttr);

/*attach region to a chnnel*/
int32_t HB_RGN_AttachToChn(RGN_HANDLE Handle, const RGN_CHN_S *pstChn,
                            const RGN_CHN_ATTR_S *pstChnAttr);

/*detach region from a channel*/
int32_t HB_RGN_DetachFromChn(RGN_HANDLE Handle, const RGN_CHN_S *pstChn);

/*set the displayed attribute*/
int32_t HB_RGN_SetDisplayAttr(RGN_HANDLE Handle, const RGN_CHN_S *pstChn,
                            const RGN_CHN_ATTR_S *pstChnAttr);

/*get the displayed attribute*/
int32_t HB_RGN_GetDisplayAttr(RGN_HANDLE Handle, const RGN_CHN_S *pstChn,
                            RGN_CHN_ATTR_S *pstChnAttr);

/*get info of canvas*/
int32_t HB_RGN_GetCanvasInfo(RGN_HANDLE Handle, RGN_CANVAS_S *pstCanvasInfo);

/*update canvas*/
int32_t HB_RGN_UpdateCanvas(RGN_HANDLE Handle);

/*draw words to specified address*/
int32_t HB_RGN_DrawWord(RGN_HANDLE Handle,
                            const RGN_DRAW_WORD_S *pstRgnDrawWord);

/*/*draw a line to specified address*/
int32_t HB_RGN_DrawLine(RGN_HANDLE hHandle,
                            const RGN_DRAW_LINE_S *pstRgnDrawLine);

/*batch draw lines to specified address*/
int32_t HB_RGN_DrawLineArray(RGN_HANDLE hHandle,
                                 const RGN_DRAW_LINE_S astRgnDrawLine[],
                                 uint32_t u32ArraySize);

/*set some region handles to a group*/
int32_t HB_RGN_BatchBegin(RGN_HANDLEGROUP *pu32Group, uint32_t u32Num,
                                 const RGN_HANDLE handle[]);

/*update all regions in the group*/
int32_t HB_RGN_BatchEnd(RGN_HANDLEGROUP u32Group);

/*set color map to a channel*/
int32_t HB_RGN_SetColorMap(const RGN_CHN_S *pstChn, uint32_t aColorMap[15]);

/*set attribute of y statistics*/
int32_t HB_RGN_SetSta(const RGN_CHN_S *pstChn, uint8_t astStaLevel[3],
                             RGN_STA_S astStaAttr[8]);

/*get the value of statistics*/
int32_t HB_RGN_GetSta(const RGN_CHN_S *pstChn, uint16_t astStaValue[8][4]);

int32_t HB_RGN_AttachToPym(RGN_HANDLE Handle, const RGN_CHN_S *pstChn,
                            const RGN_CHN_ATTR_S *pstChnAttr);

int32_t HB_RGN_DetachFromPym(RGN_HANDLE Handle, const RGN_CHN_S *pstChn);

int32_t HB_RGN_SetPymColorMap(uint32_t aColorMap[15]);

int32_t HB_RGN_SetPymSta(const RGN_CHN_S *pstChn, uint8_t astStaLevel[3],
                             RGN_STA_S astStaAttr[8]);

int32_t HB_RGN_GetPymSta(const RGN_CHN_S *pstChn, uint16_t astStaValue[8][4]);

#ifdef __cplusplus
}
#endif

#endif  // RGN_HB_RGN_H_
