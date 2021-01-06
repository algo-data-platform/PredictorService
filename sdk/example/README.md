## predictor 静态库及example编译方式

- 0.进入predictor repo主目录

- 1.编译predictor-sdk
```bash
sh sdk/build-predictor-sdk.sh release [tag-id]
```

- 2.编译运行example
```bash
#进入sdk/sdk-package/[tag]/example 目录后
sh ./build-predictor-example.sh
```
- 运行example二进制：
```bash
./predictor_example_calculate_vector
```