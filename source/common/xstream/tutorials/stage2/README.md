## 分配线程资源
在某些实际场景下，Node内部可能执行一些阻塞操作(例如，在调用BPU接口进行模型计算时),适当的分配线程资源策略，可以提高workflow的整体性能。
如果Node执行的任务（即对应的method_type）是无状态、无上下文依赖的，通常可以通过创建多个实例并利用多线程提高并发来提升性能。
在workflow json config文件中可以配置每个Node分配线程资源，可以通过两种方式给Node分配线程资源：
*  通过thread_count字段指定每个node可以在多少个thread中并行执行，当一个node指定多个线程时，连续多帧任务默认会按round-robin的方式分发到多个线程。
```json
{
  "inputs": ["in_bbox"],  // 输入的数据list，它是workflow里面定义的inputs的子集
  "outputs": ["bbox_filtered_A",
              "bbox_filtered_B"],  // 输出的数据list，它是workflow里面定义的outputs的一个子集
  "workflow": [ // node列表，
    { // BBoxFilter_A node
      "method_type": "BBoxFilter",  // Method类型是BBoxFilter
      "unique_name": "BBoxFilter_A",  // Node 在workflow中的唯一名字
      "thread_count": 3, // 给BBoxFilter_A分配了3个执行线程
      "inputs": [
        "in_bbox"  // 本Node的输入，它是从整体workflow的“inputs”中连过来的
      ],
      "outputs": [
        "bbox_filtered_A"  // 本Node的输出，它可以被连接到workflow输出或者其他Node的输入
      ],
      "method_config_file": "a_filter.json"  // 本Node对应的Method的配置文件路径，为相对本workflow配置文件的相对路径
    },
    { // BBoxFilter_B node
      "method_type": "BBoxFilter",  // Method类型是BBoxFilter
      "unique_name": "BBoxFilter_B",  // Node 在workflow中的唯一名字
      "thread_count": 2, // 给BBoxFilter_B分配2个执行线程
      "inputs": [
        "in_bbox"  // 本Node的输入，它是从整体workflow的“inputs”中连过来的
      ],
      "outputs": [
        "bbox_filtered_B"
      ],
      "method_config_file": "b_filter.json"
    }
  ]
}
```
* 通过thread_list指定每个Node可运行的线程index数组(即，thread_list指定了每个node运行在线程池的特点的第index号线程上)，通过该方式可以使多个Node之间共享线程资源。
* 下面的例子就是BBoxFilter_A node和BBoxFilter_B node共享2(第0号和第1号)个线程。
```json
{
  "inputs": ["in_bbox"],  // 输入的数据list，它是workflow里面定义的inputs的子集
  "outputs": ["bbox_filtered_A",
              "bbox_filtered_B"],  // 输出的数据list，它是workflow里面定义的outputs的一个子集
  "workflow": [ // node列表，
    { // BBoxFilter_A node
      "method_type": "BBoxFilter",  // Method类型是BBoxFilter
      "unique_name": "BBoxFilter_A",  // Node 在workflow中的唯一名字
      "thread_list": [0, 1], // 给BBoxFilter_A分配了2个执行线程, 线程index为[0, 1]
      "inputs": [
        "in_bbox"  // 本Node的输入，它是从整体workflow的“inputs”中连过来的
      ],
      "outputs": [
        "bbox_filtered_A"  // 本Node的输出，它可以被连接到workflow输出或者其他Node的输入
      ],
      "method_config_file": "a_filter.json"  // 本Node对应的Method的配置文件路径，为相对本workflow配置文件的相对路径
    },
    { // BBoxFilter_B node
      "method_type": "BBoxFilter",  // Method类型是BBoxFilter
      "unique_name": "BBoxFilter_B",  // Node 在workflow中的唯一名字
      "thread_list": [0, 1], // 给BBoxFilter_B分配2个执行线程, 线程index为[0, 1]
      "inputs": [
        "in_bbox"  // 本Node的输入，它是从整体workflow的“inputs”中连过来的
      ],
      "outputs": [
        "bbox_filtered_B"
      ],
      "method_config_file": "b_filter.json"
    }
  ]
}
```
通过thread_list的方式相比thread_count可以设置多个Node共享线程资源，比如上面的配置，BBoxFilter_A与BBoxFilter_B在两个共享线程上执行。
> Note：通过thread_list的方式，用户可设置的合法的index取值范围为[0, 999]。
