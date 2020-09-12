import sys
import os
import time
sys.setdlopenflags(os.RTLD_LAZY)
import xstream    # noqa
import vision_type as vt    # noqa

# 定义一个简单的workflow
bbox_method = xstream.Method("BBoxFilter").inputs(["in_bbox"])


def my_workflow(in_bbox):
    bbox_filtered_A = bbox_method(
        in_bbox, outputs=["bbox_filtered_A"],
        config_file="configs/method_configs/a_filter.json")
    bbox_filtered_B = bbox_method(
        in_bbox, outputs=["bbox_filtered_B"],
        config_file="configs/method_configs/b_filter.json")

    return bbox_filtered_A, bbox_filtered_B


json = xstream.serialize(my_workflow)
print(json)

# 创建session对象
session = xstream.Session(my_workflow)


def nodecb(bbox):
    print("===========node cb start==========")
    print(bbox.state)
    vt.bbox_dump(bbox)
    print("===========node cb end==========")


def flowcb(bbox1, bbox2):
    print("===========flow cb start==========")
    print(bbox1.state)
    print(bbox2.state)
    vt.bbox_dump(bbox1)
    vt.bbox_dump(bbox2)
    print("===========flow cb end==========")


session.callback(nodecb, "BBoxFilter_2")
session.callback(nodecb, "BBoxFilter_3")
session.callback(flowcb)

for idx in range(1000):
    session.forward(in_bbox=vt.bbox(0, 20, idx, 50))
    time.sleep(0.01)

print("all complete!")

time.sleep(1)
session.close()
time.sleep(2)
