/**
 *  Copyright (C) 2019 Horizon Robotics Inc.
 *  All rights reserved.
 *  @Author: yong.wu
 *  @Email: <yong.wu@horizon.ai>
 *  @Date: 2020-06-13
 *  @Version: v0.0.1
 *  @Brief: implemenation of iot log system.
 */

#ifndef INCLUDE_IOT_VIO_LOG_H_
#define INCLUDE_IOT_VIO_LOG_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdio.h>
#include <stdlib.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ?\
        (strrchr(__FILE__, '/') + 1):__FILE__)

#define STRINGIZE_NO_EXPANSION(x) #x
#define STRINGIZE(x) STRINGIZE_NO_EXPANSION(x)
#define HERE __FILE__ ":" STRINGIZE(__LINE__)

#ifndef SUBSYS_NAME
#define SUBSYS_NAME
#endif
#define SUBSYS STRINGIZE(SUBSYS_NAME)

#define L_INFO "[INFO][" SUBSYS "]"
#define L_WARNING "[WARNING]" SUBSYS "]"
#define L_ERROR "[ERROR][" SUBSYS "]"
#define L_DEBUG "[DEBUG][" SUBSYS "]"

/* output log by console */
#define CONSOLE_DEBUG_LEVEL     4
#define CONSOLE_INFO_LEVEL      3
#define CONSOLE_WARNING_LEVEL   2
#define CONSOLE_ERROR_LEVEL     1

#ifndef pr_fmt
#define pr_fmt(fmt)    fmt
#endif

/* get log level from environment variable */
/* we use console debug level in default */
#define LOGLEVEL_ENV   "IOT_VIOLOG"

static inline int get_loglevel(void)
{
    char *loglevel_env = NULL;
    int loglevel_value = CONSOLE_ERROR_LEVEL;

    loglevel_env = getenv(LOGLEVEL_ENV);
    if (loglevel_env != NULL) {
        loglevel_value = atoi(loglevel_env);
        /* loglevel value should in the configuration area */
        if ((loglevel_value >= CONSOLE_ERROR_LEVEL) &&
                (loglevel_value <= CONSOLE_DEBUG_LEVEL))
            return loglevel_value;
    }

    /* default log level */
    loglevel_value = CONSOLE_ERROR_LEVEL;

    return loglevel_value;
}

/* pr_debug defintion */
#define pr_debug(fmt, ...)\
    do {\
        int loglevel = get_loglevel();\
        if (loglevel >= CONSOLE_DEBUG_LEVEL)\
        fprintf(stdout, L_DEBUG "[%s:(%s:%d)]" " " pr_fmt(fmt), \
                __FILENAME__, __func__, __LINE__, ##__VA_ARGS__);\
    } while (0);

/* pr_info defintion */
#define pr_info(fmt, ...)\
    do {\
        int loglevel = get_loglevel();\
        if (loglevel >= CONSOLE_INFO_LEVEL)\
        fprintf(stdout, L_INFO "[%s:(%s:%d)]" " " pr_fmt(fmt), \
                __FILENAME__, __func__, __LINE__, ##__VA_ARGS__);\
    } while (0);

/* pr_warn defintion */
#define pr_warn(fmt, ...)\
    do {\
        int loglevel = get_loglevel();\
        if (loglevel >= CONSOLE_WARNING_LEVEL)\
        fprintf(stdout, L_WARNING "[%s:(%s:%d)]" " " pr_fmt(fmt), \
                __FILENAME__, __func__, __LINE__, ##__VA_ARGS__);\
    } while (0);

/* pr_err defintion */
#define pr_err(fmt, ...)\
    do {\
        int loglevel = get_loglevel();\
        if (loglevel >= CONSOLE_ERROR_LEVEL)\
        fprintf(stdout, L_ERROR "[%s:(%s:%d)]" " " pr_fmt(fmt), \
                __FILENAME__, __func__, __LINE__, ##__VA_ARGS__);\
    } while (0);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* INCLUDE_IOT_VIO_LOG_H_ */
