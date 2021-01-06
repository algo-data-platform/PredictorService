#!/bin/bash

# prebuild root path
THIRDPARTY_DIR=$(cd `dirname $0`; cd ../ ; pwd)
BUILD_TMP="/tmp/"`whoami`"/ad-core"
if [ $ADCORE_CUSTOM_BUILD_DIR"" != "" ];then
    BUILD_TMP=$ADCORE_CUSTOM_BUILD_DIR
fi
echo "BUILD_TMP is " $BUILD_TMP
PWD=`pwd`
OPENSSL_INCLUDE=$THIRDPARTY_DIR/openssl/include/
CPU_NUM=`grep 'processor' /proc/cpuinfo | sort -u | wc -l`
function check_return_exit ()
{
	if ! test 0 = $?; then
		echo -e "\n!!!!!!!!!! Error, this script will exit !!!!!!!!!!\n"
		exit 1
	fi
}

function ad_mkdir()
{
	echo "create $1 directory ...";
	if [ -d $1 ] ;then
		ls $1
		echo "directory $1 already exists. "
	else
		mkdir -p $1
		check_return_exit
	fi
}

ad_mkdir $BUILD_TMP
cd $BUILD_TMP

function install_boost()
{
	cd $BUILD_TMP
	SRCNAME="boost_1_67_0"
	if [ ! -f $SRCNAME".tar.gz" ]; then
    wget https://dl.bintray.com/boostorg/release/1.67.0/source/${SRCNAME}.tar.gz
  fi

	if [ -d $SRCNAME ]; then
		rm -rf "./"$SRCNAME
	fi

	tar -zxf $SRCNAME".tar.gz"
	check_return_exit

	cd $SRCNAME
	CMD="./bootstrap.sh \
		--with-libraries=boost_context,boost_chrono,boost_date_time,boost_program_options,boost_filesystem,boost_regex,boost_system,boost_thread,boost_python,boost_random"
	echo $CMD
	eval $CMD
	check_return_exit
	CMD="./b2  \
			--with-context \
			--with-chrono \
			--with-date_time \
			--with-program_options \
			--with-filesystem \
			--with-regex \
			--with-system \
			--with-thread \
			--with-python \
			--with-random \
        define=_GLIBCXX_USE_CXX11_ABI=0 \
      -j$CPU_NUM"
	echo $CMD
	eval $CMD
	check_return_exit

	ad_mkdir $THIRDPARTY_DIR"/boost/lib64_release/"
	cp ./stage/lib/* $THIRDPARTY_DIR/boost/lib64_release/
	check_return_exit
	cp -r ./boost $THIRDPARTY_DIR/boost/
	check_return_exit
  if [ ! -f $THIRDPARTY_DIR/boost/lib64_debug ]; then
    ln -s $THIRDPARTY_DIR/boost/lib64_release/ $THIRDPARTY_DIR/boost/lib64_debug
  fi
  check_return_exit
}

function clean_boost()
{
	if [ -d $THIRDPARTY_DIR/boost/lib64_release/ ]; then
		rm -rf $THIRDPARTY_DIR/boost/lib64_release/
	fi
	if [ -d $THIRDPARTY_DIR/boost/boost ]; then
		rm -rf $THIRDPARTY_DIR/boost/boost/
	fi
  rm -rf $THIRDPARTY_DIR/boost/lib64_debug
}

function install_openssl()
{
	cd $BUILD_TMP
  SRCNAME="OpenSSL_1_1_1c"
  if [ ! -f $SRCNAME".tar.gz" ]; then
    wget https://github.com/openssl/openssl/archive/$SRCNAME".tar.gz"
  fi

	if [ -d $SRCNAME ]; then
		rm -rf "./"$SRCNAME
	fi

	tar -zxf $SRCNAME".tar.gz"
	check_return_exit

	cd "openssl-"$SRCNAME
	CMD="./config  -fPIC no-shared"
	echo $CMD
	eval $CMD
	check_return_exit
	CMD="make -j $CPU_NUM"
	echo $CMD
	eval $CMD
	check_return_exit

	ad_mkdir $THIRDPARTY_DIR"/openssl/lib64_release/"
	cp ./libssl.a $THIRDPARTY_DIR/openssl/lib64_release/
	check_return_exit
	cp ./libcrypto.a $THIRDPARTY_DIR/openssl/lib64_release/
	check_return_exit
	cp -r ./include $THIRDPARTY_DIR/openssl/
	check_return_exit
	cp -r ./crypto $THIRDPARTY_DIR/openssl/
	check_return_exit
	cp -r ./ssl $THIRDPARTY_DIR/openssl/
	check_return_exit
	cp -r ./*.h $THIRDPARTY_DIR/openssl/
	check_return_exit
  if [ ! -f "$THIRDPARTY_DIR/openssl/lib64_debug" ]; then
    ln -s $THIRDPARTY_DIR/openssl/lib64_release/ $THIRDPARTY_DIR/openssl/lib64_debug
  fi
  check_return_exit
}

function clean_openssl()
{
   	if [ -d $THIRDPARTY_DIR/openssl/lib64_release/ ]; then
	  rm -rf $THIRDPARTY_DIR/openssl/lib64_release/
	  fi
   	if [ -d $THIRDPARTY_DIR/openssl/include ]; then
		  rm -rf $THIRDPARTY_DIR/openssl/include/
   	fi
   	if [ -d $THIRDPARTY_DIR/openssl/crypto ]; then
		  rm -rf $THIRDPARTY_DIR/openssl/crypto/
   	fi
   	if [ -d $THIRDPARTY_DIR/openssl/ssl ]; then
		  rm -rf $THIRDPARTY_DIR/openssl/ssl/
   	fi
   	rm -rf $THIRDPARTY_DIR/openssl/*.h
    rm -rf $THIRDPARTY_DIR/openssl/lib64_debug
}

function install_tf(){
    TF_RELEASE_DIR=$THIRDPARTY_DIR"/tensorflow/lib64_release/"
    TF_DEBUG_DIR=$THIRDPARTY_DIR"/tensorflow/lib64_debug"
		cd $TF_RELEASE_DIR
    tar -zxf libtensorflow_cc.tar.gz
		if [ ! -f "$TF_DEBUG_DIR" ]; then
        ln -s $TF_RELEASE_DIR $TF_DEBUG_DIR
    fi
}

function clean_tf(){
	TF_DEBUG_DIR=$THIRDPARTY_DIR"/tensorflow/lib64_debug"
	rm -rf $TF_DEBUG_DIR
}


function install_krb5()
{
	cd $BUILD_TMP
	SRCNAME="krb5-1.16.1-final"
	if [ ! -f $SRCNAME".tar.gz" ]; then
		wget https://github.com/krb5/krb5/archive/$SRCNAME".tar.gz"
	fi

	if [ -d $SRCNAME ]; then
		rm -rf "./"$SRCNAME
	fi

	tar -zxf $SRCNAME".tar.gz"
	check_return_exit

	cd "krb5-"$SRCNAME/src
	CMD="autoreconf -ivf"
	echo $CMD
	eval $CMD
	check_return_exit
  CMD="CPPFLAGS='-I$OPENSSL_INCLUDE -fPIC' ./configure --enable-static --disable-shared"
	echo $CMD
	eval $CMD
	check_return_exit
	make -j $CPU_NUM
	check_return_exit

	ad_mkdir $THIRDPARTY_DIR"/krb5/lib64_release/"
	cp ./lib/lib*.a $THIRDPARTY_DIR/krb5/lib64_release/
	check_return_exit
	cp -r ./include $THIRDPARTY_DIR/krb5/
	check_return_exit
  if [ ! -f "$THIRDPARTY_DIR/krb5/lib64_debug" ]; then
    ln -s $THIRDPARTY_DIR/krb5/lib64_release/ $THIRDPARTY_DIR/krb5/lib64_debug
  fi
  check_return_exit
}

function clean_krb5()
{
  if [ -d $THIRDPARTY_DIR/krb5/lib64_release/ ]; then
		rm -rf $THIRDPARTY_DIR/krb5/lib64_release/
	fi
	if [ -d $THIRDPARTY_DIR/krb5/include ]; then
		rm -rf $THIRDPARTY_DIR/krb5/include/
	fi
  rm -rf $THIRDPARTY_DIR/krb5/lib64_debug
}

function install_catboost(){
		CATBOOST_RELEASE_DIR=$THIRDPARTY_DIR"/catboost/lib64_release/"
		CATBOOST_DEBUG_DIR=$THIRDPARTY_DIR"/catboost/lib64_debug"
		if [ ! -f "$CATBOOST_DEBUG_DIR" ]; then
			ln -s $CATBOOST_RELEASE_DIR $CATBOOST_DEBUG_DIR
		fi
}

function clean_catboost(){
		CATBOOST_DEBUG_DIR=$THIRDPARTY_DIR"/catboost/lib64_debug"
		if [ -d $CATBOOST_DEBUG_DIR ]; then
			rm -rf $CATBOOST_DEBUG_DIR
		fi
}

function clean_jemalloc()
{   
	if [ -d $THIRDPARTY_DIR/jemalloc/lib64_release/ ]; then
	   rm -rf $THIRDPARTY_DIR/jemalloc/lib64_release/
  fi
 
	rm -rf $THIRDPARTY_DIR/jemalloc/lib64_debug
}

function install_jemalloc()
{
	cd $BUILD_TMP
	SRCNAME="5.2.1"

  if [ ! -f $SRCNAME".tar.gz" ]; then
		wget https://github.com/jemalloc/jemalloc/archive/$SRCNAME".tar.gz"
	fi

	if [ -d $SRCNAME ]; then
		rm -rf "./"$SRCNAME
	fi

  tar -zxf $SRCNAME".tar.gz"
	check_return_exit

	cd "jemalloc-"$SRCNAME
	check_return_exit
  sh autogen.sh
	check_return_exit
  make -j 16
	check_return_exit

	ad_mkdir $THIRDPARTY_DIR"/jemalloc/lib64_release/"
	cp ./lib/lib*.so* $THIRDPARTY_DIR/jemalloc/lib64_release/
  cp -r ./include $THIRDPARTY_DIR/jemalloc/include
	check_return_exit

	if [ ! -f "$THIRDPARTY_DIR/jemalloc/lib64_debug" ]; then
	  ln -s $THIRDPARTY_DIR/jemalloc/lib64_release/ $THIRDPARTY_DIR/jemalloc/lib64_debug
	fi
	check_return_exit
}

clean_boost
install_boost
clean_openssl
install_openssl
clean_tf
install_tf
clean_krb5
install_krb5
clean_catboost
install_catboost
clean_jemalloc
install_jemalloc

rm -rf $BUILD_TMP
