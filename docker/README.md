# myframe docker
主要用于创建myframe docker容器镜像；
包含编译好的myframe，可以运行example，benchmark和创建运行组件工程。

## 目录
myframe安装目录：`/usr/local`

## 构建docker镜像
```sh
# eg:
#   myframe_version: 0.9.1
bash docker_build.bash <myframe_version>
```

## 构建好的docker镜像
```sh
docker pull docker.io/likepeng0418/myframe:amd64-<myframe_version>
```

## 创建运行docker容器
```sh
docker run -itd \
    --name "myframe_run" \
    myframe:${<myframe_version>} \
    /bin/bash
```

## 进入docker容器
```sh
docker exec -it \
    -it "myframe_run" \
    /bin/bash
```

## 在docker中开发
[开发手册](../doc/development_guide.md)
