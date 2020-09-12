import os
import sys
import time
sys.setdlopenflags(os.RTLD_LAZY)

import xstream   # noqa
import vision_type as vt    # noqa
import xproto   # noqa

# it is more like a template
# attributes can be modified while defining workflow
faster_rcnn = xstream.Method("FasterRCNNMethod").inputs(["image"])
mot = xstream.Method("MOTMethod").inputs(["face_box"]) \
    .config_file("configs/method_configs/iou_method_param.json")
grading = xstream.Method("GradingMethod") \
    .inputs(["face_bbox_list", "pose", "lmk"])
snapshot = xstream.Method("SnapShotMethod") \
    .inputs([
        "image", "face_bbox_list", "select_score_list",
        "face_disappeared_track_id_list", "pose", "lmk",
        "face_bbox_list"])
cnn_method = xstream.Method("CNNMethod")


def face_solution(image):
    face_box, lmk, pose = faster_rcnn(
        image,
        outputs=["face_box", "lmk", "pose"],
        config_file="configs/method_configs/face_pose_lmk.json")
    face_bbox_list, face_disappeared_track_id_list = mot(
        face_box,
        outputs=["face_bbox_list", "face_disappeared_track_id_list"])
    select_score_list = grading(
        face_bbox_list, pose, lmk,
        outputs=["select_score_list"],
        config_file="configs/method_configs/grading.json")
    snap_list = snapshot(
        image, face_bbox_list, select_score_list,
        face_disappeared_track_id_list, pose, lmk, face_bbox_list,
        outputs=["snap_list"],
        config_file="configs/method_configs/snapshot.json")

    return image, face_bbox_list, snap_list


def face_recog_solution(image):
    face_box, lmk, pose = faster_rcnn(
        image,
        outputs=["rgb_face_box", "rgb_lmk", "rgb_pose"],
        config_file="configs/method_configs/face_pose_lmk.json")
    face_bbox_list, face_disappeared_track_id_list = mot(
        face_box,
        inputs=["rgb_face_box"],
        outputs=["face_bbox_list", "rgb_face_disappeared_track_id"])
    select_score_list = grading(
        face_bbox_list, pose, lmk,
        inputs=["face_bbox_list", "rgb_pose", "rgb_lmk"],
        outputs=["select_score_list"],
        config_file="configs/method_configs/grading.json")
    snap_list = snapshot(
        image, face_bbox_list, select_score_list,
        face_disappeared_track_id_list, pose, lmk, face_bbox_list,
        inputs=["image", "face_bbox_list", "select_score_list",
                "rgb_face_disappeared_track_id", "rgb_pose",
                "rgb_lmk", "face_bbox_list"],
        outputs=["snap_list"],
        config_file="configs/method_configs/snapshot.json")
    feature = cnn_method(
        snap_list, inputs=["snap_list"],
        outputs=["face_feature"],
        config_file="configs/method_configs/feature.json")

    return image, face_bbox_list, snap_list, feature


def smart_data_cb(*smart_rets):
    vt.bbox_dump(smart_rets[1])


def serialization(*smart_rets):
    print("a serialize function")


if __name__ == "__main__":
    # async
    print("========== start async mode ==========")

    vio = xproto.VioPlugin("96board")
    smart_face = xproto.SmartPlugin(
        face_solution, smart_data_cb, serialization)
    smart_face.bind(vio.message_type()[0])
    vio.start()
    smart_face.start()
    time.sleep(10)
    vio.stop()
    smart_face.stop()

    print("========== end async mode ==========")
