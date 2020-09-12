# common

AIoT 基础库repo：源码依赖

目前包含xstream

## 编译
### 环境准备
* 安装 bazel (当前建议安装1.2.0) 
  * Ubuntu
   参见 [Installing Bazel on Ubuntu](https://docs.bazel.build/versions/master/install-ubuntu.html)
  * Fedora and CentOS
   参见 [Installing Bazel on Fedora and CentOS](https://docs.bazel.build/versions/master/install-redhat.html)
  * macOS
   参见 [Installing Bazel on macOS](https://docs.bazel.build/versions/master/install-os-x.html)
**更多Bazel帮助信息请参见[Bazel Documentation](https://docs.bazel.build/)**
 * Artifactory 环境配置
  因部分依赖Artifactory, 如unit test。 需要配置Artifactory 环境如下
 ```
 echo "machine ci.horizon-robotics.com login deploybot password deploybot@Artifactory2016" > ~/.netrc
 ```
### 编译project
  * 编译 xstream-framework
`bazel build //xstream/framework:xstream-framework`

### 单元测试：
  * all 单元测试
`bazel test //:test_all --define cpu=x86_64 --define os=linux --spawn_strategy=local`
  * xstream-framework 单元测试，其他测试项请查看xstream/framework/BUILD rule:tests_individual
`bazel test -s //xstream/framework:test_all --define cpu=x86_64 --define os=linux --spawn_strategy=local`

### 编译框架
   如果想了解或修改bazel编译框架的封装实现，请参看repo的文档和代码
  `gitlab.hobot.cc/iot/xsdk/bazel_tools`

## Methods
在 `//xstream/methods`，目前methods中的method 有：

* `//xstream/methods/cnnmethod`
* `//xstream/methods/fasterrcnnmethod`
* `//xstream/methods/motmethod`

## 文档
### 环境准备
* 安装Sphinx
`pip install -U Sphinx`
* 安装recommonmark
`pip install --upgrade recommonmark`
* Sphinx更多信息请参看[sphinx-doc](https://www.sphinx-doc.org/en/master/index.html)

### XStream 文档生成
进入./xstream目录，执行
`sphinx-build -M html . ./doc`
用浏览器(推荐用chrome)查看 ./doc/index.html
 