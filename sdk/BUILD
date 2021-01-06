cc_library(
    name = "sdk_common_init",
    srcs = [
        "common/init.cc",
    ],
    deps = [
        "//service_router:service_router",
        "//service_router:service_router_http_server",
        "//thirdparty/folly:folly",
    ],
)

cc_library(    
    name = "predictor_client_sdk",
    srcs = [
        "predictor/predictor_sdk.cc",
        "predictor/predictor_sdk_thrift_api.cc",
        "predictor/predictor_sdk_universal_api.cc"
    ],
    deps = [
        "//predictor/if:predictor",
        "//feature_master/if:feature_master",
        "//service_router:service_router",
        "//thirdparty/fbthrift:thriftcpp2",
        "//thirdparty/proxygen:proxygenhttpserver",
        "//service_router:service_router_http_server",
        "#unwind",
        "#dl",
        "#keyutils",
        "#resolv",
    ],
)
