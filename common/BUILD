gen_rule(
  name='version_generator',
  cmd='m4 --define=HEAD_COMMIT=`git rev-parse --short HEAD` ./common/version.h.m4 > $BUILD_DIR/common/version.h',
  outs=[
    'version.h',
  ]
)

cc_library(
  name='util',
  srcs=[
    'util.cc',
    'console/table.cc',
  ],
  deps=[
      '//thirdparty/folly:folly',
  ]
)

cc_test(
  name = "util_test",
  srcs = ["util_test.cc",],
  deps = [
      ':util',
      '//thirdparty/folly:folly',
      '//thirdparty/googletest:gtest',
  ],
)

cc_library(
  name='metric',
  srcs=[
    'metrics/metrics.cc',
    'metrics/transform.cc',
    'metrics/system_metrics.cc'
  ],
  deps=[
    ':util',
    '//thirdparty/folly:folly',
    '//thirdparty/boost:boost_regex',
    '//thirdparty/hash:hash',
    '//thirdparty/boost:boost_filesystem',
    ':version_generator'
  ]
)

cc_binary(
  name='hash_test',
  srcs=[
    'hash_test.cc',
  ],
  deps=[
      '//thirdparty/folly:folly',
      '//thirdparty/hash:hash',
      '//thirdparty/googletest:gtest',
  ]
)

cc_library(
  name='fileutil',
  srcs=[
    'file/file_util.cc',
  ],
  deps=[
      '//thirdparty/folly:folly',
      '//thirdparty/boost:boost_filesystem',
  ]
)

cc_test(
  name = "file_util_test",
  srcs = ["file/file_util_test.cc",],
  deps = [
    ':fileutil',
    '//thirdparty/googletest:gtest',
    '#unwind',
    '#dl',
    '#keyutils',
    '#resolv',
  ],
)

cc_binary(
  name='serialize_test',
  srcs=[
    'serialize_test.cc',
  ],
  incs=[
  ],
  deps=[
    '//feature_master/if:feature_master',
    '//feature_master/parameter:parameter_extractor',
    '//thirdparty/fbthrift:thriftcpp2',
    '#unwind',
    '#dl',
    '#keyutils',
    '#resolv',
  ],
)

cc_test(
  name = "serialize_unit_test",
  srcs = ["serialize_unit_test.cc",],
  deps = [
    '//feature_master/if:feature_master',
    '//feature_master/parameter:parameter_extractor',
    '//thirdparty/fbthrift:thriftcpp2',
    '#unwind',
    '#dl',
    '#keyutils',
    '#resolv',
  ],
)

cc_binary(
  name='factory_registry_test',
  srcs=[
    'factory_registry_test.cc',
  ],
  deps = [
        "//thirdparty/folly:folly",
    "#unwind",
    "#dl",
    "#keyutils",
    "#resolv",
        ],
)
