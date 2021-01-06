#!/bin/bash
set -xe

#install unwind
sudo yum install -y libunwind

local_ip=$(ip addr | awk '/^[0-9]+: / {}; /inet.*global/ {print gensub(/(.*)\/(.*)/, "\\1", "g", $2)}' | head -n 1)
echo $local_ip

RUN_DIR=$(cd `dirname $0`; pwd)
cd $RUN_DIR

echo -e "check predictor status on http://$local_ip:10048/server/status"
LD_LIBRARY_PATH=$RUN_DIR/lib/:$LD_LIBRARY_PATH $RUN_DIR/bin/predictor_server --flagfile=$RUN_DIR/conf/config.gflags
