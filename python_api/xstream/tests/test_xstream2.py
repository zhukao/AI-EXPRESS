import sys
import os
sys.setdlopenflags(os.RTLD_LAZY)
# workflow.py
import xstream    # noqa

# "CNNMethod" 对应MethodFactory中的CNNMethod的名字字符串。
CNNMethod = xstream.Method("CNNMethod")

# 声明inputs和outputs可以赋予输入输出参数以名字，
# 可以在生成的json中体现，让json更加可读
VehiclePreProcess = xstream.Method(
    "VehiclePreProcess").inputs(
        ["image"]
    ).outputs(
        ["processed_image"]
    )

VehiclePostProcess = xstream.Method("VehiclePostProcess").inputs(
    ["cnn_out"]).outputs(["vehicle_out"])
# vehicle_cnn的配置定义。这里包括框架支撑的属性如thread_count，
# 也包括method自定义的配置如method_conf
vehicle_cnn_conf = {"threadCnt": 3, "methodConfig": "vechicle_cfg.json"}

PedestrianPreProcess = xstream.Method("PedestrianPreProcess").inputs(
    ["image"]).outputs(["processed_image"])
PedestrianPostProcess = xstream.Method("PedestrianPostProcess").inputs(
    ["cnn_out"]).outputs(["pedestrian_out"])
pedestrian_cnn_conf = {"threadCnt": 5, "methodConfig": "pedestrian_cfg.json"}


def cnn_workflow(pre_method, post_method, cnn_conf, inputs, name=""):
    # name scope，在这个scope下，所有定义的符号的名字，都会加入scope的前缀
    with xstream.scope(name):
        # 调用 NativeMethod.__call__ 方法。由于scope的存在，
        # 全局的名字为 name + "/" + "pre"
        pre_out0 = pre_method(inputs, unique_name="pre")
        cnn_out0 = CNNMethod(pre_out0, **cnn_conf, unique_name="cnn",
                             inputs=["processed_image"], outputs=["cnn_out"])
        outputs = post_method(cnn_out0, unique_name="post")
        return outputs


def main_workflow(image):
    vehicle_box = cnn_workflow(pre_method=VehiclePreProcess,
                               post_method=VehiclePostProcess,
                               cnn_conf=vehicle_cnn_conf,
                               inputs=image, name="vehicle_cnn")

    pedestrian_box = cnn_workflow(pre_method=PedestrianPreProcess,
                                  post_method=PedestrianPostProcess,
                                  cnn_conf=pedestrian_cnn_conf,
                                  inputs=image, name="pedestrian_cnn")

    return vehicle_box, pedestrian_box


json_data = xstream.serialize(main_workflow)

print(json_data)
