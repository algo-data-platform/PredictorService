cc_library(name = "parameter_extractor",
           srcs = [
             "parameter_extractor.cc",
             "parameter_utils.cc"],
           deps = [
             "//thirdparty/folly:folly_base",
             "//thirdparty/fbthrift:thriftcpp2",
             '//thirdparty/hash:hash',
             "//feature_master/if:feature_master",
             "//common:util",
             "#unwind",
             "#dl",
             "#keyutils",
             "#resolv",
           ],
           link_all_symbols = True,
           )

cc_binary(name = "parameter_extractor_test_tool",
        srcs = ["parameter_extractor_test_tool.cc",],
        deps = [
            ":parameter_extractor",
        ],
)

cc_test(name = "parameter_extractor_test",
        srcs = ["parameter_extractor_test.cc",],
        deps = [
            ":parameter_extractor",
            "//thirdparty/googletest:gtest",
        ],
        testdata = [
            'test_data/parameter_test.json',
        ]
)
