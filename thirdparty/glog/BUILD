gen_rule(
	name = "config_h",
	srcs = [
		'src/config.h.cmake.in'
	],
	outs = [
		'../../glog/config.h'
	],
	cmd = "awk '{ \
		   	gsub(/^#cmakedefine/, \\\"//cmakedefine\\\"); \
			print; }' $FIRST_SRC > $FIRST_OUT"
)
gen_rule(
	name = "vlog_is_on",
	srcs = [
		'src/glog/vlog_is_on.h.in'
	],
	outs = [
		'../../glog/vlog_is_on.h'
	],
	cmd = "sed -e 's/@ac_cv_cxx_using_operator@/1/g' \
    -e 's/@ac_cv_have_unistd_h@/1/g' \
    -e 's/@ac_cv_have_stdint_h@/1/g' \
    -e 's/@ac_cv_have_systypes_h@/1/g' \
    -e 's/@ac_cv_have_libgflags_h@/1/g' \
    -e 's/@ac_cv_have_uint16_t@/1/g' \
    -e 's/@ac_cv_have___builtin_expect@/1/g' \
    -e 's/@ac_cv_have_.*@/0/g' \
    -e 's/@ac_google_start_namespace@/namespace google {/g' \
    -e 's/@ac_google_end_namespace@/}/g' \
    -e 's/@ac_google_namespace@/google/g' \
    -e 's/@ac_cv___attribute___noinline@/__attribute__((noinline))/g' \
    -e 's/@ac_cv___attribute___noreturn@/__attribute__((noreturn))/g' \
    -e 's/@ac_cv___attribute___printf_4_5@/__attribute__((__format__ (__printf__, 4, 5)))/g' < $FIRST_SRC > $FIRST_OUT"
)
gen_rule(
	name = "stl_logging",
	srcs = [
		'src/glog/stl_logging.h.in'
	],
	outs = [
		'../../glog/stl_logging.h'
	],
	cmd = "sed -e 's/@ac_cv_cxx_using_operator@/1/g' \
    -e 's/@ac_cv_have_unistd_h@/1/g' \
    -e 's/@ac_cv_have_stdint_h@/1/g' \
    -e 's/@ac_cv_have_systypes_h@/1/g' \
    -e 's/@ac_cv_have_libgflags_h@/1/g' \
    -e 's/@ac_cv_have_uint16_t@/1/g' \
    -e 's/@ac_cv_have___builtin_expect@/1/g' \
    -e 's/@ac_cv_have_.*@/0/g' \
    -e 's/@ac_google_start_namespace@/namespace google {/g' \
    -e 's/@ac_google_end_namespace@/}/g' \
    -e 's/@ac_google_namespace@/google/g' \
    -e 's/@ac_cv___attribute___noinline@/__attribute__((noinline))/g' \
    -e 's/@ac_cv___attribute___noreturn@/__attribute__((noreturn))/g' \
    -e 's/@ac_cv___attribute___printf_4_5@/__attribute__((__format__ (__printf__, 4, 5)))/g' < $FIRST_SRC > $FIRST_OUT"
)
gen_rule(
	name = "logging_h",
	srcs = [
		'src/glog/logging.h.in'
	],
	outs = [
		'../../glog/logging.h'
	],
	cmd = "sed -e 's/@ac_cv_cxx_using_operator@/1/g' \
    -e 's/@ac_cv_have_unistd_h@/1/g' \
    -e 's/@ac_cv_have_stdint_h@/1/g' \
    -e 's/@ac_cv_have_systypes_h@/1/g' \
    -e 's/@ac_cv_have_libgflags_h@/1/g' \
    -e 's/@ac_cv_have_uint16_t@/1/g' \
    -e 's/@ac_cv_have___builtin_expect@/1/g' \
    -e 's/@ac_cv_have_.*@/0/g' \
    -e 's/@ac_google_start_namespace@/namespace google {/g' \
    -e 's/@ac_google_end_namespace@/}/g' \
    -e 's/@ac_google_namespace@/google/g' \
    -e 's/@ac_cv___attribute___noinline@/__attribute__((noinline))/g' \
    -e 's/@ac_cv___attribute___noreturn@/__attribute__((noreturn))/g' \
    -e 's/@ac_cv___attribute___printf_4_5@/__attribute__((__format__ (__printf__, 4, 5)))/g' < $FIRST_SRC > $FIRST_OUT"
)

gen_rule(
	name = "raw_logging",
	srcs = [
		'src/glog/raw_logging.h.in'
	],
	outs = [
		'../../glog/raw_logging.h'
	],
	cmd = "sed -e 's/@ac_cv_cxx_using_operator@/1/g' \
    -e 's/@ac_cv_have_unistd_h@/1/g' \
    -e 's/@ac_cv_have_stdint_h@/1/g' \
    -e 's/@ac_cv_have_systypes_h@/1/g' \
    -e 's/@ac_cv_have_libgflags_h@/1/g' \
    -e 's/@ac_cv_have_uint16_t@/1/g' \
    -e 's/@ac_cv_have___builtin_expect@/1/g' \
    -e 's/@ac_cv_have_.*@/0/g' \
    -e 's/@ac_google_start_namespace@/namespace google {/g' \
    -e 's/@ac_google_end_namespace@/}/g' \
    -e 's/@ac_google_namespace@/google/g' \
    -e 's/@ac_cv___attribute___noinline@/__attribute__((noinline))/g' \
    -e 's/@ac_cv___attribute___noreturn@/__attribute__((noreturn))/g' \
    -e 's/@ac_cv___attribute___printf_4_5@/__attribute__((__format__ (__printf__, 4, 5)))/g' < $FIRST_SRC > $FIRST_OUT"
)

cc_library(
	name="glog",
	srcs = [
		'src/demangle.cc',
		'src/logging.cc',
		'src/raw_logging.cc',
		'src/signalhandler.cc',
		'src/symbolize.cc',
		'src/utilities.cc',
		'src/vlog_is_on.cc',
	],
	deps = [
		':config_h',
		':logging_h',
		':vlog_is_on',
		':raw_logging',
		':stl_logging',
		'//thirdparty/gflags:gflags'
	],
	warning='no',
	defs = [
    "GLOG_BAZEL_BUILD",
    # Inject a C++ namespace.
    "GOOGLE_NAMESPACE=google", 
    # Allows src/base/mutex.h to include pthread.h.
    'HAVE_PTHREAD',
    # Allows src/logging.cc to determine the host name.
    'HAVE_SYS_UTSNAME_H',
    # For src/utilities.cc.
    'HAVE_SYS_SYSCALL_H',
    'HAVE_SYS_TIME_H',
    'HAVE_STDINT_H',
    'HAVE_STRING_H',
    # Enable dumping stacktrace upon sigaction.
    'HAVE_SIGACTION',
    # For logging.cc.
    'HAVE_PREAD',
    'HAVE___ATTRIBUTE__',
		'HAVE_LIB_GFLAGS',
	]
)
