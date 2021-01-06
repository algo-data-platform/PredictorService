#!/bin/bash
# $1 debug|relase

NAME=sdk_common

STATIC_LIB_NAME=lib_$NAME.a
RUN_DIR=$(cd `dirname $0`; cd ../../; pwd)
STATIC_LIB_PREFIX="$RUN_DIR/build64_$1"
OPENSSL_PREBUILD_PREFIX="$RUN_DIR/thirdparty/openssl/lib64_$1"
BOOST_PREBUILD_PREFIX="$RUN_DIR/thirdparty/boost/lib64_$1"
BUILD_DIR=$STATIC_LIB_PREFIX/lib_$NAME

# 初始化构建目录
if [ -d $BUILD_DIR ]; then
  rm -rf $BUILD_DIR
fi
mkdir $BUILD_DIR
cd $BUILD_DIR

STATIC_LIB=(
$STATIC_LIB_PREFIX/thirdparty/fbthrift/thrift/lib/cpp2/transport/rsocket/libConfig.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/thrift/lib/thrift/libRpcMetadata.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libasync.a
$BOOST_PREBUILD_PREFIX/libboost_context.a
$BOOST_PREBUILD_PREFIX/libboost_chrono.a
$BOOST_PREBUILD_PREFIX/libboost_date_time.a
$BOOST_PREBUILD_PREFIX/libboost_program_options.a
$BOOST_PREBUILD_PREFIX/libboost_system.a
$BOOST_PREBUILD_PREFIX/libboost_thread.a
$BOOST_PREBUILD_PREFIX/libboost_regex.a
$BOOST_PREBUILD_PREFIX/libboost_filesystem.a
$STATIC_LIB_PREFIX/thirdparty/bzip2/libbz2.a
$STATIC_LIB_PREFIX/thirdparty/double-conversion/libdouble-conversion.a
$STATIC_LIB_PREFIX/thirdparty/libevent/libevent_core.a
$STATIC_LIB_PREFIX/thirdparty/folly/libfolly.a
$STATIC_LIB_PREFIX/thirdparty/folly/libfolly_base.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/thrift/lib/thrift/libfrozen.a
$STATIC_LIB_PREFIX/thirdparty/gflags/libgflags.a
$STATIC_LIB_PREFIX/thirdparty/glog/libglog.a
$STATIC_LIB_PREFIX/thirdparty/hash/libhash.a
$STATIC_LIB_PREFIX/thirdparty/snappy/libsnappy.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libthrift.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libthrift-core.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libthriftcpp2.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libthriftfrozen2.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libthriftprotocol.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libtransport.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libconcurrency.a
$STATIC_LIB_PREFIX/thirdparty/wangle/libwangle.a
$STATIC_LIB_PREFIX/thirdparty/zstd/libzstd.a
$STATIC_LIB_PREFIX/thirdparty/zlib/libz.a
$STATIC_LIB_PREFIX/thirdparty/rsocket-cpp/libyarpl.a
$STATIC_LIB_PREFIX/thirdparty/lz4/liblz4.a
$STATIC_LIB_PREFIX/thirdparty/fbthrift/libprotocol.a
$STATIC_LIB_PREFIX/thirdparty/proxygen/libproxygenhttp.a
$STATIC_LIB_PREFIX/thirdparty/proxygen/libproxygenhttpserver.a
$STATIC_LIB_PREFIX/thirdparty/proxygen/libproxygenservices.a
$STATIC_LIB_PREFIX/thirdparty/proxygen/libproxygenutils.a
$STATIC_LIB_PREFIX/thirdparty/rsocket-cpp/librsocket.a
$OPENSSL_PREBUILD_PREFIX/libssl.a
$OPENSSL_PREBUILD_PREFIX/libcrypto.a
# common
$STATIC_LIB_PREFIX/common/libmetric.a
$STATIC_LIB_PREFIX/common/libutil.a
# service_framework
$STATIC_LIB_PREFIX/service_framework/libhttp_client.a
$STATIC_LIB_PREFIX/service_framework/libhttp_server.a
$STATIC_LIB_PREFIX/service_framework/libconsul.a
# service_router
$STATIC_LIB_PREFIX/service_router/libservice_router.a
$STATIC_LIB_PREFIX/service_router/libservice_router_http_server.a
# sdk common
$STATIC_LIB_PREFIX/sdk/libsdk_common_init.a
)

$RUN_DIR/blade build //service_router:service_router -p $1
$RUN_DIR/blade build //service_router:service_router_http_server -p $1
$RUN_DIR/blade build //sdk:sdk_common_init -p $1  

# 由于 ar x 解压后的 .o 可能有重名的，就会被覆盖，导致重新打包后的 .a 符号不全，所以需要指定对应的 .o 进行重新打包
DUP_NAME_OBJS=(
  thirdparty/hash/hash.objs/smhasher/src/Random.cpp.o 
  thirdparty/folly/folly_base.objs/folly/Random.cpp.o
  thirdparty/folly/folly_base.objs/folly/detail/IPAddress.cpp.o 
  thirdparty/folly/folly_base.objs/folly/IPAddress.cpp.o
  thirdparty/folly/folly_base.objs/folly/logging/Init.cpp.o 
  thirdparty/folly/folly_base.objs/folly/init/Init.cpp.o 
  thirdparty/folly/folly_base.objs/folly/ssl/Init.cpp.o
  thirdparty/proxygen/proxygenhttp.objs/proxygen/lib/http/codec/ErrorCode.cpp.o 
  thirdparty/rsocket-cpp/rsocket.objs/rsocket/framing/ErrorCode.cpp.o
  thirdparty/folly/folly_base.objs/folly/String.cpp.o
  thirdparty/folly/folly_base.objs/folly/portability/String.cpp.o
  thirdparty/proxygen/proxygenhttpserver.objs/proxygen/httpserver/SignalHandler.cpp.o
  thirdparty/folly/folly_base.objs/folly/io/async/AsyncSignalHandler.cpp.o
  thirdparty/folly/folly_base.objs/folly/experimental/symbolizer/SignalHandler.cpp.o
  thirdparty/rsocket-cpp/yarpl.objs/yarpl/observable/Subscription.cpp.o
  thirdparty/rsocket-cpp/yarpl.objs/yarpl/flowable/Subscription.cpp.o
  thirdparty/fbthrift/concurrency.objs/thrift/lib/cpp/concurrency/Util.cpp.o
  thirdparty/fbthrift/thriftcpp2.objs/thrift/perf/cpp2/util/Util.cpp.o
  thirdparty/bzip2/bz2.objs/compress.c.o
  thirdparty/zlib/z.objs/compress.c.o
)

for lib in ${STATIC_LIB[@]}
do
  echo $lib
  ar x $lib
done

OBJS="*.o "
for obj in ${DUP_NAME_OBJS[@]}
do
  OBJS=$OBJS" $STATIC_LIB_PREFIX/$obj "
done
echo $OBJS
ar rcs $STATIC_LIB_NAME $OBJS
ranlib $STATIC_LIB_NAME
rm -rf *.o
