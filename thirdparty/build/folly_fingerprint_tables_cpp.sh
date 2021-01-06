#!/bin/bash

# $1
# build64_release/thirdparty/folly/folly/build/a

# $2
# root dir

OUT_DIR=`dirname $1`
RUN_TOOL_PATH=$OUT_DIR"/../../"

CMD=$RUN_TOOL_PATH"/generate_fingerprint_tables --install_dir=$OUT_DIR"
echo $CMD

eval $CMD
