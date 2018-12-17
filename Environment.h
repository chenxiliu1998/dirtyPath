//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_ENVIRONMENT_H
#define ANALYSE_ENVIRONMENT_H

#include "head.h"

using namespace llvm;

class Environment
{
public:
    int direction = 0; //0-反向 1-正向
    int pos;
    int CurrentNodeID;
    int PredNodeID;
    int DirtyID;
    Module* ModuleP;
    Function* FuncP;
    BasicBlock* BBP;
    Instruction* InstrP;
    int op_pos;
    std::vector<int> PredDirtyVector;
    std::vector<int> NewPredDirtyVector;
    std::vector<std::string> visit_name = {};
    std::vector<int> funccall_id;
    std::vector<Instruction*> instr_recorder = {};
    std::vector<BasicBlock*> bb_recorder = {};
    std::vector<Value*> vector_getelementptr = {};

    std::string last_sys;
    Instruction* last_sys_insr;

    BasicBlock* pred_pred_bb;

    Environment(int pos, Module* mod_p, Function* func_p, BasicBlock* bb_p,
                Instruction* instr_p, int current_node_id, int dirty_id, int pred_node_id);

    int classify(int);

    std::vector<int> GetPredNode();

    std::vector<int> FindCallInstruction(std::string funcname);

    std::vector<int> FindRetInstruction(std::string funcname);

    std::vector<int> InsertNode(Instruction* inst);

    std::vector<int> InsertUseNode(Module* ModuleP, Function* FuncP, BasicBlock* BBP, Instruction* inst);

    bool IfCurrentStatic();

    bool IfVisted(std::string funcname);

    bool IfNodeExits(Instruction* inst);

    void update_path(int new_id);

    void update();

    int if_static();

    std::vector<int> GetSuccNode();

    bool IfSuccNodeExits(Instruction* inst);

    int InsertDirty(Value* new_value);

    void update_succ_path(int new_node_id, int new_dirty_id);

    void update_succ_path(int new_id);

    void EachInst(BasicBlock::iterator i);

    int GetSuccCall();

private:
    std::vector<int> GetPredNodeInBasicBlock();
};

#endif //ANALYSE_ENVIRONMENT_H
