/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.11.21
 */
 
#ifndef __HB_TYPE_H__
#define __HB_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

// typedef unsigned char           uint8_t;
// typedef unsigned short          uint16_t;
// typedef unsigned int            uint32_t;
// typedef unsigned long long      uint64_t;
// typedef signed char             int8_t;
// typedef short                   int16_t;
// typedef int                     int32_t;
typedef void * video_mutex_t;

/*float*/
typedef float               HB_FLOAT;
/*double*/
typedef double                  HB_DOUBLE;


#ifndef _M_IX86
    typedef unsigned long long  HB_U64;
    typedef long long           HB_S64;
#else
typedef __int64                 uint32_t;
typedef __int64                 int32_t;
#endif

typedef char                    HB_CHAR;
typedef char hb_char;
#define HB_VOID                 void

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
#ifndef HB_BOOL
typedef enum {
    HB_FALSE = 0,
    HB_TRUE  = 1,
} HB_BOOL;
#endif

#ifndef NULL
    #define NULL    0L
#endif

#define HB_NULL     0L
#define HB_SUCCESS  0
#define HB_FAILURE  (-1)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HB_TYPE_H__ */

