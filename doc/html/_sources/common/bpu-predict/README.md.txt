
# Download
git clone --recursive ...

# Config

Install cross platform compiler [gcc-linaro-6.5.0](https://releases.linaro.org/components/toolchain/binaries/6.5-2018.12/aarch64-linux-gnu/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu.tar.xz)

```bash
GCCARM_LINK=https://releases.linaro.org/components/toolchain/binaries/6.5-2018.12/aarch64-linux-gnu/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu.tar.xz
wget -O /tmp/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu.tar.xz ${GCCARM_LINK}  
tar xf /tmp/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu.tar.xz --strip-components=1 -C /opt/  
rm -rf /tmp/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu.tar.xz
```

在build.sh中将下载的交叉编译工具链的路径赋值给LINARO_GCC_ROOT变量

# Compile
    
```bash
sh build.sh (同时编译arm和x86两个版本，会产生build_arm, build_x86和release三个文件夹，其中build_arm和build_x86分别保存了arm和x86两个平台的编译结果，release文件夹中保存了要对外发布的文件，主要包括include和lib)
```

# Release
```bash
sh release version_number (运行这个脚本需要安装galler 客户端，具体安装和使用见http://wiki.hobot.cc/pages/viewpage.action?pageId=38969452)
```

# Run example and unittest

TODO

# Code Contributing

see [CONTRIBUTING.md](./CONTRIBUTING.md)
