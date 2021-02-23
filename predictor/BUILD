cc_binary(
    name='predictor_server',
    srcs=[
        'server/predictor_service/predictor_service.cc',
        'server/predictor_server.cc',
        'server/model_manager/model_manager.cc',
        'server/model_manager/framework/model.cc',
        'server/model_manager/framework/model_framework.cc',
        'server/model_manager/framework/tf/tf_framework.cc',
        'server/model_manager/framework/tf/tf_model_estimator_native.cc',
        'server/model_manager/framework/xgboost/xgboost_framework.cc',
        'server/model_manager/framework/xgboost/xgboost_model_tree.cc',
        'server/model_manager/framework/catboost/catboost_framework.cc',
        'server/model_manager/framework/catboost/catboost_model.cc',
        'server/router/predictor_router.cc',
        'server/http_service/http_service.cc',
    ],
    incs=[
        './server',
        '../thirdparty/protobuf/src',
        '../thirdparty/tensorflow/include',
    ],
    deps=[
        ':predictor_util',
        ':global_resource',
        ':feature_extractor',
        ':config',
        '//predictor/if:predictor',
        '//feature_master/if:feature_master',
        '//service_router:service_router',
        '//thirdparty/proxygen:proxygenhttpserver',
        '//common:fileutil',
        '//common:metric',
        '//service_router:service_router_http_server',
        '//common:util',
        '//thirdparty/fbthrift:thriftcpp2',
        '//thirdparty/tensorflow:tensorflow_cc',
        '//thirdparty/tensorflow:tensorflow_framework',
        '//sdk:predictor_client_sdk',
        '//predictor/server/model_manager/framework/fea_extract/xgboost_example:xgboost_example_extract',
        ':xgboost_predictor',
        '//thirdparty/xgboost:xgboost',
        '//thirdparty/catboost:catboostmodel',
        '//thirdparty/jemalloc:jemalloc',
        '#unwind',
        '#dl',
        '#keyutils',
        '#resolv',
    ],
)


cc_library(
    name="xgboost_predictor",
    extra_linkflags = [ '-fopenmp' ],
    srcs=[
        'server/model_manager/framework/xgboost/xgboost_predictor.cc',
    ],
    incs = [
        '../thirdparty/xgboost/include',
        '../thirdparty/xgboost/src/data',
        '../thirdparty/xgboost/src/gbm',
        '../thirdparty/xgboost/src/predictor',
        '../thirdparty/xgboost/src/common',
        '../thirdparty/xgboost/dmlc-core/include',
        '../thirdparty/xgboost/rabit/include',
    ],
    deps = [
        "//thirdparty/xgboost:xgboost",
    ],
)

cc_library(
    name='global_resource',
    srcs=[
        'global_resource/resource_manager.cc',
        'global_resource/kafka_producer.cc',
    ],
    deps=[
        ':predictor_util',
        '//common/kafka:libkafka',
        '#unwind',
        '#dl',
        '#keyutils',
        '#resolv',
    ],
)

cc_library(
    name='predictor_util',
    srcs=[
        'util/predictor_util.cc',
        'util/predictor_data.cc',
    ],
    incs=[
        './server'
    ],
    deps=[
        '//predictor/if:predictor',
        '//feature_master/if:feature_master',
        '//service_router:service_router',
        '#unwind',
        '#dl',
        '#keyutils',
        '#resolv',
    ],
)

cc_library(
    name='feature_extractor',
    srcs=[
        'server/feature_extractor/fea_extract_snapshot.cc',
        'server/feature_extractor/feature_extractor.cc',
        'server/feature_extractor/basic_extractor.cc',
    ],
    incs=[
        '../thirdparty/protobuf/src',
        '../thirdparty/tensorflow/include',
    ],
    deps=[
        ':global_resource',
        ':predictor_util',
        '#unwind',
        '#dl',
        '#keyutils',
        '#resolv',
    ],
    link_all_symbols = True,
)

cc_library(
    name = 'config',
    srcs = [
        'config/config_util.cc',
    ],
    deps = [
        '//common:fileutil',
        '//predictor/if:config',
        '#unwind',
    ]
)

cc_test(
    name = 'config_test',
    srcs = [
        'config/config_util_unittest.cc',
    ],
    deps = [
        ':config',
    ],
    testdata = [
        'config/testdata/',
    ] 
)
