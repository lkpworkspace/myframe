# myframe docker
主要用于创建myframe docker容器镜像；
包含编译好的myframe，可以运行example，benchmark和创建运行组件工程。

## 目录
myframe安装目录：`/usr/local`

## docker操作
```sh
# eg:
#   arch: < aarch64 | amd64 >
#   myframe_version: 0.9.1

## 构建docker镜像
bash docker_build.bash <myframe_version>

## 下载docker镜像
docker pull docker.io/likepeng0418/myframe:<arch>-<myframe_version>

## 创建运行docker容器
docker run -itd \
    --name "myframe_run" \
    myframe:${<arch>-<myframe_version>} \
    /bin/bash

## 进入docker容器
docker exec -it \
    -it "myframe_run" \
    /bin/bash
```

## 在docker中开发
[开发手册](../doc/development_guide.md)
