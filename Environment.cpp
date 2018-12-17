//
// Created by hrc on 18-12-3.
//
#include "Environment.h"

Environment::Environment(int path_pos, Module* mod_p, Function* func_p, BasicBlock* bb_p,
                         Instruction* instr_p, int current_node_id, int dirty_id, int pred_node_id)
{
    pos = path_pos;
    CurrentNodeID = current_node_id;
    PredNodeID = pred_node_id;
    DirtyID = dirty_id;
    ModuleP = mod_p;
    BBP = bb_p;
    FuncP = func_p;
    InstrP = instr_p;
}

//返回前向指令的向量的通用接口
std::vector<int> Environment::GetPredNode()
{
    if (InstrP != vector_dirty_path[pos].VectorNode[CurrentNodeID].InstrP)
        std::cout << "env instrp初始化有问题" << std::endl;
    bb_recorder = {};
    std::cout << CurrentNodeID;
    int class_t = classify(CurrentNodeID);
    switch (class_t)
    {
        case NORMAL:
        {
            std::vector<int> result = GetPredNodeInBasicBlock();
            //如果找不到前向节点，就看当前的污点是否为constant
            if (result.size() == 0)
            {
                IfCurrentStatic();
            }
            return result;
        }

        case RETURN: //当前指令为call，由函数类型和flag决定是否要遍历此函数
        {
            /*
             * 如果是系统调用或flag为1，和一般的指令一样，直接找到前一条的指令
             * 如果是自定义的函数，且flag为0，表示这个函数还没有被遍历过，需要进入
            */

            //判断函数名是否为read file或者分read from internet ...，如果是的话，标记当前dirty_path
            int op_num = InstrP->getNumOperands();
            std::string func_name = InstrP->getOperand(op_num-1)->getName();
            /*
             *
             */
            //判段是否为系统调用
            if (IfSysCall(func_name))
            {
                std::vector<int> result = GetPredNodeInBasicBlock();
                //如果找不到前向节点，就看当前的污点是否为constant
                if (result.size() == 0)
                {
                    IfCurrentStatic();
                }
                return result;
            }

            //判断是否visit过
            if (IfVisted(func_name)) //如果已经visit过了
            {
                std::vector<int> result = GetPredNodeInBasicBlock();
                //如果找不到前向节点，就看当前的污点是否为constant
                if (result.size() == 0)
                {
                    IfCurrentStatic();
                }
                return result;
            }
            else //如果没有visit过，要得到函数的ret指令
            {
                //将当前call指令的id记录下来
                for (int i = 0; i < vector_func_call.size(); i++)
                {
                    FuncCall funccall = vector_func_call[i];
                    if (funccall.InstrP == InstrP)
                        funccall_id.push_back(i);
                }
                if (funccall_id.size() == 0)
                    std::cout << "[error]IN GetPredNode: can't find func_call of function " << func_name << " in function " << FuncP->getName().str() << std::endl;
                return FindRetInstruction(func_name);
            }
            break;
        }

        case CALL: //当前指令为形参加载，前一条指令为call当前函数
        {
            std::vector<int> result = FindCallInstruction(FuncP->getName().str());
            if (result.size() == 0)
                std::cout << "[error]IN GetPredNode: FindCallInstruction size 0, check out if " << FuncP->getName().str() << " is called in this project" << std::endl;
            return result;
        }

        case GETELEMENTPTR:
        {
            Instruction* inst = vector_dirty_path[pos].VectorNode[CurrentNodeID].InstrP;
            std::cout << inst;
            std::cout << vector_dirty_path[pos].VectorDirty[DirtyID].ValueP;
            if (inst->getOperand(0) == vector_dirty_path[pos].VectorDirty[DirtyID].ValueP) //向上查找右值为dirty的getelementptr
            {
                std::vector<int> result = GetPredNodeInBasicBlock();
                return result;
            }
            else if ((unsigned long)inst == (unsigned long)vector_dirty_path[pos].VectorDirty[DirtyID].ValueP) //正向查找dirty的使用（如果前面已经找到了当前dirty的终点，就停止这一步）
            {
                GetSuccCall();
                return {};
            }
        }
    }

}

//判断前一Instruction不是调用当前函数的语句，能在这个function的基本块内找到前一条instruction
std::vector<int> Environment::GetPredNodeInBasicBlock()
{
    std::vector<int> pred = {};
    Dirty dirty = vector_dirty_path[pos].VectorDirty[DirtyID];
    Value* p_value = dirty.ValueP; //记录dirty的value地址
    BasicBlock::iterator i(InstrP);
    i--;
    while(1)
    {
        Instruction& inst = *i;
        unsigned long addr1 = (unsigned long)&(inst);
        unsigned long addr2 = (unsigned long)p_value;
        //std::cout << "inst addr: " << addr1 << "  value addr: " << addr2 << std::endl;
        if (addr1 == addr2) //在Llvm中，除store以外，inistruciton的地址和他的返回value地址相同,可以确dirty发生了变化
        {
            std::string op_name_t = inst.getOpcodeName();
            if (op_name_t.compare("call") != 0) {
                if (IfNodeExits(&inst))
                    return {};
            }
            return InsertNode(&inst);
        }
        else
        {
            std::string op_name = inst.getOpcodeName();
            if (op_name.compare("store") == 0) //如果为store指令，需要判断第二个参数是否为当前value，是则可以判断dirty发生了变化
            {
                Value* op = inst.getOperand(1);
                if ((unsigned long)op == (unsigned long)p_value)
                {
                    if (IfNodeExits(&inst))
                        return {};
                    return InsertNode(&(inst));
                }
            }

            else if (op_name.compare("getelementptr") == 0)
            {
                Value* op = inst.getOperand(0);
                if ((unsigned long)op == (unsigned long)p_value)
                {
                    if (!IfNodeExits(&inst))
                        return InsertNode(&(inst));
                }
            }
            else if (op_name.compare("call") == 0)
            {
                int op_count = inst.getNumOperands();
                Value* func_value = inst.getOperand(op_count-1);
                std::string func_name = func_value->getName();
                //如果是系统函数，判断当前dirty是否作为参数，是则将当前节点记录进API
                if (IfSysCall(func_name))
                {
                    for (int i = 0; i < op_count-1; i++)
                    {
                        Value* op_tmp = inst.getOperand(i);
                        if ((unsigned long)op_tmp == (unsigned long)p_value) //如果是函数参数，返回当前call指令加入path
                        {
                            //如果该call system已经获取了全部dirty,就不再返回当前call指令，并将记录设置为0,结束循环
                            if (&inst == last_sys_insr)
                            {
                                last_sys_insr = NULL;
                                break;
                            } else  //否则，返回当前call指令，病将记录设置为当前call
                            {
                                if (IfNodeExits(&inst))
                                    return {};
                                last_sys_insr = &inst;
                                return InsertNode(&inst);
                            }
                        }
                    }
                }
                else //如果不是系统调用，进入func，正向遍历
                {

                }
            }
        }
        //如果到了基本块的首节点，判断currentnode是否为store，如果是的话，就找到它的alloca节点并退出循环
        if (i == BBP->getInstList().begin())
        {
            std::string op_name = InstrP->getOpcodeName();
            if (op_name.compare("store") == 0)
            {
                Instruction* instr_p = (Instruction*)InstrP->getOperand(1);
                std::string op_name_new = instr_p->getOpcodeName();
                if (op_name_new.compare("alloca") == 0)
                {
                    if (IfNodeExits(instr_p))
                        return pred;

                    return InsertUseNode(ModuleP, FuncP, instr_p->getParent(), instr_p);
                    //return InsertNode(instr_p);
                }
            }
            break;
        }
        i--;
    }

    //如果当前bb是function的第一个bb, 由于在classify后，这种情况不可能发生
    if ((&(*FuncP->getBasicBlockList().begin())) == BBP)
    {
        std::cout << "in GetNodeInBasicBlock, achieve the head of funciton:" << FuncP->getName().str() << std::endl;
        std::cout << "      Instruction: " << InstrP->getOpcodeName() << std::endl;
        return {};
    }
    else//如果当前bb还有前项，基本块跳转
    {
        if (bb_recorder.size() > 0 && bb_recorder.back() != BBP)
            bb_recorder.push_back(BBP);

        pred_range pred_bbs = predecessors(BBP);
        for (BasicBlock* bb : pred_bbs)
        {
            BBP = bb;
            InstrP = &(bb->getInstList().back());
            std::vector<int> tmp_pred = GetPredNodeInBasicBlock();
            pred.insert(pred.end(), tmp_pred.begin(), tmp_pred.end());
        }
        return pred;
    }
    if (if_static())
        return {};
}


int Environment::classify(int instr_pos)
{
    DirtyPath dirty_path = vector_dirty_path[pos];
    Node tmp_node = dirty_path.VectorNode[instr_pos];
    Instruction* inst = tmp_node.InstrP;
    std::string op_name = inst->getOpcodeName();
    if (op_name.compare("alloca") == 0)
    {
        int arg_num = tmp_node.FuncP->arg_size();

        if(arg_num == 0)
            return NORMAL;

        BasicBlock::iterator it(inst);

        //get position of instruction
        int count_t = 0;
        for (auto t = tmp_node.BBP->getInstList().begin(); t != it; t++)
        {
            count_t++;

            //if (count_t >= tmp_node.BBP->getInstList().size())
              //  break;

        }
        if (count_t < arg_num) // dirty is function arg
        {
            op_pos = count_t; //set the operand postion
            if (visit_name.size() == 0 || visit_name.back().compare(tmp_node.FuncP->getName().str()) != 0)
                visit_name.push_back(tmp_node.FuncP->getName().str());
            return CALL;
        }
        else //init instruction tmp operand
        {
            return NORMAL;
        }
    }
    else if (op_name.compare("call") == 0)
    {
        return RETURN;
    }

    else if (op_name.compare("getelementptr") == 0)
        return GETELEMENTPTR;

    else
        return NORMAL;
}

//还需要把形参位置找到，在找到call指令后，将相应参数设置为dirty
std::vector<int> Environment::FindCallInstruction(std::string funcname)
{
    std::vector<int> result = {};
    int sys = 0;
    if (funccall_id.size() > 0)
    {
        FuncCall funccall = vector_func_call[funccall_id.back()];
        if (funccall.FuncName.compare(funcname) == 0)
        {
            funccall_id.erase(funccall_id.end()-1);
            Node new_node = {(int)vector_dirty_path[pos].VectorNode.size(), funccall.ModuleP,
                             funccall.FuncP, funccall.BBP, funccall.InstrP};
            //test start
            std::cout << funccall.InstrP->getOpcodeName();
            Instruction &inst = *(funccall.InstrP);
            unsigned int opnt_cnt = inst.getNumOperands();

            for (int i = 0; i < opnt_cnt; ++i)
            {
                Value *opnd = inst.getOperand(i);
                std::string o;
                if (opnd->hasName()) {
                    o = opnd->getName();
                    std::cout << " " << o << ",";
                } else {
                    std::cout << " ptr" << opnd << ",";
                }
            }
            std::cout << std::endl;
            //test end
            vector_dirty_path[pos].VectorNode.push_back(new_node);
            update_path(new_node.ID);
            result.push_back(new_node.ID);
            return result;
        }
    }
    for (auto func_call: vector_func_call) {
        if (funcname.compare(func_call.FuncName) == 0) {
            Node new_node = {(int) vector_dirty_path[pos].VectorNode.size(), func_call.ModuleP,
                             func_call.FuncP, func_call.BBP, func_call.InstrP};
            //test start
            std::cout << func_call.InstrP->getOpcodeName();
            Instruction &inst = *(func_call.InstrP);
            unsigned int opnt_cnt = inst.getNumOperands();

            for (int i = 0; i < opnt_cnt; ++i) {
                Value *opnd = inst.getOperand(i);
                std::string o;
                if (opnd->hasName()) {
                    o = opnd->getName();
                    std::cout << " " << o << ",";
                } else {
                    std::cout << " ptr" << opnd << ",";
                }
            }
            std::cout << std::endl;
            //test end
            vector_dirty_path[pos].VectorNode.push_back(new_node);
            update_path(new_node.ID);
            result.push_back(new_node.ID);
        }
    }
    std::cout << "[*]!!!!!!!!!!!!!!!!result size: " << result.size() << std::endl;
    if (result.size() != 0)
        std::cout << "[*]!!!!!!!!!!!!!!!!reault[0]: " << result.front() << std::endl;
    return result;
}

std::vector<int> Environment::FindRetInstruction(std::string funcname)
{
    std::vector<int> result = {};
    for (auto func_def: vector_func_define)
    {
        if (funcname.compare(func_def.FuncName) == 0)
        {
            BasicBlock* tmp_b = &(func_def.FuncP->back());
            Instruction* tmp_i = &(tmp_b->back());
            std::string op_t = tmp_i->getOpcodeName();
            if (op_t.compare("ret") == 0)
            {
                if (IfNodeExits(tmp_i))
                    return result;
                //visit_name.push_back(funcname);
                Node new_node = {(int)vector_dirty_path[pos].VectorNode.size(), func_def.ModuleP,
                        FuncP = func_def.FuncP, tmp_b, &(tmp_b->back())};
                //test start
                std::cout << tmp_b->back().getOpcodeName();
                Instruction &inst = tmp_b->back();
                unsigned int opnt_cnt = inst.getNumOperands();

                for (int i = 0; i < opnt_cnt; ++i)
                {
                    Value *opnd = inst.getOperand(i);
                    std::string o;
                    if (opnd->hasName()) {
                        o = opnd->getName();
                        std::cout << " " << o << ",";
                    } else {
                        std::cout << " ptr" << opnd << ",";
                    }
                }
                std::cout << std::endl;
                //test end
                vector_dirty_path[pos].VectorNode.push_back(new_node);
                update_path(new_node.ID);
                result.push_back(new_node.ID);
            }
        }
    }
    return result;
}

//update environment according to currentnodeid
void Environment::update()
{
    Node tmp_node = vector_dirty_path[pos].VectorNode[CurrentNodeID];
    ModuleP = tmp_node.ModuleP;
    FuncP = tmp_node.FuncP;
    BBP = tmp_node.BBP;
    InstrP = tmp_node.InstrP;
}

//update pred path
void Environment::update_path(int new_id)
{
    vector_dirty_path[pos].VectorNode[CurrentNodeID].PredNodeID.push_back(new_id);
    vector_dirty_path[pos].VectorNode[new_id].SuccNodeID.push_back(CurrentNodeID);
    vector_dirty_path[pos].VectorDirty[DirtyID].FlowOutNodeID.push_back(new_id);
    vector_dirty_path[pos].VectorNode[new_id].DirtyOutID.push_back(DirtyID);
}

int Environment::if_static()
{
    int sym = 0;
    Value* tmp_value_p = vector_dirty_path[pos].VectorDirty[DirtyID].ValueP;
    if (tmp_value_p == NULL)
        return sym;
    int type_value = IfStatic(tmp_value_p);
    switch (type_value)
    {
        case INT:
        {
            vector_dirty_path[pos].VectorInt.push_back(DirtyID);
            sym = 1;
            break;
        }
        case DOUBLE:
        {
            vector_dirty_path[pos].VectorDouble.push_back(DirtyID);
            sym = 1;
            break;
        }
        case FLOAT:
        {
            vector_dirty_path[pos].VectorFloat.push_back(DirtyID);
            sym = 1;
            break;
        }
        case STRING:
        {
            vector_dirty_path[pos].VectorString.push_back(DirtyID);
            sym = 1;
            break;
        }
    }

    return sym;
}

bool Environment::IfCurrentStatic()
{
    Node tmp_node = vector_dirty_path[pos].VectorNode[CurrentNodeID];
    std::string op_name = tmp_node.InstrP->getOpcodeName();
    if (op_name.compare("alloca") == 0)
    {
        if (tmp_node.SuccNodeID.size() > 0)
        {
            Node tmp_suc_node = vector_dirty_path[pos].VectorNode[tmp_node.SuccNodeID[0]];
            std::string tmp_op_name = tmp_suc_node.InstrP->getOpcodeName();
            if (tmp_op_name.compare("store") == 0 || tmp_op_name.compare("load") == 0)
            {
                DirtyID = tmp_suc_node.DirtyInID[0];
                if (if_static())
                    return true;
            }
        }
        std::cout << "[error] can't find a not constant pred dirty of alloca's dirty id " << DirtyID << std::endl;
    }
    else
    {
        if (if_static())
            return true;
    }
    std::cout << "[error] can't find a not constant pred dirty of " << op_name  << "'s dirty id "<< DirtyID <<std::endl;
    return false;
}

//只能判断前向节点
bool Environment::IfNodeExits(Instruction* inst)
{
    std::vector<int> pred = {};
    for (auto inst_t: vector_dirty_path[pos].VectorNode)
    {
        if (inst == inst_t.InstrP) //pred node already exit
        {
            update_path(inst_t.ID);
            return true;
        }
    }
    return false;
}

//自定义模块函数等插入新的节点
std::vector<int> Environment::InsertUseNode(Module* ModuleP, Function* FuncP, BasicBlock* BBP, Instruction* inst)
{
    std::vector<int> pred = {};
    Node new_node = {(int)vector_dirty_path[pos].VectorNode.size(), ModuleP, FuncP, BBP, inst, {}, {}, {}, {}};
    //test start
    std::cout << inst->getOpcodeName();
    unsigned int opnt_cnt = inst->getNumOperands();

    for (int i = 0; i < opnt_cnt; ++i)
    {
        Value *opnd = inst->getOperand(i);
        std::string o;
        if (opnd->hasName())
        {
            o = opnd->getName();
            std::cout << " " << o << ",";
        } else {
            std::cout << " ptr" << opnd << ",";
        }
    }
    std::cout << std::endl;
    //test end
    vector_dirty_path[pos].VectorNode.push_back(new_node);
    update_path(new_node.ID);
    pred.push_back(new_node.ID);
    return pred;
}

//根据当前env插入新的节点
std::vector<int> Environment::InsertNode(Instruction* inst)
{
    std::vector<int> pred = {};
    Node new_node = {(int)vector_dirty_path[pos].VectorNode.size(), ModuleP, FuncP, BBP, inst, {}, {}, {}, {}};
    //test start
    std::cout << inst->getOpcodeName();
    unsigned int opnt_cnt = inst->getNumOperands();

    for (int i = 0; i < opnt_cnt; ++i)
    {
        Value *opnd = inst->getOperand(i);
        std::string o;
        if (opnd->hasName())
        {
            o = opnd->getName();
            std::cout << " " << o << ",";
        } else {
            std::cout << " ptr" << opnd << ",";
        }
    }
    std::cout << std::endl;
    //test end
    vector_dirty_path[pos].VectorNode.push_back(new_node);
    update_path(new_node.ID);
    pred.push_back(new_node.ID);
    return pred;
}

bool Environment::IfVisted(std::string funcname)
{
    for (auto itera = visit_name.begin(); itera != visit_name.end(); itera ++)
    {
        std::string n = *itera;
        if (n.compare(funcname) == 0) {
            visit_name.erase(itera);
            return true;
        }
    }
    return false;
}

//在调用这个函数之前，要将当前dirtyid放进preddirty中
std::vector<int> Environment::GetSuccNode()
{
    std::vector<int> succ = {};
    BasicBlock::iterator i(InstrP);
    i++;

    while(i != FuncP->getBasicBlockList().back().getInstList().end())
    {
        //如果i是当前bb的end
        if (BBP->getInstList().end() == i)
        {
            succ_range succ_bbs = successors(BBP);
            for (BasicBlock* bb: succ_bbs)
            {
                BBP = bb;
                InstrP = &BBP->getInstList().front();
                BasicBlock::iterator i(InstrP);
            }
        }

    }
}

int Environment::GetSuccCall()
{
    Value* value_p = vector_dirty_path[pos].VectorDirty[DirtyID].ValueP;
    BasicBlock::iterator i(InstrP);
    i++;
    while (i != BBP->getInstList().end())
    {
        Instruction* instr_p = &(*i);
        std::string op_name = instr_p->getOpcodeName();
        int arg_num = instr_p->getNumOperands();
        if (op_name.compare("call") == 0)
        {

            for (int j = 0; j < arg_num-1; j++)
            {
                if (instr_p->getOperand(j) == value_p)
                {
                    if (IfSuccNodeExits(instr_p))
                        return 0;
                    else
                    {
                        std::string func_name = instr_p->getOperand(arg_num-1)->getName();
                        if (IfSysCall(func_name))
                            vector_dirty_path[pos].VectorAPI.push_back(func_name);
                        int new_node_id = InsertNode(instr_p)[0];
                        update_succ_path(new_node_id, -1);
                        return 1;
                    }
                }
            }
        }
        i++;
    }
    return -1;
}


void Environment::EachInst(BasicBlock::iterator i)
{
    for (auto id: PredDirtyVector)
    {
        Dirty dirty = vector_dirty_path[pos].VectorDirty[id];
        Value *p_value = dirty.ValueP; //记录dirty的value地址
        Instruction *instr = &(*i);
        std::string op_name = instr->getOpcodeName();
        if (op_name.compare("call") == 0)
        {
            int op_num = instr->getNumOperands();
            for (int j = 0; j < op_num - 1; j++)
            {
                Value *v_p = instr->getOperand(j);
                if (v_p == p_value)
                {
                    std::string func_name = instr->getOperand(op_num-1)->getName();
                    if (IfSysCall(func_name))
                        vector_dirty_path[pos].VectorAPI.push_back(func_name);
                    if (IfSuccNodeExits(instr))
                    {
                        update();
                        return;
                    }
                    int new_node_id = InsertNode(instr)[0];
                    Value* new_value = (Value*)instr;
                    int new_dirty_id = InsertDirty(new_value);
                    update_succ_path(new_node_id, new_dirty_id);
                    CurrentNodeID = new_node_id;
                    update();
                    return;
                }
            }
        }
        else if (op_name.compare("getelementptr") == 0)
        {
            Value *v_p = instr->getOperand(0);
            if (v_p == p_value) {
                if (IfSuccNodeExits(instr))
                {
                    update();
                    return;
                }
                int new_node_id = InsertNode(instr)[0];
                Value* new_value = (Value*)instr;
                int new_dirty_id = InsertDirty(new_value);
                update_succ_path(new_node_id, new_dirty_id);
                CurrentNodeID = new_node_id;
                update();
                return;
            }
        }
        else if (op_name.compare("store") == 0)
        {
            Value *v_p = instr->getOperand(0);
            if (v_p == p_value) {
                if (IfSuccNodeExits(instr))
                {
                    update();
                    return;
                }
                int new_node_id = InsertNode(instr)[0];
                Value* new_value = instr->getOperand(1);
                int new_dirty_id = InsertDirty(new_value);
                update_succ_path(new_node_id, new_dirty_id);
                CurrentNodeID = new_node_id;
                update();
            }
        }
        else
        {
            int op_num = instr->getNumOperands();
            for (int j = 0; j < op_num; j++)
            {
                Value *v_p = instr->getOperand(j);
                if (v_p == p_value)
                {
                    if (IfSuccNodeExits(instr))
                    {
                        update();
                        return;
                    }
                    int new_node_id = InsertNode(instr)[0];
                    Value* new_value = (Value*)instr;
                    int new_dirty_id = InsertDirty(new_value);
                    update_succ_path(new_node_id, new_dirty_id);
                    CurrentNodeID = new_node_id;
                    update();
                    return;
                }
            }
        }

    } //end each dirty in pred
}

//判断是否已经存在，如果存在，更新路径和currentnodeid，并返回true
bool Environment::IfSuccNodeExits(Instruction* inst)
{
    std::vector<int> pred = {};
    for (auto inst_t: vector_dirty_path[pos].VectorNode)
    {
        if (inst == inst_t.InstrP) //节点已经插入
        {
            int new_node_id = inst_t.ID;
            vector_dirty_path[pos].VectorNode[CurrentNodeID].SuccNodeID.push_back(new_node_id);
            vector_dirty_path[pos].VectorNode[new_node_id].PredNodeID.push_back(CurrentNodeID);
            vector_dirty_path[pos].VectorDirty[DirtyID].FlowInNodeID.push_back(new_node_id);
            vector_dirty_path[pos].VectorNode[new_node_id].DirtyInID.push_back(DirtyID);
            CurrentNodeID = new_node_id;
            return true;
        }
    }
    return false;
}

//在insert node，insert dirty 之后调用，完成所有的更新
void Environment::update_succ_path(int new_node_id, int new_dirty_id)
{
    vector_dirty_path[pos].VectorNode[CurrentNodeID].SuccNodeID.push_back(new_node_id);
    vector_dirty_path[pos].VectorNode[new_node_id].PredNodeID.push_back(CurrentNodeID);
    vector_dirty_path[pos].VectorDirty[DirtyID].FlowInNodeID.push_back(new_node_id);
    vector_dirty_path[pos].VectorNode[new_node_id].DirtyOutID.push_back(new_dirty_id);
    vector_dirty_path[pos].VectorNode[new_node_id].DirtyInID.push_back(DirtyID);
    if (new_dirty_id > 0)
        vector_dirty_path[pos].VectorDirty[new_dirty_id].FlowOutNodeID.push_back(new_node_id);
}

//只创建，没有更新l流入流出节点id
int Environment::InsertDirty(Value* new_value)
{
    Dirty new_dirty;
    new_dirty.ValueP = new_value;
    new_dirty.ID = vector_dirty_path[pos].VectorDirty.size();
    if (new_value->hasName())
        new_dirty.Name = new_value->getName().str();
    vector_dirty_path[pos].VectorDirty.push_back(new_dirty);
    int sym = 0;
    for (auto i: PredDirtyVector)
    {
        if (i == new_dirty.ID)
        {
            sym = 1;
            break;
        }
    }
    if (sym == 0)
        PredDirtyVector.push_back(new_dirty.ID);
    return new_dirty.ID;
}