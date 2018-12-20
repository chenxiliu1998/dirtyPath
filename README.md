# env
llvm 7.0 + 
ubuntu 14.04.5

# edit
注意修改SearchDirtyDest.cpp中下方红框处的api，改为你设置的关键API

![](http://ww1.sinaimg.cn/large/0070QjJhly1fy9zi9w11uj30mc044abq.jpg)

# usage

```
clang++-7 -g -O3 Analyse.cpp Environment.cpp SearchDirtyDest.cpp main.cpp test.cpp utils.cpp `llvm-config-7 --cxxflags --ldflags --system-libs --libs core` -o main
```
源文件test.c生成对应的test.ll文件
```
clang++-7 -emit-llvm -S -g test.c
```
执行main
```
./main test.ll
```
