# 3rd
该目录主要用于下载/构建/安装依赖包使用

## 基于CMAKE安装依赖
进入代码库根目录执行以下命令:
```sh
cmake -S 3rd -B build_3rd -DCMAKE_INSTALL_PREFIX=output
cmake --build build_3rd -j --config Release
```

## 基于包管理安装依赖
进入代码库根目录执行以下命令:
```sh
mypm install --one-folder -p ${PWD}/output jsoncpp,1.9.5
mypm install --one-folder -p ${PWD}/output glog,0.6.0
# 需要编译python绑定时安装此依赖
mypm install --one-folder -p ${PWD}/output swig,4.3.1
```
