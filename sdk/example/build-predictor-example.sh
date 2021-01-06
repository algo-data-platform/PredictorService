#!/bin/bash
/usr/bin/g++ -g -o predictor_example_calculate_vector predictor_example_calculate_vector.cc -std=c++11 -I../include -L../lib  -l_sdk_predictor -l_sdk_common  -lpthread  -lunwind -ldl
/usr/bin/g++ -g -o predictor_example_predict predictor_example_predict.cc -std=c++11 -I../include -L../lib  -l_sdk_predictor -l_sdk_common  -lpthread  -lunwind -ldl
