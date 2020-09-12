// 'use strict';
let AwesomeMessage = null;
let socket;
let FPS = 13;
let frames = [];
let timeout;
let ports = [];
let socketParameter = {};
let socketParameters = getURLParameter();
let messageShowSelect = changeCheckboxSelect();
// let openCVLoad = false;

protobufInit();
wsInit();
changeCheckboxShow();

// function onOpenCvReady() {
//   // document.getElementById('status').innerHTML = 'OpenCV.js is ready.';
//   openCVLoad = true
// }

function changeCheckboxSelect() {
  let messageShows = Array.from(document.querySelectorAll('.message-show'))
  let obj = {}
  messageShows.forEach(item => {
    obj[item.getAttribute('dataType')] = item.checked
    item.onclick = function () {
      obj[item.getAttribute('dataType')] = item.checked
    }
  })
  // console.log(obj)
  return obj
}

function changeImgShow(width, height) {
  window.onresize = function () { return; }
  const video = document.querySelector('.video');
  const cam = document.querySelector('.cam');

  let videoWidth = video.offsetWidth;
  let videoHeight = video.offsetHeight;

  const widthRadio = videoWidth / width
  const heihgtRadio = videoHeight / height
  const ratio = widthRadio - heihgtRadio

  if (ratio < 0) {
    widths = '100%';
    heights = height * widthRadio + 'px';
  } else {
    widths = width * heihgtRadio + 'px'
    heights = '100%'
  }
  cam.style.width = widths
  cam.style.height = heights

  // img.style.transform = `scale3d(${widths / width}, ${heights / height}, 1)`
}

function changeCheckboxShow() {
  let messageChangeV = document.querySelector('.message-v')
  let messageChangeH = document.querySelector('.message-h')
  let messageDiv = document.getElementById('message')

  messageChangeH.onclick = function () {
    messageDiv.style.display = 'block'
    this.style.display = 'none'
    messageChangeV.style.display = 'block'
  }
  messageChangeV.onclick = function () {
    messageDiv.style.display = 'none'
    this.style.display = 'none'
    messageChangeH.style.display = 'block'
  }
}

function getURLParameter() {
  let socketIP = getUrlQueryParameter('ws');
  let port = getUrlQueryParameter('port');
  let netId = getUrlQueryParameter('netid');
  let cameraId = getUrlQueryParameter('camera');
  let id = getUrlQueryParameter('id');

  let doc_socketIP = getUrlQueryParameter('doc_ws');
  let doc_port = getUrlQueryParameter('doc_port');
  let cam_id = getUrlQueryParameter('cam_id');

  return {
    socketIP,
    port,
    netId,
    cameraId,
    id,
    doc_socketIP,
    doc_port,
    cam_id
  }
}

function protobufInit() {
  protobuf.load('../../protos/x3.proto', function (err, root) {
    if (err) throw err;
    AwesomeMessage = root.lookupType('x3.FrameMessage');
    // console.log(111, AwesomeMessage)
  });
}

function wsInit() {
  let { socketIP, port, cameraId, id, netId } = socketParameters;

  // 部署
  hostport = document.location.host;
  socketIP = hostport.replace(/_/g, '.');
  socket = new ReconnectingWebSocket('ws://' + socketIP + ':' + '8080', null, { binaryType: 'arraybuffer' });

  // 本地开发用
  // let ip =
  //   // '10.31.35.189'
  //   // '10.31.35.26'
  //   '10.64.35.114'
  //   // '10.31.43.133'
  //   // '10.64.35.11' // 分割
  //   // '10.31.35.187'
  //   // '10.31.35.50' // 分割
  //   // '10.64.35.50'
  //   // '10.64.35.119'
  //   // '10.31.35.184'
  // socket = new ReconnectingWebSocket('ws://' + ip + ':' + '8080', null, { binaryType: 'arraybuffer' });

  socket.onopen = function (e) {
    let data = {
      filter_prefix: netId + '/' + cameraId + '/' + id
    };
    socket.send(JSON.stringify(data));
    console.log('opened');
  };

  socket.onclose = function (e) {
    console.log('close:::', e);
    // clearTimeout(timeout);
  };

  socket.onerror = function (e) {
    console.log('error:::', e);
  };

  socket.onmessage = function (e) {
    if (frames.length < 3) {
      let frame = transformData(e.data);
      frames.push(frame);
    }
    delete e.data;
  };
  clearTimeout(timeout);
  sendMessage();
}

function wsClose() {
  if (socket) {
    socket.close();
  }
}

const renderFrame1 = new RenderFrame1({ canvasId: 'canvas-1' }, { canvasId: 'canvas-2' }, 'video-1');
// const renderFrame2 = new RenderFrame1('video-1'); // socketParameters
function sendMessage() {
  let frame = frames.shift();
  if (typeof frame !== 'undefined') {
    renderFrame1.render(frame);
  }
  timeout = setTimeout(function () {
    sendMessage();
  }, 1000 / FPS);
}

/**
 * 数据转换
 * @param {*} buffer
 */
function transformData(buffer) {
  console.time('渲染计时器')
  console.time('解析计时器')
  // let startTime = new Date().valueOf();
  let unit8Array = new Uint8Array(buffer);
  let message = AwesomeMessage.decode(unit8Array);
  let object = AwesomeMessage.toObject(message);
  // console.log(111, object)
  // 性能数据
  let statisticsMsg = object['StatisticsMsg_'];
  let performance = []
  if (typeof statisticsMsg !== 'undefined') {
    if (typeof statisticsMsg['attributes_'] !== 'undefined') {
      performance = statisticsMsg['attributes_']
    }
  }
  // 图片
  let image = object['img_'];
  let imageBlob = undefined;
  let imageWidth;
  let imageHeight;
  if (typeof image !== 'undefined') {
    if (typeof image['buf_'] !== 'undefined') {
      imageBlob = new Blob([image['buf_']], { type: 'image/jpeg' });
      imageWidth = image['width_'] || 1920
      imageHeight = image['height_'] || 1080
      changeImgShow(imageWidth, imageHeight)
    }
  }
  // 智能数据
  let smartMsg = object['smartMsg_'];
  let smartMsgData = []
  if (typeof smartMsg !== 'undefined') {
    let targets = smartMsg['targets_'];
    if (typeof targets !== 'undefined' && targets.length > 0) {
      smartMsgData = targets.map(function (item) {
        let id = item['trackId_'];
        let attributes = { attributes: [], type: item['type_'] }
        let boxes = []
        let points = []
        let subTargets = { boxes: [] }
        let floatMatrixs;
        let fall = { fallShow: false };
        // 轮廓框数据
        if (typeof item['boxes_'] !== 'undefined' && item['boxes_'].length > 0) {
          item['boxes_'].map((val, ind) => {
            let box1 = {
              x: val['topLeft_']['x_'],
              y: val['topLeft_']['y_']
            };
            let box2 = {
              x: val['bottomRight_']['x_'],
              y: val['bottomRight_']['y_']
            };
            if (typeof box1.x !== 'undefined'
              && typeof box1.y !== 'undefined'
              && typeof box2.x !== 'undefined'
              && typeof box2.y !== 'undefined'
            ) {
              boxes.push({ type: val['type_'] || '', p1: box1, p2: box2 })
              if (ind === 0) {
                attributes.box = { p1: box1, p2: box2 }
                fall.box = { p1: box1, p2: box2 }
              }
            }
            if (messageShowSelect.scoreShow && ind === 0) {
              attributes.score = val.score
            }
          })
        }
        // 关节点数据
        if (typeof item['points_'] !== 'undefined' && item['points_'].length > 0) {
          item['points_'].map(item => {
            if (typeof item['points_'] !== 'undefined') {
              let bodyType = messageShowSelect.body ? '' : 'body_landmarks'
              let faceType = messageShowSelect.face ? '' : 'face_landmarks'
              if (item['type_'] === 'mask') {
                floatMatrixs = messageShowSelect.floatMatrixsMask
                  ? {
                    type: 'mask',
                    points: item['points_']
                  }
                  : undefined
              } else if (item['type_'] === 'hand_landmarks') {
                if (messageShowSelect.handMarks) {
                  let skeletonPoints = [];
                  item['points_'].map((val) => { // index
                    // let key = Config.handSkeletonKey[index];
                    skeletonPoints.push({
                      x: val['x_'],
                      y: val['y_'],
                      score: val['score_']
                    });
                  });
                  points.push({ type: 'hand_landmarks', skeletonPoints })
                }
              } else if (item['type_'] !== bodyType && item['type_'] !== faceType) {
                let skeletonPoints = [];
                item['points_'].map((val, index) => {
                  let key = Config.skeletonKey[index];
                  skeletonPoints[key] = {
                    x: val['x_'],
                    y: val['y_'],
                    score: val['score_']
                  };
                });
                points.push({ type: item['type_'], skeletonPoints })
              }
            }
          })
        }
        // 车牌
        if (typeof item['subTargets_'] !== 'undefined' && item['subTargets_'].length > 0) {
          item['subTargets_'].map(val => {
            if (typeof val['boxes_'] !== 'undefined' && val['boxes_'].length > 0) {
              let box1 = {
                x: val['boxes_'][0]['topLeft_']['x_'],
                y: val['boxes_'][0]['topLeft_']['y_']
              };
              let box2 = {
                x: val['boxes_'][0]['bottomRight_']['x_'],
                y: val['boxes_'][0]['bottomRight_']['y_']
              };
              if (typeof box1.x !== 'undefined'
                && typeof box1.y !== 'undefined'
                && typeof box2.x !== 'undefined'
                && typeof box2.y !== 'undefined'
              ) {
                subTargets.boxes.push({ p1: box1, p2: box2 })
              }
            }
            // 车牌框数据
            if (typeof val['attributes_'] !== 'undefined' && val['attributes_'].length > 0) {
              val['attributes_'].map(obj => {
                if (typeof obj['valueString_'] !== 'undefined') {
                  attributes.attributes.push({
                    type: obj['type_'],
                    value: obj['valueString_'],
                    score: obj['score_']
                  })
                }
              })
            }
          })
        }
        // 属性
        let labelStart = 0
        let labelCount = 20
        if (typeof item['attributes_'] !== 'undefined') {
          item['attributes_'].map(function (val) {
            // 分割的颜色参数
            labelStart = val['type_'] === "segmentation_label_start" ? val['value_'] : 0
            labelCount = val['type_'] === "segmentation_label_count" ? val['value_'] : 20
            // 摔倒
            if (val['type_'] === 'fall' && val['value_'] === 1) {
              fall.fallShow = true;
              fall.attributes = {
                type: val['type_'],
                value: val['value_'],
                score: messageShowSelect.scoreShow ? val['score_'] : undefined
              }
            } else if (typeof val['valueString_'] !== 'undefined') {
              attributes.attributes.push({
                type: val['type_'],
                value: val['valueString_'],
                score: messageShowSelect.scoreShow ? val['score_'] : undefined
              })
            }
          })
        }
        // 全图分割 mask 目标分割 segmentation 全图分割
        if (typeof item['floatMatrixs_'] !== 'undefined' && item['floatMatrixs_'].length > 0) {

          let w = item['floatMatrixs_'][0]['arrays_'][0]['value_'].length;
          let h = item['floatMatrixs_'][0]['arrays_'].length;

          let step = 255 * 3 / labelCount;
          let color = [];
          for (let i = 0; i < labelCount; ++i) {
            let R = (labelStart / 3 * 3) % 256;
            let G = (labelStart / 3 * 2) % 256;
            let B = (labelStart / 3) % 256;
            color.push([R, G, B]);
            labelStart += step;
          }
          if (item['floatMatrixs_'][0]['type_'] === 'segmentation') {
            floatMatrixs = { type: item['floatMatrixs_'][0]['type_'], data: [], w, h }
            item['floatMatrixs_'][0]['arrays_'].map(values => {
              values['value_'].map(index => {
                let colors = color[index]
                floatMatrixs.data.push(colors[0], colors[1], colors[2], 155)
              })
            })
            floatMatrixs = messageShowSelect.floatMatrixs ? floatMatrixs : undefined
          }
        }
        boxes = messageShowSelect.boxes
          ? !messageShowSelect.handBox && boxes.length > 0
            ? boxes.filter(item => item.type !== 'hand')
            : boxes
          : []
        // boxes = !messageShowSelect.handBox && boxes.length > 0 ? boxes.filter(item => item.type !== 'hand') : boxes
        attributes.attributes = messageShowSelect.attributes ? attributes.attributes : []
        subTargets.boxes = messageShowSelect.boxes ? subTargets.boxes : []
        return {
          id,
          attributes,
          boxes,
          points,
          subTargets,
          floatMatrixs,
          fall
        }
      })
    }
  }
  
  console.timeEnd('解析计时器')
  return {
    imageBlob,
    imageWidth,
    imageHeight,
    performance,
    smartMsgData,
    messageShowSelect
  };
}
