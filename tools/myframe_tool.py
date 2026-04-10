#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import argparse
import os, os.path
import re
import shutil

def setup():
    print(f'not impl!!!')
    pass

def createProject(path, name):
    # 检查路径
    if os.path.exists(path):
        path = os.path.join(path, name)
        # print(f'{path}')
    else:
        print(f'{path} not exists!!!')
        return

    # 拷贝到指定目录
    proj_src_dir = os.path.split(os.path.realpath(__file__))[0]
    proj_src_dir = os.path.join(proj_src_dir, '..', 'templates')
    # print(f'{proj_src_dir}')
    shutil.copytree(proj_src_dir, path, True)

    # 重命名模板工程文件
    proj_rename_dict = {}
    proj_rename_dict[os.path.join(path, 'template.cpp')] = os.path.join(path, f'{name}.cpp')
    proj_rename_dict[os.path.join(path, 'template.json')] = os.path.join(path, f'{name}.json')
    # print(f'{proj_rename_dict}')
    for k in proj_rename_dict:
        os.rename(k, proj_rename_dict[k])

    # 替换文件中变量
    proj_modify_file_list = []
    proj_modify_file_list.append(os.path.join(path, 'CMakeLists.txt'))
    proj_modify_file_list.append(os.path.join(path, f'{name}.json'))
    proj_modify_file_list.append(os.path.join(path, f'{name}.cpp'))
    for file in proj_modify_file_list:
        file_content = ""
        rf = open(file, 'r')
        while True:
            line = rf.readline()
            if line == '' or line is None:
                break
            replace_content = re.sub("@template_name@", name, line)
            file_content = file_content + replace_content
        # print(file_content)
        rf.close()

        wf = open(file, 'w')
        wf.write(file_content)
        wf.close()

    print(f'Create project {path}')
    print("Success!!!")

def main():
    parser = argparse.ArgumentParser(description='myframe tools')
    subparsers = parser.add_subparsers(dest='command', required=True)

    # setup command
    setup_parser = subparsers.add_parser('setup', help='Setup env')

    # create project command
    create_proj_parser = subparsers.add_parser('create', help='create myframe module project')
    create_proj_parser.add_argument('--name', '-n', help='myframe module name')
    create_proj_parser.add_argument('--path', '-p', help='myframe module gen dir')

    args = parser.parse_args()
    if args.command == 'setup':
        setup()
    elif args.command == 'create':
        createProject(path=args.path, name=args.name)

if __name__ == "__main__":
    main()
