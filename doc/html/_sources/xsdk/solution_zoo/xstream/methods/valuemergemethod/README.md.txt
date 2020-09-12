# ValueMerge Method

## 介绍
数值融合method。

当前主要用于rgb、nir活体值。具体介绍可参考：http://wiki.hobot.cc/pages/viewpage.action?pageId=67788285

## 输入

| Slot | 内容           | 备注                |
| :--- | -------------- | ------------------- |
| 0    | rgb_boxes      | rgb图像对应的人脸框 |
| 1    | rgb_anti_spfs  | rgb框对应的活体值   |
| 2    | nir_boxes      | nir图像对应的人脸框 |
| 3    | nir_anti_spfs  | nir框对应的活体值   |
| 4    | rgb_box_state1 | rgb人脸框过滤值1    |
| 5    | rgb_box_state2 | rgb人脸框过滤值2    |
| 6    | nir_box_state1 | nir人脸框过滤值1    |
| 7    | nir_box_state2 | nir人脸框过滤值2    |

## 输出

| Slot | 内容            | 备注            |
| ---- | --------------- | --------------- |
| 0    | merged_anti_spf | merge后的活体值 |

## 配置文件

```json
{
  "merge_basis": "rgb_and_nir",
  "desp": "support four modes, 1. rgb_and_nir 2. rgb_or_nir 3. rgb 4. nir"
}
```

merge_basis: value merge的模式，目前支持4种方式：rgb_and_nir 、rgb_or_nir 、rgb 、nir

## 编译

- **32位**

```shell
# cp build.properties.local.x2_32bit build.properties.local
# mkdir build
# cd build
# cmake .. & make
```

- **64位**

```shell
# cp build.properties.local.x2_64bit build.properties.local
# mkdir build
# cd build
# cmake .. & make
```

