import sys
import os
import json
import time
sys.path.append("..")
sys.setdlopenflags(os.RTLD_LAZY)

import xstream   # noqa
import vision_type as vt    # noqa
from native_xproto import XPlgAsync, NativeVioPlg, SmartHelper   # noqa

__all__ = [
  "VioPlugin",
  "SmartPlugin"
]


class XPluginAsync:
    """
    intermediate class
    native_xplgasync_: instance of native xpluginasync
    """

    def __init__(self, num=0):
        print("xpluginasync init")
        self.native_xplgasync_ = XPlgAsync() if num == 0 else XPlgAsync(num)

    def reg_msg(self, msg_type, func):
        # check type
        self.native_xplgasync_.register_msg(msg_type, func)

    def init(self):
        self.native_xplgasync_.init()

    def push_msg(self, msg):
        self.native_xplgasync_.push_msg(msg)


class VioPlugin(object):
    """
    VioPlugin
    platform_: Which platform will the code run on. Support 96board, 2610
    cam_type_: camera type. Support mono
    data_source_: Input type.
    Support panel_camera, ipc_camera,
            jpeg_image_list, nv12_image_list, cached_image_list
    name_list_: Images name list file.
    image_list_: cached image list
    _native_vio_: An instance of native vioplugin.
    """

    def __gen_cfg_file(self):
        template_file = ""
        if self.platform_ == "96board":
            template_file = "configs/vio_config.json.96board"
        elif self.platform_ == "2610":
            template_file = "configs/vio_config.json.2610"
        with open(template_file) as json_file:
            vio_template = json.load(json_file)
            # set cam_type & data_source
            vio_template["cam_type"] = self.cam_type_
            vio_template["data_source"] = self.data_source_
            # set name_list
            if len(self.name_list_) != 0:
                vio_template["file_path"] = self.name_list_
            # set image_list
            if len(self.image_list_) != 0:
                vio_template["image_list"] = self.image_list_
            # write to local file
            try:
                with open("configs/py_vio_cfg.json", "w") as cfg_file:
                    json.dump(vio_template, cfg_file)
            except:
                print("Failed to wirte vio config file")

    def __init__(
      self, platform, data_source="panel_camera", cam_type="mono", **kwargs):
        self.supported_platform_ = ["96board", "2610"]
        self.supported_cam_type_ = ["mono"]
        self.supported_data_source = [
            "panel_camera", "ipc_camera", "jpeg_image_list",
            "nv12_image_list", "cached_image_list"]
        # check & set platform
        assert platform in self.supported_platform_, \
            "Given platform is not supported"
        self.platform_ = platform
        # check & set camera type
        assert cam_type in self.supported_cam_type_, \
            "Given camera type is not supported"
        self.cam_type_ = cam_type
        # check & set data source
        assert data_source in self.supported_data_source, \
            "Given data_source is not supported"
        self.data_source_ = data_source
        # check & set name list file
        self.name_list_ = "" if "name_list" not in kwargs else kwargs["name_list"]    # noqa
        if len(self.name_list_) != 0:
            assert os.access(self.name_list_, os.F_OK), \
                "Failed access image_list"
        # check & set image list
        self.image_list_ = [] if "image_list" not in kwargs else kwargs["image_list"]   # noqa
        if self.data_source_ == "cached_image_list":
            assert len(self.image_list_) != 0, \
                "image list cannot be empty in cached_image_list mode"
        # generate config file
        self.__gen_cfg_file()
        self.msg_cb_ = {}
        self._native_vio_ = NativeVioPlg("configs/py_vio_cfg.json")
        self._helper_ = SmartHelper()

    def start(self, sync_mode=False):
        self._native_vio_.set_mode(sync_mode)
        self._helper_.set_mode(sync_mode)
        if not self._native_vio_.is_inited():
            self._native_vio_.init()
        assert self._native_vio_.is_inited(), \
            "Cannot start vioplugin before init"
        ret = self._native_vio_.start()
        assert ret == 0, "Failed to start vioplugin, Code: %d" % ret

    def stop(self):
        ret = self._native_vio_.stop()
        assert ret == 0, "Failed to stop vioplugin, Code: %d" % ret
        ret = self._native_vio_.deinit()
        assert ret == 0, "Failed to deinit vioplugin, Code: %d" % ret

    def message_type(self):
        return ["XPLUGIN_IMAGE_MESSAGE", "XPLUGIN_DROP_MESSAGE"]

    def bind(self, msg_type, msg_cb=None):
        assert callable(msg_cb), "Callback is not callable for %s" % msg_type
        self.msg_cb_[msg_type] = msg_cb
        ret = self._native_vio_.add_msg_cb(msg_type, msg_cb)
        assert ret == 0, "Failed to add callback for %s" % msg_type

    def get_image(self):
        # sync but not real-time
        # need to monitor #images for feedback
        time.sleep(1)
        vio_msg = self._native_vio_.get_image()
        return self._helper_.to_xstream_data(vio_msg)


class SmartPlugin(XPluginAsync):
    """
    SmartPlugin
    xstream_sess_: xstream session
    workflow_: xstream workflow
    callback_: smart data callback
    serialize_: serializing function of custom message
    push_result_: push data to xplgflow, otherwise display in command line
    helper_: instance of SmartPlgHelper, C++/Python data conversion
    """

    def __init__(self, workflow, callback, serialize, push_result=False):
        XPluginAsync.__init__(self)
        self._xstream_sess_ = xstream.Session(workflow)
        self.callback_ = callback
        self.msg_cb_ = {}
        self.workflow_ = workflow
        self.push_result_ = push_result
        self._helper_ = SmartHelper()

    def start(self):
        # actual xstream callback
        def OnSmartData(*smart_rets):
            native_msg = self._helper_.to_native_msg(*smart_rets)
            if self.push_result_:
                self.push_msg(native_msg)
            else:
                self.callback_(*smart_rets)
        self._xstream_sess_.callback(OnSmartData)

        # flow message listener
        def OnFlowMsg(message):
            xstream_inputs = self._helper_.to_xstream_data(message)
            # get input names in workflow
            arg_names = self.workflow_.__code__.co_varnames
            arg_cnt = self.workflow_.__code__.co_argcount
            arg_names = arg_names[:arg_cnt]
            assert len(arg_names) == len(xstream_inputs), \
                "Input amounts do not match"
            # TODO(shiyu.fu): input name mismatch
            for idx, arg_name in enumerate(arg_names):
                assert arg_name in xstream_inputs, \
                    "No input with name %s" % arg_name

            self._xstream_sess_.forward(**xstream_inputs)

        for msg_type, msg_cb in self.msg_cb_.items():
            if msg_type == "XPLUGIN_IMAGE_MESSAGE":
                super().reg_msg(msg_type, OnFlowMsg)
            else:
                assert callable(msg_cb), \
                    "Callback for %s is not callable" % msg_type
                super().reg_msg(msg_type, msg_cb)
        super().init()

    def stop(self):
        self._xstream_sess_.close()

    def message_type(self):
        return ["XPLUGIN_SMART_MESSAGE"]

    def bind(self, msg_type, msg_cb=None):
        self.msg_cb_[msg_type] = msg_cb

    def feed(self, inputs):
        # get input names in workflow
        arg_names = self.workflow_.__code__.co_varnames
        arg_cnt = self.workflow_.__code__.co_argcount
        arg_names = arg_names[:arg_cnt]
        assert len(arg_names) == len(inputs), \
            "Input amounts do not match"
        # TODO(shiyu.fu): input name mismatch
        for idx, arg_name in enumerate(arg_names):
            assert arg_name in inputs, \
                "No input with name %s" % arg_name
        self._xstream_sess_.callback(self.callback_)
        # input data have already been converted
        # from VioMessage to BaseDataWrapper
        # TODO(shiyu.fu): using xstream syncpredict
        self._xstream_sess_.forward(**inputs)
