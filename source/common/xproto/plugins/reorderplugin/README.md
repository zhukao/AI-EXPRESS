# ReorderPlugin
## Intro

ReorderPlugin为重排序插件，主要负责按frame_id/sequence_id对智能帧和Drop帧进行重排序，保证输出与输入的顺序相同。

## Detail

ReorderPlugin监听SmartMessage和VioMessage。收到Message后，首先构造ReorderMessage，之后将其放入按frame_id/sequence_id排序的最小堆中，最后从堆中取出当前应该发送的Message并push到总线。

ReorderPlugin初始化时传入配置文件路径，配置文件中指定是否进行排序。

```json
{
  "need_reorder": 1
}
1: 排序, 0: bypass
```

## Usage

### ReorderMessage定义

```c++
struct ReorderMessage : XProtoMessage {
  ReorderMessage() { type_ = TYPE_REORDER_MESSAGE; }
  std::string Serialize() override { return "Default reorder message"; }
  virtual ~ReorderMessage() = default;

  XProtoMessagePtr actual_msg_;
  std::string actual_type_;
  uint64_t frame_id_;
};
```
