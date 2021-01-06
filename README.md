# Predictor

## Introduction
- Predictor is an Online Inference Server for Machine/Deep Learning models.
- It is designed to aim for Low Latency and High Throughput.
- It is designed to aim for Distrubuted and Concurrent real-time system.

## Get Started - Build
### 1. clone the repo
```sh
$ git clone http://github.com/algo-data-platform/PredictorService.git
$ cd PredictorService/
```
### 2. build
(assuming you are at the repo base dir: `PredictorService/`)
```sh
$ sh ./build.sh [release/debug]
```

## Get Started - Run Server
### 1. start predictor server
(assuming you are at the repo base dir: `PredictorService/`)
```sh
$ cd runtime/
$ sh ./start_predictor.sh
```
it should print out a message with an url to see the server status, such as:
> check predictor status on http://local_host:10048/server/status
### 2. load model into the predictor service
(assuming you are at the repo base dir: `PredictorService/`)
```sh
$ cd runtime/
$ sh ./load_model.sh
```
it should print out a message with an url to see the model status, such as:
> check model status on http://local_host:10048/get_service_model_info

Done! Now you have a predictor server running (with a model loaded into the memory) and ready to inference model requests!

## Get Started - Run Client
### 1. build sdk
(assuming you are at the repo base dir: `PredictorService/`)
```sh
$ sh sdk/build-predictor-sdk.sh release
```
### 2. build example (client)
(assuming you are at the repo base dir: `PredictorService/`)
```sh
$ cd sdk/sdk_package/latest/example
$ sh ./build-predictor-example.sh
```
This should build an executable binary such as `predictor_example_calculate_vector` and `predictor_example_predict`, you can run them as regular binary programs:
```
$ ./predictor_example_predict
```
And if you have your server up in previous step, this example sends requests to your server and gets back predict results.
