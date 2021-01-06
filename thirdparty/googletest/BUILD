cc_library(
	name='gtest',
	srcs=[
		'googletest/src/gtest.cc',
		'googletest/src/gtest-death-test.cc',
		'googletest/src/gtest-filepath.cc',
		'googletest/src/gtest-port.cc',
		'googletest/src/gtest-printers.cc',
		'googletest/src/gtest-test-part.cc',
		'googletest/src/gtest-typed-test.cc',
		'googlemock/src/gmock-cardinalities.cc',
		'googlemock/src/gmock.cc',
		'googlemock/src/gmock-internal-utils.cc',
		'googlemock/src/gmock-matchers.cc',
		'googlemock/src/gmock-spec-builders.cc'
	],
	incs=[
		'googletest/',
		'googlemock/',
	],
	deps=['#pthread'],
)

cc_library(
	name = 'gtest_main',
	srcs = [
		'googlemock/src/gmock_main.cc',
	],
	deps = [
		':gtest'
	],
	incs=[
		'googletest/',
		'googlemock/',
	],
)

cc_library(
	name='gtest_sample_lib',
	srcs = [
		'googletest/samples/sample1.cc',
		'googletest/samples/sample2.cc',
		'googletest/samples/sample4.cc',
	],
	incs = [
		'googletest/samples'
	]
)

cc_test(
  name = "sample1_unittest",
	srcs = ["googletest/samples/sample1_unittest.cc"],
	deps = [
		":gtest_sample_lib"
	]
)

cc_test(
  name = "sample2_unittest",
	srcs = ["googletest/samples/sample2_unittest.cc"],
	deps = [
		":gtest_sample_lib"
	]
)

cc_test(
  name = "sample3_unittest",
	srcs = ["googletest/samples/sample3_unittest.cc"]
)

cc_test(
  name = "sample4_unittest",
	srcs = ["googletest/samples/sample4_unittest.cc"],
	deps = [
		":gtest_sample_lib"
	]
)

cc_test(
  name = "sample5_unittest",
	srcs = ["googletest/samples/sample5_unittest.cc"],
	deps = [
		":gtest_sample_lib"
	]
)

cc_test(
  name = "sample6_unittest",
	srcs = ["googletest/samples/sample6_unittest.cc"]
)

cc_test(
  name = "sample7_unittest",
	srcs = ["googletest/samples/sample7_unittest.cc"]
)

cc_test(
  name = "sample8_unittest",
	srcs = ["googletest/samples/sample8_unittest.cc"]
)

cc_test(
  name = "sample9_unittest",
	srcs = ["googletest/samples/sample9_unittest.cc"]
)

cc_test(
	name = "sample10_unittest",
	srcs = ["googletest/samples/sample10_unittest.cc"],
)
