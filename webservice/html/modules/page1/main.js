function RenderFrame1(canvasObj, canvasOjb2, videoId) {
  this.videoId = videoId;
  this.imgMain = document.getElementById(videoId);

  this.smartCanvas = new HCanvas(canvasObj);
  this.smartCanvas2 = new HCanvas(canvasOjb2);

  // this.openCVLoad = false
}

/**
 * 渲染页面
 * @param {*} frame
 */
RenderFrame1.prototype.render = function (frame) {
  // this.openCVLoad = frame.openCVLoad
  if (frame.imageBlob) {
    let obj = this.smartCanvas.getImageWH()
    if (obj.w !== frame.imageWidth || obj.h !== frame.imageHeight) {
      this.smartCanvas.changeImageWH(frame.imageWidth, frame.imageHeight);
      this.smartCanvas2.changeImageWH(frame.imageWidth, frame.imageHeight);
    }
    this.canvasOffset = this.calculateOffset(frame.imageWidth, frame.imageHeight);
    var urlCreator = window.URL || window.webkitURL;
    var imageUrl = urlCreator.createObjectURL(frame.imageBlob);
    requestAnimationFrame(() => {
      this.renderVideo(imageUrl, frame);
      setTimeout(function () {
        urlCreator.revokeObjectURL(imageUrl);
      }, 10);
    });
  }
  if (frame.performance.length > 0) {
    this.renderPerformanceData(frame.performance);
  }
}

/**
 * 性能数据
 * @param {*} performance
 */

RenderFrame1.prototype.renderPerformanceData = function (performance) {
  let performanceHtml = document.querySelector('#performance-message');
  let html = '';
  performance.map((item) => {
    html += `<li>${item['type_']}: ${item['valueString_']}</li>`
  })
  performanceHtml.innerHTML = html;
}

/**
 * 渲染视频流
 * @param {*} imageUrl
 */

RenderFrame1.prototype.renderVideo = function (imageUrl, frame) {
  var imgMain = this.imgMain
  var _this = this
  imgMain.src = imageUrl;
  imgMain.onload = function () {
    _this.renderFrameStart(frame);
  }
}

RenderFrame1.prototype.renderFrameStart = function ({ smartMsgData, imageWidth, imageHeight }) { // frame
  this.smartCanvas.clear();
  this.smartCanvas2.clear();
  let parentContainer = document.querySelector('.info-panel-1');
  let parentContainerAlertHtml = document.querySelector('.info-panel-2');
  parentContainer.innerHTML = '';
  parentContainerAlertHtml.innerHTML = '';
  if (smartMsgData.length > 0) {
    let htmls = '';
    let htmls2 = '';
    smartMsgData.map(item => {
      if (item.boxes.length > 0) {
        // let box = item.boxes.filter(obj => obj.type === 'body')
        this.renderFrameBoxes(item.boxes, item.fall.fallShow);
        if (typeof item.attributes !== 'undefined' && typeof item.attributes.box !== 'undefined') {
          htmls += this.renderAttributes(item.attributes, imageWidth, imageHeight)
        }
      }
      if (item.fall.fallShow) {
        htmls2 += this.createAlertHtml(item.fall, imageWidth, imageHeight);
      }
      if (item.subTargets.boxes.length > 0) {
        this.renderFrameBoxes(item.subTargets.boxes);
      }
      if (item.points.length > 0) {
        this.renderFramePoints(item.points)
      }
      if (typeof item.floatMatrixs !== 'undefined') {
        if (item.floatMatrixs.type === 'segmentation') {
          this.floatMatrixs(item.floatMatrixs)
        } else if (item.floatMatrixs.type === 'mask') {
          this.floatMatrixsMask(item.floatMatrixs)
        }
      }
    })
    parentContainer.innerHTML = htmls;
    parentContainerAlertHtml.innerHTML = htmls2;
  }
  console.timeEnd('渲染计时器')
}

// 渲染轮廓框
RenderFrame1.prototype.renderFrameBoxes = function (boxes, fall) {
  let color = undefined
  if (fall) {
    color = [254, 108, 113];
  }
  boxes.map(item => {
    this.smartCanvas.drawBodyBox(item.p1, item.p2, color);
  })
}

// 渲染骨骼线
RenderFrame1.prototype.renderFramePoints = function (points) {
  points.map(item => {
    switch (item.type) {
      case "hand_landmarks":
        this.smartCanvas.drawHandSkeleton(item.skeletonPoints);
        break;
      default:
        this.smartCanvas.drawSkeleton(item.skeletonPoints);
        break;
    }
  })
}

// 计算canvas像素和屏幕像素之间的比例关系以及偏移量
RenderFrame1.prototype.calculateOffset = function (width, height) {
  const canvas = document.querySelector('.canvas');
  const parentEle = canvas.parentNode;
  const canvasWidth = canvas.offsetWidth;
  const canvasHeight = canvas.offsetHeight;

  const canvasoffsetX = canvas.offsetLeft + parentEle.offsetLeft;
  const canvasoffsetY = canvas.offsetTop + parentEle.offsetTop;

  const xScale = canvasWidth / width;
  const yScale = canvasHeight / height;
  return {
    xScale,
    yScale,
    offsetX: canvasoffsetX,
    offsetY: canvasoffsetY
  };
}

RenderFrame1.prototype.createTemplateAttributesHtml = function (attributes, className, top, left) {
  let html = `<li class="${className}" style="top:${top}; left:${left}"><ol>`
  if (typeof attributes.type !== 'undefined') {
    html += `<li class="${attributes.type}">${attributes.type}
      ${typeof attributes.score !== 'undefined' && attributes.score > 0
        ? ':' + attributes.score.toFixed(3) : ''}
    </li>`
  }
  // if (typeof attributes.score !== 'undefined' && attributes.score > 0) {
  //   html += `<li class="${attributes.score}">${attributes.score.toFixed(3)}</li>`
  // }
  if (attributes.attributes.length > 0) {
    attributes.attributes.map(val => {
      // console.log(val)
      html += `<li class="${val.type}">${val.type}: <span class="${val.type === 'gesture' ? 'gesture' : ''}">${val.value || ''}</span>
        ${typeof val.score !== 'undefined' && val.score > 0
          ? '(' + val[score].toFixed(3) + ')' : ''}
      </li>`;
    });
  }
  html += '</ol></li>'
  return html;
}

// 渲染属性框
RenderFrame1.prototype.renderAttributes = function (attributes, w, h) {
  // if (typeof attributes.box === 'undefined') {
  //   return '';
  // }
  // let height = this.imgMain.offsetHeight
  // let width = this.imgMain.offsetWidth
  let box = attributes.box;
  let len = attributes.attributes.length;
  if (typeof attributes.type !== 'undefined') {
    len += 1;
  }
  let className = 'attribute-panel small';
  let left = box.p1.x * this.canvasOffset.xScale;
  let top = box.p1.y * this.canvasOffset.yScale;
  // let y = box.p2.y * this.canvasOffset.yScale - box.p1.y * this.canvasOffset.yScale
  // let top = box.p1.y * this.canvasOffset.yScale + y + 3;

  if (top - len * 40 >= 0) {
    top = top - len * 40
  }
  let html = this.createTemplateAttributesHtml(attributes, className, top + 'px', left + 'px');
  return html
}

RenderFrame1.prototype.createTemplateAlertHtml = function (score, top, left) {
  let html = `<li class="alert" style="top:${top}; left:${left}">
    <p class="img"><img src="../assets/images/danger.png" width="16px" alt=""/>有人摔倒啦</p>
  `
  if (typeof score !== 'undefined' && score > 0) {
    html += `<p>score: ${score.toFixed(3)}</p>`;
  }
  html + `</li>`
  return html;
}

// 摔倒弹窗提示
RenderFrame1.prototype.createAlertHtml = function (fall, w, h) {
  if (typeof fall.box === 'undefined') {
    return
  }
  let box = fall.box
  // let parentContainer = document.querySelector('.info-panel-2');
  let html = ''
  if (fall.value === 1) {
    if (box.p1.y <= 1) {
      return;
    }
    let x = box.p2.x * this.canvasOffset.yScale - box.p1.x * this.canvasOffset.yScale
    // let y = (box.p2.y - box.p1.y)
    let left = box.p2.x * this.canvasOffset.xScale - x + 'px';
    let top = box.p1.y * this.canvasOffset.yScale - 3 + 'px';
    html = this.createTemplateAlertHtml(fall.score, top, left);
  }
  return html
}

// 全图分割
RenderFrame1.prototype.floatMatrixs = function (floatMatrixs) {
  if (typeof floatMatrixs.data !== 'undefined') {
    this.smartCanvas2.drawFloatMatrixs(floatMatrixs);
  }
}

// 目标分割
RenderFrame1.prototype.floatMatrixsMask = function (floatMatrixs) {
  // console.log(111, floatMatrixs) //  && this.openCVLoad
  if (typeof floatMatrixs.points !== 'undefined') {
    // let data = updateDAta(floatMatrixs)

    // this.smartCanvas2.drawFloatMask(floatMatrixs);

    // const maskX = floatMatrixs.floatWH.p1.x;
    // const maskY = floatMatrixs.floatWH.p1.y;
    let color = `rgba(0, 255, 25, 0.5)`;
    // let segmentPoints = updateData(floatMatrixs)
    this.smartCanvas2.drawSegmentBorder(floatMatrixs.points, color) //, maskX, maskY

    // let img = cv.imread(this.imgMain)
    // for(let i = 1; i < data.length; i++) {
    //   console.log(data[i])
    //   cv.line(img, data[i].p1, data[i-1].p1, [0,255, 50, 155], 1, 8, 6 )
    // }
    // cv.imshow('canvas-2', img);
  }
}
