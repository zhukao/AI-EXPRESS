//
// Copyright (c) 2019, Horizon Robotics, Inc.
// Created by kalehe on 18-2-2.
//

#ifndef HORIZON_RTSPSERVER_API_H
#define HORIZON_RTSPSERVER_API_H

#define HORIZON_SERVER_VERSION_STRING "1.0"
#define HORIZON_LIBRARY_VERSION_STRING  "2018.3.1"

typedef struct SERVER_PARAM {
  bool hasConnAuth;
  char username[64];
  char password[64];
  uint32_t data_buf_size;
  uint32_t packet_size;
} SERVER_PARAM_S;

enum MessageType {
  MsgH264 = 1,
  MsgSmart,
  MsgFeature,
  MsgImage,
  MsgBackground
};

int server_run(SERVER_PARAM_S *param_s);
int server_send(unsigned char *buffer, int len, int msgType);
int server_stop();
int server_send_h264(unsigned char *buffer, int len);

#endif //STREAMING_MEDIA_TRANSMISSION_API_H
