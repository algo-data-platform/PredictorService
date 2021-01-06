#!/bin/bash
set -xe

function get_last_commit_id() {
  LAST_COMMIT_ORIGIN=`git rev-parse --short origin/master`
  if [ "" != ""$1 ]; then
    LAST_COMMIT_ORIGIN=$1
  fi
  echo $LAST_COMMIT_ORIGIN
}

MODE=$1

if [ $# -lt 1 ]; then
  # MODE 编译模式，TAG 自定义版本标记，不加将获取last origin commit_id
  echo -e "Usage: sh build-predictor-sdk.sh {debug|release} {tag}"
  exit 1
fi

TAG_NAME=`get_last_commit_id $2`

if [ "" == ""$1 ]; then
  MODE=debug
fi

RUN_DIR=$(cd `dirname $0`;cd ../; pwd)

mkdir -p $RUN_DIR/sdk/sdk_package
BUILD_DIR=$RUN_DIR/sdk/sdk_package/$TAG_NAME

if [ ! -d $BUILD_DIR ]; then
  mkdir -p $BUILD_DIR
  mkdir -p $BUILD_DIR/include
  mkdir -p $BUILD_DIR/lib
  mkdir -p $BUILD_DIR/example
fi

# build common lib
sh $RUN_DIR/thirdparty/build/package_lib_sdk_common.sh $MODE
# copy lib
cp $RUN_DIR/blade-bin/lib_sdk_common/lib_sdk_common.a $BUILD_DIR/lib/lib_sdk_common.a
# copy header
cp $RUN_DIR/sdk/common/init.h $BUILD_DIR/include/init.h

# build predictor sdk lib
sh $RUN_DIR/thirdparty/build/package_lib_sdk_predictor.sh $MODE
# copy lib
cp $RUN_DIR/blade-bin/lib_sdk_predictor/lib_sdk_predictor.a $BUILD_DIR/lib/lib_sdk_predictor.a
# copy header
cp $RUN_DIR/sdk/predictor/predictor_client_sdk.h $BUILD_DIR/include/predictor_client_sdk.h

# copy a specific example
cp -r $RUN_DIR/sdk/example/* $BUILD_DIR/example/
echo "tag_id:$TAG_NAME" > $BUILD_DIR/version.txt

rm -rf $RUN_DIR/sdk/sdk_package/latest
if [ ! -f "$RUN_DIR/sdk/sdk_package/latest" ]; then
  ln -s $BUILD_DIR $RUN_DIR/sdk/sdk_package/latest
fi
