/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: yong.wu
 * @Mail: yong.wu@horizon.ai
 * @Date: 2020-06-05
 * @Version: v0.0.1
 * @Brief: implemenation of vio cfg.
 */
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "hobotlog/hobotlog.hpp"
#include "vioplugin/iot_vio_cfg.h"
#include "vioplugin/iot_cfg_type.h"

extern iot_vio_cfg_t g_iot_vio_cfg;

bool IotVioConfig::LoadConfig() {
    std::ifstream ifs(path_);
    if (!ifs.is_open()) {
        LOGE << "Open config file " << path_ << " failed";
        return false;
    }
    LOGI << "Open config file " << path_ << " success";
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    std::string content = ss.str();
    Json::Value value;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING error;
    std::shared_ptr<Json::CharReader> reader(builder.newCharReader());
    try {
        bool ret = reader->parse(content.c_str(),
                content.c_str() + content.size(),
                &json_, &error);
        if (ret) {
            LOGE << "Open config file1 " << path_;
            HOBOT_CHECK(json_);
            return true;
        } else {
            LOGE << "Open config file2 " << path_;
            return false;
        }
    } catch (std::exception &e) {
        LOGE << "Open config file3 " << path_;
        return false;
    }
}

int IotVioConfig::GetIntValue(const std::string &key) const {
  std::lock_guard<std::mutex> lk(mutex_);
  if (json_[key].empty()) {
    LOGW << "Can not find key: " << key;
    return -1;
  }

  return json_[key].asInt();
}

std::string IotVioConfig::GetStringValue(const std::string &key) const {
  std::lock_guard<std::mutex> lk(mutex_);
  if (json_[key].empty()) {
    LOGW << "Can not find key: " << key;
    return "";
  }

  return json_[key].asString();
}

Json::Value IotVioConfig::GetJson() const { return this->json_; }

bool IotVioConfig::ParserConfig() {
    int pym_num;
    int grp_start_index, grp_max_num;
    int i, j, k, m, n;

    iot_vio_cfg_.need_cam = GetIntValue("need_cam");
    iot_vio_cfg_.cam_num = GetIntValue("cam_num");
    HOBOT_CHECK(iot_vio_cfg_.cam_num <= MAX_CAM_NUM);
    iot_vio_cfg_.vps_dump = GetIntValue("vps_dump");
    iot_vio_cfg_.vps_layer_dump = GetIntValue("vps_layer_dump");
    for (n = 0; n < iot_vio_cfg_.cam_num; n++) {
        /* 1. sensor config */
        std::string cam_name = "cam" + std::to_string(n);
        iot_vio_cfg_.sensor_id[n] =
            json_[cam_name]["sensor"]["sensor_id"].asInt();
        iot_vio_cfg_.sensor_port[n] =
            json_[cam_name]["sensor"]["sensor_port"].asInt();
        if (n < MAX_MIPIID_NUM) {
            iot_vio_cfg_.mipi_idx[n] =
                json_[cam_name]["sensor"]["mipi_idx"].asInt();
        }
        iot_vio_cfg_.i2c_bus[n] =
            json_[cam_name]["sensor"]["i2c_bus"].asInt();
        iot_vio_cfg_.serdes_index[n] =
            json_[cam_name]["sensor"]["serdes_index"].asInt();
        iot_vio_cfg_.serdes_port[n] =
            json_[cam_name]["sensor"]["serdes_port"].asInt();
        iot_vio_cfg_.temper_mode[n] =
            json_[cam_name]["isp"]["temper_mode"].asInt();
        iot_vio_cfg_.grp_num[n] = json_[cam_name]["grp_num"].asInt();
        HOBOT_CHECK(iot_vio_cfg_.grp_num[n] <= MAX_GRP_NUM);
        if (iot_vio_cfg_.need_cam == 1) {
            grp_start_index = n;
            grp_max_num = 1;
        } else {
            grp_start_index = 0;
            grp_max_num = iot_vio_cfg_.grp_num[n];
            HOBOT_CHECK(iot_vio_cfg_.cam_num <= 1);
        }
        /* 2. group config, a sensor use a group if in a process */
        for (i = grp_start_index; i < grp_max_num; i++) {
            pym_num = 0;
            std::string grp_name = "grp" + std::to_string(i);
            iot_vio_cfg_.fb_width[i] =
                json_[cam_name][grp_name]["fb_width"].asInt();
            iot_vio_cfg_.fb_height[i] =
                json_[cam_name][grp_name]["fb_height"].asInt();
            iot_vio_cfg_.fb_buf_num[i] =
                json_[cam_name][grp_name]["fb_buf_num"].asInt();
            iot_vio_cfg_.vin_fd[i] =
                json_[cam_name][grp_name]["vin_fd"].asInt();
            iot_vio_cfg_.vin_vps_mode[i] =
                json_[cam_name][grp_name]["vin_vps_mode"].asInt();
            iot_vio_cfg_.need_clk[i] =
                json_[cam_name][grp_name]["need_clk"].asInt();
            iot_vio_cfg_.need_md[i] =
                json_[cam_name][grp_name]["need_md"].asInt();
            iot_vio_cfg_.need_chnfd[i] =
                json_[cam_name][grp_name]["need_chnfd"].asInt();
            iot_vio_cfg_.need_dis[i] =
                json_[cam_name][grp_name]["need_dis"].asInt();
            iot_vio_cfg_.need_gdc[i] =
                json_[cam_name][grp_name]["need_gdc"].asInt();
            iot_vio_cfg_.grp_rotate[i] =
                json_[cam_name][grp_name]["grp_rotate"].asInt();
            iot_vio_cfg_.dol2_vc_num[i] =
                json_[cam_name][grp_name]["dol2_vc_num"].asInt();
            /* 3. channel config */
            iot_vio_cfg_.chn_num[i] = json_[cam_name][grp_name]\
                                      ["chn_num"].asInt();
            HOBOT_CHECK(iot_vio_cfg_.chn_num[i] <= MAX_CHN_NUM);
            for (j = 0; j < iot_vio_cfg_.chn_num[i]; j++) {
                std::string chn_name = "chn" + std::to_string(j);
                iot_vio_cfg_.ipu_chn_en[i][j] =
                    json_[cam_name][grp_name][chn_name]["ipu_chn_en"].asInt();
                iot_vio_cfg_.pym_chn_en[i][j] =
                    json_[cam_name][grp_name][chn_name]["pym_chn_en"].asInt();
                iot_vio_cfg_.scale_en[i][j] =
                    json_[cam_name][grp_name][chn_name]["scale_en"].asInt();
                iot_vio_cfg_.width[i][j] =
                    json_[cam_name][grp_name][chn_name]["width"].asInt();
                iot_vio_cfg_.height[i][j] =
                    json_[cam_name][grp_name][chn_name]["height"].asInt();
                iot_vio_cfg_.frame_depth[i][j] =
                    json_[cam_name][grp_name][chn_name]["frame_depth"].asInt();
                if (iot_vio_cfg_.pym_chn_en[i][j] == 1) {
                    /* 4. pym config */
                    /* 4.1 pym ctrl config */
                    iot_vio_cfg_.pym_cfg[i][j].frame_id =
                        json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["frame_id"].asUInt();
                    iot_vio_cfg_.pym_cfg[i][j].ds_uv_bypass =
                        json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["ds_uv_bypass"].asUInt();
                    iot_vio_cfg_.pym_cfg[i][j].ds_layer_en =
                        (uint16_t) json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["ds_layer_en"].asUInt();
                    iot_vio_cfg_.pym_cfg[i][j].us_layer_en =
                        (uint8_t)json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["us_layer_en"].asUInt();
                    iot_vio_cfg_.pym_cfg[i][j].us_uv_bypass =
                        (uint8_t)json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["us_uv_bypass"].asUInt();
                    iot_vio_cfg_.pym_cfg[i][j].frameDepth =
                        json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["frame_depth"].asUInt();
                    iot_vio_cfg_.pym_cfg[i][j].timeout =
                        json_[cam_name][grp_name]["pym"]\
                        ["pym_ctrl_config"]["timeout"].asInt();
                    /* 4.2 pym downscale config */
                    for (k = 0 ; k < MAX_PYM_DS_NUM; k++) {
                        if (k % 4 == 0) continue;
                        std::string factor_name = "factor_" + std::to_string(k);
                        std::string roi_x_name = "roi_x_" + std::to_string(k);
                        std::string roi_y_name = "roi_y_" + std::to_string(k);
                        std::string roi_w_name = "roi_w_" + std::to_string(k);
                        std::string roi_h_name = "roi_h_" + std::to_string(k);
                        iot_vio_cfg_.pym_cfg[i][j].ds_info[k].factor =
                            (uint8_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_ds_config"][factor_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_x =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_ds_config"][roi_x_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_y =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_ds_config"][roi_y_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_width =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_ds_config"][roi_w_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_height =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_ds_config"][roi_h_name].asUInt();
                    }
                    /* 4.3 pym upscale config */
                    for (m = 0 ; m < MAX_PYM_US_NUM; m++) {
                        std::string factor_name = "factor_" + std::to_string(m);
                        std::string roi_x_name = "roi_x_" + std::to_string(m);
                        std::string roi_y_name = "roi_y_" + std::to_string(m);
                        std::string roi_w_name = "roi_w_" + std::to_string(m);
                        std::string roi_h_name = "roi_h_" + std::to_string(m);
                        iot_vio_cfg_.pym_cfg[i][j].us_info[m].factor =
                            (uint8_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_us_config"][factor_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_x =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_us_config"][roi_x_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_y =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_us_config"][roi_y_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_width =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_us_config"][roi_w_name].asUInt();
                        iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_height =
                            (uint16_t)json_[cam_name][grp_name]["pym"]\
                            ["pym_us_config"][roi_h_name].asUInt();
                    }
                    /* 4.4 calculate pym num */
                    pym_num++;
                }   // end pym channel config
            }  // end all channel config
            /* 5. check pym config */
            if (pym_num > 1) {
                /* every grp only surpport a pym chn */
                pym_num = 1;
                if (iot_vio_cfg_.pym_chn_en[i][6] == 1) {
                    /* choose online pym channel(chn6) */
                    for (j = 0; j < iot_vio_cfg_.chn_num[i]; j++) {
                        if (j == 6 ) break;
                        /* set all offline pym disable */
                        iot_vio_cfg_.pym_chn_en[i][j] = 0;
                    }
                } else {
                    /* choose first offline pym channel */
                    for (j = 0; j < iot_vio_cfg_.chn_num[i]; j++) {
                        if (iot_vio_cfg_.pym_chn_en[i][j] == 1 ) continue;
                        /* set other offline pym disable */
                        iot_vio_cfg_.pym_chn_en[i][j] = 0;
                    }
                }
            }  // end check pym config
        }  // end group config
    }  // end cam config

    g_iot_vio_cfg = iot_vio_cfg_;
#ifdef DEBUG
    PrintConfig();
#endif
    return true;
}

bool IotVioConfig::PrintConfig() {
    int i, j, k, m, n;
    int grp_start_index, grp_max_num;
    /* 1. print sensor config */
    LOGD << "*********** iot vio config start ***********";
    LOGD << "cam_num: " << iot_vio_cfg_.cam_num;
    LOGD << "vps_dump: " << iot_vio_cfg_.vps_dump;
    LOGD << "vps_layer_dump: " << iot_vio_cfg_.vps_layer_dump;
    for (n = 0; n < iot_vio_cfg_.cam_num; n++) {
        LOGD << "#########cam:" << n << " cam config start#########";
        LOGD << "cam_index: "<< n << " " << "sensor_id: "
            << iot_vio_cfg_.sensor_id[n];
        LOGD << "cam_index: "<< n << " " << "sensor_port: "
            << iot_vio_cfg_.sensor_port[n];
        if (n < MAX_MIPIID_NUM) {
            LOGD << "cam_index: "<< n << " " << "mipi_idx: "
                << iot_vio_cfg_.mipi_idx[n];
        }
        LOGD << "cam_index: "<< n << " " << "i2c_bus: "
            << iot_vio_cfg_.i2c_bus[n];
        LOGD << "cam_index: "<< n << " " << "serdes_index: "
            << iot_vio_cfg_.serdes_index[n];
        LOGD << "cam_index: "<< n << " " << "serdes_port: "
            << iot_vio_cfg_.serdes_port[n];
        LOGD << "cam_index: "<< n << " " << "temper_mode: "
            << iot_vio_cfg_.temper_mode[n];
        LOGD << "cam_index: "<< n << " " << "grp_num: "
            << iot_vio_cfg_.grp_num[n];
        /* 2. print group config */
        if (iot_vio_cfg_.need_cam == 1) {
            grp_start_index = n;
            grp_max_num = 1;
        } else {
            grp_start_index = 0;
            grp_max_num = iot_vio_cfg_.grp_num[n];
        }
        for (i = grp_start_index; i < grp_max_num; i++) {
            LOGD << "=========grp:" << i << " group config start==========";
            LOGD << "grp_index: "<< i << " " << "fb_width: "
                << iot_vio_cfg_.fb_width[i];
            LOGD << "grp_index: "<< i << " " << "fb_height: "
                << iot_vio_cfg_.fb_height[i];
            LOGD << "grp_index: "<< i << " " << "fb_buf_num: "
                << iot_vio_cfg_.fb_buf_num[i];
            LOGD << "grp_index: "<< i << " " << "vin_fd: "
                << iot_vio_cfg_.vin_fd[i];
            LOGD << "grp_index: "<< i << " " << "vin_vps_mode: "
                << iot_vio_cfg_.vin_vps_mode[i];
            LOGD << "grp_index: "<< i << " " << "need_clk: "
                << iot_vio_cfg_.need_clk[i];
            LOGD << "grp_index: "<< i << " " << "need_md: "
                << iot_vio_cfg_.need_md[i];
            LOGD << "grp_index: "<< i << " " << "need_chnfd: "
                << iot_vio_cfg_.need_chnfd[i];
            LOGD << "grp_index: "<< i << " " << "need_dis: "
                << iot_vio_cfg_.need_dis[i];
            LOGD << "grp_index: "<< i << " " << "need_gdc: "
                << iot_vio_cfg_.need_gdc[i];
            LOGD << "grp_index: "<< i << " " << "grp_rotate: "
                << iot_vio_cfg_.grp_rotate[i];
            LOGD << "grp_index: "<< i << " " << "dol2_vc_num: "
                << iot_vio_cfg_.dol2_vc_num[i];
            LOGD << "grp_index: "<< i << " " << "chn_num: "
                << iot_vio_cfg_.chn_num[i];
            /* 3. print channel config */
            for (j = 0 ; j < iot_vio_cfg_.chn_num[i]; j++) {
                LOGD << "chn_index: "<< j << " " << "ipu_chn_en: "
                    << iot_vio_cfg_.ipu_chn_en[i][j];
                LOGD << "chn_index: "<< j << " " << "pym_chn_en: "
                    << iot_vio_cfg_.pym_chn_en[i][j];
                LOGD << "chn_index: "<< j << " " << "scale_en: "
                    << iot_vio_cfg_.scale_en[i][j];
                LOGD << "chn_index: "<< j << " " << "width: "
                    << iot_vio_cfg_.width[i][j];
                LOGD << "chn_index: "<< j << " " << "height: "
                    << iot_vio_cfg_.height[i][j];
                LOGD << "chn_index: "<< j << " " << "frame_depth: "
                    << iot_vio_cfg_.frame_depth[i][j];
                if (iot_vio_cfg_.pym_chn_en[i][j] == 1) {
                    LOGD << "--------chn:" << j << " pym config start---------";
                    LOGD << "chn_index: "<< j << " " << "frame_id: "
                        << iot_vio_cfg_.pym_cfg[i][j].frame_id;
                    LOGD << "chn_index: "<< j << " " << "ds_layer_en: "
                        << iot_vio_cfg_.pym_cfg[i][j].ds_layer_en;
                    LOGD << "chn_index: "<< j << " " << "ds_uv_bypass: "
                        << iot_vio_cfg_.pym_cfg[i][j].ds_uv_bypass;
                    LOGD << "chn_index: "<< j << " " << "us_layer_en: "
                        << static_cast<int>(iot_vio_cfg_.pym_cfg[i][j].\
                                us_layer_en);
                    LOGD << "chn_index: "<< j << " " << "us_uv_bypass: "
                        << static_cast<int>(iot_vio_cfg_.pym_cfg[i][j].\
                                us_uv_bypass);
                    LOGD << "chn_index: "<< j << " " << "frameDepth: "
                        << iot_vio_cfg_.pym_cfg[i][j].frameDepth;
                    LOGD << "chn_index: "<< j << " " << "timeout: "
                        << iot_vio_cfg_.pym_cfg[i][j].timeout;

                    for (k = 0 ; k < MAX_PYM_DS_NUM; k++) {
                        if (k % 4 == 0) continue;
                        LOGD << "ds_pym_layer: "<< k << " " << "ds roi_x: "
                            << iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_x;
                        LOGD << "ds_pym_layer: "<< k << " " << "ds roi_y: "
                            << iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_y;
                        LOGD << "ds_pym_layer: "<< k << " " << "ds roi_width: "
                            << iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_width;
                        LOGD << "ds_pym_layer: "<< k << " " << "ds roi_height: "
                            << iot_vio_cfg_.pym_cfg[i][j].ds_info[k].roi_height;
                        LOGD << "ds_pym_layer: "<< k << " " << "ds factor: "
                            << static_cast<int>(iot_vio_cfg_.pym_cfg[i][j].\
                                    ds_info[k].factor);
                    }
                    /* 4.3 pym upscale config */
                    for (m = 0 ; m < MAX_PYM_US_NUM; m++) {
                        LOGD << "us_pym_layer: "<< m << " " << "us roi_x: "
                            << iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_x;
                        LOGD << "us_pym_layer: "<< m << " " << "us roi_y: "
                            << iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_y;
                        LOGD << "us_pym_layer: "<< m << " " << "us roi_width: "
                            << iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_width;
                        LOGD << "us_pym_layer: "<< m << " " << "us roi_height: "
                            << iot_vio_cfg_.pym_cfg[i][j].us_info[m].roi_height;
                        LOGD << "us_pym_layer: "<< m << " " << "us factor: "
                            << static_cast<int>(iot_vio_cfg_.pym_cfg[i][j].\
                                    us_info[m].factor);
                    }
                    LOGD << "---------chn:" << j << " pym config end----------";
                }
            }
            LOGD << "=========grp:" << i << " group config end==========";
        }
        LOGD << "#########cam:" << n << " cam config end#########";
    }
    LOGD << "*********** iot vio config end ***********";

    return true;
}
