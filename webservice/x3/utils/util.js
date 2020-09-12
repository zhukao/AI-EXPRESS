// // 插件相关
// function handleMessage(message) {
//   onimg(message.data);
//   //img_test.src = "data:image/jpeg;base64,"+ message.data;
// }

// function success(s) {
//   stream = s;
//   plugin.postMessage({ command: "init", track: stream.getVideoTracks()[0] });
// }

// function failure(e) {
//   console.log(e);
// }

// function initialize() {
//   plugin = document.getElementById("plugin");
//   plugin.addEventListener("message", handleMessage, false);
// }
// var save_jpeg_file_flg = false;

// function plugin_findwnd() {
//   plugin.postMessage({ FindWnd: {} });
// }
// function plugin_play() {
//   plugin.postMessage({
//     cmd: "Play",
//     args: {
//       ip: _ip,
//       stream_type: 0,
//       rtsp_port: 554,
//       username: "admin",
//       password: "123456",
//       data_type: "protobuf"
//     }
//   });
// }
// function plugin_stop() {
//   plugin.postMessage({
//     cmd: "Stop",
//     args: {}
//   });
// }

// function plugin_config() {
//   var isbuff = true;
//   if (_isbuff == "false") {
//     isbuff = false;
//   }
//   plugin.postMessage({
//     cmd: "Config",
//     args: {
//       enable_jpegfiles_save: false,
//       save_jpeg_dir: "d://jpg_test",
//       buffered_frames_num: 5, //如果等不到智能帧，最多等多少视频帧就开始播放
//       max_buffered_frames_num: 8, //video list 超过x。就开始丢帧，避免延时
//       min_buffered_frames_num: 4, //videe list 少于x。就执行缓冲策略,缓冲到x在解码显示
//       enable_smart_results_filter: true, //启用插件同步视频智能结果的逻辑
//       smart_results_filter_interval_num: 0, //视频 智能uiTimeStamp相差多少个 40*90
//       enable_send_smart_results_to_js: false, //是否回调丢弃的智能帧数据给js
//       face_snap_config_file_path: "D:\\chrome\\11.31\\snap_config.json"
//     }
//   });
// }

// function plugin_piccut(data) {
//   plugin.postMessage({
//     cmd: "PicCut",
//     args: data
//   });
// }

// function changeSize(width, height) {
//   plugin.postMessage({ command: "size", width: width, height: height });
// }
// document.addEventListener("DOMContentLoaded", initialize, false);
// // 插件相关 结束

function ajaxpicdata(subdata, call) {
  var url = "http://10.31.32.92:8018/crop_rects";
  $.ajax({
    type: "POST",
    url: url,
    data: JSON.stringify(subdata),
    dataType: "json",
    success: function(data) {
      if (call) call(data);
    },
    error: function(json) {
      console.log("error");
    }
  });
}

// 获取 url query 参数
function getUrlQueryParameter (name) {
  var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
  var r = window.location.search.substr(1).match(reg);
  if (r != null) return decodeURI(r[2]);
  return null;
}

// 人脸识别 2
function feature_face_data(data, call) {
  var subdata = data.subdata;
  var idata = data.data;
  $.ajax({
    type: "POST",
    url: "http://10.31.32.92:8078/feature_face", //orderModifyStatus
    //url:"http://10.31.11.153:8088/feature_face",
    data: subdata,
    dataType: "json",
    success: function(data) {
      if (data.is_success == 0) {
        call({
          data: idata,
          subdata: data
        });
      }
    },
    error: function(json) {
      console.log("error");
    }
  });
}

// 生成随机数
function randomNum(minNum, maxNum) {
  switch (arguments.length) {
    case 1:
      return parseInt(Math.random() * minNum + 1, 10);
      break;
    case 2:
      return parseInt(Math.random() * (maxNum - minNum + 1) + minNum, 10);
      break;
    default:
      return 0;
      break;
  }
}

function toNormalTime(timestamp) {
  var date = new Date(timestamp),
    hour = date.getHours() + ":",
    minutes = date.getMinutes(),
    seconds = ":" + date.getSeconds();

  return hour + minutes + seconds;
}

// 视频保存数据相关
/**
 * 开始保存  保存名字
 * */
function save_file_start(name) {
  var file_name = name || new Date().valueOf();
  $.ajax({
    type: "POST",
    url: "/save_file_start",
    data: {
      name: file_name
    },
    dataType: "json",
    success: function(data) {
      console.log(data);
      _issavedata = true;
    },
    error: function(json) {
      console.log("error");
    }
  });
}
/**
 * 添加数据
 * */
function save_file_data(imgdata) {
  $.ajax({
    type: "POST",
    url: "/save_file_data",
    data: imgdata,
    dataType: "json",
    success: function(data) {
      console.log(data);
    },
    error: function(json) {
      console.log("error");
    }
  });
}
/**
 * 保存数据
 * */
function save_file_json() {
  isimg = false;
  _issavedata = false;
  $.ajax({
    type: "POST",
    url: "/save_file_json",
    dataType: "json",
    success: function(data) {
      console.log(data);
    },
    error: function(json) {
      console.log("error");
    }
  });
}
