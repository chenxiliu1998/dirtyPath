//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_HEAD_H
#define ANALYSE_HEAD_H

#include <iostream>
#include <string>
#include <vector>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/CFG.h>
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

using namespace llvm;

typedef struct
{
    int ID;
    std::string Name;
    Value* ValueP;
    std::vector<int> FlowOutNodeID;
    std::vector<int> FlowInNodeID;
    //symbol address
}Dirty;

typedef struct
{
    std::string FuncName;
    std::vector<int> ArgPos;
}KeyAPI;

typedef struct
{
    std::string FuncName;
    Module* ModuleP;
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
    std::vector<int> VectorOpPos;
}DirtyPosition;

typedef struct
{
    std::string FuncName;
    Module* ModuleP;
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
}FuncCall;

typedef struct
{
    std::string FuncName;
    Module* ModuleP;
    Function* FuncP;
}FuncDefine;

typedef struct
{
    int ID;
    Module* ModuleP;
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
    std::vector<int> PredNodeID;
    std::vector<int> SuccNodeID;
    std::vector<int> DirtyInID;
    std::vector<int> DirtyOutID;
}Node;

typedef struct
{
    std::vector<Dirty> VectorDirty;
    std::vector<Node> VectorNode;
    std::vector<std::string> VectorAPI;
    std::vector<int> VectorInt;
    std::vector<int> VectorDouble;
    std::vector<int> VectorFloat;
    std::vector<int> VectorString;
}DirtyPath;



extern std::unique_ptr<Module> array_ptr[10];
extern std::vector<Module*> vector_module;
extern std::vector<DirtyPosition> vector_dirty_position;
extern std::vector<FuncDefine> vector_func_define;
extern std::vector<FuncCall> vector_func_call;
extern std::vector<DirtyPath> vector_dirty_path;


#endif //ANALYSE_HEAD_H
