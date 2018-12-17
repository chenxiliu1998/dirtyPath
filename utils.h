//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_UTILS_H
#define ANALYSE_UTILS_H

#include "head.h"

vector<Value*> GetDirty(Instruction* ins, vector<int> pos); // 通过指令指针inst_p和参数位置pos,获得关键函数的污点变量指针

int IfStatic(Value *v);
double GetIntValue(Value *v);
double GetDoubleValue(Value* v);
float GetFloatValue(Value* v);
string GetStringValue(Value* opnd);
bool IfSysCall(string);

#endif //ANALYSE_UTILS_H
