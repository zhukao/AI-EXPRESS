AIExpress 统一编译开发测试指南
<!-- TOC -->

- [介绍](#介绍)
- [开发](#开发)
    - [创建统一编译工程](#创建统一编译工程)
    - [准备交叉编译工具链](#准备交叉编译工具链)
    - [代码编译](#代码编译)
    - [代码修改](#代码修改)
        - [创建本地分支](#创建本地分支)
        - [修改代码](#修改代码)
    - [代码评审](#代码评审)
    - [代码提交](#代码提交)
- [测试](#测试)
    - [创建部署包](#创建部署包)
    - [上板运行](#上板运行)
- [覆盖率统计](#覆盖率统计)
    - [编译](#编译)
    - [上板运行](#上板运行-1)
    - [生成报表](#生成报表)
- [对外发版](#对外发版)
    - [打tag](#打tag)
    - [打包](#打包)
    - [编译](#编译-1)
    - [部署](#部署)
    - [运行](#运行)
    - [使用发版包进行开发及测试](#使用发版包进行开发及测试)
- [常见问题](#常见问题)
    - [开发流程相关](#开发流程相关)
        - [arc land 报 No pushable remote "iot" exists](#arc-land-报-no-pushable-remote-iot-exists)
        - [cmake 工程结构](#cmake-工程结构)
    - [硬件相关](#硬件相关)
        - [hsp2610串口识别不出来](#hsp2610串口识别不出来)
        - [CAM 报错](#cam-报错)
        - [IAR 报错](#iar-报错)
    - [应用开发相关](#应用开发相关)
        - [使用valgrind](#使用valgrind)
        - [gdb/valgrind调试应用程序被killed掉](#gdbvalgrind调试应用程序被killed掉)
        - [valgrind跑程序load模型失败](#valgrind跑程序load模型失败)

<!-- /TOC -->

# 介绍
git-repo是一个基于git的仓库管理工具，可对多个git仓库同时管理，由一系列python脚本组成。

目前使用内部维护版本 `http://gitlab.hobot.cc/iot/devices/x2solution/git-repo`

git-repo需要一个manifest配置文件，里面列出项目需要的仓库及所需的分支或者tag信息，
这些配置文件存到
`http://gitlab.hobot.cc/iot/devices/x2solution/manifest` 仓库里。

BuildAll repo是基于cmake + gradle 的统一编译环境，无法做源码依赖的均使用gradle做二进制依赖，正式发版时直接将gradle依赖拷贝到发版包里。

BuildAll repo里包含一系列编译、测试、部署、打包脚本供使用，可分为
`BuildAll 开发环境`和`ai_express_release发版包环境`两类：
* BuildAll 开发环境

| 文件名称                    | 说明                                     |
| --------------------------- | ---------------------------------------- |
| README.md                   | 开发环境指导文档                         |
| make.sh                     | 开发环境编译                             |
| package.sh                  | 创建发版包                               |
| deploy.sh                   | 创建上板测试用部署包                     |
| run.sh                      | 在板子上运行测试程序                     |
| build_coverage_test.sh      | 编译带有覆盖率信息的可执行程序和库       |
| run_coverage_test.sh        | 在板子上运行测试程序并生成覆盖率统计信息 |
| generate_coverage_report.sh | 生成代码覆盖率报告                       |
| download_model.sh           | 下载发版包所需模型                       |
| download_xstream_model.sh   | 下载自测所需模型                         |

* ai_express_release发版包环境

在调用`package.sh`创建发版包的过程中，以下文件会被重命名为`README.md`等。

| 文件名称           | 说明                 |
| ------------------ | -------------------- |
| README_external.md | 指导文档             |
| build.sh_external  | 代码编译             |
| deploy.sh_external | 创建上板测试用部署包 |
| run_external.sh    | 在板子上运行测试程序 |


# 开发
## 创建统一编译工程
```
# 新建一个项目，比如xstream-all
[ruoting@gpu03 repos]$ mkdir xstream_all
[ruoting@gpu03 repos]$ cd xstream_all/
[ruoting@gpu03 xstream_all]$ repo init -u http://gitlab.hobot.cc/iot/devices/x2solution/manifest.git -m solution_zoo.xml
Get /home/ruoting/docker_share/git-repo/.git
remote: Counting objects: 4224, done.
......
Your identity is: ruoting <ruoting.ding@horizon.ai>
If you want to change this, please re-run 'repo init' with --config-name

repo has been initialized in /home/ruoting/docker_share/repos/xstream_all
```
初始化完后，`.repo/manifest.xml`文件指向了我们希望使用的solution_zoo.xml。如果后续想要本地修改manifest配置文件，可直接修改它。
```
[ruoting@gpu03 xstream_all]$ ll .repo/manifest.xml
lrwxrwxrwx 1 ruoting ruoting 26 Mar 11 10:49 .repo/manifest.xml -> manifests/solution_zoo.xml
```

接着执行`repo sync`拉取代码
```
[ruoting@gpu03 xstream_all]$ repo sync
......
Fetching projects: 100% (6/6)
Fetching projects: 100% (6/6), done.
......
Syncing work tree: 100% (6/6), done.

[ruoting@gpu03 xstream_all]$ ls
BuildAll  common  x2_prebuilt  xsdk
```
## 准备交叉编译工具链

需提前准备好交叉编译工具链，默认路径如下：
```
cmake_c_compiler /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
cmake_cxx_compiler /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++
```
 如果交叉编译工具链地址变动，需同步修改build.property.local.aarch64
## 代码编译
执行如下命令，编译出来的可执行文件和库在build/bin和build/lib目录下

```
bash BuildAll/make.sh x2|x3|all release|debug|coverage
```

* release 编译release版本，使用默认优化等级
* debug 编译带有debug信息、使用Og等级的优化选项
* coverage编译debug版本，并且打开覆盖率统计选项。

## 代码修改
下面以修改BuildAll repo为例：

### 创建本地分支
```
[ruoting@gpu03 xstream_all]$ cd BuildAll/
## m/master指向了我们使用的iot/solution_zoo
[ruoting@gpu03 BuildAll]$ git checkout -b dev m/master
Branch dev set up to track remote branch solution_zoo from iot.
Switched to a new branch 'dev'
```
### 修改代码
```
[ruoting@gpu03 BuildAll]$ vi README.md
[ruoting@gpu03 BuildAll]$ git add README.md
[ruoting@gpu03 BuildAll]$ git commit -m "add how-to-coding"
```

## 代码评审
如果修改的是源码，则需要重新编译程序，进入[测试](#测试)环节进行上板验证。编译&测试都OK后，就可以进入代码评审环节：

```
[ruoting@gpu03 BuildAll]$ arc diff
Linting...
No paths are lintable.
Running unit tests...
No unit test engine is configured for this project.
 SKIP STAGING  Unable to determine repository for this change.
Updating commit message...
Created a new Differential revision:
        Revision URI: https://cr.hobot.cc/D33632

Included changes:
  M       README.md

```
## 代码提交
 cr accept后就可以提交

```
[ruoting@gpu03 BuildAll]$ arc land
```
# 测试
## 创建部署包
在统一编译工程目录下，执行：
```
bash BuildAll/deploy.sh
```
脚本执行完生成deploy_dev部署文件夹。
目前该部署包里的测试用例较少，需要继续补充：

| 名称             | 备注                 |
| ---------------- | -------------------- |
| run.sh           | 运行脚本             |
| models           | 模型                 |
| lib              | 二进制依赖库         |
| configs          | vio 配置文件         |
| body_solution    | 人体解决方案测试用例 |
| face_solution    | 人脸解决方案测试用例 |
| vehicle_solution | 车辆解决方案测试用例 |
| face_body_multisource    |     多路输入多workflow解决方案 |
| methods          | method测试用例集合   |
| vision_type      | vision_type测试用例  |
| image_tools      | image_tools测试用例  |
## 上板运行
```
sh run.sh face|body|vehicle...
```
一个运行case如下：
```
root@X2_96BOARD:/userdata/data/repos/xstream_all/deploy_dev# sh run.sh grading_method
[==========] Running 1 test from 1 test case.
[----------] Global test environment set-up.
[----------] 1 test from XStreamGradingMethodTest
[ RUN      ] XStreamGradingMethodTest.GradingTest
[       OK ] XStreamGradingMethodTest.GradingTest (1006 ms)
[----------] 1 test from XStreamGradingMethodTest (1006 ms total)
[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (1006 ms total)
[  PASSED  ] 1 test.
```

# 覆盖率统计
## 编译
编译带有覆盖率统计信息的可执行程序
```
bash BuildAll/build_coverage_test.sh
```
该脚本会为每个源文件输出一个配套的gcno文件，以xstream-framework为例：

```
[root@gpu03 xstream_all]$ ls build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/*.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/method.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/method_manager.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/node.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/profiler.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/scheduler.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/thread_pool.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/xstream_config.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/xstream.cpp.gcno
build/xsdk/common/xstream/framework/CMakeFiles/xstream-framework.dir/src/xstream_data.cpp.gcno
```

## 上板运行
为了统计方便，我们假定用户将整个工程都通过NFS挂载到开发板上了，打开BuildAll/run_coverage_test.sh文件，根据个人实际开发环境配置，可灵活传递GCOV_PREFIX_STRIP变量的值。
> GCOV_PREFIX_STRIP是一个整数，用于设置开发环境代码目录树中，有几级目录需要被裁剪掉

`run_coverage_test.sh`脚本里包含所有测试用例的运行环境设置&执行命令，运行该脚本，获得测试用例覆盖率统计文件

其中solution解决方案（face solution & body solution & vehicle solution）都是实时拉码流运行的，建议在开发板摄像头先放置人脸照片/人体照片/车辆照片来模拟真实场景，提高覆盖率。
```
sh BuildAll/run_coverage_test.sh GCOV_PREFIX_STRIP变量的值
```
## 生成报表
返回开发环境，运行`generate_coverage_report.sh`脚本即可在当前路径下获得coverage_build.zip覆盖率报告，里面包含详细的行覆盖率和函数覆盖率，一般要求函数覆盖率要达到85%以上。
```
bash BuildAll/generate_coverage_report.sh
```

# 对外发版
## 打tag
* 发版前确认好系统镜像版本，镜像确定后，参与发版的全体研发同学统一更新镜像，CI/CD的板子也换镜像。
* 对[BuildAll](http://gitlab.hobot.cc/iot/devices/x2solution/BuildAll),[common](http://gitlab.hobot.cc/iot/xsdk/common),[solution_zoo](http://gitlab.hobot.cc/iot/xsdk/solution_zoo), [x2_prebuilt](http://gitlab.hobot.cc/iot/devices/x2solution/x2_prebuilt), [x3_prebuilt](http://gitlab.hobot.cc/iot/xsdk/x3_prebuilt), [VioWrapper](http://gitlab.hobot.cc/iot/xsdk/viowrapper)代码仓库创建tag，Release notes注明版本变更内容。创建tag name建议：
  * 内部发版前测试：以发布版本2.0.0为例，先打一个AIEXPRESS-V2.0.0-RC0，然后进行打包、编译、部署、运行。如果有问题，解决后，再创建AIEXPRESS-V2.0.0-RC1，RC序号依次累计。
  * 正式发版：打一个AIEXPRESS-V2.0.0，发版。

## 打包
* 编译不开源的库，如bpu_predict,ipc_tracking等。为保证有权限执行脚本，建议使用sudo权限执行一下脚本.
`sudo bash BuildAll/make.sh all release`
* 打包，创建ai_express_release发版包
`sudo bash BuildAll/package.sh`
* 生成文档，在ai_express_release/doc创建html格式的文档
`sudo bash BuildAll/generate_doc.sh`
  * 准备条件，安装pip `sudo apt-get install python-pip`
  * 安装文档生软件Sphinx `sudo pip install -U Sphinx`
  * 安装Sphinx插件recommonmark用于转化markdown语法的*.md文件`sudo pip install --upgrade recommonmark`
  * 安装Sphinx插件sphinx-markdown-tables用于转化markdown中的表格`sudo pip install sphinx-markdown-tables`
  * 安装Sphinx插件sphinx-rtd-theme用于使用sphinx-rtd-theme样式`sudo pip install sphinx-rtd-theme`

## 编译
* 在生成的发布包中，可以直接运行`sudo bash ./build.sh`，完成整个开发包的编译。
* 如果您开发了自己的method, 或者其他solution方案，请在代码所在路径的CMakeLists.txt中增加对应的编译命令。

## 部署
* 在编译完成后，执行`sudo bash ./deploy.sh`, 会在根目录下生成deploy目录。
* 请将deploy目录完整拷贝到X2板子上。

## 运行
* deploy中有run.sh，运行这个脚本，可以运行软件包中默认提供的几个solution方案。
* `usage: sh run.sh [ face | body | vehicle ]`
* 如果您自己开发了solution，可以参考run.sh设置启动脚本。
* 关键点:
  *  将deploy/lib加入到LD_LIBRARY_PATH， 设置`export LD_LIBRARY_PATH=./lib/`。
  *  设置如`./configs/vio_config.json.96board ./body_solution/configs/body_solution.json -i`这样的运行参数,其中vio_config.json.96board是96board的配置， ./body_solution/configs/body_solution.json是solution的配置。

`ai_express_release`发版包包含如下几个部分：

| 名称           |           备注 |
| -------------- | -------------: |
| README.md      |       介绍文档 |
| source         |           源码 |
| deps           |         依赖库 |
| models         |           模型 |
| doc            |           文档 |
| build.sh       |       编译脚本 |
| deploy.sh      |       部署脚本 |
| run.sh         | 部署包运行脚本 |
| CMakeLists.txt |  cmake配置文件 |


## 使用发版包进行开发及测试
请查看发版包下的README.md文件

# 常见问题
## 开发流程相关
### arc land 报 No pushable remote "iot" exists
```
[ruoting@gpu03 BuildAll]$ arc land
Landing current branch 'dev'.
 TARGET  Landing onto "solution_zoo", selected by following tracking branches upstream to the closest remote.
 REMOTE  Using remote "iot", selected by following tracking branches upstream to the closest remote.
 Exception
No pushable remote "iot" exists. Use the "--remote" flag to choose a valid, pushable remote to land changes onto.
(Run with `--trace` for a full exception trace.)
```
可能是因为git 版本太低（centos7.4.5 自带 git version 1.8.3.1），
可以考虑
到https://mirrors.edge.kernel.org/pub/software/scm/git/ 下载一个较新版本，编译成功后将可执行程序路径加到PATH里，比如：
```
export PATH=~/tools/git/git-2.26.0-rc1/bin-wrappers:/$PATH
```
再次执行arc land 即可成功。

### cmake 工程结构

基于http://gitlab.hobot.cc/ptd/cp/devops/hobotclitools
，每个工程下的`CMakeLists.txt`建议以如下结构组织:

```
set(SOURCE_FILES
        src/myapi.cpp
        include/${module_name}/myapi.h
        )

add_library(${module_name} SHARED|STATIC ${SOURCE_FILES})

```

## 硬件相关
### hsp2610串口识别不出来
安装PL2303驱动
### CAM 报错
找系统软件王芬芬支持
### IAR 报错
找系统软件郭睿支持

## 应用开发相关

### 使用valgrind
* 获取工具：`\\bjnas\bjnas\智能IoT产品线\智能IoT研发部\XExpress\开发工具\prerootfs.tar`
* 将prerootfs.tar拷贝到开发板挂载的nfs上,解压缩
```
[ruoting@gpu03 docker_share]$ tar -xf prerootfs.tar
[ruoting@gpu03 docker_share]$ ls prerootfs
bin  dev  etc  home  lib  mnt  proc  run  sbin  sys  tmp  usr  var
[ruoting@gpu03 docker_share]$ ls prerootfs/usr/bin/valgrind
prerootfs/usr/bin/valgrind
```
* 登陆开发板，配置环境变量&运行

以prerootfs挂载到了`/mnt/share_ruoting/prerootfs`为例，修改run.sh如下：
```
export PATH=/mnt/share_ruoting/prerootfs/usr/bin:$PATH
export VALGRIND_LIB=/mnt/share_ruoting/prerootfs/usr/lib/valgrind
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=definite --log-file=valgrind.txt --track-fds=yes"
valgrind $VALGRIND_OPTS ./vehicle_solution/vehicle_solution $vio_cfg_file ./vehicle_solution/configs/smart_config.json -i
```
打开valgrind.txt可查看结果。

### gdb/valgrind调试应用程序被killed掉
执行`memstat`查看系统内存分布情况，以
`Linux X2_96BOARD 4.14.74 #12 SMP PREEMPT Thu Nov 14 21:18:15 PST 2019 aarch64 GNU/Linux`
为例：96board总共1G物理内存，其中776M都分配给了ion,应用可用内存仅187M。
```
root@X2_96BOARD:/mnt/share_ruoting/repos/xstream_all/deploy_dev# free
             total       used       free     shared    buffers     cached
Mem:        191944      46992     144952        568        536       7924
-/+ buffers/cache:      38532     153412
Swap:            0          0          0
root@X2_96BOARD:/mnt/share_ruoting/repos/xstream_all/deploy_dev# memstat
----------------------------------------------------
address    size(hex)  size name
---------- ---------- ---- <reserved dts>-----------
0x02000000 0x00080000 512K bifbase_reserved(nomap)
0x02080000 0x00040000 256K ramoops
0x020C0000 0x0003F000 252K fc_reserved
0x020FF000 0x00001000   4K sw_reserved(nomap)
0x02100000 0x02000000  32M iar_reserved(nomap)
0x04100000 0x30800000 776M ion_reserved
---------- ---------- ---- <memory cal>-------------
-          -      1015292K map-memory(reserved+others)
-          -        33284K nomap-memory(see:dts)
---------- ---------- ---- -------------------------
0x00000000 0x40000000   1G X2_96BOARD
----------------------------------------------------
```
ion_reserved是isp、ipu、bpu预留物理内存，可根据实际情况进入uboot调整：
```
Hobot>setenv ion_size '256'
Hobot>saveenv
```
### valgrind跑程序load模型失败
和[gdb/valgrind调试应用程序被killed掉](#gdbvalgrind调试应用程序被killed掉) 情况相反，预留给ion的物理内存太小，可进入uboot加大ion_size试试。
