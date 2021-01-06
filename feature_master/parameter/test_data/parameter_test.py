# -*- coding: UTF8 -*-
import json

# 把原始特征抽取为线上可用的参数

config = {
    "parameter_extractor_config" : {
      "uid" : {
         "group" : 1,
         "feature_names" : ["id1", "id2"],
         "extractor" : "id" ,
      },

      "follow_list" : {
        "group" : 2,
        "feature_names" : ["follow_list"],
        "extractor": "id",
      },

      "city-gender-tags" : {
        "group" : 3,
        "feature_names" : ["city|gender|tags"],
        "extractor" : "cross",
      },

      "emp_ctr" : {
        "group" : 4,
        "feature_names" : ["ctr1", "ctr2"],
        "extractor": "discrete",
        "extractor_args": "1,0.001,100,0,100", #"converter_args : denominator,smooth,max_bucket_num,min_bucket_num,buckets"
      },

      "user_test" : {
         "group" : 5,
         "feature_names" : ["city", "gender", "tags"],
         "extractor" : "id" ,
      },

  },
}

print json.dumps(config, indent=2);
