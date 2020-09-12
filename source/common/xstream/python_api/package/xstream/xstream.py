import copy
import json
import inspect
import collections

__all__ = [
    'scope',
    'Variable',
    'Method',
    'serialize'
]

# 当前scope
globalScope = []

# 命名分隔符
delimiter = '_'


class VariableScope():
    thisScope = ''

    def __init__(self, scope):
        self.thisScope = scope

    def __enter__(self):
        globalScope.append(self.thisScope)

    def __exit__(self, exc_type, exc_value, exc_tb):
        globalScope.pop()
        if exc_tb is not None:
            print('[Exit %s]: Exited with exception raised.' % self.thisScope)


# 命名空间的scope
def scope(name):
    return VariableScope(name)


class Variable:
    """Methods 之间的数据流"""

    # 数据名称
    dataName = ''

    # 产生本数据的Method
    node = None

    # 输出名称时是否加上前缀
    # 当variable作为workflow的input或outout时, 不能带scope前缀
    withPrefix = True

    def __init__(self, dataName):
        self.dataName = dataName

    # 返回当前Variable的名字
    def name(self):
        if self.withPrefix:
            return "%s%s%s" % (
                self.node.unique_name(),
                delimiter,
                self.dataName
            )
        else:
            return self.dataName


class Node:
    # node的输入和输出
    nodeCfg = {
        # node的输入和输出
        "inputs": [],
        "outputs": [],

        "method_type": "",
        "unique_name": "",

        "config_file": "null",
        "config": {},
        "thread_count": 1,
        "thread_list": []
    }

    # 该node对应的输入输出Variable
    inputsVariable = ()
    outputsVariable = ()

    # 当前Scope
    currentScope = []

    def __init__(self, method):
        assert isinstance(method, Node)
        self.nodeCfg = copy.copy(method.nodeCfg)

    # 默认workflow参数控制
    def __getattr__(self, name):
        assert name in self.nodeCfg, \
            "Unknow node attribute: %s" % name

        def accessor(value=None):
            if value is not None:
                assert name != "method_type", \
                    "Method type cannot be modified!"
                assert isinstance(value, type(self.nodeCfg[name])), \
                    "%s type error. expect %s" % (
                        name, type(self.nodeCfg[name]))
                self.nodeCfg[name] = value
                return self
            elif name in self.nodeCfg:
                return self.nodeCfg[name]
            else:
                return None
        return accessor

    # 设置method 名字
    def unique_name(self, name=None):
        # 返回当前node unique name
        if name is None:
            if len(self.currentScope) > 0:
                return "%s%s%s" % (
                    delimiter.join(self.currentScope),
                    delimiter,
                    self.nodeCfg["unique_name"]
                )
            else:
                return self.nodeCfg["unique_name"]
        else:
            assert isinstance(name, str)
            self.nodeCfg["unique_name"] = name
            return self

    # dump node 数据
    def dump_node_cfg(self):
        node_cfg = {
            "thread_count": self.thread_count(),
            "method_type": self.method_type(),
            "unique_name": self.unique_name(),
            "inputs": [
                self.inputsVariable[idx].name()
                for idx in range(len(self.inputsVariable))
            ],
            "outputs": [
                self.outputsVariable[idx].name()
                for idx in range(len(self.outputsVariable))
            ],
            "method_config_file": self.config_file(),
            "method_config": self.config(),
            "thread_list": self.thread_list()
        }

        if len(node_cfg["thread_list"]) == 0:
            del node_cfg["thread_list"]
        else:
            del node_cfg["thread_count"]

        if len(node_cfg["method_config"]) == 0:
            del node_cfg["method_config"]
        return node_cfg


class Method(Node):
    """xstream workflow method"""

    # method type计数
    methodTypeCnt = 0

    def __init__(self, type):
        # 拷贝,而不是引用
        self.nodeCfg = copy.copy(Node.nodeCfg)
        # 设置method type
        self.nodeCfg["method_type"] = type

    # 调用该Method
    def __call__(self, *inputs, **attrs):
        node_info = Node(self)
        # 生成该node的unique name
        node_info.unique_name(
            "%s%s%d" % (
                self.method_type(),
                delimiter,
                self.methodTypeCnt
            )
        )
        self.methodTypeCnt += 1

        # 更新配置
        if "method_type" in attrs:
            del attrs["method_type"]

        node_info.nodeCfg.update(attrs)

        assert len(node_info.outputs()) > 0 and len(node_info.inputs()) > 0, \
            "node at least have 1 output and 1 input"

        assert len(inputs) == len(node_info.inputs()), \
            "too few input Variable!"

        for idx in range(len(inputs)):
            assert isinstance(inputs[idx], Variable)
            assert inputs[idx].dataName in node_info.inputs(), \
                "unsupport input data type %s!" % (inputs[idx].dataName)

        # 浅拷贝当前scope
        node_info.currentScope = copy.copy(globalScope)

        outputs = []

        # 生成输出variable
        for idx in range(len(node_info.outputs())):
            outputs.insert(idx, Variable(node_info.outputs()[idx]))
            outputs[idx].node = node_info

        node_info.inputsVariable = inputs
        node_info.outputsVariable = tuple(outputs)

        # 返回该node的输出数据
        return tuple(outputs) if len(outputs) > 1 else outputs[0]

    # 覆盖父类中的unique name设置. 定义Method模板的时候,设置unique name没有意义
    def unique_name(self, value):
        pass


# 拓扑排序
def _topological_soring(prereqs):
    order, seen, keys = [], {}, []

    def _visitall(keys, prereqs):
        for key in keys:
            if key not in seen:
                seen[key] = True
                if key in prereqs:
                    _visitall(prereqs[key], prereqs)

                order.append(key)

    for key in prereqs.keys():
        keys.append(key)

    _visitall(keys, prereqs)
    return order


# 将workflow生成json
def serialize(workflow, *inputs):
    assert callable(workflow), \
        "Workflow is not a function!"

    args = inspect.getargspec(workflow)
    assert len(args.args) != 0, \
        "Workflow must contain at least one input!"

    for idx in range(len(inputs)):
        assert isinstance(inputs[idx], Variable)

    if len(inputs) == 0:
        inputs = tuple([
            Variable(args.args[idx])
            for idx in range(len(args.args))
        ])

    assert len(inputs) == len(args.args), \
        "Inconsistent number of parameters"

    outputs = workflow(*inputs)
    if not isinstance(outputs, tuple):
        outputs = (outputs,)

    for idx in range(len(outputs)):
        assert isinstance(outputs[idx], Variable)

    # workflow的信息
    workflow_info = {
        "inputs": [],
        "outputs": [],
        "nodes": {},
        "dependency": {}
    }

    # 回溯函数
    def _back_tracking(results, deps=0):
        for idx in range(len(results)):
            # 记录输出variable
            if deps == 0 and results[idx] not in workflow_info["outputs"]:
                workflow_info["outputs"].append(results[idx])

            if results[idx].node is None:
                if results[idx] not in workflow_info["inputs"]:
                    workflow_info["inputs"].append(results[idx])

                continue
            else:
                unique_name = results[idx].node.unique_name()
                if unique_name not in workflow_info["nodes"]:
                    workflow_info["nodes"][unique_name] = results[idx].node
                    workflow_info["dependency"][unique_name] = []

                # 记录node的依赖关系
                for idx2 in range(len(results[idx].node.inputsVariable)):
                    node2 = results[idx].node.inputsVariable[idx2]
                    if node2.node is None:
                        continue

                    uni_name2 = node2.node.unique_name()
                    if uni_name2 in workflow_info["dependency"][unique_name]:
                        continue
                    else:
                        workflow_info["dependency"][unique_name].append(
                            uni_name2)

            _back_tracking(results[idx].node.inputsVariable, deps + 1)

    _back_tracking(outputs)

    # 拓扑排序
    nodes_order = _topological_soring(workflow_info["dependency"])

    assert len(workflow_info["inputs"]) > 0 and len(
        workflow_info["outputs"]) > 0 and len(workflow_info["nodes"]) > 0

    # 对input和output变量进行处理, 导出时取消prefix
    for idx in range(len(workflow_info["inputs"])):
        workflow_info["inputs"][idx].withPrefix = False

    for idx in range(len(workflow_info["outputs"])):
        workflow_info["outputs"][idx].withPrefix = False

    # 导出为json
    workflow_json = {
        "inputs": [],
        "outputs": [],
        "workflow": []
    }
    for node in nodes_order:
        node_obj = workflow_info["nodes"][node]
        workflow_json["workflow"].append(node_obj.dump_node_cfg())

    workflow_json["inputs"] = [
        workflow_info["inputs"][idx].name()
        for idx in range(len(workflow_info["inputs"]))
    ]
    workflow_json["outputs"] = [
        workflow_info["outputs"][idx].name()
        for idx in range(len(workflow_info["outputs"]))
    ]

    return json.dumps(workflow_json, sort_keys=True)
