#!/bin/sh
cd `dirname $0`
PWD=`pwd`
cd $PWD/../../

export LD_LIBRARY_PATH=thirdparty/boost/lib64_release:$LD_LIBRARY_PATH
# 当前目录为构建项目根目录，由于 py complier 依赖的 frontend.so 通过 blade 构建后依赖的so文件是带路径的，所以只能在根目录下执行
echo `pwd`
python -mthrift_compiler.main $@
