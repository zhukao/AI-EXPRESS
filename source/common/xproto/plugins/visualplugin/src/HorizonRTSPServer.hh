// Copyright (c) 1996-2017, Live Networks, Inc.  All rights reserved
// A subclass of "RTSPServer" that creates "ServerMediaSession"s on demand,
// based on whether or not the specified stream name exists as a file
// Header file
/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/


#ifndef _HORIZON_RTSP_SERVER_HH
#define _HORIZON_RTSP_SERVER_HH

#include <RTSPServerSupportingHTTPStreaming.hh>
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include <InputFile.hh>


#include "OnDemandServerMediaSubsession.hh"

class HorizonRTSPServer : public RTSPServerSupportingHTTPStreaming {
 public:
  static HorizonRTSPServer *createNew(UsageEnvironment &env, Port ourPort,
                                      UserAuthenticationDatabase *authDatabase,
                                      unsigned reclamationTestSeconds = 65);

 protected:
  HorizonRTSPServer(UsageEnvironment &env, int ourSocket, Port ourPort,
                    UserAuthenticationDatabase *authDatabase, unsigned reclamationTestSeconds);
  // called only by createNew();
  virtual ~HorizonRTSPServer();

 protected: // redefined virtual functions
  virtual ServerMediaSession *
  lookupServerMediaSession(char const *streamName, Boolean isFirstLookupInSession);
};

class HisiH264FramedSource : public FramedSource {
 public:
  static HisiH264FramedSource *createNew(UsageEnvironment &env);
  static void onH264Frame(unsigned char *buff, int len);

  static void sendFrame(void *client);

  static HisiH264FramedSource *frameSources[100];

 public:
//  static EventTriggerId eventTriggerId;
  EventTriggerId eventTriggerId;

  // used to count how many instances of this class currently exist
  static unsigned int referenceCount;

 protected:
  HisiH264FramedSource(UsageEnvironment &env);
  ~HisiH264FramedSource();

 private:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

  void getH264Frame(unsigned char *buff, int len);

  int addSource(HisiH264FramedSource *source);
  int eraseSource(HisiH264FramedSource *source);

  unsigned char frameBuffer[300000];
  unsigned char videoBuffer[2000000];
  int videoBuffCnt;
  int videoCurrentCnt;
  //video buffer 头指针，读取发送以这个开始
  int videoFront;
  //video buffer 尾指针,缓存数据以这个结束
  int videoTail;
  int videoOffset[1000];


};

class HisiH264ServerMediaSubsession : public OnDemandServerMediaSubsession {
 public:
  static HisiH264ServerMediaSubsession *
  createNew(UsageEnvironment &env, Boolean reuseFirstSource);

 protected:
  HisiH264ServerMediaSubsession(UsageEnvironment &env,
                                Boolean reuseFirstSource);
  ~HisiH264ServerMediaSubsession();

  virtual FramedSource *createNewStreamSource(unsigned clientSessionId,
                                              unsigned &estBitrate);
  // "estBitrate" is the stream's estimated bitrate, in kbps
  virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource *inputSource);

 private:

};

class MetadataFramedSource : public FramedSource {
 public:
  static MetadataFramedSource *createNew(UsageEnvironment &env,
                                         bool sendH264,
                                         bool sendImage,
                                         bool sendSmart,
                                         bool sendFeature,
                                         bool sendBackground);
  static void onDataFrame(unsigned char *buff, int len, int msgType);
//  static void onHisiFrame(VENC_STREAM_S *pstStream);
  static void onH264Frame(unsigned char *buff, int len);

  static void sendFrame(void *client);

  static MetadataFramedSource *frameSources[100];

  bool needH264() {
    return sendH264;
  }

  bool needImage() {
    return sendImage;
  }

  bool needBackground() {
    return sendBackground;
  }

  bool needSmart() {
    return sendSmart;
  }

  bool needFeature() {
    return sendFeature;
  }

  int sendH264Cnt;
  int sendImageCnt;
  int sendSmartCnt;
  int sendBackgroudCnt;
  int sendFeatureCnt;

 public:
  //hisi在一个线程通知数据，全局唯一的事件
  EventTriggerId hisiEventTriggerId;
  //元数据可能在不同的线程上，每个实例都有自己的事件通知，防止线程不安全
  EventTriggerId dataEventTriggerId;

  static unsigned referenceCount;

 protected:
  MetadataFramedSource(UsageEnvironment &env,
                       bool sendH264,
                       bool sendImage,
                       bool sendSmart,
                       bool sendFeature,
                       bool sendBackground);
  ~MetadataFramedSource();

  void getDataFrame(unsigned char *buff, int len);
  void getH264Frame(unsigned char *buff, int len);

 private:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

  int addSource(MetadataFramedSource *source);
  int eraseSource(MetadataFramedSource *source);


  unsigned char videoBuffer[500000];
  unsigned char frameBuffer[2000000];
  int buffSize;

  int videoBuffCnt;
  int videoCurrentCnt;
  //video buffer 头指针，读取发送以这个开始
  int videoFront;
  //video buffer 尾指针,缓存数据以这个结束
  int videoTail;
  int videoOffset[1000];

  int dataBuffCnt;
  int currentCnt;
  //buffer 头指针，读取发送以这个开始
  int dataFront;
  //buffer 尾指针,缓存数据以这个结束
  int dataTail;
  int dataOffset[1000];
  // unsigned char dataFrameBuffer[6000000];
  // 1920 * 1080 * 1.5 * 8 = 24883200
  // 960 * 540 * 1.5 * 4 = 3110400
  unsigned char *dataFrameBuffer;

  bool sendH264;
  bool sendImage;
  bool sendSmart;
  bool sendFeature;
  bool sendBackground;

};

class MetadataServerMediaSubsession : public OnDemandServerMediaSubsession {
 public:
  static MetadataServerMediaSubsession *
  createNew(UsageEnvironment &env, Boolean reuseFirstSource, char *key, char *value);

  static void onSendError(void *clientData);

 protected:
  MetadataServerMediaSubsession(UsageEnvironment &env,
                                Boolean reuseFirstSource, char *key, char *value);
  ~MetadataServerMediaSubsession();

  virtual FramedSource *createNewStreamSource(unsigned clientSessionId,
                                              unsigned &estBitrate);
  // "estBitrate" is the stream's estimated bitrate, in kbps
  virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource *inputSource);

  virtual char const *sdpLines();

 private:
  void setSDPLinesFromRTPSink(RTPSink *rtpSink, FramedSource *inputSource,
                              unsigned estBitrate);

  bool sendH264;
  bool sendImage;
  bool sendSmart;
  bool sendFeature;
  bool sendBackground;

  char m_key[64];
  char m_value[64];

};

#endif
