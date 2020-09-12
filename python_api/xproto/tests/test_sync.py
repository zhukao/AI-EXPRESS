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
merge = xstream.Method("MergeMethod") \
    .config_file("configs/method_configs/merge_head_body.json")


def body_solution(image):
    body_box, head_box, face_box, lmk, pose, kps = faster_rcnn(
        image,
        outputs=["body_box", "head_box", "face_box", "lmk", "pose", "kps"],
        config_file="configs/method_configs/multitask_config.json")
    face_bbox_list, face_disappeared_track_id_list = mot(
        face_box,
        outputs=["face_bbox_list", "face_disappeared_track_id_list"])
    head_bbox_list, head_disappeared_track_id_list = mot(
        head_box,
        inputs=["head_box"],
        outputs=["head_bbox_list", "head_disappeared_track_id_list"])
    body_bbox_list, body_disappeared_track_id_list = mot(
        body_box,
        inputs=["body_box"],
        outputs=["body_bbox_list", "body_disappeared_track_id_list"])
    face_final_box, head_final_box, body_final_box, disappeared_track_id = merge(   # noqa
        face_bbox_list, head_bbox_list, body_bbox_list,
        face_disappeared_track_id_list, head_disappeared_track_id_list,
        body_disappeared_track_id_list, kps,
        inputs=["face_bbox_list", "head_bbox_list", "body_bbox_list",
                "face_disappeared_track_id_list",
                "head_disappeared_track_id_list",
                "body_disappeared_track_id_list", "kps"],
        outputs=["face_final_box", "head_final_box",
                 "body_final_box", "disappeared_track_id"])

    return image, face_final_box, head_final_box, body_final_box, \
        disappeared_track_id, pose, lmk, kps


def smart_data_cb(*smart_rets):
    vt.bbox_dump(smart_rets[1])


def serialization(*smart_rets):
    print("a serialize function")


if __name__ == "__main__":
    # sync
    print("========== start sync mode ==========")

    vio_sync = xproto.VioPlugin("96board", "jpeg_image_list",
                                name_list="data/image.list")
    smart_body = xproto.SmartPlugin(
        body_solution, smart_data_cb, serialization)
    vio_sync.start(True)
    with open(vio_sync.name_list_) as in_file:
        lines = (line.rstrip() for line in in_file)
        lines = list(line for line in lines if line)
    processed = 0
    while processed < len(lines):
        image = vio_sync.get_image()
        smart_body.feed(image)
        processed += 1
    time.sleep(1)
    vio_sync.stop()
    smart_body.stop()

    print("========== end sync mode ==========")
