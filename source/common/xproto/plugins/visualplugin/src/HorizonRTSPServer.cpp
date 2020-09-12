// Copyright (c) 2019, Horizon Robotics, Inc.
#include "HorizonRTSPServer.hh"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <chrono>

#include "visualplugin/horizonserver_api.h"


#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>


static pthread_t gs_RtspPid;
static pthread_mutex_t gs_mutex;

uint32_t g_data_buf_size_ = 0;

TaskScheduler *scheduler;
UsageEnvironment *env;

HorizonRTSPServer *
HorizonRTSPServer::createNew(UsageEnvironment &env, Port ourPort,
                             UserAuthenticationDatabase *authDatabase,
                             unsigned reclamationTestSeconds) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) {
    return NULL;
  }

  return new HorizonRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

HorizonRTSPServer::HorizonRTSPServer(UsageEnvironment &env,
                                     int ourSocket,
                                     Port ourPort,
                                     UserAuthenticationDatabase *authDatabase,
                                     unsigned reclamationTestSeconds)
  : RTSPServerSupportingHTTPStreaming(env,
                                      ourSocket,
                                      ourPort,
                                      authDatabase,
                                      reclamationTestSeconds) {
}

HorizonRTSPServer::~HorizonRTSPServer() {
}

static ServerMediaSession *createNewSMS(UsageEnvironment &env,
                                        char const *streamName, char *name,
                                        char *key, char *value);

ServerMediaSession *HorizonRTSPServer
::lookupServerMediaSession(char const *streamName, Boolean isFirstLookupInSession) {
  int len = strlen(streamName);
  char name[64] = {0};
  char key[64] = {0};
  char value[64] = {0};

  int namePos = 0;
  int keyPos = -1;
  int valuePos = -1;

  for (int i = 0; i < len; i++) {
    if (streamName[i] == '?') {
      name[namePos] = 0;
      namePos = -1;
      keyPos = 0;
      continue;
    }
    if (namePos >= 0) {
      name[namePos] = streamName[i];
      namePos++;
    }
    if (streamName[i] == '=') {
      key[keyPos] = 0;
      keyPos = -1;
      valuePos = 0;
      continue;
    }
    if (keyPos >= 0) {
      key[keyPos] = streamName[i];
      keyPos++;
    }
    if (valuePos >= 0) {
      value[valuePos] = streamName[i];
      valuePos++;
    }
  }
  value[valuePos] = 0;

  envir() << "streamName:" << streamName
          << " name:" << name
          << " key:" << key
          << " value:" << value << "\n";

  int eventTriggerCnt = HisiH264FramedSource::referenceCount +
    MetadataFramedSource::referenceCount * 2;
  if (eventTriggerCnt >= 32) {
    //最大支持的连接数量达到上限，不再接受新的连接
    return NULL;
  }

  // Next, check whether we already have a "ServerMediaSession" for this file:
  ServerMediaSession *sms = RTSPServer::lookupServerMediaSession(streamName);

  if (sms == NULL
    && (strcmp(name, "horizonMeta") == 0)
    && (strcmp(key, "MediaType") == 0)) {
    int flag = atoi(value);
    int sendflag = flag & 0xFF;
//    envir() << "sendflag:" << sendflag << "\n";
    if (sendflag == 0) {
      return NULL;
    }
    sms = createNewSMS(envir(), streamName, name, key, value);
    addServerMediaSession(sms);
  }

  return sms;

}

static ServerMediaSession *createNewSMS(UsageEnvironment &env,
                                        char const *streamName, char *name,
                                        char *key, char *value) {

  ServerMediaSession *sms = ServerMediaSession::createNew(env,
                                                          streamName,
                                                          streamName,
                                                          "session by hisiH264",
                                                          False);
  MetadataServerMediaSubsession *subsession = MetadataServerMediaSubsession::createNew(env,
                                                                                       False,
                                                                                       key,
                                                                                       value);
  sms->addSubsession(subsession);

  return sms;
}

HisiH264FramedSource *HisiH264FramedSource::createNew(UsageEnvironment &env) {
  HisiH264FramedSource *source = new HisiH264FramedSource(env);
  return source;
}

HisiH264FramedSource *HisiH264FramedSource::frameSources[100];

void HisiH264FramedSource::onH264Frame(unsigned char *buff, int len) {
  int count = sizeof(frameSources) / sizeof(HisiH264FramedSource *);

  for (int i = 0; i < count; i++) {
    pthread_mutex_lock(&gs_mutex);

    HisiH264FramedSource *source = frameSources[i];
    if (source != NULL) {
      source->getH264Frame(buff, len);
    }

    pthread_mutex_unlock(&gs_mutex);
  }
}


unsigned int HisiH264FramedSource::referenceCount = 0;

HisiH264FramedSource::HisiH264FramedSource(UsageEnvironment &env)
  : FramedSource(env) {
  envir() << "HisiH264FramedSource create . " << this << "\n";

  videoFront = 0;
  videoTail = 0;
  videoCurrentCnt = 0;
  videoBuffCnt = 0;
  memset(videoOffset, 0, sizeof(videoOffset));

  referenceCount++;

  eventTriggerId = 0;
  if (eventTriggerId == 0) {
    eventTriggerId = envir().taskScheduler().createEventTrigger(sendFrame);
  }

  addSource(this);
}

HisiH264FramedSource::~HisiH264FramedSource() {
  envir() << "HisiH264FramedSource destory." << this << " \n";

  eraseSource(this);
  referenceCount--;

  if (eventTriggerId > 0) {
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }
}

int HisiH264FramedSource::addSource(HisiH264FramedSource *source) {
  int count = sizeof(frameSources) / sizeof(HisiH264FramedSource *);
  for (int i = 0; i < count; i++) {
    if (frameSources[i] == NULL) {
      envir() << "HisiH264FramedSource " << source
              << " addSource i = " << i << "\n";
      frameSources[i] = source;
      return i;
    }
  }
  envir() << "HisiH264FramedSource::addSource failed\n ";
  return -1;
}

int HisiH264FramedSource::eraseSource(HisiH264FramedSource *source) {
  pthread_mutex_lock(&gs_mutex);
  int count = sizeof(frameSources) / sizeof(HisiH264FramedSource *);
  for (int i = 0; i < count; i++) {
    if (frameSources[i] == source) {
      envir() << "HisiH264FramedSource " << source
              << " eraseSource i = " << i << "\n";
      frameSources[i] = NULL;
      pthread_mutex_unlock(&gs_mutex);
      return i;
    }
  }
  envir() << "HisiH264FramedSource::eraseSource failed!\n";
  pthread_mutex_unlock(&gs_mutex);
  return -1;
}

void HisiH264FramedSource::getH264Frame(unsigned char *buff, int len) {
    int offset = 0;
    memcpy(frameBuffer + offset, buff, len);
    offset += len;

    int totalSize = sizeof(videoBuffer) / sizeof(char);
    int totalCnt = sizeof(videoOffset) / sizeof(int) - 1;

    //缓存满需要另外处理
    if (videoTail + offset > totalSize || videoBuffCnt >= totalCnt) {
        envir() << "HisiH264FramedSource drop frame videoCurrentCnt:"
                << videoCurrentCnt
                << " videoBuffCnt:" << videoBuffCnt
                << " videoTail:" << videoTail
                << " videoFront:" << videoFront
                << " object:" << this << "\n";

        videoFront = 0;
        videoTail = 0;
        videoCurrentCnt = 0;
        videoBuffCnt = 0;
        memset(videoOffset, 0, sizeof(videoOffset));

        return;
    }

    memcpy(videoBuffer + videoTail, frameBuffer, offset);
    videoTail += offset;
    videoOffset[videoBuffCnt] = videoTail;
    videoBuffCnt++;

    if (eventTriggerId > 0) {
        envir().taskScheduler().triggerEvent(eventTriggerId, this);
    }
}

void HisiH264FramedSource::sendFrame(void *client) {
  HisiH264FramedSource *source = (HisiH264FramedSource *) client;
  source->doGetNextFrame();
}

void HisiH264FramedSource::doGetNextFrame() {
  if (isCurrentlyAwaitingData() == False) { //上次读的数据还没有处理完
    return;
  }

  if (videoTail > videoFront && eventTriggerId > 0) {
    pthread_mutex_lock(&gs_mutex);
    int offset = videoOffset[videoCurrentCnt];
    int size = offset - videoFront;
    memcpy(fTo, videoBuffer + videoFront, size);
  //   Save_Buffer("send.264", videoBuffer + videoFront, size);
    fFrameSize = size;
    videoFront = offset;
    videoCurrentCnt++;

    if (videoFront >= videoTail) {
      videoFront = 0;
      videoTail = 0;
      videoCurrentCnt = 0;
      videoBuffCnt = 0;
      memset(videoOffset, 0, sizeof(videoOffset));
    }

    // gettimeofday(&fPresentationTime, NULL);
    fPresentationTime.tv_usec =
        std::chrono::system_clock::now().time_since_epoch().count();
    afterGetting(this);

    pthread_mutex_unlock(&gs_mutex);
    return;
  }
}

void HisiH264FramedSource::doStopGettingFrames() {
  envir() << "HisiH264FramedSource::doStopGettingFrames\n";
  eraseSource(this);

  pthread_mutex_lock(&gs_mutex);

  if (eventTriggerId > 0) {
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }

  videoFront = 0;
  videoTail = 0;
  videoCurrentCnt = 0;
  videoBuffCnt = 0;
  memset(videoOffset, 0, sizeof(videoOffset));

  pthread_mutex_unlock(&gs_mutex);

  FramedSource::doStopGettingFrames();
}

HisiH264ServerMediaSubsession *HisiH264ServerMediaSubsession::createNew(UsageEnvironment &env,
                                                                        Boolean reuseFirstSource) {
  HisiH264ServerMediaSubsession *subsession = new HisiH264ServerMediaSubsession(env,
                                                                                reuseFirstSource);
  return subsession;
}

HisiH264ServerMediaSubsession::HisiH264ServerMediaSubsession(UsageEnvironment &env,
                                                             Boolean reuseFirstSource)
  : OnDemandServerMediaSubsession(env, reuseFirstSource) {

}

HisiH264ServerMediaSubsession::~HisiH264ServerMediaSubsession() {

}

FramedSource *HisiH264ServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                                   unsigned &estBitrate) {
  envir() << "createNewStreamSource clientSessionId:" << clientSessionId << "\n";
  estBitrate = 8000; // 1080p 8000kbit/s
  FramedSource *source = HisiH264FramedSource::createNew(envir());

  //add a framer in front of the source:
  source = H264VideoStreamDiscreteFramer::createNew(envir(), source);
  return source;
}

RTPSink *HisiH264ServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock,
                                                         unsigned char rtpPayloadTypeIfDynamic,
                                                         FramedSource *inputSource) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

MetadataFramedSource *MetadataFramedSource::createNew(UsageEnvironment &env,
    bool sendH264,
    bool sendImage,
    bool sendSmart,
    bool sendFeature,
    bool sendBackground) {
    MetadataFramedSource *source = new MetadataFramedSource(env,
        sendH264,
        sendImage,
        sendSmart,
        sendFeature,
        sendBackground);
  return source;
}

MetadataFramedSource *MetadataFramedSource::frameSources[100];

void MetadataFramedSource::getH264Frame(unsigned char *buff, int len) {
    int offset = len;
    buffSize = len;
    int totalSize = sizeof(frameBuffer) / sizeof(char) - offset;
    int totalCnt = sizeof(videoOffset) / sizeof(int) - 1;

    //缓存满需要另外处理
    if (videoTail + buffSize > totalSize || videoBuffCnt >= totalCnt) {
        envir() << "------drop frame videoCurrentCnt:" << videoCurrentCnt
                << " videoBuffCnt:" << videoBuffCnt
                << " videoTail:" << videoTail
                << " videoFront:" << videoFront
                << " object:" << this << "\n";
        videoFront = 0;
        videoTail = 0;
        videoCurrentCnt = 0;
        videoBuffCnt = 0;
        memset(videoOffset, 0, sizeof(videoOffset));

        return;
    }

    offset = videoTail;
    // perception.SerializeToArray(frameBuffer + offset, buffSize);
    memcpy(frameBuffer + offset, buff, len);
    buffSize = len;

    videoTail += buffSize;
    videoOffset[videoBuffCnt] = videoTail;

    videoBuffCnt++;

    if (hisiEventTriggerId > 0) {
        scheduler->triggerEvent(hisiEventTriggerId, this);
    }
}

void MetadataFramedSource::sendFrame(void *client) {
  if (client != NULL) {
    // 单个连接发送数据
    MetadataFramedSource *source = (MetadataFramedSource *) client;
    source->doGetNextFrame();
  } else {
    // 所有连接发送数据
    int count = sizeof(frameSources) / sizeof(MetadataFramedSource *);
    for (int i = 0; i < count; i++) {
      MetadataFramedSource *source = frameSources[i];
      if (source != NULL) {
        source->doGetNextFrame();
      }
    }
  }
}

void MetadataFramedSource::onDataFrame(unsigned char *buff, int len, int msgType) {
  int count = sizeof(frameSources) / sizeof(MetadataFramedSource *);
  for (int i = 0; i < count; i++) {
    pthread_mutex_lock(&gs_mutex);

    MetadataFramedSource *source = frameSources[i];
    if (source != NULL) {
      if (msgType == MsgSmart && source->needSmart()) {
        source->sendSmartCnt++;
//        if (source->sendSmartCnt <= 1000) {
          source->getDataFrame(buff, len);
//        }
      }
      if (msgType == MsgFeature && source->needFeature()) {

        source->getDataFrame(buff, len);
      }
      if (msgType == MsgImage && source->needImage()) {
        source->sendImageCnt++;
//        if (source->sendImageCnt <= 150) {
          source->getDataFrame(buff, len);
//        }
      }
      if (msgType == MsgBackground && source->needBackground()) {
        source->sendBackgroudCnt++;
//        if (source->sendBackgroudCnt <= 150) {
          source->getDataFrame(buff, len);
//        }
      }
    } else {
  //    printf("source is null\n");
    }

    pthread_mutex_unlock(&gs_mutex);
  }
}

void MetadataFramedSource::getDataFrame(unsigned char *buff, int len) {
  int totalSize = sizeof(dataFrameBuffer) / sizeof(char);
  // int totalSize = g_data_buf_size_;
  int totalCnt = sizeof(dataOffset) / sizeof(int) - 1;

  //缓存满需要另外处理
  if (dataTail + len > totalSize || dataBuffCnt >= totalCnt) {
    envir() << "------drop frame currentCnt:" << currentCnt
            << " dataBuffCnt:" << dataBuffCnt
            << " dataTail:" << dataTail
            << " dataFront:" << dataFront
            << " object:" << this << "\n";
    dataFront = 0;
    dataTail = 0;
    currentCnt = 0;
    dataBuffCnt = 0;
    memset(dataOffset, 0, sizeof(dataOffset));

    return;
  }

  int offset = dataTail;
  memcpy(dataFrameBuffer + offset, buff, len);
  dataTail += len;
  dataOffset[dataBuffCnt] = dataTail;

  dataBuffCnt++;

  if (dataEventTriggerId > 0 ) {
    scheduler->triggerEvent(dataEventTriggerId, this);
  }

}

void MetadataFramedSource::onH264Frame(unsigned char *buff, int len) {
  int count = sizeof(frameSources) / sizeof(MetadataFramedSource *);

  for (int i = 0; i < count; i++) {
    pthread_mutex_lock(&gs_mutex);

    MetadataFramedSource *source = frameSources[i];
    if (source != NULL) {
     //   if (source->needMedia()) {
      if (source->needImage()) {
        source->getH264Frame(buff, len);
      }
    }

    pthread_mutex_unlock(&gs_mutex);
  }
}


unsigned MetadataFramedSource::referenceCount = 0;

MetadataFramedSource::MetadataFramedSource(UsageEnvironment &env,
    bool sendH264,
    bool sendImage,
    bool sendSmart,
    bool sendFeature,
    bool sendBackground)
  : FramedSource(env) {
  envir() << "MetadataFramedSource create . " << this << "\n";
  ++referenceCount;
  
  dataFrameBuffer = (unsigned char *)malloc(g_data_buf_size_);

  buffSize = 0;
  videoFront = 0;
  videoTail = 0;
  videoCurrentCnt = 0;
  videoBuffCnt = 0;
  memset(videoOffset, 0, sizeof(videoOffset));

  dataFront = 0;
  dataTail = 0;
  currentCnt = 0;
  dataBuffCnt = 0;
  memset(dataOffset, 0, sizeof(dataOffset));

  this->sendH264 = sendH264;
  this->sendImage = sendImage;
  this->sendSmart = sendSmart;
  this->sendFeature = sendFeature;
  this->sendBackground = sendBackground;

  sendH264Cnt = 0;
  sendImageCnt = 0;
  sendSmartCnt = 0;
  sendBackgroudCnt = 0;
  sendFeatureCnt = 0;

  hisiEventTriggerId = 0;
  if (hisiEventTriggerId == 0) {
    hisiEventTriggerId = envir().taskScheduler().createEventTrigger(sendFrame);
  }
  dataEventTriggerId = 0;
  if (dataEventTriggerId == 0) {
    dataEventTriggerId = envir().taskScheduler().createEventTrigger(sendFrame);
  }

  addSource(this);
}

MetadataFramedSource::~MetadataFramedSource() {
  envir() << "MetadataFramedSource destory . " << this << "\n";
  eraseSource(this);

  --referenceCount;
  if (referenceCount == 0) {
    // Reclaim our 'event trigger'
  }

  if (hisiEventTriggerId > 0) {
    envir().taskScheduler().deleteEventTrigger(hisiEventTriggerId);
    hisiEventTriggerId = 0;
  }
  if (dataEventTriggerId > 0) {
    envir().taskScheduler().deleteEventTrigger(dataEventTriggerId);
    dataEventTriggerId = 0;
  }

  if (dataFrameBuffer) {
      free(dataFrameBuffer);
      dataFrameBuffer = nullptr;
  }
}

int MetadataFramedSource::addSource(MetadataFramedSource *source) {
  int count = sizeof(frameSources) / sizeof(MetadataFramedSource *);
  for (int i = 0; i < count; i++) {
    if (frameSources[i] == NULL) {
      envir() << "MetadataFramedSource " << source
              << " addSource i = " << i << "\n";
      frameSources[i] = source;
      return i;
    }
  }
  envir() << "MetadataFramedSource::addSource failed\n ";
  return -1;
}

int MetadataFramedSource::eraseSource(MetadataFramedSource *source) {
  pthread_mutex_lock(&gs_mutex);
  int count = sizeof(frameSources) / sizeof(MetadataFramedSource *);
  for (int i = 0; i < count; i++) {
    if (frameSources[i] == source) {
      envir() << "MetadataFramedSource " << source
              << " eraseSource i = " << i << "\n";
      frameSources[i] = NULL;
      pthread_mutex_unlock(&gs_mutex);
      return i;
    }
  }
  envir() << "MetadataFramedSource::eraseSource failed!\n";
  pthread_mutex_unlock(&gs_mutex);
  return -1;
}

void MetadataFramedSource::doGetNextFrame() {
  //上次读的数据还没有处理完
  if (isCurrentlyAwaitingData() == False) {
//    envir() << "isCurrentlyAwaitingData()" << isCurrentlyAwaitingData() << "\n";
    return;
  }

  if (videoTail > videoFront && hisiEventTriggerId > 0) {
    pthread_mutex_lock(&gs_mutex);

    int offset = videoOffset[videoCurrentCnt];
    int size = offset - videoFront;
    memcpy(fTo, frameBuffer + videoFront, size);
    fFrameSize = size;
    videoFront = offset;
    videoCurrentCnt++;

    if (videoFront >= videoTail) {
      videoFront = 0;
      videoTail = 0;
      videoCurrentCnt = 0;
      videoBuffCnt = 0;
      memset(videoOffset, 0, sizeof(videoOffset));
    }

    // gettimeofday(&fPresentationTime, NULL);
    fPresentationTime.tv_usec =
        std::chrono::system_clock::now().time_since_epoch().count();
    afterGetting(this);

    pthread_mutex_unlock(&gs_mutex);
    return;
  }
  if (dataTail > dataFront && dataEventTriggerId > 0) {
    pthread_mutex_lock(&gs_mutex);

    unsigned int offset = dataOffset[currentCnt];
    unsigned int size = offset - dataFront;
    if (size >= fMaxSize) {
      envir() << "drop buffer! buffsize : " << size
              << "larger than fMaxSize : " << fMaxSize << " \n";

      dataFront = offset;
      currentCnt++;
      if (dataFront >= dataTail) {
        dataFront = 0;
        dataTail = 0;
        currentCnt = 0;
        dataBuffCnt = 0;
        memset(dataOffset, 0, sizeof(dataOffset));
      }

      pthread_mutex_unlock(&gs_mutex);
      return;
    }

    memcpy(fTo, dataFrameBuffer + dataFront, size);
    fFrameSize = size;
    dataFront = offset;
    currentCnt++;

    if (dataFront >= dataTail) {
      dataFront = 0;
      dataTail = 0;
      currentCnt = 0;
      dataBuffCnt = 0;
      memset(dataOffset, 0, sizeof(dataOffset));
    }

    // gettimeofday(&fPresentationTime, NULL);
    fPresentationTime.tv_usec =
        std::chrono::system_clock::now().time_since_epoch().count();
    afterGetting(this);

    pthread_mutex_unlock(&gs_mutex);
    return;
  }

}

void MetadataFramedSource::doStopGettingFrames() {
  envir() << "MetadataFramedSource::doStopGettingFrames" << this << "\n";
  eraseSource(this);

  pthread_mutex_lock(&gs_mutex);

  if (hisiEventTriggerId > 0) {
    envir().taskScheduler().deleteEventTrigger(hisiEventTriggerId);
    hisiEventTriggerId = 0;
  }
  if (dataEventTriggerId > 0) {
    envir().taskScheduler().deleteEventTrigger(dataEventTriggerId);
    dataEventTriggerId = 0;
  }

  videoFront = 0;
  videoTail = 0;
  videoCurrentCnt = 0;
  videoBuffCnt = 0;
  memset(videoOffset, 0, sizeof(videoOffset));

  dataFront = 0;
  dataTail = 0;
  currentCnt = 0;
  dataBuffCnt = 0;
  memset(dataOffset, 0, sizeof(dataOffset));

  pthread_mutex_unlock(&gs_mutex);

  FramedSource::doStopGettingFrames();
}

MetadataServerMediaSubsession *MetadataServerMediaSubsession::createNew(UsageEnvironment &env,
                                                                        Boolean reuseFirstSource,
                                                                        char *key, char *value) {
  MetadataServerMediaSubsession *subsession = new MetadataServerMediaSubsession(env,
                                                                                reuseFirstSource,
                                                                                key, value);
  return subsession;
}

void MetadataServerMediaSubsession::onSendError(void *clientData) {
  static int counter = 0;
  counter++;
  *env << "onSendErrorFunc():counter:" << counter << "\n";
}

MetadataServerMediaSubsession::MetadataServerMediaSubsession(UsageEnvironment &env,
                                                             Boolean reuseFirstSource,
                                                             char *key,
                                                             char *value)
  : OnDemandServerMediaSubsession(env, reuseFirstSource) {
  sendH264 = true;
  sendSmart = true;
  sendFeature = true;
  sendImage = true;
  sendBackground = true;

  if (key != NULL) {
    snprintf(m_key, sizeof(m_key), "%s", key);
  }
  if (value != NULL) {
    snprintf(m_value, sizeof(m_value), "%s", value);
    int flag = atoi(value);
    if (flag & 0x01) {
      sendH264 = true;
    } else {
      sendH264 = false;
    }
    if (flag & 0x02) {
      sendSmart = true;
    } else {
      sendSmart = false;
    }
    if (flag & 0x04) {
      sendFeature = true;
    } else {
      sendFeature = false;
    }
    if (flag & 0x08) {
      sendImage = true;
    } else {
      sendImage = false;
    }
    if (flag & 0x10) {
      sendBackground = true;
    } else {
      sendBackground = false;
    }
  }

}

MetadataServerMediaSubsession::~MetadataServerMediaSubsession() {

}

FramedSource *MetadataServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                                   unsigned &estBitrate) {
  MetadataFramedSource *source = MetadataFramedSource::createNew(envir(),
                                                                 sendH264,
                                                                 sendImage,
                                                                 sendSmart,
                                                                 sendFeature,
                                                                 sendBackground);

  envir() << "key:" << m_key << " value:" << m_value
          << " sendH264:" << sendH264
          << " sendImage:" << sendImage
          << " sendSmart:" << sendSmart
          << " sendFeature:" << sendFeature
          << " sendBackground:" << sendBackground
          << "\n";

  estBitrate = 4000;

  return source;
}

RTPSink *MetadataServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock,
                                                         unsigned char rtpPayloadTypeIfDynamic,
                                                         FramedSource *inputSource) {
  SimpleRTPSink *sink = SimpleRTPSink::createNew(envir(), rtpGroupsock, 112,
                                                 90000, "video", "HORIZON", 1, False, True);
  sink->setOnSendErrorFunc(onSendError, this);
  return sink;
}

char const *MetadataServerMediaSubsession::sdpLines() {
  if (fSDPLines == NULL) {
    // We need to construct a set of SDP lines that describe this
    // subsession (as a unicast stream).  To do so, we first create
    // dummy (unused) source and "RTPSink" objects,
    // whose parameters we use for the SDP lines:
    unsigned estBitrate;
    FramedSource *inputSource = createNewStreamSource(0, estBitrate);
    if (inputSource == NULL) {
      return NULL;
    } // file not found

    struct in_addr dummyAddr;
    dummyAddr.s_addr = 0;
    Groupsock *dummyGroupsock = createGroupsock(dummyAddr, 0);
    unsigned char rtpPayloadType = 96 + trackNumber() - 1; // if dynamic
    RTPSink *dummyRTPSink = createNewRTPSink(dummyGroupsock, rtpPayloadType, inputSource);
    if (dummyRTPSink != NULL && dummyRTPSink->estimatedBitrate() > 0) {
      estBitrate = dummyRTPSink->estimatedBitrate();
    }

    setSDPLinesFromRTPSink(dummyRTPSink, inputSource, estBitrate);
    Medium::close(dummyRTPSink);
    delete dummyGroupsock;
    closeStreamSource(inputSource);
  }

  envir() << "fSDP = " << fSDPLines << "\n";

  return fSDPLines;
}

void MetadataServerMediaSubsession::setSDPLinesFromRTPSink(RTPSink *rtpSink,
                                                           FramedSource *inputSource,
                                                           unsigned estBitrate) {
  if (rtpSink == NULL) {
    return;
  }

  char const *mediaType = rtpSink->sdpMediaType();
  unsigned char rtpPayloadType = rtpSink->rtpPayloadType();
  AddressString ipAddressStr(fServerAddressForSDP);
  char *rtpmapLine = rtpSink->rtpmapLine();
  char const *rangeLine = rangeSDPLine();

  char const *const sdpFmt =
    "m=%s %u RTP/AVP %d\r\n"
    "c=IN IP4 %s\r\n"
    "b=AS:%u\r\n"
    "%s"
    "%s"
    "a=control:%s\r\n";
  unsigned sdpFmtSize = strlen(sdpFmt)
    + strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
    + strlen(ipAddressStr.val())
    + 20 /* max int len */
    + strlen(rtpmapLine)
    + strlen(rangeLine)
    + strlen(trackId());
  char *sdpLines = new char[sdpFmtSize];
  sprintf(sdpLines, sdpFmt,
          mediaType, // m= <media>
          fPortNumForSDP, // m= <port>
          rtpPayloadType, // m= <fmt list>
          ipAddressStr.val(), // c= address
          estBitrate, // b=AS:<bandwidth>
          rtpmapLine, // a=rtpmap:... (if present)
          rangeLine, // a=range:... (if present)
          trackId()); // a=control:<track-id>
  delete[] (char *) rangeLine;
  delete[] rtpmapLine;

  fSDPLines = strDup(sdpLines);
  delete[] sdpLines;
}

void *RtspServerRun(void *pVoid) {
  SERVER_PARAM_S *param_s = (SERVER_PARAM_S *) pVoid;
  // Begin by setting up our usage environment:
  scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase *authDB = NULL;
  if (param_s->hasConnAuth) {
    // To implement client access control to the RTSP server, do the following:
    authDB = new UserAuthenticationDatabase;
    authDB->addUserRecord(param_s->username, param_s->password);

  }
  *env << "RTSP server use auth: " << param_s->hasConnAuth
       << "  username:" << param_s->username
       << " password" << param_s->password << "\n";

  g_data_buf_size_ = param_s->data_buf_size;
  // OutPacketBuffer::maxSize = 2000000; // bytes
  // OutPacketBuffer::maxSize = 1920 * 1080 * 1.5; // 分包应该大于1帧
//  OutPacketBuffer::maxSize = param_s->packet_size;
  OutPacketBuffer::maxSize = 2000000;
  // Create the RTSP server.  Try first with the default port number (555),
  // and then with the alternative port number (8555):
  RTSPServer *rtspServer;
  portNumBits rtspServerPortNum = 555;
//  rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, authDB);
  rtspServer = HorizonRTSPServer::createNew(*env, rtspServerPortNum, authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  ServerMediaSession *sms = ServerMediaSession::createNew(*env,
                                                          "horizonStream",
                                                          "h264_fifo",
                                                          "session by x3H264");
  // sms->addSubsession(OnDemandServerMediaSubsession::createNew(*env, False));
    sms->addSubsession(H264VideoFileServerMediaSubsession
           ::createNew(*env, "/tmp/h264_fifo", True));
  rtspServer->addServerMediaSession(sms);

  ServerMediaSession *metaSession = ServerMediaSession::createNew(*env,
                                                                  "horizonMeta",
                                                                  "horizon metadata stream",
                                                                  "session by horizon",
                                                                  False);
  metaSession->addSubsession(MetadataServerMediaSubsession::createNew(*env, False, NULL, NULL));
  rtspServer->addServerMediaSession(metaSession);

  *env << "Horizon Media Server\n";
  *env << "\tversion " << HORIZON_SERVER_VERSION_STRING
       << " (Horizon Streaming Media library version "
       << HORIZON_LIBRARY_VERSION_STRING << ")."
#if WIN32
       << " pthread:" << pthread_self().x << "\n";
#else
       << " pthread:" << (uint32_t)pthread_self() << "\n";
#endif
  char *url = rtspServer->rtspURL(sms);
  char *metaUrl = rtspServer->rtspURL(metaSession);
  *env << "Play this stream using the URL \n\t" << url << "\n\t" << metaUrl << " \n";
  delete[] url;
  delete[]metaUrl;

  env->taskScheduler().doEventLoop(); // does not return
  return nullptr;
}

void RtspStop() {
  pthread_cancel(gs_RtspPid);
  pthread_join(gs_RtspPid, 0);
}

int server_send(unsigned char *buffer, int len, int msgType) {
  if (msgType == MsgH264) {
    MetadataFramedSource::onH264Frame(buffer, len);
  } else {
    MetadataFramedSource::onDataFrame(buffer, len, msgType);
  }
  return 0;
}

int server_send_h264(unsigned char *buffer, int len) {
  HisiH264FramedSource::onH264Frame(buffer, len);

  return 0;
}


int server_run(SERVER_PARAM_S *param_s) {

  pthread_mutex_init(&gs_mutex, NULL);

  memset(HisiH264FramedSource::frameSources, 0,
         sizeof(HisiH264FramedSource::frameSources));
  memset(MetadataFramedSource::frameSources, 0,
         sizeof(MetadataFramedSource::frameSources));


  //RTSP SERVER需要跑在独立线程上，保证与主逻辑线程相互独立
  pthread_create(&gs_RtspPid, 0, RtspServerRun, param_s);

  return 0;
}

int server_stop() {
  RtspStop();
  return 0;
}