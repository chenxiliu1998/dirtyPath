//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_ENVIRONMENT_H
#define ANALYSE_ENVIRONMENT_H

#include "head.h"

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
    vector<int> PredDirtyVector;
    vector<int> NewPredDirtyVector;
    vector<string> visit_name = {};
    vector<int> funccall_id;
    vector<Instruction*> instr_recorder = {};
    vector<BasicBlock*> bb_recorder = {};
    vector<Value*> vector_getelementptr = {};

    string last_sys;
    Instruction* last_sys_insr;

    BasicBlock* pred_pred_bb;

    Environment(int pos, Module* mod_p, Function* func_p, BasicBlock* bb_p,
                Instruction* instr_p, int current_node_id, int dirty_id, int pred_node_id);

    int classify(int);

    vector<int> GetPredNode();

    vector<int> FindCallInstruction(string funcname);

    vector<int> FindRetInstruction(string funcname);

    vector<int> InsertNode(Instruction* inst);

    vector<int> InsertUseNode(Module* ModuleP, Function* FuncP, BasicBlock* BBP, Instruction* inst);

    bool IfCurrentStatic();

    bool IfVisted(string funcname);

    bool IfNodeExits(Instruction* inst);

    void update_path(int new_id);

    void update();

    int if_static();

    vector<int> GetSuccNode();

    bool IfSuccNodeExits(Instruction* inst);

    int InsertDirty(Value* new_value);

    void update_succ_path(int new_node_id, int new_dirty_id);

    void update_succ_path(int new_id);

    void EachInst(BasicBlock::iterator i);

    int GetSuccCall();

private:
    vector<int> GetPredNodeInBasicBlock();
};

#endif //ANALYSE_ENVIRONMENT_H
