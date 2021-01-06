gen_rule(
	name = "define_h",
	srcs = [
		'src/defines.h.in'
	],
	outs = [
		'../../gflags/defines.h'
	],
	cmd = "awk '{ \
		   	gsub(/^#cmakedefine/, \\\"//cmakedefine\\\"); \
			print; }' $FIRST_SRC > $FIRST_OUT"
)

gen_rule(
	name = "gflags_declare_h",
	srcs = [
		'src/gflags_declare.h.in'
	],
	outs = [
		'../../gflags/gflags_declare.h'
	],
	cmd = "awk '{ \
		   	gsub(/@GFLAGS_NAMESPACE@/, \\\"google\\\"); \
			gsub(/@(HAVE_STDINT_H|HAVE_SYS_TYPES_H|HAVE_INTTYPES_H|GFLAGS_INTTYPES_FORMAT_C99)@/, \\\"1\\\"); \
			gsub(/@([A-Z0-9_]+)@/, \\\"0\\\"); \
			print; }' $FIRST_SRC > $FIRST_OUT"
)
gen_rule(
	name = "gflags_gflags_h",
	srcs = [
		'src/gflags_ns.h.in'
	],
	outs = [
		'../../gflags/gflags_gflags.h'
	],
	cmd = "awk '{ \
			gsub(/@ns@/, \\\"gflags\\\"); \
			gsub(/@NS@/, \\\"GFLAGS\\\"); \
			print; }' $FIRST_SRC > $FIRST_OUT"
)
gen_rule(
	name = "gflags_h",
	srcs = [
		'src/gflags.h.in'
	],
	outs = [
		'../../gflags/gflags.h'
	],
	cmd = "awk '{ \
			gsub(/@GFLAGS_ATTRIBUTE_UNUSED@/, \\\"\\\"); \
			gsub(/@INCLUDE_GFLAGS_NS_H@/, \\\"#include \\\\\\\"gflags/gflags_gflags.h\\\\\\\"\\\"); \
			print; }' $FIRST_SRC > $FIRST_OUT"
)
gen_rule(
	name = "gflags_completions_h",
	srcs = [
		'src/gflags_completions.h.in'
	],
	outs = [
		'../../gflags/gflags_completions.h'
	],
	cmd = "awk '{ \
			gsub(/@GFLAGS_NAMESPACE@/, \\\"google\\\"); \
			print; }' $FIRST_SRC > $FIRST_OUT"
)

cc_library(
    name = "gflags",
	srcs = [
        "src/gflags.cc",
        "src/gflags_completions.cc",
        "src/gflags_reporting.cc",
	],
	deps = [
    ':define_h',
		':gflags_declare_h',
		':gflags_gflags_h',
		':gflags_h',
		':gflags_completions_h',
		"#pthread"
	],
	defs = [
		"GFLAGS_BAZEL_BUILD",
		"GFLAGS_INTTYPES_FORMAT_C99",
		"GFLAGS_IS_A_DLL=0",
		# macros otherwise defined by CMake configured defines.h file
		"HAVE_STDINT_H",
		"HAVE_SYS_TYPES_H",
		"HAVE_INTTYPES_H",
		"HAVE_SYS_STAT_H",
		"HAVE_STRTOLL",
		"HAVE_STRTOQ",
		"HAVE_RWLOCK",
		"HAVE_UNISTD_H",
		"HAVE_FNMATCH_H",
		"HAVE_PTHREAD",
	],
	warning='no'
)
