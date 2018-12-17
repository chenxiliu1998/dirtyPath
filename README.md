# env
llvm 7.0 + 
ubuntu 14.04.5

# edit
注意修改下方红框处的api，改为你设置的关键API

![](http://ss1.sinaimg.cn/large/0070QjJhly1fy9zfjb979j30qv04v40k.jpg

# usage

```
clang++-7 -g -O3 Analyse.cpp Environment.cpp SearchDirtyDest.cpp main.cpp test.cpp utils.cpp `llvm-config-7 --cxxflags --ldflags --system-libs --libs core` -o main
```
源文件test.c生成对应的test.ll文件
```
clang++-7 -O3 -S -g test.c
```
执行main
```
./main test.ll
```
