/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: yong.wu
 * @Mail: yong.wu@horizon.ai
 * @Date: 2020-06-22
 * @Version: v1.0.0
 * @Brief: implemenation of vio hapi.
 */

#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <memory>
#include <queue>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#include "hb_sys.h"
#include "hb_mipi_api.h"
#include "hb_vin_api.h"
#include "hb_vio_interface.h"
#include "hb_vps_api.h"
#include "hb_vp_api.h"
}
#endif
#include "vioplugin/iot_vio_api.h"
#include "vioplugin/iot_vio_cfg.h"
#include "vioplugin/iot_vio_log.h"
#include "hobotlog/hobotlog.hpp"

#define MAX_ID_NUM 32
#define MAX_PLANE 4

#define MD_DATA_FUNC 15
#define BIT(n) (1UL << (n))

#define RAW_ENABLE BIT(HB_VIO_SIF_RAW_DATA)  //  for raw dump/fb
#define YUV_ENABLE BIT(HB_VIO_ISP_YUV_DATA)  // for yuv dump
#define RAW_FEEDBACK_ENABLE BIT(HB_VIO_SIF_FEEDBACK_SRC_DATA)  // feedback
#define MD_ENABLE BIT(MD_DATA_FUNC)                           // MD  15

#define VPS_CHN0_ENABLE BIT(HB_VIO_IPU_DS0_DATA)
#define VPS_CHN1_ENABLE BIT(HB_VIO_IPU_DS1_DATA)
#define VPS_CHN2_ENABLE BIT(HB_VIO_IPU_DS2_DATA)
#define VPS_CHN3_ENABLE BIT(HB_VIO_IPU_DS3_DATA)
#define VPS_CHN4_ENABLE BIT(HB_VIO_IPU_DS4_DATA)
#define VPS_CHN5_ENABLE BIT(HB_VIO_IPU_US_DATA)
#define VPS_CHN6_ENABLE BIT(HB_VIO_PYM_FEEDBACK_SRC_DATA)

#define BIT2CHN(chns, chn) (chns & (1 << chn))
#define MAX_POOL_CNT (32)
#define __FILENAME__ (strrchr(__FILE__, '/') ?\
        (strrchr(__FILE__, '/') + 1):__FILE__)

#define TYPE_NAME(x)    #x
static const char *iot_type_info[] = {
    NULL,  // 0
    TYPE_NAME(IOT_VIO_SRC_INFO),  // 1
    TYPE_NAME(IOT_VIO_PYM_INFO),  // 2
    TYPE_NAME(IOT_VIO_SIF_INFO),  // 3
    TYPE_NAME(IOT_VIO_IPU_STATE_INFO),  // 4
    TYPE_NAME(IOT_VIO_FRAME_START_INFO),  // 5
    TYPE_NAME(IOT_VIO_PYM_MULT_INFO),  // 6
    TYPE_NAME(IOT_VIO_SRC_MULT_INFO),  // 7
    TYPE_NAME(IOT_VIO_FEEDBACK_SRC_INFO),  // 8
    TYPE_NAME(IOT_VIO_FEEDBACK_FLUSH),  // 9
    TYPE_NAME(IOT_VIO_FEEDBACK_SRC_MULT_INFO),  // 10
    TYPE_NAME(IOT_VIO_PYM_INFO_CONDITIONAL),  // 11
    TYPE_NAME(IOT_VIO_FEEDBACK_PYM_PROCESS),  // 12
};

#define check_type_valid(type)\
    do {\
        uint32_t iot_type_num =\
        static_cast<uint32_t>(sizeof(iot_type_info) / sizeof(char*));\
        if (type < 0 || type > iot_type_num) {\
            pr_err("check type valid falied, type:%d\n", type);\
            return -1;\
        }\
    } while (0)

#define check_params_valid(p)\
    do {\
        if (p == NULL)\
        return -1;\
    } while (0)

/* default params value */
static int g_vin_fd[MAX_CAM_NUM] = {
    0,
};
static int sensorId[MAX_CAM_NUM] = {
    1,
};
static int mipiIdx[MAX_MIPIID_NUM] = {
    1,
};
static int bus[MAX_CAM_NUM] = {
    5,
};
static int port[MAX_CAM_NUM] = {
    0,
};
static int serdes_index[MAX_CAM_NUM] = {
    0,
};
static int serdes_port[MAX_CAM_NUM] = {
    0,
};
static int temper_mode[MAX_CAM_NUM] = {
    0,
};
static int vin_vps_mode[MAX_CAM_NUM] = {
    0,
};  // VIN_ONLIE_VPS_ONLINE
static int need_clk[MAX_CAM_NUM] = {
    0,
};  // need_clk= 1, means x3 output clk used to sensor
static int need_md[MAX_CAM_NUM] = {
    0,
};
static int need_chnfd[MAX_CAM_NUM] = {
    0,
};
static int need_dis[MAX_CAM_NUM] = {
    0,
};
static int need_gdc[MAX_CAM_NUM] = {
    0,
};
static int grp_rotate[MAX_CAM_NUM] = {
    0,
};
static int vc_num[MAX_CAM_NUM] = {
    0,
};
static int ipu_chn[MAX_CAM_NUM][MAX_CHN_NUM] = {
    {
        0,
    },
};
static int pym_chn[MAX_CAM_NUM][MAX_CHN_NUM] = {
    {
        0,
    },
};
static VPS_PYM_CHN_ATTR_S pym_cfg[MAX_GRP_NUM][MAX_CHN_NUM];

static int groupMask = 1;  // single cam,grp_num is 0
static int vps_dump = 0;
static int vps_layer_dump = 0;
static int need_cam = 1;
/* static int data_type; */
static hb_vio_buffer_t *g_feedback_buf[MAX_GRP_NUM];
static std::queue<hb_vio_buffer_t*> g_fb_queue[MAX_GRP_NUM];
iot_vio_cfg_t g_iot_vio_cfg;

static void parser_iot_vio_cfg();
static void print_iot_vio_cfg();
static void adapter_vin_vps_config(int pipeId);

typedef enum group_id {
    GROUP_0 = 0,
    GROUP_1,
    GROUP_2,
    GROUP_3,
    GROUP_MAX
} group_id_e;

typedef struct {
    uint32_t frame_id;
    uint32_t plane_count;
    uint32_t xres[MAX_PLANE];
    uint32_t yres[MAX_PLANE];
    char *addr[MAX_PLANE];
    uint32_t size[MAX_PLANE];
} raw_t;

typedef enum HB_MIPI_SNS_TYPE_E {
    SENSOR_ID_INVALID,
    IMX327_30FPS_1952P_RAW12_LINEAR,   // 1
    IMX327_30FPS_2228P_RAW12_DOL2,     // 2
    AR0233_30FPS_1080P_RAW12_954_PWL,  // 3
    AR0233_30FPS_1080P_RAW12_960_PWL,  // 4
    OS8A10_30FPS_3840P_RAW10_LINEAR,   // 5
    OS8A10_30FPS_3840P_RAW10_DOL2,     // 6
    OV10635_30FPS_720p_954_YUV,        // 7
    OV10635_30FPS_720p_960_YUV,        // 8
    SIF_TEST_PATTERN0_1080P,           // 9
    FEED_BACK_RAW12_1952P,             // 10
    SIF_TEST_PATTERN_YUV_720P,         // 11
    SIF_TEST_PATTERN_12M_RAW12,        // 12
    S5KGM1SP_30FPS_4000x3000_RAW10,    // 13
    SAMPLE_SENOSR_ID_MAX,
} MIPI_SNS_TYPE_E;

MIPI_SENSOR_INFO_S SENSOR_4LANE_IMX327_30FPS_12BIT_LINEAR_INFO = {
    .deseEnable = 0,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {},
    .sensorInfo = {
        /* port,dev_port,bus_type, bus_num, fps, resolution, sensor_addr,
           serial_addr*/
        0,
        0,
        0,
        0,
        30,
        1097,
        0x36,
        0,
        /*entry_index, mode, reg_width, sensor_name*/
        1,
        static_cast<MIPI_SENSOR_MODE_E>(1),
        16,
        const_cast<char*>("imx327"),
        /*extra_mode*/
        0,
        /*deserial_index deserial_port*/
        -1,
        0,
        {0, 0, 0} /* spi_info */
    }};

MIPI_SENSOR_INFO_S SENSOR_4LANE_IMX327_30FPS_12BIT_DOL2_INFO = {
    .deseEnable = 0,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {},
    .sensorInfo = {
        /*port,dev_port,bus_type, bus_num, fps, resolution, sensor_addr,
           serial_addr*/
        0,
        0,
        0,
        0,
        30,
        2228,
        0x36,
        0,
        /*entry_index, mode,  reg_width, sensor_name*/
        0,
        static_cast<MIPI_SENSOR_MODE_E>(2),
        16,
        const_cast<char*>("imx327"),
        /*extra_mode*/
        0,
        /*deserial_index deserial_port*/
        -1,
        0,
        {0, 0, 0} /* spi_info */
    }};

MIPI_SENSOR_INFO_S SENSOR_OS8A10_30FPS_10BIT_LINEAR_INFO = {
    .deseEnable = 0,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {},
    .sensorInfo = {
        /* port,dev_port,bus_type, bus_num, fps, resolution, sensor_addr,
           serial_addr*/
        0,
        0,
        0,
        0,
        30,
        2160,
        0x36,
        0,
        /*entry_index, mode, reg_width, sensor_name*/
        0,
        static_cast<MIPI_SENSOR_MODE_E>(1),
        16,
        const_cast<char*>("os8a10"),
        /*extra_mode*/
        0,
        /*deserial_index deserial_port*/
        -1,
        0,
        {0, 0, 0} /* spi_info */
    }};

MIPI_SENSOR_INFO_S SENSOR_OS8A10_30FPS_10BIT_DOL2_INFO = {
    .deseEnable = 0,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {},
    .sensorInfo = {
        /* port,dev_port,bus_type, bus_num, fps, resolution, sensor_addr,
           serial_addr*/
        0,
        0,
        0,
        0,
        30,
        2160,
        0x36,
        0,
        /*entry_index, mode, reg_width, sensor_name*/
        0,
        static_cast<MIPI_SENSOR_MODE_E>(2),
        16,
        const_cast<char*>("os8a10"),
        /*extra_mode*/
        0,
        /*deserial_index deserial_port*/
        -1,
        0,
        {0, 0, 0} /* spi_info */
    }};

MIPI_SENSOR_INFO_S SENSOR_4LANE_AR0233_30FPS_12BIT_1080P_954_INFO = {
    .deseEnable = 1,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo =
        {
            .bus_type = 0,
            .bus_num = 4,
            .deserial_addr = 0x3d,
            .deserial_name = const_cast<char*>("s954"),
        },
    .sensorInfo = {.port = 0,
                   .dev_port = 0,
                   .bus_type = 0,
                   .bus_num = 4,
                   .fps = 30,
                   .resolution = 1080,
                   .sensor_addr = 0x10,
                   .serial_addr = 0x18,
                   .entry_index = 1,
                   .sensor_mode = PWL_M,
                   .reg_width = 16,
                   .sensor_name = const_cast<char*>("ar0233"),
                   .extra_mode = 0,
                   .deserial_index = 0,
                   .deserial_port = 0}};

MIPI_SENSOR_INFO_S SENSOR_4LANE_AR0233_30FPS_12BIT_1080P_960_INFO = {
    .deseEnable = 1,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {.bus_type = 0,
                     .bus_num = 4,
                     .deserial_addr = 0x30,
                     .deserial_name = const_cast<char*>("s960")},
    .sensorInfo = {.port = 0,
                   .dev_port = 0,
                   .bus_type = 0,
                   .bus_num = 4,
                   .fps = 30,
                   .resolution = 1080,
                   .sensor_addr = 0x10,
                   .serial_addr = 0x18,
                   .entry_index = 1,
                   .sensor_mode = PWL_M,
                   .reg_width = 16,
                   .sensor_name = const_cast<char*>("ar0233"),
                   .extra_mode = 0,
                   .deserial_index = 0,
                   .deserial_port = 0}};

MIPI_SENSOR_INFO_S SENSOR_2LANE_OV10635_30FPS_YUV_720P_954_INFO = {
    .deseEnable = 1,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {.bus_type = 0,
                     .bus_num = 4,
                     .deserial_addr = 0x3d,
                     .deserial_name = const_cast<char*>("s954")},
    .sensorInfo = {.port = 0,
                   .dev_port = 0,
                   .bus_type = 0,
                   .bus_num = 4,
                   .fps = 30,
                   .resolution = 720,
                   .sensor_addr = 0x40,
                   .serial_addr = 0x1c,
                   .entry_index = 1,
                   .sensor_mode = {},
                   .reg_width = 16,
                   .sensor_name = const_cast<char*>("ov10635"),
                   .extra_mode = 0,
                   .deserial_index = 0,
                   .deserial_port = 0,
                   .spi_info = {}
                   }};

MIPI_SENSOR_INFO_S SENSOR_2LANE_OV10635_30FPS_YUV_720P_960_INFO = {
    .deseEnable = 1,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {.bus_type = 0,
                     .bus_num = 4,
                     .deserial_addr = 0x30,
                     .deserial_name = const_cast<char*>("s960")},
    .sensorInfo = {.port = 0,
                   .dev_port = 0,
                   .bus_type = 0,
                   .bus_num = 4,
                   .fps = 30,
                   .resolution = 720,
                   .sensor_addr = 0x40,
                   .serial_addr = 0x1c,
                   .entry_index = 1,
                   .sensor_mode = {},
                   .reg_width = 16,
                   .sensor_name = const_cast<char*>("ov10635"),
                   .extra_mode = 0,
                   .deserial_index = 0,
                   .deserial_port = 0}};

MIPI_SENSOR_INFO_S SENSOR_TESTPATTERN_INFO = {
    .deseEnable = {},
    .inputMode = {},
    .deserialInfo = {},
    .sensorInfo = {
      .port = {},
      .dev_port = {},
      .bus_type = {},
      .bus_num = {},
      .fps = {},
      .resolution = {},
      .sensor_addr = {},
      .serial_addr = {},
      .entry_index = {},
      .sensor_mode = {},
      .reg_width = {},
      .sensor_name = const_cast<char*>("virtual"),
    }};

MIPI_SENSOR_INFO_S SENSOR_S5KGM1SP_30FPS_10BIT_LINEAR_INFO = {
    .deseEnable = 0,
    .inputMode = INPUT_MODE_MIPI,
    .deserialInfo = {},
    .sensorInfo = {.port = 0,
                   .dev_port = 0,
                   .bus_type = 0,
                   .bus_num = 4,
                   .fps = 30,
                   .resolution = 3000,
                   .sensor_addr = 0x10,
                   .serial_addr = 0,
                   .entry_index = 1,
                   .sensor_mode = NORMAL_M ,
                   .reg_width = 16,
                   .sensor_name = const_cast<char*>("s5kgm1sp"),
                   .extra_mode = 0,
                   .deserial_index = -1,
                   .deserial_port = 0,
                   .spi_info = {}
    }
};

MIPI_ATTR_S MIPI_2LANE_OV10635_30FPS_YUV_720P_954_ATTR = {
    .mipi_host_cfg = {2,    /* lane */
                      0x1e, /* datatype */
                      24,   /* mclk */
                      1600, /* mipiclk */
                      30,   /* fps */
                      1280, /* width */
                      720,  /*height */
                      3207, /* linlength */
                      748,  /* framelength */
                      30,   /* settle */
                      4,
                      {0, 1, 2, 3}},
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_2LANE_OV10635_30FPS_YUV_720P_960_ATTR = {
    .mipi_host_cfg = {2,    /* lane */
                      0x1e, /* datatype */
                      24,   /* mclk */
                      3200, /* mipiclk */
                      30,   /* fps */
                      1280, /* width  */
                      720,  /*height */
                      3207, /* linlength */
                      748,  /* framelength */
                      30,   /* settle */
                      4,
                      {0, 1, 2, 3}},
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_AR0233_30FPS_12BIT_1080P_954_ATTR = {
    .mipi_host_cfg = {4,    /* lane */
                      0x2c, /* datatype */
                      24,   /* mclk */
                      1224, /* mipiclk */
                      30,   /* fps */
                      1920, /* width */
                      1080, /*height */
                      2000, /* linlength */
                      1700, /* framelength */
                      30,   /* settle */
                      4,
                      {0, 1, 2, 3}},
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_AR0233_30FPS_12BIT_1080P_960_ATTR = {
    .mipi_host_cfg = {4,    /* lane */
                      0x2c, /* datatype */
                      24,   /* mclk */
                      3200, /* mipiclk */
                      30,   /* fps */
                      1920, /* width */
                      1080, /*height */
                      2000, /* linlength */
                      1111, /* framelength */
                      30,   /* settle */
                      4,
                      {0, 1, 2, 3}},
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_NORMAL_ATTR = {
    .mipi_host_cfg =
        {
            4,    /* lane */
            0x2c, /* datatype */
            24,   /* mclk */
            891,  /* mipiclk */
            30,   /* fps */
            1952, /* width */
            1097, /*height */
            2152, /* linlength */
            1150, /* framelength */
            20    /* settle */
        },
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_NORMAL_SENSOR_CLK_ATTR = {
    .mipi_host_cfg =
        {
            4,    /* lane */
            0x2c, /* datatype */
            3713, /* mclk */
            891,  /* mipiclk */
            30,   /* fps */
            1952, /* width */
            1097, /*height */
            2152, /* linlength */
            1150, /* framelength */
            20    /* settle */
        },
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_DOL2_ATTR = {
    .mipi_host_cfg =
        {
            4,    /* lane */
            0x2c, /* datatype */
            24,   /* mclk    */
            1782, /* mipiclk */
            30,   /* fps */
            1952, /* width  */
            2228, /*height */
            2152, /* linlength */
            2300, /* framelength */
            20    /* settle */
        },
    .dev_enable = 0 /* mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_OS8A10_30FPS_10BIT_LINEAR_ATTR = {
    .mipi_host_cfg =
        {
            4,           /* lane */
            0x2b,        /* datatype */
            24,          /* mclk */
            1440,        /* mipiclk */
            30,          /* fps */
            3840,        /* width */
            2160,        /*height */
            6326,        /* linlength */
            4474,        /* framelength */
            50,          /* settle */
            4,           /*chnnal_num*/
            {0, 1, 2, 3} /*vc */
        },
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_OS8A10_30FPS_10BIT_LINEAR_SENSOR_CLK_ATTR = {
    .mipi_host_cfg =
        {
            4,           /* lane */
            0x2b,        /* datatype */
            2400,        /* mclk */
            1440,        /* mipiclk */
            30,          /* fps */
            3840,        /* width  */
            2160,        /*height */
            6326,        /* linlength */
            4474,        /* framelength */
            50,          /* settle */
            4,           /*chnnal_num*/
            {0, 1, 2, 3} /*vc */
        },
    .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_OS8A10_30FPS_10BIT_DOL2_ATTR = {
    .mipi_host_cfg =
        {
            4,           /* lane */
            0x2b,        /* datatype */
            24,          /* mclk */
            2880,        /* mipiclk */
            30,          /* fps */
            3840,        /* width */
            2160,        /*height */
            5084,        /* linlength */
            4474,        /* framelength */
            20,          /* settle */
            4,           /*chnnal_num*/
            {0, 1, 2, 3} /*vc */
        },
    .dev_enable = 0 /*  mipi dev enable */
};


MIPI_ATTR_S MIPI_SENSOR_S5KGM1SP_30FPS_10BIT_LINEAR_ATTR =
{
    .mipi_host_cfg =
    {
        4,            /* lane */
        0x2b,         /* datatype */
        24,           /* mclk */
        4600,         /* mipiclk */
        30,           /* fps */
        4000,         /* width */
        3000,         /* height */
        5024,         /* linlength */
        3194,         /* framelength */
        30,           /* settle */
        2,            /*chnnal_num*/
        {0, 1}        /*vc */
    },
    .dev_enable = 0  /*  mipi dev enable */
};

VIN_DEV_ATTR_S DEV_ATTR_AR0233_1080P_BASE = {
    .stSize =
        {
            0,    /*format*/
            1920, /*width*/
            1080, /*height*/
            2     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 1920,
                           .height = 1080,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 2880,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_IMX327_LINEAR_BASE = {
    .stSize =
        {
            0,    /*format*/
            1952, /*width*/
            1097, /*height*/
            2     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 1952,
                           .height = 1097,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 2928,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
        .short_maxexp_lines = 0,
        .medium_maxexp_lines = 0,
        .vc_short_seq = 0,
        .vc_medium_seq = 0,
        .vc_long_seq = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_IMX327_DOL2_BASE = {
    .stSize =
        {
            0,    /*format*/
            1948, /*width*/
            1109, /*height*/
            2     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 2,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 1,
            .set_init_frame_id = 0,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 1952,
                           .height = 1097,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 2928,
            .buffer_num = 10,
        },
    .outIspAttr =
        {
            .dol_exp_num = 2,
            .enable_dgain = 0,
            .set_dgain_short = 0,
            .set_dgain_medium = 0,
            .set_dgain_long = 0,
            .short_maxexp_lines = 0,
            .medium_maxexp_lines = 0,
            .vc_short_seq = 0,
            .vc_medium_seq = 0,
            .vc_long_seq = 1,
        },
};

VIN_DEV_ATTR_S DEV_ATTR_OS8A10_LINEAR_BASE = {
    .stSize =
        {
            0,    /*format*/
            3840, /*width*/
            2160, /*height*/
            1     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 3840,
                           .height = 2160,
                           .pix_length = 1,
                       }},
    .outDdrAttr =
        {
            .stride = 4800,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_OS8A10_DOL2_BASE = {
    .stSize =
        {
            0,    /*format*/
            3840, /*width*/
            2160, /*height*/
            1     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 2,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 3840,
                           .height = 2160,
                           .pix_length = 1,
                       }},
    .outDdrAttr =
        {
            .stride = 4800,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 2,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_OV10635_YUV_BASE = {
    .stSize =
        {
            8,    /*format*/
            1280, /*width*/
            720,  /*height*/
            0     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 8,
                           .width = 1280,
                           .height = 720,
                           .pix_length = 0,
                       }},
    .outDdrAttr =
        {
            .stride = 1280,
            .buffer_num = 8,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_FEED_BACK_1097P_BASE = {
    .stSize =
        {
            0,    /*format*/
            1952, /*width*/
            1097, /*height*/
            2     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 4,
                   .raw_feedback_en = 1,
                   .data =
                       {
                           .format = 0,
                           .width = 1952,
                           .height = 1097,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 2928,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_S5KGM1SP_LINEAR_BASE = {
    .stSize =
        {
            0,       /*format*/
            4000,    /*width*/
            3000,    /*height*/
            1        /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
        },
    },
    .DdrIspAttr =
        {
            .stride = 0,
            .buf_num = 4,
            .raw_feedback_en = 0,
            .data =
                {
                    .format = 0,
                    .width = 4000,
                    .height = 3000,
                    .pix_length = 1,
                }
        },
    .outDdrAttr =
        {
            .stride = 5000,
            .buffer_num = 8,
        },
    .outIspAttr =
        {
            .dol_exp_num = 1,
            .enable_dgain = 0,
            .set_dgain_short = 0,
            .set_dgain_medium = 0,
            .set_dgain_long = 0,
        }
};

VIN_DEV_ATTR_EX_S DEV_ATTR_IMX327_MD_BASE = {
    .path_sel = 0,
    .roi_top = 0,
    .roi_left = 0,
    .roi_width = 1280,
    .roi_height = 640,
    .grid_step = 128,
    .grid_tolerance = 10,
    .threshold = 10,
    .weight_decay = 128,
    .precision = 0};

VIN_PIPE_ATTR_S PIPE_ATTR_OV10635_YUV_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 1280,
            .height = 720,
        },
    .cfaPattern = PIPE_BAYER_RGGB,
    .temperMode = 0,
    .ispBypassEn = 1,
    .ispAlgoState = 0,
    .bitwidth = 12,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 0,
        .lname = NULL,
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_IMX327_DOL2_BASE = {
    .ddrOutBufNum = 5,
    .snsMode = SENSOR_DOL2_MODE,
    .stSize =
        {
            .format = 0,
            .width = 1920,
            .height = 1080,
        },
    .cfaPattern = PIPE_BAYER_RGGB,
    .temperMode = 2,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 12,
    .startX = 0,
    .startY = 12,
    .calib = {
        .mode = 1,
        .lname = const_cast<char*>("/etc/cam/libimx327_linear.so"),
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_IMX327_LINEAR_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 1920,
            .height = 1080,
        },
    .cfaPattern = PIPE_BAYER_RGGB,
    .temperMode = 2,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 12,
    .startX = 0,
    .startY = 12,
    .calib = {
        .mode = 1,
        .lname = const_cast<char*>("/etc/cam/libimx327_linear.so"),
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_AR0233_1080P_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_PWL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 1920,
            .height = 1080,
        },
    .cfaPattern = static_cast<VIN_PIPE_CFA_PATTERN_E>(1),
    .temperMode = 2,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 12,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 1,
        .lname = const_cast<char*>("/etc/cam/lib_ar0233_pwl.so"),
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_OS8A10_LINEAR_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 3840,
            .height = 2160,
        },
    .cfaPattern = static_cast<VIN_PIPE_CFA_PATTERN_E>(3),
    .temperMode = 3,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 10,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 1,
        .lname = const_cast<char*>("/etc/cam/libos8a10_linear.so"),
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_OS8A10_DOL2_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_DOL2_MODE,
    .stSize =
        {
            .format = 0,
            .width = 3840,
            .height = 2160,
        },
    .cfaPattern = static_cast<VIN_PIPE_CFA_PATTERN_E>(3),
    .temperMode = 2,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 10,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 1,
        .lname = const_cast<char*>("/etc/cam/libos8a10_dol2.so"),
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_S5KGM1SP_LINEAR_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize = {
        .format = 0,
        .width = 4000,
        .height = 3000,
    },
    .cfaPattern = PIPE_BAYER_GRBG,
    .temperMode = 2,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 10,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 1,
        .lname = const_cast<char*>("/etc/cam/s5kgm1sp_linear.so"),
    }
};

VIN_PIPE_ATTR_S PIPE_ATTR_TEST_PATTERN_1080P_BASE = {
    .ddrOutBufNum = 4,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 1920,
            .height = 1080,
        },
    .cfaPattern = PIPE_BAYER_RGGB,
    .temperMode = 0,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 12,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 0,
        .lname = NULL,
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_TEST_PATTERN_12M_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 4000,
            .height = 3000,
        },
    .cfaPattern = PIPE_BAYER_RGGB,
    .temperMode = 0,
    .ispBypassEn = 0,
    .ispAlgoState = 1,
    .bitwidth = 12,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 0,
        .lname = NULL,
    }};

VIN_PIPE_ATTR_S PIPE_ATTR_TEST_PATTERN_4K_BASE = {
    .ddrOutBufNum = 8,
    .snsMode = SENSOR_NORMAL_MODE,
    .stSize =
        {
            .format = 0,
            .width = 3840,
            .height = 2160,
        },
    .cfaPattern = PIPE_BAYER_RGGB,
    .temperMode = 3,
    .ispBypassEn = 0,
    .ispAlgoState = 0,
    .bitwidth = 10,
    .startX = 0,
    .startY = 0,
    .calib = {
        .mode = 0,
        .lname = NULL,
    }};

VIN_DIS_ATTR_S DIS_ATTR_BASE = {
    .picSize =
        {
            .pic_w = 1919,
            .pic_h = 1079,
        },
    .disPath =
        {
            .rg_dis_enable = 0,
            .rg_dis_path_sel = 1,
        },
    .disHratio = 65536,
    .disVratio = 65536,
    .xCrop =
        {
            .rg_dis_start = 0,
            .rg_dis_end = 1919,
        },
    .yCrop =
        {
            .rg_dis_start = 0,
            .rg_dis_end = 1079,
        },
    .disBufNum = 8,
};

VIN_DIS_ATTR_S DIS_ATTR_OV10635_BASE = {.picSize =
                                            {
                                                .pic_w = 1279,
                                                .pic_h = 719,
                                            },
                                        .disPath =
                                            {
                                                .rg_dis_enable = 0,
                                                .rg_dis_path_sel = 1,
                                            },
                                        .disHratio = 65536,
                                        .disVratio = 65536,
                                        .xCrop =
                                            {
                                                .rg_dis_start = 0,
                                                .rg_dis_end = 1279,
                                            },
                                        .yCrop = {
                                            .rg_dis_start = 0,
                                            .rg_dis_end = 719,
                                        }};

VIN_DIS_ATTR_S DIS_ATTR_OS8A10_BASE = {.picSize =
                                           {
                                               .pic_w = 3839,
                                               .pic_h = 2159,
                                           },
                                       .disPath =
                                           {
                                               .rg_dis_enable = 0,
                                               .rg_dis_path_sel = 1,
                                           },
                                       .disHratio = 65536,
                                       .disVratio = 65536,
                                       .xCrop =
                                           {
                                               .rg_dis_start = 0,
                                               .rg_dis_end = 3839,
                                           },
                                       .yCrop = {
                                           .rg_dis_start = 0,
                                           .rg_dis_end = 2159,
                                       }};

VIN_DIS_ATTR_S DIS_ATTR_12M_BASE = {.picSize =
                                        {
                                            .pic_w = 3999,
                                            .pic_h = 2999,
                                        },
                                    .disPath =
                                        {
                                            .rg_dis_enable = 0,
                                            .rg_dis_path_sel = 1,
                                        },
                                    .disHratio = 65536,
                                    .disVratio = 65536,
                                    .xCrop =
                                        {
                                            .rg_dis_start = 0,
                                            .rg_dis_end = 3999,
                                        },
                                    .yCrop = {
                                        .rg_dis_start = 0,
                                        .rg_dis_end = 2999,
                                    }};

VIN_LDC_ATTR_S LDC_ATTR_12M_BASE = {.ldcEnable = 0,
                                    .ldcPath =
                                        {
                                            .rg_y_only = 0,
                                            .rg_uv_mode = 0,
                                            .rg_uv_interpo = 0,
                                            .reserved1 = 0,
                                            .rg_h_blank_cyc = 32,
                                            .reserved0 = 0
                                        },
                                    .yStartAddr = 524288,
                                    .cStartAddr = 786432,
                                    .picSize =
                                        {
                                            .pic_w = 3999,
                                            .pic_h = 2999,
                                        },
                                    .lineBuf = 99,
                                    .xParam =
                                        {
                                            .rg_algo_param_b = 1,
                                            .rg_algo_param_a = 1,
                                        },
                                    .yParam =
                                        {
                                            .rg_algo_param_b = 1,
                                            .rg_algo_param_a = 1,
                                        },
                                    .offShift =
                                        {
                                            .rg_center_xoff = 0,
                                            .rg_center_yoff = 0,
                                        },
                                    .xWoi =
                                        {
                                            .rg_start = 0,
                                            .reserved1 = 0,
                                            .rg_length = 3999,
                                            .reserved0 = 0
                                        },
                                    .yWoi = {
                                        .rg_start = 0,
                                        .reserved1 = 0,
                                        .rg_length = 2999,
                                        .reserved0 = 0
                                    }};

VIN_LDC_ATTR_S LDC_ATTR_BASE = {.ldcEnable = 0,
                                .ldcPath =
                                    {
                                        .rg_y_only = 0,
                                        .rg_uv_mode = 0,
                                        .rg_uv_interpo = 0,
                                        .reserved1 = 0,
                                        .rg_h_blank_cyc = 32,
                                        .reserved0 = 0
                                    },
                                .yStartAddr = 524288,
                                .cStartAddr = 786432,
                                .picSize =
                                    {
                                        .pic_w = 1919,
                                        .pic_h = 1079,
                                    },
                                .lineBuf = 99,
                                .xParam =
                                    {
                                        .rg_algo_param_b = 1,
                                        .rg_algo_param_a = 1,
                                    },
                                .yParam =
                                    {
                                        .rg_algo_param_b = 1,
                                        .rg_algo_param_a = 1,
                                    },
                                .offShift =
                                    {
                                        .rg_center_xoff = 0,
                                        .rg_center_yoff = 0,
                                    },
                                .xWoi =
                                    {
                                        .rg_start = 0,
                                        .reserved1 = 0,
                                        .rg_length = 1919,
                                        .reserved0 = 0
                                    },
                                .yWoi = {
                                    .rg_start = 0,
                                    .reserved1 = 0,
                                    .rg_length = 1079,
                                    .reserved0 = 0
                                }};

VIN_LDC_ATTR_S LDC_ATTR_OV10635_BASE = {.ldcEnable = 0,
                                        .ldcPath =
                                            {
                                                .rg_y_only = 0,
                                                .rg_uv_mode = 0,
                                                .rg_uv_interpo = 0,
                                                .reserved1 = 0,
                                                .rg_h_blank_cyc = 32,
                                                .reserved0 = 0
                                            },
                                        .yStartAddr = 524288,
                                        .cStartAddr = 786432,
                                        .picSize =
                                            {
                                                .pic_w = 1279,
                                                .pic_h = 719,
                                            },
                                        .lineBuf = 99,
                                        .xParam =
                                            {
                                                .rg_algo_param_b = 1,
                                                .rg_algo_param_a = 1,
                                            },
                                        .yParam =
                                            {
                                                .rg_algo_param_b = 1,
                                                .rg_algo_param_a = 1,
                                            },
                                        .offShift =
                                            {
                                                .rg_center_xoff = 0,
                                                .rg_center_yoff = 0,
                                            },
                                        .xWoi =
                                            {
                                                .rg_start = 0,
                                                .reserved1 = 0,
                                                .rg_length = 1279,
                                                .reserved0 = 0
                                            },
                                        .yWoi = {
                                            .rg_start = 0,
                                            .reserved1 = 0,
                                            .rg_length = 719,
                                            .reserved0 = 0
                                        }};

VIN_LDC_ATTR_S LDC_ATTR_OS8A10_BASE = {.ldcEnable = 0,
                                       .ldcPath =
                                           {
                                               .rg_y_only = 0,
                                               .rg_uv_mode = 0,
                                               .rg_uv_interpo = 0,
                                               .reserved1 = 0,
                                               .rg_h_blank_cyc = 32,
                                               .reserved0 = 0,
                                           },
                                       .yStartAddr = 524288,
                                       .cStartAddr = 786432,
                                       .picSize =
                                           {
                                               .pic_w = 3839,
                                               .pic_h = 2159,
                                           },
                                       .lineBuf = 99,
                                       .xParam =
                                           {
                                               .rg_algo_param_b = 3,
                                               .rg_algo_param_a = 2,
                                           },
                                       .yParam =
                                           {
                                               .rg_algo_param_b = 5,
                                               .rg_algo_param_a = 4,
                                           },
                                       .offShift =
                                           {
                                               .rg_center_xoff = 0,
                                               .rg_center_yoff = 0,
                                           },
                                       .xWoi =
                                           {
                                               .rg_start = 0,
                                               .reserved1 = 0,
                                               .rg_length = 3839,
                                               .reserved0 = 0
                                           },
                                       .yWoi = {
                                           .rg_start = 0,
                                           .reserved1 = 0,
                                           .rg_length = 2159,
                                           .reserved0 = 0
                                       }};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_YUV422_BASE = {
    .stSize =
        {
            8,    /*format*/
            1280, /*width*/
            720,  /*height*/
            0     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
            .enable_pattern = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 8,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 8,
                           .width = 1280,
                           .height = 720,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 1280,
            .buffer_num = 8,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_1080P_BASE = {
    .stSize =
        {
            0,    /*format*/
            1920, /*width*/
            1080, /*height*/
            2     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
            .enable_pattern = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 6,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 1920,
                           .height = 1080,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 2880,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_4K_BASE = {
    .stSize =
        {
            0,    /*format*/
            3840, /*width*/
            2160, /*height*/
            1     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
            .enable_pattern = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 8,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 3840,
                           .height = 2160,
                           .pix_length = 1,
                       }},
    .outDdrAttr =
        {
            .stride = 4800,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_12M_BASE = {
    .stSize =
        {
            0,    /*format*/
            4000, /*width*/
            3000, /*height*/
            2     /*pix_length*/
        },
    {
    .mipiAttr =
        {
            .enable = 1,
            .ipi_channels = 1,
            .ipi_mode = 0,
            .enable_mux_out = 1,
            .enable_frame_id = 1,
            .enable_bypass = 0,
            .enable_line_shift = 0,
            .enable_id_decoder = 0,
            .set_init_frame_id = 1,
            .set_line_shift_count = 0,
            .set_bypass_channels = 1,
            .enable_pattern = 1,
        },
    },
    .DdrIspAttr = {.stride = 0,
                   .buf_num = 8,
                   .raw_feedback_en = 0,
                   .data =
                       {
                           .format = 0,
                           .width = 4000,
                           .height = 3000,
                           .pix_length = 2,
                       }},
    .outDdrAttr =
        {
            .stride = 6000,
            .buffer_num = 10,
        },
    .outIspAttr = {
        .dol_exp_num = 1,
        .enable_dgain = 0,
        .set_dgain_short = 0,
        .set_dgain_medium = 0,
        .set_dgain_long = 0,
    }};


int SAMPLE_MIPI_GetSnsAttrBySns(MIPI_SNS_TYPE_E enSnsType,
        MIPI_SENSOR_INFO_S *pstSnsAttr)
{
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
        memcpy(pstSnsAttr, &SENSOR_4LANE_IMX327_30FPS_12BIT_LINEAR_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case IMX327_30FPS_2228P_RAW12_DOL2:
        memcpy(pstSnsAttr, &SENSOR_4LANE_IMX327_30FPS_12BIT_DOL2_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case AR0233_30FPS_1080P_RAW12_954_PWL:
        memcpy(pstSnsAttr, &SENSOR_4LANE_AR0233_30FPS_12BIT_1080P_954_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case AR0233_30FPS_1080P_RAW12_960_PWL:
        memcpy(pstSnsAttr, &SENSOR_4LANE_AR0233_30FPS_12BIT_1080P_960_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_LINEAR:
        memcpy(pstSnsAttr, &SENSOR_OS8A10_30FPS_10BIT_LINEAR_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_DOL2:
        memcpy(pstSnsAttr, &SENSOR_OS8A10_30FPS_10BIT_DOL2_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case OV10635_30FPS_720p_954_YUV:
        memcpy(pstSnsAttr, &SENSOR_2LANE_OV10635_30FPS_YUV_720P_954_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case OV10635_30FPS_720p_960_YUV:
        memcpy(pstSnsAttr, &SENSOR_2LANE_OV10635_30FPS_YUV_720P_960_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case SIF_TEST_PATTERN0_1080P:
    case SIF_TEST_PATTERN_YUV_720P:
    case SIF_TEST_PATTERN_12M_RAW12:
        memcpy(pstSnsAttr, &SENSOR_TESTPATTERN_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    case S5KGM1SP_30FPS_4000x3000_RAW10:
        memcpy(pstSnsAttr, &SENSOR_S5KGM1SP_30FPS_10BIT_LINEAR_INFO,
                sizeof(MIPI_SENSOR_INFO_S));
        break;
    default:
        pr_err("not surpport sensor type enSnsType %d\n", enSnsType);
        break;
    }
    pr_info("SAMPLE_MIPI_GetSnsAttrBySns success\n");
    return 0;
}

int SAMPLE_MIPI_GetMipiAttrBySns(MIPI_SNS_TYPE_E enSnsType,
        MIPI_ATTR_S *pstMipiAttr)
{
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
        memcpy(pstMipiAttr,
                &MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_NORMAL_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case IMX327_30FPS_2228P_RAW12_DOL2:
        memcpy(pstMipiAttr,
                &MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_DOL2_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case AR0233_30FPS_1080P_RAW12_954_PWL:
        memcpy(pstMipiAttr,
                &MIPI_4LANE_SENSOR_AR0233_30FPS_12BIT_1080P_954_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case AR0233_30FPS_1080P_RAW12_960_PWL:
        memcpy(pstMipiAttr,
                &MIPI_4LANE_SENSOR_AR0233_30FPS_12BIT_1080P_960_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_LINEAR:
        if (need_clk[0] == 1) {
            memcpy(pstMipiAttr,
                    &MIPI_SENSOR_OS8A10_30FPS_10BIT_LINEAR_SENSOR_CLK_ATTR,
                    sizeof(MIPI_ATTR_S));
        } else {
            memcpy(pstMipiAttr, &MIPI_SENSOR_OS8A10_30FPS_10BIT_LINEAR_ATTR,
                    sizeof(MIPI_ATTR_S));
        }
        break;
    case OS8A10_30FPS_3840P_RAW10_DOL2:
        memcpy(pstMipiAttr, &MIPI_SENSOR_OS8A10_30FPS_10BIT_DOL2_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case OV10635_30FPS_720p_954_YUV:
        memcpy(pstMipiAttr, &MIPI_2LANE_OV10635_30FPS_YUV_720P_954_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case OV10635_30FPS_720p_960_YUV:
        memcpy(pstMipiAttr, &MIPI_2LANE_OV10635_30FPS_YUV_720P_960_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    case S5KGM1SP_30FPS_4000x3000_RAW10:
        memcpy(pstMipiAttr, &MIPI_SENSOR_S5KGM1SP_30FPS_10BIT_LINEAR_ATTR,
                sizeof(MIPI_ATTR_S));
        break;
    default:
        pr_err("not surpport sensor type\n");
        break;
    }
    return 0;
}

int SAMPLE_VIN_GetDevAttrBySns(MIPI_SNS_TYPE_E enSnsType,
        VIN_DEV_ATTR_S *pstDevAttr)
{
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
        memcpy(pstDevAttr, &DEV_ATTR_IMX327_LINEAR_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case IMX327_30FPS_2228P_RAW12_DOL2:
        memcpy(pstDevAttr, &DEV_ATTR_IMX327_DOL2_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case AR0233_30FPS_1080P_RAW12_954_PWL:
    case AR0233_30FPS_1080P_RAW12_960_PWL:
        memcpy(pstDevAttr, &DEV_ATTR_AR0233_1080P_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_LINEAR:
        memcpy(pstDevAttr, &DEV_ATTR_OS8A10_LINEAR_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_DOL2:
        memcpy(pstDevAttr, &DEV_ATTR_OS8A10_DOL2_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case OV10635_30FPS_720p_954_YUV:
    case OV10635_30FPS_720p_960_YUV:
        memcpy(pstDevAttr, &DEV_ATTR_OV10635_YUV_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case SIF_TEST_PATTERN0_1080P:
        memcpy(pstDevAttr, &DEV_ATTR_TEST_PATTERN_1080P_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case FEED_BACK_RAW12_1952P:
        memcpy(pstDevAttr, &DEV_ATTR_FEED_BACK_1097P_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case SIF_TEST_PATTERN_YUV_720P:
        memcpy(pstDevAttr, &DEV_ATTR_TEST_PATTERN_YUV422_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case SIF_TEST_PATTERN_12M_RAW12:
        memcpy(pstDevAttr, &DEV_ATTR_TEST_PATTERN_12M_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    case S5KGM1SP_30FPS_4000x3000_RAW10:
        memcpy(pstDevAttr, &DEV_ATTR_S5KGM1SP_LINEAR_BASE,
                sizeof(VIN_DEV_ATTR_S));
        break;
    default:
        pr_err("not surpport sensor type\n");
        break;
    }
    pr_info("SAMPLE_VIN_GetDevAttrBySns success\n");

    return 0;
}

int SAMPLE_VIN_GetDevAttrExBySns(MIPI_SNS_TYPE_E enSnsType,
        VIN_DEV_ATTR_EX_S *pstDevAttrEx)
{
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
        memcpy(pstDevAttrEx, &DEV_ATTR_IMX327_MD_BASE,
                sizeof(VIN_DEV_ATTR_EX_S));
        break;

    default:
        pr_warn("not surpport sensor type\n");
        break;
    }
    pr_info("SAMPLE_VIN_GetDevAttrBySns success\n");
    return 0;
}

int SAMPLE_VIN_GetPipeAttrBySns(MIPI_SNS_TYPE_E enSnsType,
        VIN_PIPE_ATTR_S *pstPipeAttr) {
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
    case FEED_BACK_RAW12_1952P:
        memcpy(pstPipeAttr, &PIPE_ATTR_IMX327_LINEAR_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case IMX327_30FPS_2228P_RAW12_DOL2:
        memcpy(pstPipeAttr, &PIPE_ATTR_IMX327_DOL2_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case AR0233_30FPS_1080P_RAW12_954_PWL:
    case AR0233_30FPS_1080P_RAW12_960_PWL:
        memcpy(pstPipeAttr, &PIPE_ATTR_AR0233_1080P_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_LINEAR:
        memcpy(pstPipeAttr, &PIPE_ATTR_OS8A10_LINEAR_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_DOL2:
        memcpy(pstPipeAttr, &PIPE_ATTR_OS8A10_DOL2_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case OV10635_30FPS_720p_954_YUV:
    case OV10635_30FPS_720p_960_YUV:
    case SIF_TEST_PATTERN_YUV_720P:
        memcpy(pstPipeAttr, &PIPE_ATTR_OV10635_YUV_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case SIF_TEST_PATTERN0_1080P:
        memcpy(pstPipeAttr, &PIPE_ATTR_TEST_PATTERN_1080P_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case SIF_TEST_PATTERN_12M_RAW12:
        memcpy(pstPipeAttr, &PIPE_ATTR_TEST_PATTERN_12M_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    case S5KGM1SP_30FPS_4000x3000_RAW10:
        memcpy(pstPipeAttr, &PIPE_ATTR_S5KGM1SP_LINEAR_BASE,
                sizeof(VIN_PIPE_ATTR_S));
        break;
    default:
        pr_err("not surpport sensor type\n");
        break;
    }
    pr_info("SAMPLE_VIN_GetPipeAttrBySns success\n");

    return 0;
}

int SAMPLE_VIN_GetDisAttrBySns(MIPI_SNS_TYPE_E enSnsType,
        VIN_DIS_ATTR_S *pstDisAttr)
{
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
    case IMX327_30FPS_2228P_RAW12_DOL2:
    case AR0233_30FPS_1080P_RAW12_954_PWL:
    case AR0233_30FPS_1080P_RAW12_960_PWL:
    case SIF_TEST_PATTERN0_1080P:
    case FEED_BACK_RAW12_1952P:
        memcpy(pstDisAttr, &DIS_ATTR_BASE, sizeof(VIN_DIS_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_LINEAR:
    case OS8A10_30FPS_3840P_RAW10_DOL2:
        memcpy(pstDisAttr, &DIS_ATTR_OS8A10_BASE, sizeof(VIN_DIS_ATTR_S));
        break;
    case OV10635_30FPS_720p_954_YUV:
    case OV10635_30FPS_720p_960_YUV:
    case SIF_TEST_PATTERN_YUV_720P:
        memcpy(pstDisAttr, &DIS_ATTR_OV10635_BASE, sizeof(VIN_DIS_ATTR_S));
        break;
    case SIF_TEST_PATTERN_12M_RAW12:
    case S5KGM1SP_30FPS_4000x3000_RAW10:
        memcpy(pstDisAttr, &DIS_ATTR_12M_BASE, sizeof(VIN_DIS_ATTR_S));
        break;
    default:
        pr_err("not surpport sensor type\n");
        break;
    }
    pr_info("SAMPLE_VIN_GetDisAttrBySns success\n");

    return 0;
}

int SAMPLE_VIN_GetLdcAttrBySns(MIPI_SNS_TYPE_E enSnsType,
        VIN_LDC_ATTR_S *pstLdcAttr)
{
    switch (enSnsType) {
    case IMX327_30FPS_1952P_RAW12_LINEAR:
    case IMX327_30FPS_2228P_RAW12_DOL2:
    case AR0233_30FPS_1080P_RAW12_954_PWL:
    case AR0233_30FPS_1080P_RAW12_960_PWL:
    case SIF_TEST_PATTERN0_1080P:
    case FEED_BACK_RAW12_1952P:
        memcpy(pstLdcAttr, &LDC_ATTR_BASE, sizeof(VIN_LDC_ATTR_S));
        break;
    case OS8A10_30FPS_3840P_RAW10_LINEAR:
    case OS8A10_30FPS_3840P_RAW10_DOL2:
        memcpy(pstLdcAttr, &LDC_ATTR_OS8A10_BASE, sizeof(VIN_LDC_ATTR_S));
        break;
    case OV10635_30FPS_720p_954_YUV:
    case OV10635_30FPS_720p_960_YUV:
    case SIF_TEST_PATTERN_YUV_720P:
        memcpy(pstLdcAttr, &LDC_ATTR_OV10635_BASE, sizeof(VIN_LDC_ATTR_S));
        break;
    case SIF_TEST_PATTERN_12M_RAW12:
    case S5KGM1SP_30FPS_4000x3000_RAW10:
        memcpy(pstLdcAttr, &LDC_ATTR_12M_BASE, sizeof(VIN_LDC_ATTR_S));
        break;
    default:
        pr_err("not surpport sensor type\n");
        break;
    }
    pr_info("SAMPLE_VIN_GetLdcAttrBySns success\n");

    return 0;
}

int time_cost_ms(struct timeval *start, struct timeval *end)
{
    int time_ms = -1;
    time_ms = ((end->tv_sec * 1000 + end->tv_usec / 1000) -
            (start->tv_sec * 1000 + start->tv_usec / 1000));
    pr_info("time cost %d ms \n", time_ms);
    return time_ms;
}

void print_sensor_dev_info(VIN_DEV_ATTR_S *devinfo)
{
    pr_debug("devinfo->stSize.format %d\n", devinfo->stSize.format);
    pr_debug("devinfo->stSize.height %d\n", devinfo->stSize.height);
    pr_debug("devinfo->stSize.width %d\n", devinfo->stSize.width);
    pr_debug("devinfo->stSize.pix_length %d\n", devinfo->stSize.pix_length);
    pr_debug("devinfo->mipiAttr.enable_frame_id %d\n",
            devinfo->mipiAttr.enable_frame_id);
    pr_debug("devinfo->mipiAttr.enable_mux_out %d\n",
            devinfo->mipiAttr.enable_mux_out);
    pr_debug("devinfo->mipiAttr.set_init_frame_id %d\n",
            devinfo->mipiAttr.set_init_frame_id);
    pr_debug("devinfo->mipiAttr.ipi_channels %d\n",
            devinfo->mipiAttr.ipi_channels);
    pr_debug("devinfo->mipiAttr.enable_line_shift %d\n",
            devinfo->mipiAttr.enable_line_shift);
    pr_debug("devinfo->mipiAttr.enable_id_decoder %d\n",
            devinfo->mipiAttr.enable_id_decoder);
    pr_debug("devinfo->mipiAttr.set_bypass_channels %d\n",
            devinfo->mipiAttr.set_bypass_channels);
    pr_debug("devinfo->mipiAttr.enable_bypass %d\n",
            devinfo->mipiAttr.enable_bypass);
    pr_debug("devinfo->mipiAttr.set_line_shift_count %d\n",
            devinfo->mipiAttr.set_line_shift_count);
    pr_debug("devinfo->mipiAttr.enable_pattern %d\n",
            devinfo->mipiAttr.enable_pattern);

    pr_debug("devinfo->outDdrAttr.stride %d\n", devinfo->outDdrAttr.stride);
    pr_debug("devinfo->outDdrAttr.buffer_num %d\n",
            devinfo->outDdrAttr.buffer_num);
    return;
}

void print_sensor_pipe_info(VIN_PIPE_ATTR_S *pipeinfo)
{
    pr_debug("isp_out ddr_out_buf_num %d\n", pipeinfo->ddrOutBufNum);
    pr_debug("isp_out width %d\n", pipeinfo->stSize.width);
    pr_debug("isp_out height %d\n", pipeinfo->stSize.height);
    pr_debug("isp_out sensor_mode %d\n", pipeinfo->snsMode);
    pr_debug("isp_out format %d\n", pipeinfo->stSize.format);

    return;
}

void print_sensor_info(MIPI_SENSOR_INFO_S *snsinfo)
{
    pr_debug("bus_num %d\n", snsinfo->sensorInfo.bus_num);
    pr_debug("bus_type %d\n", snsinfo->sensorInfo.bus_type);
    pr_debug("sensor_name %s\n", snsinfo->sensorInfo.sensor_name);
    pr_debug("reg_width %d\n", snsinfo->sensorInfo.reg_width);
    pr_debug("sensor_mode %d\n", snsinfo->sensorInfo.sensor_mode);
    pr_debug("sensor_addr 0x%x\n", snsinfo->sensorInfo.sensor_addr);
    pr_debug("serial_addr 0x%x\n", snsinfo->sensorInfo.serial_addr);
    pr_debug("resolution %d\n", snsinfo->sensorInfo.resolution);

    return;
}

static void print_viobuf_info(hb_vio_buffer_t *buf)
{
    pr_debug("normal pipe_id (%d)type(%d)frame_id(%d)buf_index(%d)w x h(%dx%d) "
            "data_type %d img_format %d\n",
            buf->img_info.pipeline_id, buf->img_info.data_type,
            buf->img_info.frame_id, buf->img_info.buf_index,
            buf->img_addr.width,
            buf->img_addr.height, buf->img_info.data_type,
            buf->img_info.img_format);
}

void dis_crop_set(uint32_t pipe_id, uint32_t event, VIN_DIS_MV_INFO_S *data,
        void *userdata)
{
    pr_debug("dis_crop_set callback come in\n");
    pr_debug("data gmvX %d\n", data->gmvX);
    pr_debug("data gmvY %d\n", data->gmvY);
    pr_debug("data xUpdate %d\n", data->xUpdate);
    pr_debug("data yUpdate %d\n", data->yUpdate);
    return;
}

int hb_vin_vps_init(int pipeId, uint32_t sensorId, uint32_t mipiIdx,
        uint32_t deseri_port, uint32_t vin_vps_mode)
{
    int ret = 0;
    VIN_DEV_ATTR_S *devinfo = NULL;
    VIN_PIPE_ATTR_S *pipeinfo = NULL;
    VIN_DIS_ATTR_S *disinfo = NULL;
    VIN_LDC_ATTR_S *ldcinfo = NULL;
    VIN_DEV_ATTR_EX_S *devexinfo = NULL;
    VIN_DIS_CALLBACK_S pstDISCallback;
    pstDISCallback.VIN_DIS_DATA_CB = dis_crop_set;

    VPS_GRP_ATTR_S grp_attr;
    VPS_CHN_ATTR_S chn_attr;
    VPS_PYM_CHN_ATTR_S pym_chn_attr;

    devinfo = static_cast<VIN_DEV_ATTR_S*>(malloc(sizeof(VIN_DEV_ATTR_S)));
    if (devinfo == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    devexinfo =
        static_cast<VIN_DEV_ATTR_EX_S*>(malloc(sizeof(VIN_DEV_ATTR_EX_S)));
    if (devinfo == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    pipeinfo = static_cast<VIN_PIPE_ATTR_S*>(malloc(sizeof(VIN_PIPE_ATTR_S)));
    if (pipeinfo == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    disinfo = static_cast<VIN_DIS_ATTR_S*>(malloc(sizeof(VIN_DIS_ATTR_S)));
    if (disinfo == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    ldcinfo = static_cast<VIN_LDC_ATTR_S*>(malloc(sizeof(VIN_LDC_ATTR_S)));
    if (ldcinfo == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    memset(devinfo, 0, sizeof(VIN_DEV_ATTR_S));
    memset(pipeinfo, 0, sizeof(VIN_PIPE_ATTR_S));
    memset(disinfo, 0, sizeof(VIN_DIS_ATTR_S));
    memset(ldcinfo, 0, sizeof(VIN_LDC_ATTR_S));

    adapter_vin_vps_config(pipeId);
    SAMPLE_VIN_GetDevAttrBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId), devinfo);
    SAMPLE_VIN_GetPipeAttrBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId),
            pipeinfo);
    SAMPLE_VIN_GetDisAttrBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId), disinfo);
    SAMPLE_VIN_GetLdcAttrBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId), ldcinfo);
    SAMPLE_VIN_GetDevAttrExBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId),
            devexinfo);
    print_sensor_dev_info(devinfo);
    print_sensor_pipe_info(pipeinfo);

    ret = HB_SYS_SetVINVPSMode(pipeId,
            static_cast<SYS_VIN_VPS_MODE_E>(vin_vps_mode));
    if (ret < 0) {
        pr_err("HB_SYS_SetVINVPSMode%d error!\n", vin_vps_mode);
        return ret;
    }
    ret = HB_VIN_CreatePipe(pipeId, pipeinfo);  // isp init
    if (ret < 0) {
        pr_err("HB_MIPI_InitSensor error!\n");
        return ret;
    }
    ret = HB_VIN_SetMipiBindDev(pipeId, mipiIdx);
    if (ret < 0) {
        pr_err("HB_VIN_SetMipiBindDev error!\n");
        return ret;
    }
    ret = HB_VIN_SetDevVCNumber(pipeId, deseri_port);
    if (ret < 0) {
        pr_err("HB_VIN_SetDevVCNumber error!\n");
        return ret;
    }
    if (sensorId == IMX327_30FPS_2228P_RAW12_DOL2 ||
            sensorId == OS8A10_30FPS_3840P_RAW10_DOL2) {
        ret = HB_VIN_AddDevVCNumber(pipeId, vc_num[pipeId]);
        if (ret < 0) {
            pr_err("HB_VIN_AddDevVCNumber error!\n");
            return ret;
        }
    }
    ret = HB_VIN_SetDevAttr(pipeId, devinfo);  // sif init
    if (ret < 0) {
        pr_err("HB_MIPI_InitSensor error!\n");
        return ret;
    }
    if (need_md[pipeId]) {
        ret = HB_VIN_SetDevAttrEx(pipeId, devexinfo);
        if (ret < 0) {
            pr_err("HB_VIN_SetDevAttrEx error!\n");
            return ret;
        }
    }
    ret = HB_VIN_SetPipeAttr(pipeId, pipeinfo);  // isp init
    if (ret < 0) {
        pr_err("HB_VIN_SetPipeAttr error!\n");
        goto pipe_err;
    }
    ret = HB_VIN_SetChnDISAttr(pipeId, 1, disinfo);  //  dis init
    if (ret < 0) {
        pr_err("HB_VIN_SetChnDISAttr error!\n");
        goto pipe_err;
    }
    if (need_dis[pipeId]) {
        HB_VIN_RegisterDisCallback(pipeId, &pstDISCallback);
    }
    ret = HB_VIN_SetChnLDCAttr(pipeId, 1, ldcinfo);  //  ldc init
    if (ret < 0) {
        pr_err("HB_VIN_SetChnLDCAttr error!\n");
        goto pipe_err;
    }
    ret = HB_VIN_SetChnAttr(pipeId, 1);  //  dwe init
    if (ret < 0) {
        pr_err("HB_VIN_SetChnAttr error!\n");
        goto pipe_err;
    }
    ret = HB_VIN_SetDevBindPipe(pipeId, pipeId);  //  bind init
    if (ret < 0) {
        pr_err("HB_VIN_SetDevBindPipe error!\n");
        goto chn_err;
    }
    memset(&grp_attr, 0, sizeof(VPS_GRP_ATTR_S));
    grp_attr.maxW = pipeinfo->stSize.width;
    grp_attr.maxH = pipeinfo->stSize.height;
    ret = HB_VPS_CreateGrp(pipeId, &grp_attr);
    if (ret) {
        pr_err("HB_VPS_CreateGrp error!!!\n");
        goto chn_err;
    } else {
        pr_info("created a group ok:GrpId = %d\n", pipeId);
    }

    /* set group gdc */
    if (need_gdc[pipeId]) {
        pr_debug("need_gdc:%d, vin_vps_mode:%d, sensorId:%d\n",
                need_gdc[pipeId], vin_vps_mode, sensorId);
        if (vin_vps_mode == VIN_ONLINE_VPS_ONLINE ||
                vin_vps_mode == VIN_OFFLINE_VPS_ONLINE ||
                vin_vps_mode == VIN_SIF_ONLINE_DDR_ISP_ONLINE_VPS_ONLINE ||
                vin_vps_mode == VIN_SIF_OFFLINE_ISP_OFFLINE_VPS_ONLINE ||
                vin_vps_mode == VIN_FEEDBACK_ISP_ONLINE_VPS_ONLINE ||
                vin_vps_mode == VIN_SIF_VPS_ONLINE) {
            pr_err("vin_vps_mode error!!! "
                    "vps need be set offline mode if gdc enable\n");
            ret = -1;
            goto chn_err;
        }
        if ((sensorId == OS8A10_30FPS_3840P_RAW10_LINEAR) ||
            (sensorId == OS8A10_30FPS_3840P_RAW10_DOL2)) {
            pr_info("start to set GDC!!!\n");
            std::ifstream ifs("/app/bin/hapi_xj3/os8a10.bin");
            if (!ifs.is_open()) {
                pr_err("GDC file open failed!\n");
                ret = -1;
                goto chn_err;
            }
            ifs.seekg(0, std::ios::end);
            auto len = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            auto buf = new char[len];
            ifs.read(buf, len);

            ret = HB_VPS_SetGrpGdc(pipeId, buf, len, ROTATION_0);
            if (ret) {
                pr_err("HB_VPS_SetGrpGdc error!!!\n");
                goto chn_err;
            } else {
                pr_info("HB_VPS_SetGrpGdc ok: pipeId = %d\n", pipeId);
            }
            free(buf);
        }
    }

    for (int i = 0; i < g_iot_vio_cfg.chn_num[pipeId]; i++) {
        /* 1. set ipu chn */
        if (g_iot_vio_cfg.ipu_chn_en[pipeId][i] == 1) {
            memset(&chn_attr, 0, sizeof(VPS_CHN_ATTR_S));
            chn_attr.enScale = g_iot_vio_cfg.scale_en[pipeId][i];
            chn_attr.width = g_iot_vio_cfg.width[pipeId][i];
            chn_attr.height = g_iot_vio_cfg.height[pipeId][i];
            chn_attr.frameDepth = g_iot_vio_cfg.frame_depth[pipeId][i];
            ret = HB_VPS_SetChnAttr(pipeId, i, &chn_attr);
            if (ret) {
                pr_err("HB_VPS_SetChnAttr error!!!\n");
                goto chn_err;
            } else {
                pr_info("set ipu chn Attr ok: GrpId = %d, chn_id = %d\n",
                        pipeId, i);
            }
            HB_VPS_EnableChn(pipeId, i);
        }
        /* 2. set pym chn */
        if (g_iot_vio_cfg.pym_chn_en[pipeId][i] == 1) {
            memset(&pym_chn_attr, 0, sizeof(VPS_PYM_CHN_ATTR_S));
            memcpy(&pym_chn_attr,
                    &g_iot_vio_cfg.pym_cfg[pipeId][i],
                    sizeof(VPS_PYM_CHN_ATTR_S));
            ret = HB_VPS_SetPymChnAttr(pipeId, i, &pym_chn_attr);
            if (ret) {
                pr_err("HB_VPS_SetPymChnAttr error!!!\n");
                goto chn_err;
            } else {
                pr_info("HB_VPS_SetPymChnAttr ok: grp_id = %d g_pym_chn = %d\n",
                        pipeId, i);
            }
            HB_VPS_EnableChn(pipeId, i);
            pr_debug("ds5 factor:%d, x:%d, y:%d, roi_w:%d, roi_h:%d\n",
                    pym_chn_attr.ds_info[5].factor,
                    pym_chn_attr.ds_info[5].roi_x,
                    pym_chn_attr.ds_info[5].roi_y,
                    pym_chn_attr.ds_info[5].roi_width,
                    pym_chn_attr.ds_info[5].roi_height);
            pr_debug("ds6 factor:%d, x:%d, y:%d, roi_w:%d, roi_h:%d\n",
                    pym_chn_attr.ds_info[6].factor,
                    pym_chn_attr.ds_info[6].roi_x,
                    pym_chn_attr.ds_info[6].roi_y,
                    pym_chn_attr.ds_info[6].roi_width,
                    pym_chn_attr.ds_info[6].roi_height);
        }
    }

    struct HB_SYS_MOD_S src_mod, dst_mod;
    src_mod.enModId = HB_ID_VIN;
    src_mod.s32DevId = pipeId;
    if (vin_vps_mode == VIN_ONLINE_VPS_ONLINE ||
            vin_vps_mode == VIN_OFFLINE_VPS_ONLINE ||
            vin_vps_mode == VIN_SIF_ONLINE_DDR_ISP_ONLINE_VPS_ONLINE ||
            vin_vps_mode == VIN_SIF_OFFLINE_ISP_OFFLINE_VPS_ONLINE ||
            vin_vps_mode == VIN_FEEDBACK_ISP_ONLINE_VPS_ONLINE ||
            vin_vps_mode == VIN_SIF_VPS_ONLINE)
        src_mod.s32ChnId = 1;
    else
        src_mod.s32ChnId = 0;
    dst_mod.enModId = HB_ID_VPS;
    dst_mod.s32DevId = pipeId;
    dst_mod.s32ChnId = 0;
    ret = HB_SYS_Bind(&src_mod, &dst_mod);
    if (ret != 0) {
        pr_err("HB_SYS_Bind failed\n");
        goto chn_err;
    }

    if (need_chnfd[pipeId]) {
        g_vin_fd[pipeId] = HB_VIN_GetChnFd(pipeId, 0);
        if (g_vin_fd[pipeId] < 0) {
            pr_err("HB_VIN_GetChnFd error!!!\n");
        }
    }
    return ret;

chn_err:
    HB_VIN_DestroyPipe(pipeId);  // isp && dwe deinit
pipe_err:
    HB_VIN_DestroyDev(pipeId);   // sif deinit

    return ret;
}

static void hb_vin_vps_deinit(int pipeId, int sensorId)
{
    HB_VIN_DestroyDev(pipeId);     // sif deinit && destroy
    HB_VIN_DestroyChn(pipeId, 1);  // dwe deinit
    HB_VIN_DestroyPipe(pipeId);    // isp deinit && destroy
    if (need_chnfd[pipeId]) {
        HB_VIN_CloseFd();
    }
    HB_VPS_DestroyGrp(pipeId);
}

static int hb_vin_vps_start(int pipeId)
{
    int ret = 0;

    ret = HB_VIN_EnableChn(pipeId, 0);  // dwe start
    if (ret < 0) {
        pr_err("HB_VIN_EnableChn error!\n");
        return ret;
    }

    ret = HB_VPS_StartGrp(pipeId);
    if (ret) {
        pr_err("HB_VPS_StartGrp error!!!\n");
        return ret;
    } else {
        pr_info("start grp ok: grp_id = %d\n", pipeId);
    }

    ret = HB_VIN_StartPipe(pipeId);  // isp start
    if (ret < 0) {
        pr_err("HB_VIN_StartPipe error!\n");
        return ret;
    }
    ret = HB_VIN_EnableDev(pipeId);  // sif start && start thread
    if (ret < 0) {
        pr_err("HB_VIN_EnableDev error!\n");
        return ret;
    }

    return ret;
}

static void hb_vin_vps_stop(int pipeId)
{
    HB_VIN_DisableDev(pipeId);     // thread stop && sif stop
    HB_VIN_StopPipe(pipeId);       // isp stop
    HB_VIN_DisableChn(pipeId, 1);  // dwe stop
    HB_VPS_StopGrp(pipeId);
}

static int alloc_vb_buf_2lane(int index, void *buf,
        uint32_t size_y, uint32_t size_uv)
{
    int ret;
    hb_vio_buffer_t *buffer = reinterpret_cast<hb_vio_buffer_t*>(buf);

    if (buffer == NULL) {
        pr_err("buffer is NULL error!!!\n");
        return -1;
    }

    ret = HB_SYS_Alloc(&buffer->img_addr.paddr[0],
            reinterpret_cast<void**>(&buffer->img_addr.addr[0]),
            size_y);
    if (ret) {
        pr_err("index:%d, alloc vb buffer error, ret:%d\n",
                index, ret);
        return ret;
    }

    ret = HB_SYS_Alloc(&buffer->img_addr.paddr[1],
            reinterpret_cast<void**>(&buffer->img_addr.addr[1]),
            size_uv);
    if (ret) {
        pr_err("index:%d, alloc vb buffer error, ret:%d\n",
                index, ret);
        return ret;
    }

    pr_debug("index:%d, vio_buf_addr:%p, buf_y: vaddr = %p paddr = 0x%lx,"
            " buf_uv: vaddr = %p, paddr = 0x%lx\n",
            index, buffer,
            buffer->img_addr.addr[0], buffer->img_addr.paddr[0],
            buffer->img_addr.addr[1], buffer->img_addr.paddr[1]);

    return 0;
}


static int hb_feedback_buf_init(int pipeId)
{
    int ret, fb_buf_num;
    uint32_t fb_width, fb_height;
    uint32_t size_y, size_uv;
    hb_vio_buffer_t *fb_buf;

    fb_buf_num = g_iot_vio_cfg.fb_buf_num[pipeId];
    fb_width = g_iot_vio_cfg.fb_width[pipeId];
    fb_height = g_iot_vio_cfg.fb_height[pipeId];
    size_y = fb_width * fb_height;
    size_uv = size_y / 2;
    pr_debug("fb_buf_num:%d, width:%d, height:%d\n",
            fb_buf_num, fb_width, fb_height);
    g_feedback_buf[pipeId] = static_cast<hb_vio_buffer_t*>\
                             (malloc(sizeof(hb_vio_buffer_t) * fb_buf_num));
    if (g_feedback_buf[pipeId] == NULL) {
        pr_err("feedback buffer malloc failed!\n");
        return -1;
    }
    fb_buf = g_feedback_buf[pipeId];

    VP_CONFIG_S vp_config;
    memset(&vp_config, 0x00, sizeof(VP_CONFIG_S));
    vp_config.u32MaxPoolCnt = MAX_POOL_CNT;
    ret = HB_VP_SetConfig(&vp_config);
    if (ret) {
        pr_err("hb vp setconfig failed, ret:%d\n", ret);
        return ret;
    }

    ret = HB_VP_Init();
    if (ret) {
        pr_err("hb vp init failed, ret:%d\n", ret);
        return ret;
    }

    for (int i = 0; i < fb_buf_num; i++) {
        memset(&fb_buf[i], 0, sizeof(hb_vio_buffer_t));
        ret = alloc_vb_buf_2lane(i, &fb_buf[i], size_y, size_uv);
        if (ret) {
            pr_err("alloc vb buffer 2 land failed, ret:%d\n", ret);
            return ret;
        }
        fb_buf[i].img_info.planeCount = 2;
        fb_buf[i].img_info.img_format = 8;
        fb_buf[i].img_addr.width = fb_width;
        fb_buf[i].img_addr.height = fb_height;
        fb_buf[i].img_addr.stride_size = fb_width;
        g_fb_queue[pipeId].push(&fb_buf[i]);
    }

    return 0;
}

static int hb_feedback_init(int pipeId)
{
    int ret;
    VPS_GRP_ATTR_S grp_attr;
    VPS_CHN_ATTR_S chn_attr;
    VPS_PYM_CHN_ATTR_S pym_chn_attr;

    /* 1. set feedback buffer init */
    ret = hb_feedback_buf_init(pipeId);
    if (ret) {
        pr_err("hb feedback buffer init failed\n");
        return ret;
    }

    /* 2. set feedback group input resolution */
    memset(&grp_attr, 0, sizeof(VPS_GRP_ATTR_S));
    grp_attr.maxW = g_iot_vio_cfg.fb_width[pipeId];
    grp_attr.maxH = g_iot_vio_cfg.fb_height[pipeId];
    ret = HB_VPS_CreateGrp(pipeId, &grp_attr);
    if (ret) {
        pr_err("HB_VPS_CreateGrp error!!!\n");
        return ret;
    } else {
        pr_info("created a group ok:GrpId = %d\n", pipeId);
    }

    /* 3. set ipu and pym chn config */
    for (int i = 0; i < g_iot_vio_cfg.chn_num[pipeId]; i++) {
        /* 3.1 set ipu chn, feedback ipu chn is not must */
        if (g_iot_vio_cfg.ipu_chn_en[pipeId][i] == 1) {
            memset(&chn_attr, 0, sizeof(VPS_CHN_ATTR_S));
            chn_attr.enScale = g_iot_vio_cfg.scale_en[pipeId][i];
            chn_attr.width = g_iot_vio_cfg.width[pipeId][i];
            chn_attr.height = g_iot_vio_cfg.height[pipeId][i];
            chn_attr.frameDepth = g_iot_vio_cfg.frame_depth[pipeId][i];
            ret = HB_VPS_SetChnAttr(pipeId, i, &chn_attr);
            if (ret) {
                pr_err("HB_VPS_SetChnAttr error!!!\n");
                return ret;
            } else {
                pr_info("set ipu chn Attr ok: GrpId = %d, chn_id = %d\n",
                        pipeId, i);
            }
            HB_VPS_EnableChn(pipeId, i);
        }
    }

    for (int j = 0; j < g_iot_vio_cfg.chn_num[pipeId]; j++) {
        /**
         * 3.2 set pym chn, pym chn is must set
         *     and every grp only has one chn
         */
        if (g_iot_vio_cfg.pym_chn_en[pipeId][j] == 1) {
            memset(&pym_chn_attr, 0, sizeof(VPS_PYM_CHN_ATTR_S));
            memcpy(&pym_chn_attr,
                    &g_iot_vio_cfg.pym_cfg[pipeId][j],
                    sizeof(VPS_PYM_CHN_ATTR_S));
            ret = HB_VPS_SetPymChnAttr(pipeId, j, &pym_chn_attr);
            if (ret) {
                pr_err("HB_VPS_SetPymChnAttr error!!!\n");
                return ret;
            } else {
                pr_info("HB_VPS_SetPymChnAttr ok: grp_id = %d g_pym_chn = %d\n",
                        pipeId, j);
            }
            HB_VPS_EnableChn(pipeId, j);
            pr_debug("ds5 factor:%d, x:%d, y:%d, roi_w:%d, roi_h:%d\n",
                    pym_chn_attr.ds_info[5].factor,
                    pym_chn_attr.ds_info[5].roi_x,
                    pym_chn_attr.ds_info[5].roi_y,
                    pym_chn_attr.ds_info[5].roi_width,
                    pym_chn_attr.ds_info[5].roi_height);
            pr_debug("ds6 factor:%d, x:%d, y:%d, roi_w:%d, roi_h:%d\n",
                    pym_chn_attr.ds_info[6].factor,
                    pym_chn_attr.ds_info[6].roi_x,
                    pym_chn_attr.ds_info[6].roi_y,
                    pym_chn_attr.ds_info[6].roi_width,
                    pym_chn_attr.ds_info[6].roi_height);
            break;  // a group only surpport a pym chn
        }
    }

    return 0;
}

static int hb_feedback_deinit(int pipeId)
{
    int ret, cnt;
    hb_vio_buffer_t *buffer;

    cnt = g_fb_queue[pipeId].size();

    for (int i = 0; i < cnt; i++) {
        buffer = g_fb_queue[pipeId].front();
        g_fb_queue[pipeId].pop();
        if (buffer == NULL) {
            pr_err("feedback queue buf[%d] is NULL\n", i);
            return -1;
        }
        ret = HB_SYS_Free(buffer->img_addr.paddr[0], buffer->img_addr.addr[0]);
        if (ret == 0) {
            ret = HB_SYS_Free(buffer->img_addr.paddr[1],
                    buffer->img_addr.addr[1]);
            if (ret == 0) {
                pr_debug("index:%d , vio_buf_addr:%p, "
                        "mmzFree y vaddr = %p, paddr = 0x%lx, "
                        "mmzFree uv vaddr = %p, paddr = 0x%lx\n",
                        i, buffer,
                        buffer->img_addr.addr[0],
                        buffer->img_addr.paddr[0],
                        buffer->img_addr.addr[1],
                        buffer->img_addr.paddr[1]);
            } else {
                pr_err("hb sys free uv vio buf[%d] falied, ret:%d\n", i, ret);
                return ret;
            }
        } else {
            pr_err("hb sys free y vio buf[%d] falied, ret:%d\n", i, ret);
            return ret;
        }
    }

    if (g_feedback_buf[pipeId]) {
        free(g_feedback_buf[pipeId]);
    }
    ret = HB_VP_Exit();
    if (ret == 0) {
        pr_info("vp exit ok!\n");
    } else {
        pr_err("vp exit error!\n");
        return ret;
    }
    ret = HB_VPS_DestroyGrp(pipeId);
    if (ret) {
        pr_err("hb vps destroy group falied\n");
        return ret;
    }

    return 0;
}

static int hb_feedback_start(int pipeId)
{
    int ret = 0;
    ret = HB_VPS_StartGrp(pipeId);
    if (ret) {
        pr_err("HB_VPS_StartGrp error!!!\n");
        return ret;
    } else {
        pr_info("start grp ok: grp_id = %d\n", pipeId);
    }

    return 0;
}

static int hb_feedback_stop(int pipeId)
{
    int ret;

    ret = HB_VPS_StopGrp(pipeId);
    if (ret) {
        pr_err("HB_VPS_StopGrp error!!!\n");
        return ret;
    } else {
        pr_info("stop grp ok: grp_id = %d\n", pipeId);
    }

    return 0;
}

static int hb_sensor_init(int devId, int sensorId, int bus, int port,
        int mipiIdx, int sedres_index, int sedres_port)
{
    int ret = 0;
    MIPI_SENSOR_INFO_S *snsinfo = NULL;

    snsinfo =
        static_cast<MIPI_SENSOR_INFO_S*>(malloc(sizeof(MIPI_SENSOR_INFO_S)));
    if (snsinfo == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    memset(snsinfo, 0, sizeof(MIPI_SENSOR_INFO_S));
    SAMPLE_MIPI_GetSnsAttrBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId),
            snsinfo);

    HB_MIPI_SetBus(snsinfo, bus);
    HB_MIPI_SetPort(snsinfo, port);
    HB_MIPI_SensorBindSerdes(snsinfo, sedres_index, sedres_port);
    HB_MIPI_SensorBindMipi(snsinfo, mipiIdx);
    print_sensor_info(snsinfo);

    ret = HB_MIPI_InitSensor(devId, snsinfo);
    if (ret < 0) {
        pr_err("hb mipi init sensor error!\n");
        return ret;
    }
    pr_info("hb sensor init success...\n");

    return 0;
}

int hb_mipi_init(int sensorId, int mipiIdx)
{
    int ret = 0;
    MIPI_ATTR_S *mipi_attr = NULL;

    mipi_attr = static_cast<MIPI_ATTR_S*>(malloc(sizeof(MIPI_ATTR_S)));
    if (mipi_attr == NULL) {
        pr_err("malloc error\n");
        return -1;
    }
    memset(mipi_attr, 0, sizeof(MIPI_ATTR_S));
    SAMPLE_MIPI_GetMipiAttrBySns(static_cast<MIPI_SNS_TYPE_E>(sensorId),
            mipi_attr);

    ret = HB_MIPI_SetMipiAttr(mipiIdx, mipi_attr);
    if (ret < 0) {
        pr_err("hb mipi set mipi attr error!\n");
        return ret;
    }
    pr_info("hb mipi init success...\n");

    return 0;
}

int hb_sensor_deinit(int devId)
{
    int ret = 0;

    ret = HB_MIPI_DeinitSensor(devId);
    if (ret < 0) {
        pr_err("hb deinit sensor error!\n");
        return ret;
    }

    return 0;
}

int hb_mipi_deinit(int mipiIdx)
{
    int ret = 0;

    ret = HB_MIPI_Clear(mipiIdx);
    if (ret < 0) {
        pr_err("hb mipi clear error!\n");
        return ret;
    }
    pr_info("hb_mipi_deinit success...\n");

    return 0;
}

int hb_sensor_start(int devId)
{
    int ret = 0;

    ret = HB_MIPI_ResetSensor(devId);
    if (ret < 0) {
        pr_err("hb mipi reset sensor error!\n");
        return ret;
    }

    return 0;
}

int hb_mipi_start(int mipiIdx)
{
    int ret = 0;

    ret = HB_MIPI_ResetMipi(mipiIdx);
    if (ret < 0) {
        pr_err("hb mipi reset mipi error!\n");
        return ret;
    }

    return 0;
}

int hb_sensor_stop(int devId)
{
    int ret = 0;

    ret = HB_MIPI_UnresetSensor(devId);
    if (ret < 0) {
        pr_err("hb mipi unreset sensor error!\n");
        return ret;
    }

    return 0;
}

int hb_mipi_stop(int mipiIdx)
{
    int ret = 0;
    ret = HB_MIPI_UnresetMipi(mipiIdx);
    if (ret < 0) {
        pr_err("HB_MIPI_UnresetMipi error!\n");
        return ret;
    }

    return 0;
}


int iot_cam_init(uint32_t cfg_index, const char *cfg_file)
{
    int i, ret;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot cam init index:%d\n", i);
            if (need_clk[i] == 1) {
                ret = HB_MIPI_EnableSensorClock(mipiIdx[i]);
                if (ret) {
                    pr_err("hb mipi enable sensor clock error,"
                            " index:%d, ret:%d\n",
                            i, ret);
                    return ret;
                }
            }
            ret = hb_sensor_init(i, sensorId[i], bus[i], port[i], mipiIdx[i],
                    serdes_index[i], serdes_port[i]);
            if (ret < 0) {
                pr_err("hb_sensor_init error! do vio deinit,"
                        " index:%d, ret:%d\n",
                        i, ret);
                hb_vin_vps_deinit(i, mipiIdx[i]);
                return ret;
            }
            ret = hb_mipi_init(sensorId[i], mipiIdx[i]);
            if (ret < 0) {
                pr_err("hb_mipi_init error! do vio deinit, index:%d, ret:%d\n",
                        i, ret);
                hb_vin_vps_deinit(i, mipiIdx[i]);
                return ret;
            }
        }
    }

    return 0;
}

int iot_cam_deinit(uint32_t cfg_index)
{
    int i, ret;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot cam deinit index:%d\n", i);
            ret = hb_mipi_deinit(mipiIdx[i]);
            if (ret < 0) {
                pr_err("hb mipi deinit error, ret:%d\n", ret);
                return ret;
            }
            ret = hb_sensor_deinit(i);
            if (ret < 0) {
                pr_err("hb sensor deinit error, ret:%d\n", ret);
                return ret;
            }
            if (need_clk[i] == 1) {
                ret = HB_MIPI_DisableSensorClock(mipiIdx[i]);
                if (ret) {
                    pr_err("hb mipi disable sensor clock error, ret:%d\n", ret);
                    return ret;
                }
            }
        }
    }

    return 0;
}

int iot_cam_start(uint32_t port)
{
    int i, ret;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot cam start index:%d\n", i);
            ret = hb_sensor_start(i);
            if (ret < 0) {
                pr_err("hb_mipi_start error! do cam && vio deinit\n");
                hb_sensor_stop(i);
                hb_mipi_stop(mipiIdx[i]);
                hb_sensor_deinit(i);
                hb_mipi_deinit(mipiIdx[i]);
                hb_vin_vps_stop(i);
                hb_vin_vps_deinit(i, sensorId[i]);
                return ret;
            }

            ret = hb_mipi_start(mipiIdx[i]);
            if (ret < 0) {
                pr_err("hb_mipi_start error! do cam && vio deinit\n");
                hb_sensor_stop(i);
                hb_mipi_stop(mipiIdx[i]);
                hb_sensor_deinit(i);
                hb_mipi_deinit(mipiIdx[i]);
                hb_vin_vps_stop(i);
                hb_vin_vps_deinit(i, sensorId[i]);
                return ret;
            }
        }
    }

    return 0;
}

int iot_cam_stop(uint32_t port)
{
    int i, ret;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot cam stop index:%d\n", i);
            ret = hb_sensor_stop(i);
            if (ret < 0) {
                pr_err("hb sensor stop error, index:%d, ret:%d\n", i, ret);
                return ret;
            }
            ret = hb_mipi_stop(mipiIdx[i]);
            if (ret < 0) {
                pr_err("hb mipi stop error, index:%d, ret:%d\n", i, ret);
                return ret;
            }
        }
    }

    return 0;
}

int iot_vio_init(const char *cfg_file)
{
    int i, ret;
    auto config = std::make_shared<IotVioConfig>(cfg_file);
    if (!config || !config->LoadConfig()) {
        pr_err("falied to load config file: %s\n", cfg_file);
        return -1;
    }
    config->ParserConfig();
    parser_iot_vio_cfg();

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot vio init index:%d\n", i);
            if (need_cam) {
                pr_info("vio_index:%d, groupMask:%d, sensorId:%d, mipiIdx:%d\n",
                        i, groupMask, sensorId[i], mipiIdx[i]);
                ret = hb_vin_vps_init(i, sensorId[i], mipiIdx[i],
                        serdes_port[i], vin_vps_mode[i]);
                if (ret < 0) {
                    pr_err("hb_vin_init error, index:%d, ret:%d\n", i, ret);
                    return ret;
                }
            } else {
                ret = hb_feedback_init(i);
                if (ret < 0) {
                    pr_err("hb_feedback_init error, index:%d, ret:%d\n",
                            i, ret);
                    return ret;
                }
            }
        }
    }

    return 0;
}

int iot_vio_deinit()
{
    int i;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot vio init index:%d\n", i);
            if (need_cam) {
                hb_vin_vps_deinit(i, sensorId[i]);
            } else {
                hb_feedback_deinit(i);
            }
        }
    }
    return 0;
}

int iot_vio_start(void)
{
    int i, ret;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot vio start index:%d\n", i);
            if (need_cam) {
                pr_info("cam_index:%d, groupMask:%d\n",
                        i, groupMask);
                ret = hb_vin_vps_start(i);
                if (ret < 0) {
                    pr_err("hb_vin_sif_isp_start error!"
                            " do cam && vio deinit\n");
                    hb_sensor_deinit(i);
                    hb_mipi_deinit(mipiIdx[i]);
                    hb_vin_vps_stop(i);
                    hb_vin_vps_deinit(i, sensorId[i]);
                    return ret;
                }
            } else {
                pr_info("feedback_index:%d, groupMask:%d\n",
                        i, groupMask);
                ret = hb_feedback_start(i);
                if (ret < 0) {
                    pr_err("hb_feedback_start error!"
                            " do feedback stop&deinit\n");
                    hb_feedback_stop(i);
                    hb_feedback_deinit(i);
                    return ret;
                }
            }
        }
    }

    return 0;
}

int iot_vio_stop(void)
{
    int i;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_info("iot vio stop index:%d\n", i);
            if (need_cam)
                hb_vin_vps_stop(i);
            else
                hb_feedback_stop(i);
        }
    }

    return 0;
}

static int dumpToFile2plane(char *filename, char *srcBuf, char *srcBuf1,
        unsigned int size, unsigned int size1,
        int width, int height, int stride)
{
    FILE *yuvFd = NULL;
    char *buffer = NULL;
    int i = 0;

    yuvFd = fopen(filename, "w+");

    if (yuvFd == NULL) {
        pr_err("open(%s) fail", filename);
        return -1;
    }

    buffer = reinterpret_cast<char *>(malloc(size + size1));

    if (buffer == NULL) {
        pr_err("malloc falied");
        fclose(yuvFd);
        return -1;
    }

    if (width == stride) {
        memcpy(buffer, srcBuf, size);
        memcpy(buffer + size, srcBuf1, size1);
    } else {
        // jump over stride - width Y
        for (i = 0; i < height; i++) {
            memcpy(buffer+i*width, srcBuf+i*stride, width);
        }

        // jump over stride - width UV
        for (i = 0; i < height/2; i++) {
            memcpy(buffer+size+i*width, srcBuf1+i*stride, width);
        }
    }

    fflush(stdout);

    fwrite(buffer, 1, size + size1, yuvFd);

    fflush(yuvFd);

    if (yuvFd)
        fclose(yuvFd);
    if (buffer)
        free(buffer);

    pr_debug("filedump(%s, size(%d) is successed!!\n", filename, size);

    return 0;
}

static void vio_dump_pym_data(int grp_id, int pym_chn,
        pym_buffer_t *out_pym_buf)
{
    int i;
    char file_name[100] = {0};

    for (i = 0; i < 6; i++) {
        snprintf(file_name,
                sizeof(file_name),
                "grp%d_chn%d_pym_out_basic_layer_DS%d_%d_%d.yuv",
                grp_id, pym_chn, i * 4,
                out_pym_buf->pym[i].width,
                out_pym_buf->pym[i].height);
        dumpToFile2plane(
                file_name,
                out_pym_buf->pym[i].addr[0],
                out_pym_buf->pym[i].addr[1],
                out_pym_buf->pym[i].width * out_pym_buf->pym[i].height,
                out_pym_buf->pym[i].width * out_pym_buf->pym[i].height / 2,
                out_pym_buf->pym[i].width,
                out_pym_buf->pym[i].height,
                out_pym_buf->pym[i].stride_size);
        for (int j = 0; j < 3; j++) {
            snprintf(file_name,
                    sizeof(file_name),
                    "grp%d_chn%d_pym_out_roi_layer_DS%d_%d_%d.yuv",
                    grp_id, pym_chn, i * 4 + j + 1,
                    out_pym_buf->pym_roi[i][j].width,
                    out_pym_buf->pym_roi[i][j].height);
            if (out_pym_buf->pym_roi[i][j].width != 0)
                dumpToFile2plane(
                        file_name,
                        out_pym_buf->pym_roi[i][j].addr[0],
                        out_pym_buf->pym_roi[i][j].addr[1],
                        out_pym_buf->pym_roi[i][j].width *
                        out_pym_buf->pym_roi[i][j].height,
                        out_pym_buf->pym_roi[i][j].width *
                        out_pym_buf->pym_roi[i][j].height / 2,
                        out_pym_buf->pym_roi[i][j].width,
                        out_pym_buf->pym_roi[i][j].height,
                        out_pym_buf->pym_roi[i][j].stride_size);
        }
        snprintf(file_name,
                sizeof(file_name),
                "grp%d_chn%d_pym_out_us_layer_US%d_%d_%d.yuv",
                grp_id, pym_chn, i,
                out_pym_buf->us[i].width,
                out_pym_buf->us[i].height);
        if (out_pym_buf->us[i].width != 0)
            dumpToFile2plane(
                    file_name,
                    out_pym_buf->us[i].addr[0],
                    out_pym_buf->us[i].addr[1],
                    out_pym_buf->us[i].width * out_pym_buf->us[i].height,
                    out_pym_buf->us[i].width * out_pym_buf->us[i].height / 2,
                    out_pym_buf->us[i].width,
                    out_pym_buf->us[i].height,
                    out_pym_buf->us[i].stride_size);
    }
}

static int vio_dump_pym_layer_data(int grp_id, int pym_chn, int layer,
        pym_buffer_t *out_pym_buf)
{
    char file_name[100] = {0};
    address_info_t pym_addr;
    int y_len, uv_len;
    static int index;

    uint32_t frame_id = out_pym_buf->pym_img_info.frame_id;
    uint64_t ts = out_pym_buf->pym_img_info.time_stamp;

    pr_debug("ts:%lu, frameID:%d\n", ts, frame_id);

    if (out_pym_buf == NULL) {
        pr_err("out_pym_buf is NULL,error!!!\n");
        return -1;
    }

    if (layer % 4 == 0) {
        pym_addr = out_pym_buf->pym[layer / 4];
        snprintf(file_name,
                sizeof(file_name),
                "%d_grp%d_chn%d_pym_out_basic_layer_DS%d_%d_%d.yuv",
                index++, grp_id, pym_chn, layer,
                pym_addr.width, pym_addr.height);
    } else {
        pym_addr = out_pym_buf->pym_roi[layer / 4][layer % 4 - 1];
        snprintf(file_name,
                sizeof(file_name),
                "%d_grp%d_chn%d_pym_out_roi_layer_DS%d_%d_%d.yuv",
                index++, grp_id, pym_chn, layer,
                pym_addr.width, pym_addr.height);
    }

    if (pym_addr.width == 0 ||
            pym_addr.height == 0) {
        pr_err("pym_addr width:%d or height:%d error!!!\n",
                pym_addr.width, pym_addr.height);
        return -1;
    }

    y_len = pym_addr.width * pym_addr.height;
    uv_len = y_len / 2;
    dumpToFile2plane(file_name,
            pym_addr.addr[0],
            pym_addr.addr[1],
            y_len, uv_len,
            pym_addr.width,
            pym_addr.height,
            pym_addr.stride_size);

    return 0;
}

static void vio_dump_ipu_data(int grp_id, int ipu_chn,
        hb_vio_buffer_t *out_ipu_buf)
{
    char file_name[100] = {0};

    print_viobuf_info(out_ipu_buf);
    snprintf(file_name,
            sizeof(file_name),
            "grp%d_chn%d_%d_%d.yuv",
            grp_id, ipu_chn,
            out_ipu_buf->img_addr.width,
            out_ipu_buf->img_addr.height);

    dumpToFile2plane(
            file_name,
            out_ipu_buf->img_addr.addr[0],
            out_ipu_buf->img_addr.addr[1],
            out_ipu_buf->img_addr.width * out_ipu_buf->img_addr.height,
            out_ipu_buf->img_addr.width * out_ipu_buf->img_addr.height / 2,
            out_ipu_buf->img_addr.width,
            out_ipu_buf->img_addr.height,
            out_ipu_buf->img_addr.stride_size);
}

static int vio_check_pym(pym_buffer_t *pym_buf)
{
    int i;
    int ret = 0;
    address_info_t pym_addr;

    for (i = 0; i < DOWN_SCALE_MAX; ++i) {
        if (i % 4 == 0) {  // base pym layer
            pym_addr = pym_buf->pym[i / 4];
            if (pym_addr.width == 0 || pym_addr.height == 0) {
                ret = -1;
            }
        } else {  // roi pym layer
            pym_addr = pym_buf->pym_roi[i / 4][i % 4 - 1];
        }
        pr_debug("layer:%d, width:%d, height:%d, stride_size:%d, "
                "y_addr:%p, uv_addr:%p\n",
                i,
                pym_addr.width, pym_addr.height, pym_addr.stride_size,
                pym_addr.addr[0], pym_addr.addr[1]);
        if (ret) {
            pr_err("vio check pym failed, "
                    "pym_layer:%d, width:%d, height:%d\n",
                    i, pym_addr.width, pym_addr.height);
            return ret;
        }
    }

    return 0;
}


static int vio_get_info(int grp_id, int info_type, void *buf, int timeout)
{
    int ret = 0;
    int chret = 0;
    hb_vio_buffer_t *ipu_buf, *vb_buf, *fb_src_buf;
    pym_buffer_t *pym_buf;
    // pym_buffer_t out_pym_buf;

    switch (info_type) {
    case IOT_VIO_SRC_INFO:
        ipu_buf = reinterpret_cast<hb_vio_buffer_t *>(buf);
        if (ipu_chn[grp_id][0] < 0) {
            pr_err("info type:%s, no ipu chn is enable, "
                    "grp_id:%d, chn value:%d\n",
                    iot_type_info[info_type], grp_id, pym_chn[grp_id][0]);
            return -1;
        }
        ret = HB_VPS_GetChnFrame(grp_id, ipu_chn[grp_id][0], ipu_buf, timeout);
        if (ret != 0) {
            pr_err("info_type:%s, HB_VPS_GetChnFrame error, ret:%d\n",
                    iot_type_info[info_type], ret);
            return ret;
        }
        if (vps_dump) {
            vio_dump_ipu_data(grp_id, ipu_chn[grp_id][0], ipu_buf);
        }
        break;
    case IOT_VIO_PYM_INFO:
        pym_buf = reinterpret_cast<pym_buffer_t *>(buf);
        if (pym_chn[grp_id][0] < 0) {
            LOGE << "info type:" << iot_type_info[info_type]
                 << ", no pym chn is enable, grp_id:"
                 << grp_id
                 << ", chn value:" << pym_chn[grp_id][0];
            return -1;
        }
        ret = HB_VPS_GetChnFrame(grp_id, pym_chn[grp_id][0], pym_buf, timeout);
        if (ret != 0) {
            LOGE << "info_type:" << iot_type_info[info_type]
                 << ", HB_VPS_GetChnFrame error, ret:"
                 << ret;
            return ret;
        }
        chret = vio_check_pym(pym_buf);
        if (chret != 0) {
            LOGE << "info_type:" << iot_type_info[info_type]
                 << ", vio check pym error chret:"
                 << chret;
            return chret;
        }
        if (vps_dump) {
            vio_dump_pym_data(grp_id, pym_chn[grp_id][0], pym_buf);
            if (vps_layer_dump >=0 && vps_layer_dump < MAX_PYM_DS_NUM)
                vio_dump_pym_layer_data(grp_id, pym_chn[grp_id][0],
                        vps_layer_dump, pym_buf);
        }
        break;
    case IOT_VIO_PYM_MULT_INFO:
#if 0
        mult_pym_buf = reinterpret_cast<iot_mult_img_info_t *>(buf);
        ret = HB_VPS_GetChnFrame(grp_id, pym_chn, &out_pym_buf, timeout);
        if (ret != 0) {
            pr_err("info_type:%s, HB_VPS_GetChnFrame error, ret:%d\n",
                    iot_type_info[info_type], ret);
            return ret;
        }
        mult_pym_buf->img_info[mult_pym_buf->img_num] = out_pym_buf;
        mult_pym_buf->img_num++;
#endif
        break;
    case IOT_VIO_FEEDBACK_SRC_INFO:
        fb_src_buf = reinterpret_cast<hb_vio_buffer_t*>(buf);
        if (g_fb_queue[0].size() == 0) {
            pr_err("info_type:%s, g_fb_queue size is zero\n",
                    iot_type_info[info_type]);
            return -1;
        }
        vb_buf = g_fb_queue[0].front();
        g_fb_queue[0].pop();
        if (vb_buf == NULL) {
            pr_err("info_type:%s, vb buf is NULL\n", iot_type_info[info_type]);
            return -1;
        }
        memcpy(fb_src_buf, vb_buf, sizeof(hb_vio_buffer_t));
        g_fb_queue[0].push(vb_buf);  // push again for cycle queue
        break;
    case IOT_VIO_FEEDBACK_SRC_MULT_INFO:
        break;
    default:
        pr_err("info type:%d error\n", info_type);
        return -1;
    }

    return ret;
}

static int vio_set_info(int grp_id, int info_type, void *buf, int timeout)
{
    int ret = 0;
    hb_vio_buffer_t *fb_buf;

    switch (info_type) {
    case IOT_VIO_FEEDBACK_FLUSH:
        ret = 0;
        break;
    case IOT_VIO_FEEDBACK_PYM_PROCESS:
        fb_buf = reinterpret_cast<hb_vio_buffer_t*>(buf);
        ret = HB_VPS_SendFrame(grp_id, fb_buf, timeout);
        if (ret) {
            pr_err("info_type:%s, HB_VPS_SendFrame error, ret:%d\n",
                    iot_type_info[info_type], ret);
            return ret;
        }
        break;
    default:
        pr_err("info type:%d error\n", info_type);
        return -1;
    }

    return ret;
}

int iot_vio_set_info(uint32_t info_type, void *data)
{
    int i, ret;

    check_type_valid(info_type);
    check_params_valid(data);
    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            ret = vio_set_info(i, info_type, data, 1000);
            if (ret) {
                pr_err("vio set info falied,"
                        "grp_index:%d, info type:%s\n",
                        i, iot_type_info[info_type]);
                return ret;
            }
            pr_debug("grp_index:%d , info_type: %s success...\n",
                    i, iot_type_info[info_type]);
        }
    }

    return 0;
}

int iot_vio_get_info(uint32_t info_type, void *data)
{
    int i;
    int ret = 0;

    check_type_valid(info_type);
    check_params_valid(data);
    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            ret = vio_get_info(i, info_type, data, 2000);
            if (ret) {
                pr_err("vio get info falied,"
                        "grp_index:%d, info type:%s\n",
                        i, iot_type_info[info_type]);
                return ret;
            }
            pr_debug("grp_index:%d , info_type: %s success...\n",
                    i, iot_type_info[info_type]);
        }
    }

    return 0;
}

static int vio_free_info(int grp_id, int info_type, void *buf)
{
    int ret = 0;

    switch (info_type) {
    case IOT_VIO_SRC_INFO:
        if (ipu_chn[grp_id][0] < 0) {
            pr_err("info type:%s, no ipu chn is enable, "
                    "grp_id:%d, chn value:%d\n",
                    iot_type_info[info_type], grp_id, pym_chn[grp_id][0]);
            return -1;
        }
        ret = HB_VPS_ReleaseChnFrame(grp_id, ipu_chn[grp_id][0], buf);
        if (ret != 0) {
            pr_err("info_type:%s, HB_VPS_ReleaseChnFrame error, ret:%d\n",
                    iot_type_info[info_type], ret);
            return ret;
        }
        break;
    case IOT_VIO_PYM_INFO:
        if (pym_chn[grp_id][0] < 0) {
            pr_err("info type:%s, no pym chn is enable, "
                    "grp_id:%d, chn value:%d\n",
                    iot_type_info[info_type], grp_id, pym_chn[grp_id][0]);
            return -1;
        }
        ret = HB_VPS_ReleaseChnFrame(grp_id, pym_chn[grp_id][0], buf);
        if (ret != 0) {
            pr_err("info_type:%s, HB_VPS_ReleaseChnFrame error, ret:%d\n",
                    iot_type_info[info_type], ret);
            return ret;
        }
        break;
    case IOT_VIO_FEEDBACK_SRC_INFO:
        ret = 0;
        break;
#if 0
    case IOT_VIO_PYM_MULT_INFO:
        ret = HB_VPS_ReleaseChnFrame(grp_id, pym_chn, buf);
        if (ret != 0) {
            pr_err("info_type:%s, HB_VPS_ReleaseChnFrame error, ret:%d\n",
                    iot_type_info[info_type], ret);
        }
        break;
#endif
    default:
        pr_err("info type:%d error\n", info_type);
        return -1;
    }

    return ret;
}

int iot_vio_free_info(uint32_t info_type, void *data)
{
    int i;
    int ret = 0;

    check_type_valid(info_type);
    check_params_valid(data);
    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            ret = vio_free_info(i, info_type, data);
            if (ret) {
                pr_err("vio free info falied, grp_index:%d, info type:%s\n",
                        i, iot_type_info[info_type]);
                return ret;
            }
            pr_debug("grp_index:%d , info_type: %s success...\n",
                    i, iot_type_info[info_type]);
        }
    }

    return 0;
}

int iot_vio_free(void *data) {
    int i, ret;

    check_params_valid(data);
    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            ret = iot_vio_free_info(IOT_VIO_PYM_INFO, data);
            if (ret) {
                pr_err("vio free pym info falied, grp_index:%d\n", i);
                return ret;
            }
        }
    }

    return 0;
}

int iot_vio_pym_process(hb_vio_buffer_t *src_img_info) {
    int i, ret;

    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            ret = iot_vio_set_info(IOT_VIO_FEEDBACK_PYM_PROCESS, src_img_info);
            if (ret) {
                pr_err("vio set feedback pym process falied,"
                        " grp_index:%d\n", i);
                return ret;
            }
        }
    }

    return 0;
}

int iot_vio_mult_pym_process(iot_mult_src_info_t *src_img_info)
{
    return 0;
}

static void print_iot_vio_cfg()
{
    int i, j, k, m, n;
    int grp_start_index, grp_max_num;

    /* 1. print sensor config */
    pr_debug("************ AIoT Vio Config Start *************\n");
    pr_debug("need_cam: %d\n", g_iot_vio_cfg.need_cam);
    pr_debug("cam_num: %d\n", g_iot_vio_cfg.cam_num);
    pr_debug("vps_dump: %d\n", g_iot_vio_cfg.vps_dump);
    pr_debug("vps_layer_dump: %d\n", g_iot_vio_cfg.vps_layer_dump);
    for (n = 0; n < g_iot_vio_cfg.cam_num; n++) {
        pr_debug("#########cam:%d config start##########\n", n);
        pr_debug("cam_index: %d sensor_id: %d\n",
                n, g_iot_vio_cfg.sensor_id[n]);
        pr_debug("cam_index: %d sensor_port: %d\n",
                n, g_iot_vio_cfg.sensor_port[n]);
        if (n < MAX_MIPIID_NUM) {
            pr_debug("cam_index: %d mipi_idx: %d\n",
                    n, g_iot_vio_cfg.mipi_idx[n]);
        }
        pr_debug("cam_index: %d i2c_bus: %d\n",
                n, g_iot_vio_cfg.i2c_bus[n]);
        pr_debug("cam_index: %d serdes_index: %d\n",
                n, g_iot_vio_cfg.serdes_index[n]);
        pr_debug("cam_index: %d serdes_port: %d\n",
                n, g_iot_vio_cfg.serdes_port[n]);
        pr_debug("cam_index: %d temper_mode: %d\n",
                n, g_iot_vio_cfg.temper_mode[n]);
        pr_debug("cam_index: %d grp_num: %d\n",
                n, g_iot_vio_cfg.grp_num[n]);
        /* 2. print group config */
        if (g_iot_vio_cfg.need_cam == 1) {
            grp_start_index = n;
            grp_max_num = 1;
        } else {
            grp_start_index = 0;
            grp_max_num = g_iot_vio_cfg.grp_num[n];
        }
        for (i = grp_start_index; i < grp_max_num; i++) {
            pr_debug("grp_index: %d fb_width: %d\n",
                    i, g_iot_vio_cfg.fb_width[i]);
            pr_debug("grp_index: %d fb_height: %d\n",
                    i, g_iot_vio_cfg.fb_height[i]);
            pr_debug("grp_index: %d fb_buf_num: %d\n",
                    i, g_iot_vio_cfg.fb_buf_num[i]);
            pr_debug("grp_index: %d vin_fd: %d\n",
                    i, g_iot_vio_cfg.vin_fd[i]);
            pr_debug("grp_index: %d vin_vps_mode: %d\n",
                    i, g_iot_vio_cfg.vin_vps_mode[i]);
            pr_debug("grp_index: %d need_clk: %d\n",
                    i, g_iot_vio_cfg.need_clk[i]);
            pr_debug("grp_index: %d need_md: %d\n",
                    i, g_iot_vio_cfg.need_md[i]);
            pr_debug("grp_index: %d need_chnfd: %d\n",
                    i, g_iot_vio_cfg.need_chnfd[i]);
            pr_debug("grp_index: %d need_dis: %d\n",
                    i, g_iot_vio_cfg.need_dis[i]);
            pr_debug("grp_index: %d need_gdc: %d\n",
                    i, g_iot_vio_cfg.need_gdc[i]);
            pr_debug("grp_index: %d grp_rotate: %d\n",
                    i, g_iot_vio_cfg.grp_rotate[i]);
            pr_debug("grp_index: %d dol2_vc_num: %d\n",
                    i, g_iot_vio_cfg.dol2_vc_num[i]);
            pr_debug("grp_index: %d chn_num: %d\n",
                    i, g_iot_vio_cfg.chn_num[i]);
            /* 3. print channel config */
            for (j = 0 ; j < g_iot_vio_cfg.chn_num[i]; j++) {
                pr_debug("chn_index: %d ipu_chn_en: %d\n",
                        i, g_iot_vio_cfg.ipu_chn_en[i][j]);
                pr_debug("chn_index: %d pym_chn_en: %d\n",
                        i, g_iot_vio_cfg.pym_chn_en[i][j]);
                pr_debug("chn_index: %d scale_en: %d\n",
                        i, g_iot_vio_cfg.scale_en[i][j]);
                pr_debug("chn_index: %d width: %d\n",
                        i, g_iot_vio_cfg.width[i][j]);
                pr_debug("chn_index: %d height: %d\n",
                        i, g_iot_vio_cfg.height[i][j]);
                pr_debug("chn_index: %d frame_depth: %d\n",
                        i, g_iot_vio_cfg.frame_depth[i][j]);
                if (g_iot_vio_cfg.pym_chn_en[i][j] == 1) {
                    pr_debug("----------chn:%d pym config start---------\n", j);
                    pr_debug("chn_index: %d frame_id: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].frame_id);
                    pr_debug("chn_index: %d ds_layer_en: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].ds_layer_en);
                    pr_debug("chn_index: %d ds_uv_bypass: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].ds_uv_bypass);
                    pr_debug("chn_index: %d us_layer_en: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].us_layer_en);
                    pr_debug("chn_index: %d us_uv_bypass: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].us_uv_bypass);
                    pr_debug("chn_index: %d frameDepth: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].frameDepth);
                    pr_debug("chn_index: %d timeout: %d\n",
                            i, g_iot_vio_cfg.pym_cfg[i][j].timeout);

                    for (k = 0 ; k < MAX_PYM_DS_NUM; k++) {
                        if (k % 4 == 0) continue;
                        pr_debug("ds_pym_layer: %d ds roi_x: %d\n",
                                k, g_iot_vio_cfg.pym_cfg[i][j].\
                                ds_info[k].roi_x);
                        pr_debug("ds_pym_layer: %d ds roi_y: %d\n",
                                k, g_iot_vio_cfg.pym_cfg[i][j].\
                                ds_info[k].roi_y);
                        pr_debug("ds_pym_layer: %d ds roi_width: %d\n",
                                k,
                                g_iot_vio_cfg.pym_cfg[i][j].\
                                ds_info[k].roi_width);
                        pr_debug("ds_pym_layer: %d ds roi_height: %d\n",
                                k,
                                g_iot_vio_cfg.pym_cfg[i][j].\
                                ds_info[k].roi_height);
                        pr_debug("ds_pym_layer: %d ds factor: %d\n",
                                k, g_iot_vio_cfg.pym_cfg[i][j].\
                                ds_info[k].factor);
                    }
                    /* 4.3 pym upscale config */
                    for (m = 0 ; m < MAX_PYM_US_NUM; m++) {
                        pr_debug("us_pym_layer: %d us roi_x: %d\n",
                                m, g_iot_vio_cfg.pym_cfg[i][j].\
                                us_info[m].roi_x);
                        pr_debug("us_pym_layer: %d us roi_y: %d\n",
                                m, g_iot_vio_cfg.pym_cfg[i][j].\
                                us_info[m].roi_y);
                        pr_debug("us_pym_layer: %d us roi_width: %d\n",
                                m, g_iot_vio_cfg.pym_cfg[i][j].\
                                us_info[m].roi_width);
                        pr_debug("us_pym_layer: %d us roi_height: %d\n",
                                m, g_iot_vio_cfg.pym_cfg[i][j].\
                                us_info[m].roi_height);
                        pr_debug("us_pym_layer: %d us factor: %d\n",
                                m, g_iot_vio_cfg.pym_cfg[i][j].\
                                us_info[m].factor);
                    }
                    pr_debug("----------chn:%d pym config end-----------\n", j);
                }
            }
            pr_debug("=========grp:%d config end==========\n", i);
        }
        pr_debug("#########cam:%d config end##########\n", n);
    }
    pr_debug("************ AIoT Vio Config End *************\n");
}

static void parser_iot_vio_cfg()
{
    int i, j;
    int ipu_chn_index = 0;
    int pym_chn_index = 0;
    print_iot_vio_cfg();

    /* 1. get params from gloal iot vio cfg */
    memcpy(g_vin_fd,      g_iot_vio_cfg.vin_fd,        sizeof(g_vin_fd));
    memcpy(sensorId,      g_iot_vio_cfg.sensor_id,     sizeof(sensorId));
    memcpy(mipiIdx,       g_iot_vio_cfg.mipi_idx,      sizeof(mipiIdx));
    memcpy(bus,           g_iot_vio_cfg.i2c_bus,       sizeof(bus));
    memcpy(port,          g_iot_vio_cfg.sensor_port,   sizeof(port));
    memcpy(serdes_index,  g_iot_vio_cfg.serdes_index,  sizeof(serdes_index));
    memcpy(serdes_port,   g_iot_vio_cfg.serdes_port,   sizeof(serdes_port));
    memcpy(temper_mode,   g_iot_vio_cfg.temper_mode,   sizeof(temper_mode));
    memcpy(vin_vps_mode,  g_iot_vio_cfg.vin_vps_mode,  sizeof(vin_vps_mode));
    memcpy(need_clk,      g_iot_vio_cfg.need_clk,      sizeof(need_clk));
    memcpy(need_md,       g_iot_vio_cfg.need_md,       sizeof(need_md));
    memcpy(need_chnfd,    g_iot_vio_cfg.need_chnfd,    sizeof(need_chnfd));
    memcpy(need_dis,      g_iot_vio_cfg.need_dis,      sizeof(need_dis));
    memcpy(need_gdc,      g_iot_vio_cfg.need_gdc,      sizeof(need_gdc));
    memcpy(grp_rotate,    g_iot_vio_cfg.grp_rotate,    sizeof(grp_rotate));
    memcpy(vc_num,        g_iot_vio_cfg.dol2_vc_num,   sizeof(vc_num));
    memcpy(pym_cfg,       g_iot_vio_cfg.pym_cfg,       sizeof(pym_cfg));

    need_cam       = g_iot_vio_cfg.need_cam;
    vps_dump       = g_iot_vio_cfg.vps_dump;
    vps_layer_dump = g_iot_vio_cfg.vps_layer_dump;
    if (need_cam == 1) {
        groupMask = (1 << g_iot_vio_cfg.cam_num) - 1;
    } else {
        groupMask = (1 << g_iot_vio_cfg.grp_num[0]) - 1;
    }

    /* 2. print config info */
    pr_debug("need_cam:%d, groupMask:%d, vps_dump:%d, vps_layer_dump:%d\n",
            need_cam, groupMask, vps_dump, vps_layer_dump);
    for (i = 0; i < MAX_ID_NUM; i++) {
        if (BIT(i) & groupMask) {
            pr_debug("[index:%d], sensorId:%d, bus:%d, mipiIdx:%d, "
                    "temper_mode:%d, need_clk:%d, need_gdc:%d\n",
                    i, sensorId[i], bus[i], mipiIdx[i], temper_mode[i],
                    need_clk[i], need_gdc[i]);
        }
    }

    /* 3. init ipu and pym chn value */
    for (i = 0; i < MAX_GRP_NUM; i++) {
        for (j = 0; j < MAX_CHN_NUM; j++) {
            ipu_chn[i][j] = -1;
            pym_chn[i][j] = -1;
        }
    }

    /* 4. get valid ipu and pym chn */
    for (i = 0; i < g_iot_vio_cfg.cam_num; i++) {
        for (j = 0; j < g_iot_vio_cfg.chn_num[i]; j++) {
            if (g_iot_vio_cfg.ipu_chn_en[i][j] == 1) {
                ipu_chn[i][ipu_chn_index] = j;
                pr_debug("(%s:%d) index:%d, ipu chn valid:%d\n",
                        __FILENAME__, __LINE__, ipu_chn_index, j);
                ipu_chn_index++;
            }
            if (g_iot_vio_cfg.pym_chn_en[i][j] == 1) {
                pym_chn[i][pym_chn_index] = j;
                pr_debug("(%s:%d) index:%d,  pym chn valid:%d\n",
                        __FILENAME__, __LINE__, pym_chn_index, j);
                pym_chn_index++;
            }
        }
    }

}

static void adapter_vin_vps_config(int pipeId)
{
    /* 1. 3dnr level adapter to temperMode*/
    PIPE_ATTR_OV10635_YUV_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_IMX327_DOL2_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_IMX327_LINEAR_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_AR0233_1080P_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_OS8A10_LINEAR_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_OS8A10_DOL2_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_S5KGM1SP_LINEAR_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_TEST_PATTERN_1080P_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_TEST_PATTERN_12M_BASE.temperMode = temper_mode[pipeId];
    PIPE_ATTR_TEST_PATTERN_4K_BASE.temperMode = temper_mode[pipeId];
}

