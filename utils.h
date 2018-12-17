//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_UTILS_H
#define ANALYSE_UTILS_H

#include "head.h"
using namespace llvm;


std::vector<Value*> GetDirty(Instruction* ins, std::vector<int> pos);

int IfStatic(Value *v);
double GetIntValue(Value *v);
double GetDoubleValue(Value* v);
float GetFloatValue(Value* v);
std::string GetStringValue(Value* opnd);
bool IfSysCall(std::string);

#endif //ANALYSE_UTILS_H
