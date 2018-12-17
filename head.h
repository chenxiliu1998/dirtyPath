//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_HEAD_H
#define ANALYSE_HEAD_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/CFG.h>
using namespace llvm;
#include "utils.h"

#define NORMAL 0
#define CALL 1
#define RETURN 2
#define GETELEMENTPTR 3

#define INT 0
#define FLOAT 1
#define DOUBLE 2
#define CHAR 3
#define STRING 4

// 污点, 指关键函数的关键变量(一个DirtyPosition可以存在多个污点)
typedef struct
{
    int ID;
    string Name;    // 变量名
    Value* ValueP;  // LLVM变量
    vector<int> FlowOutNodeID;  // 传出的污点节点id
    vector<int> FlowInNodeID;   // 传入的污点节点id
    //symbol address            // 符号表地址
}Dirty;

// 关键函数
typedef struct
{
    string FuncName;
    vector<int> ArgPos;
}KeyAPI;

// 污点位置, 即调用关键函数位置
typedef struct
{
    string FuncName;            // 使用函数名
    Module* ModuleP;
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
    vector<int> VectorOpPos;    // 参数位置
}DirtyPosition;

// 系统函数调用
typedef struct
{
    string FuncName;
    Module* ModuleP;    // 调用该函数的模块指针
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
}FuncCall;

// 用户函数定义
typedef struct
{
    string FuncName;
    Module* ModuleP;
    Function* FuncP;
}FuncDefine;

// 污点节点
typedef struct
{
    int ID;
    Module* ModuleP;
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
    vector<int> PredNodeID; // 前向结点
    vector<int> SuccNodeID; // 后续结点 -1即尾节点
    vector<int> DirtyInID;  // 传入污点id
    vector<int> DirtyOutID; // 传出污点id
}Node;

// 污点传播路径, 与DirtyPosition对应
typedef struct
{
    vector<Dirty> VectorDirty;  // 污点
    vector<Node> VectorNode;
    vector<string> VectorAPI;   // 调用函数名
    vector<int> VectorInt;
    vector<int> VectorDouble;
    vector<int> VectorFloat;
    vector<int> VectorString;
}DirtyPath;



extern unique_ptr<Module> g_array_ptr[10];
extern vector<Module*> g_vector_module;
extern vector<DirtyPosition> g_vector_dirty_position;
extern vector<FuncDefine> g_vector_func_define;
extern vector<FuncCall> g_vector_func_call;
extern vector<DirtyPath> g_vector_dirty_path;


#endif //ANALYSE_HEAD_H
