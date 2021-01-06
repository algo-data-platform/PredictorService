#!/bin/bash
set -ex

RUN_DIR=$(
  cd $(dirname $BASH_SOURCE)
  pwd
)
PRE_BUILD_LOCK=$RUN_DIR/prebuild.lock
CPU_NUM=$(grep 'processor' /proc/cpuinfo | sort -u | wc -l)
cd $RUN_DIR

# 不检查升级
export BLADE_AUTO_UPGRADE=no

function _setup() {
  sudo yum -y install binutils
}

function exec_prebuild() {
  dep_file="$RUN_DIR/thirdparty/build/install_deps.sh"
  sh $dep_file
  sha256sum $dep_file >$PRE_BUILD_LOCK
}

function prebuild() {
  if [ -f $PRE_BUILD_LOCK ]; then
    {
      # 如果sha256校验失败，则执行prebuild
      sha256sum -c $PRE_BUILD_LOCK
    } || {
      exec_prebuild
    }
  else
    exec_prebuild
  fi
}

function blade_build() {
  $RUN_DIR/blade build -p $1 -j$CPU_NUM $2
}

function build_init() {
  prebuild
  INIT_BUILD_LOCK=$RUN_DIR/build64_$1/initbuild.lock
  if [ ! -f $INIT_BUILD_LOCK ]; then
    blade_build $1 "//thirdparty/proxygen:common_header_h_gen //thirdparty/proxygen:common_header_cpp_gen"
    blade_build $1 //thirdparty/fbthrift:thrift1
    blade_build $1 //thirdparty/fbthrift
    blade_build $1 //thirdparty/fbthrift/thrift/compiler/py
    touch $INIT_BUILD_LOCK
  fi
  # 还原运行目录
  cd $RUN_DIR
}

function build_all() {
  echo "CPU_NUM=$CPU_NUM"
  if [ "$1" = "release" ]; then
    build_init debug
  fi
  build_init $1
  blade_build $1 "predictor"
}

# if the script is sourced by another script, don't run the logic below
if [[ "${#BASH_SOURCE[@]}" -eq 1 ]]; then
  _setup

  case "$1" in
    debug)
      build_all debug
      ;;
    release)
      build_all release
      ;;
    *)
      echo $"Usage: $0 {debug|release}"
      exit 1
      ;;
  esac
fi
