
function RenderFrame2(videoId) {
  this.smartCanvas = null;
  this.videoId = videoId;
  this.imgMain = document.getElementById(videoId);
}

// 修改图片展示样式
RenderFrame2.prototype.changeImgShow = function (canvasObj, width, height) {
  window.onresize = function() {return;}
  const video = document.querySelector('.video');
  const cam = document.querySelector('.cam');
  // const canvas = document.querySelector('#canvas-1');

  let videoWidth = video.offsetWidth;
  let videoHeight = video.offsetHeight;

  const widthRadio = videoWidth / width
  const heihgtRadio = videoHeight / height
  const ratio = widthRadio - heihgtRadio

  let widths = width
  let heights = height

  if (ratio < 0) {
    widths = '100%';
    heights = height * widthRadio + 'px';
  } else {
    widths = width * heihgtRadio + 'px'
    heights =  '100%'
  }
  cam.style.width = widths
  cam.style.height = heights
}

/**
 * 渲染页面
 * @param {*} frame
 */
RenderFrame2.prototype.render = function (canvasObj, frame) {
  if(frame.imageBlob) {
    console.log(1111)
    this.imgMain.onload = function() {
      this.changeImgShow(canvasObj, frame.imageWidth, frame.imageHeight)
      this.smartCanvas = new HCanvas(canvasObj, frame.imageWidth, frame.imageHeight);
      this.renderFrameStart(frame.smartMsgData, frame.messageShowSelect);
    }
  }
}

/**
 * 渲染轮廓框
 * @param {*} performance
 */

RenderFrame2.prototype.renderFrameStart = function (smartMsgData, messageShowSelect) {
  this.smartCanvas.clear();
  if (smartMsgData.length > 0) {
    smartMsgData.map(item => {
      if (item.floatMatrixs) {
        console.log(1111, item.floatMatrixs)
        this.smartCanvas.drawFloatMatrixs(item.floatMatrixs);
      }
    })
  }
}
