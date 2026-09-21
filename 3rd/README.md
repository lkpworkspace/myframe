# 3rd
该目录主要用于下载/构建/安装依赖包使用

## 基于CMAKE安装依赖
进入代码库根目录执行以下命令:
```sh
cmake -S 3rd -B ".mypm/build_3rd" -DCMAKE_INSTALL_PREFIX=".mypm/deps" -DMYFRAME_ENABLE_PYBIND=ON
cmake --build ".mypm/build_3rd" -j --config Release
```
