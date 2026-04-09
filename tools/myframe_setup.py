#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import os
import sys
import platform

def setup():
    script_dir = os.path.split(os.path.realpath(__file__))[0]
    root_dir = os.path.dirname(script_dir)
    python_lib_dir = os.path.join(root_dir, 'lib', 'python')
    os_name = platform.system()
    if os_name == "Windows":
        lib_dir = os.path.join(root_dir, 'bin')
    else:
        lib_dir = os.path.join(root_dir, 'lib')

    # print(f'script dir: {script_dir}')
    # print(f'root dir: {root_dir}')
    # print(f'lib dir: {lib_dir}')
    # print(f'python lib dir: {python_lib_dir}')

    # 添加动态库搜索路径
    os.add_dll_directory(lib_dir)
    # 添加python模块搜索路径
    sys.path.insert(0, python_lib_dir)

setup()
