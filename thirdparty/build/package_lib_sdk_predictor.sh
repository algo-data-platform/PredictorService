#!/bin/bash
# $1 debug|relase

NAME=sdk_predictor

STATIC_LIB_NAME=lib_$NAME.a
RUN_DIR=$(cd `dirname $0`;cd ../../; pwd)
STATIC_LIB_PREFIX="$RUN_DIR/build64_$1"
BUILD_DIR=$STATIC_LIB_PREFIX/lib_$NAME
# 初始化构建目录
if [ -d $BUILD_DIR ]; then
  rm -rf $BUILD_DIR
fi
mkdir $BUILD_DIR
cd $BUILD_DIR

STATIC_LIB=(
# feature_master
$STATIC_LIB_PREFIX/feature_master/if/libfeature_master.a
# predictor
$STATIC_LIB_PREFIX/predictor/if/libpredictor.a
$STATIC_LIB_PREFIX/sdk/libpredictor_client_sdk.a
)

# build
$RUN_DIR/blade build //sdk:predictor_client_sdk -p $1

for lib in ${STATIC_LIB[@]}
do
  echo $lib
  ar x $lib
done

ar cr $STATIC_LIB_NAME *.o
ranlib $STATIC_LIB_NAME
rm -rf *.o
