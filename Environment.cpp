//
// Created by hrc on 18-12-3.
//
#include "Environment.h"

Environment::Environment(int path_pos, Module* mod_p, Function* func_p, BasicBlock* bb_p,
                         Instruction* instr_p, int current_node_id, int dirty_id, int pred_node_id)
{
    pos = path_pos;//污点传播路径
    CurrentNodeID = current_node_id;
    PredNodeID = pred_node_id;
    DirtyID = dirty_id;
    ModuleP = mod_p;
    BBP = bb_p;
    FuncP = func_p;
    InstrP = instr_p;
}

//返回前向指令的向量
std::vector<int> Environment::GetPredNode()
{
    //判断一下初始化是否正确
    if (InstrP != vector_dirty_path[pos].VectorNode[CurrentNodeID].InstrP)
        std::cout << "env instrp 初始化有问题" << std::endl;
    bb_recorder = {};
    std::cout << CurrentNodeID;
    //得到该结点的类型
    int class_t = classify(CurrentNodeID);
    switch (class_t)
    {
        case NORMAL:
        {
            std::vector<int> result = GetPredNodeInBasicBlock();
            //如果没有前向结点
            if (result.size() == 0)
            {
                IfCurrentStatic();
            }
            return result;
        }

        case RETURN: //当前指令为call
        {
            /*
             * 如果是系统调用会flag=1，则和一般的指令一样，直接找到前一条的指令
             * 若是自定义的函数且flag=0，则表示这个函数还没有被遍历过，需要进入
            */

            
            int op_num = InstrP->getNumOperands();
            std::string func_name = InstrP->getOperand(op_num-1)->getName();
            /*
             *
             */
            //判断是否为系统调用
            if (IfSysCall(func_name))
            {
                std::vector<int> result = GetPredNodeInBasicBlock();
                //若找不到前向结点，看当前污点是否为constant
                if (result.size() == 0)
                {
                    IfCurrentStatic();
                }
                return result;
            }

            //判断是否visit过
            if (IfVisted(func_name)) //若已经visit过
            {
                std::vector<int> result = GetPredNodeInBasicBlock();
                
                if (result.size() == 0)
                {
                    IfCurrentStatic();
                }
                return result;
            }
            else //若没有visit过，则要得到函数的ret指令
            {
                //得到当前函数在vector_func_call的id
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
            //得到对应的call指令
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
            //如果指向的东西是污点
            if (inst->getOperand(0) == vector_dirty_path[pos].VectorDirty[DirtyID].ValueP) 
            {
                std::vector<int> result = GetPredNodeInBasicBlock();
                return result;
            }
            else if ((unsigned long)inst == (unsigned long)vector_dirty_path[pos].VectorDirty[DirtyID].ValueP) ////正向查找dirty的使用（如果前面已经找到了当前dirty的终点，就停止这一步）
            {
                GetSuccCall();
                return {};
            }
        }
    }

}

std::vector<int> Environment::GetPredNodeInBasicBlock()
{
    std::vector<int> pred = {};
    //得到当前pos的污点
    Dirty dirty = vector_dirty_path[pos].VectorDirty[DirtyID];
    //得到当前污点的value
    Value* p_value = dirty.ValueP; 
    BasicBlock::iterator i(InstrP);
    //i--;
    Instruction& inst = *i;
    while(1)
    {
        //如果当前的指令是当前bb块的第一个
        if (i == BBP->getInstList().begin())
        {
            //得到当前指令的类型
            std::string op_name = InstrP->getOpcodeName();
            //store类型
            if (op_name.compare("store") == 0)
            {
                //得到当前指令的第二个操作数
                Instruction* instr_p = (Instruction*)InstrP->getOperand(1);
                //得到第二个操作数对应的指令的名字
                std::string op_name_new = instr_p->getOpcodeName();
                //如果是alloca
                if (op_name_new.compare("alloca") == 0)
                {
                    //若在node路径中已存在instr_p
                    if (IfNodeExits(instr_p))
                        return pred;
                    //否则调用InsertUseNode更新路径范湖前向结点
                    return InsertUseNode(ModuleP, FuncP, instr_p->getParent(), instr_p);
                    //return InsertNode(instr_p);
                }
            }
            break;
        }
        i--;
        
        //若没有找到，则得到上一条指令
        Instruction& inst = *i;
        //addr1为现在的指令的地址，p_value是现在的指令的下一个的value的地址（原来的污点）
        unsigned long addr1 = (unsigned long)&(inst);
        unsigned long addr2 = (unsigned long)p_value;
        //std::cout << "inst addr: " << addr1 << "  value addr: " << addr2 << std::endl;
        //若当前指令地址为污点的地址
        if (addr1 == addr2) 
        {
            //得到当前指令的名字
            std::string op_name_t = inst.getOpcodeName();
            if (op_name_t.compare("call") != 0) {
                if (IfNodeExits(&inst))
                    return {};
            }
            return InsertNode(&inst);
        }
        else//当前指令地址不为污点地址
        {
            //得到当前指令的类型
            std::string op_name = inst.getOpcodeName();
            if (op_name.compare("store") == 0) 
            {
                //得到第二个操作数
                Value* op = inst.getOperand(1);
                //如果第二个操作数是污点的话
                if ((unsigned long)op == (unsigned long)p_value)
                {
                    if (IfNodeExits(&inst))
                        return {};
                    return InsertNode(&(inst));
                }
            }
            
            else if (op_name.compare("getelementptr") == 0)
            {
                //得到第一个参数，若第一个参数为污点
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
                //得到funcname
                Value* func_value = inst.getOperand(op_count-1);
                std::string func_name = func_value->getName();
                //该func是不是系统调用
                if (IfSysCall(func_name))
                {
                    for (int i = 0; i < op_count-1; i++)
                    {
                        Value* op_tmp = inst.getOperand(i);
                        //若某个参数等于污点
                        if ((unsigned long)op_tmp == (unsigned long)p_value)
                        {
                            //如果该call system已经获取了所有的污点，就不再返回当前call，将记录设置为0，结束循环
                            if (&inst == last_sys_insr)
                            {
                                last_sys_insr = NULL;
                                break;
                            } else  //否则返回call指令，将记录设置为当前call
                            {
                                if (IfNodeExits(&inst))
                                    return {};
                                last_sys_insr = &inst;
                                return InsertNode(&inst);
                            }
                        }
                    }
                }
                else //若不是系统调用，则进入该func，正向遍历
                {

                }
            }
        }
    }

    //若当前bb块是所在的func的第一个bb块
    if ((&(*FuncP->getBasicBlockList().begin())) == BBP)
    {
        std::cout << "in GetNodeInBasicBlock, achieve the head of funciton:" << FuncP->getName().str() << std::endl;
        std::cout << "      Instruction: " << InstrP->getOpcodeName() << std::endl;
        return {};
    }
    else//若bb块还有前向bb
    {
        bb_recorder.push_back(BBP);
        //得到前向bb块
        pred_range pred_bbs = predecessors(BBP);
        //对每一个前向bb块（可能有多个）
        for (BasicBlock* bb : pred_bbs)
        {
            int count = 0;
            //如果前向bb在之前出现过（则代表有循环）
            for (auto bbt: bb_recorder)
            {
                if (bbt == bb)
                    count += 1;
            }
            //出现的次数超过20次则强制退出循环
            if (count >= 20)
                continue;
            BBP = bb;
            //得到前向bb的最后一条指令
            InstrP = &(bb->getInstList().back());
            std::vector<int> tmp_pred = GetPredNodeInBasicBlock();
            pred.insert(pred.end(), tmp_pred.begin(), tmp_pred.end());
        }
        return pred;
    }
    if (if_static())
        return {};
}

//还需要把形参位置找到，在找到call指令后，将相应参数设置为dirty
std::vector<int> Environment::FindCallInstruction(std::string funcname)
{
    std::vector<int> result = {};
    int sys = 0;
    if (funccall_id.size() > 0)
    {
        //最后一个funccall
        FuncCall funccall = vector_func_call[funccall_id.back()];
        //若最后一个就是现在的func
        if (funccall.FuncName.compare(funcname) == 0)
        {
            //从funccall_id中删除
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
            //把新的节点（记录的是func call所在的位置）记录下来
            vector_dirty_path[pos].VectorNode.push_back(new_node);
            update_path(new_node.ID);
            result.push_back(new_node.ID);
            return result;
        }
    }
    //若没有，则在vector_func_call里面找
    for (auto func_call: vector_func_call) {
        //若找到
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
    //遍历函数定义向量
    for (auto func_def: vector_func_define)
    {
        //若找到的话，记录函数定义所在的funcP
        if (funcname.compare(func_def.FuncName) == 0)
        {
            BasicBlock* tmp_b = &(func_def.FuncP->back());
            //得到该bb块最后一条指令
            Instruction* tmp_i = &(tmp_b->back());
            std::string op_t = tmp_i->getOpcodeName();
            //若最后一条指令是ret类型
            if (op_t.compare("ret") == 0)
            {
                //若已存在，则返回现在的result
                if (IfNodeExits(tmp_i))
                    return result;
                //visit_name.push_back(funcname);
                //新建一个该函数定义块的最后一条指令的结点
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
    vector_dirty_path[pos].VectorNode[CurrentNodeID].PredNodeID.push_back(new_id);//更新当前污点的前向结点
    vector_dirty_path[pos].VectorNode[new_id].SuccNodeID.push_back(CurrentNodeID);//更新前向结点的后继结点
    vector_dirty_path[pos].VectorDirty[DirtyID].FlowOutNodeID.push_back(new_id);//更新当前污点的流出结点
    vector_dirty_path[pos].VectorNode[new_id].DirtyOutID.push_back(DirtyID);//更新前向结点的污点流出结点
}

int Environment::if_static()
{
    int sym = 0;
    //得到当前污点的value类
    Value* tmp_value_p = vector_dirty_path[pos].VectorDirty[DirtyID].ValueP;
    if (tmp_value_p == NULL)
        return sym;
    //得到value的类型
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
    //得到当前污点所在的结点信息
    Node tmp_node = vector_dirty_path[pos].VectorNode[CurrentNodeID];
    std::string op_name = tmp_node.InstrP->getOpcodeName();
    //若当前污点所在的指令类型为alloca
    if (op_name.compare("alloca") == 0)
    {
        //有后继node
        if (tmp_node.SuccNodeID.size() > 0)
        {
            //得到第一个后继结点信息
            Node tmp_suc_node = vector_dirty_path[pos].VectorNode[tmp_node.SuccNodeID[0]];
            std::string tmp_op_name = tmp_suc_node.InstrP->getOpcodeName();
            //如果第一个后继结点为store或者load类型
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
    //遍历这个污点路径上的所有结点
    for (auto inst_t: vector_dirty_path[pos].VectorNode)
    {
        //如果该指令已存在，更新路径
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
    //获得当前指令的指令名和操作数个数
    std::cout << inst->getOpcodeName();
    unsigned int opnt_cnt = inst->getNumOperands();
    //获得每个操作数
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
 //在污点的传播结点中加入新节点  
    vector_dirty_path[pos].VectorNode.push_back(new_node);
    //更新路径
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

    //不等于该函数的最后一个bb的最后一条指令
    while(i != FuncP->getBasicBlockList().back().getInstList().end())
    {
        //如果是当前bb块的最后一条指令
        if (BBP->getInstList().end() == i)
        {
            //得到后继bb块
            succ_range succ_bbs = successors(BBP);
            for (BasicBlock* bb: succ_bbs)
            {
                //修改当前bb块
                BBP = bb;
                //修改当前指令为当前bb块的第一个
                InstrP = &BBP->getInstList().front();
                BasicBlock::iterator i(InstrP);
            }
        }

    }
}

int Environment::GetSuccCall()
{
    //得到当前污点
    Value* value_p = vector_dirty_path[pos].VectorDirty[DirtyID].ValueP;
    BasicBlock::iterator i(InstrP);
    i++;
    //i不是当前bb块的最后一条指令
    while (i != BBP->getInstList().end())
    {
        Instruction* instr_p = &(*i);
        std::string op_name = instr_p->getOpcodeName();
        int arg_num = instr_p->getNumOperands();
        //如果是call指令
        if (op_name.compare("call") == 0)
        {
            //若参数中有一个是污点
            for (int j = 0; j < arg_num-1; j++)
            {
                if (instr_p->getOperand(j) == value_p)
                {
                    //若已在节点记录里面
                    if (IfSuccNodeExits(instr_p))
                        return 0;
                    else//不在
                    {
                        std::string func_name = instr_p->getOperand(arg_num-1)->getName();
                        //判断call的函数名是否为系统函数，是的话把该函数名加入到VectorAPI里
                        if (IfSysCall(func_name))
                            vector_dirty_path[pos].VectorAPI.push_back(func_name);
                        //把该指令加入到节点队列中
                        int new_node_id = InsertNode(instr_p)[0];
                        update_succ_path(new_node_id, -1);
                        return 1;
                    }
                }
            }
        }
        //若当前指令为store类型
        else if (op_name.compare("store") == 0)
        {
            //第一个操作数为污点
            if (instr_p->getOperand(0) == value_p)
            {
                int new_node_id = InsertNode(instr_p)[0];
                //把第二个操作数加入污点队列
                int new_dirty_id = InsertDirty(instr_p->getOperand(1));
                update_succ_path(new_node_id, new_dirty_id);
                DirtyID = new_dirty_id;
                CurrentNodeID = new_node_id;
                InstrP = instr_p;
                //不返回 继续往下找
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
        //如果该指令为call
        if (op_name.compare("call") == 0)
        {
            int op_num = instr->getNumOperands();
            for (int j = 0; j < op_num - 1; j++)
            {
                Value *v_p = instr->getOperand(j);
                //参数为污点
                if (v_p == p_value)
                {
                    std::string func_name = instr->getOperand(op_num-1)->getName();
                    //若call的函数为系统函数
                    if (IfSysCall(func_name))
                        vector_dirty_path[pos].VectorAPI.push_back(func_name);
                    
                    //存在的话更新了currentNodeID为找到的id，更新当前的信息
                    if (IfSuccNodeExits(instr))
                    {
                        update();
                        return;
                    }
                    //不存在的话
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
        //如果是getelementptr类型的话
        else if (op_name.compare("getelementptr") == 0)
        {
            //得到操作数一
            Value *v_p = instr->getOperand(0);
            //如果这个就是污点的话
            if (v_p == p_value) {
                //存在的话就更新
                if (IfSuccNodeExits(instr))
                {
                    update();
                    return;
                }
                //不存在的话先新建节点和dirty再更新
                int new_node_id = InsertNode(instr)[0];
                Value* new_value = (Value*)instr;
                int new_dirty_id = InsertDirty(new_value);
                update_succ_path(new_node_id, new_dirty_id);
                CurrentNodeID = new_node_id;
                update();
                return;
            }
        }
        //如果是store类型
        else if (op_name.compare("store") == 0)
        {
            //得到第一个参数
            Value *v_p = instr->getOperand(0);
            //和getelementptr相同，只是这里吧第二个操作数记为污点
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
        //如果都不是的话
        else
        {
            int op_num = instr->getNumOperands();
            //得到操作数
            for (int j = 0; j < op_num; j++)
            {
                Value *v_p = instr->getOperand(j);
                //若操作数为污点，进行和getelementptr一样的操作
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
    //当前污点路径的节点记录
    for (auto inst_t: vector_dirty_path[pos].VectorNode)
    {
        if (inst == inst_t.InstrP) //在污点路径中已存在该条指令
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
    /*
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
    */
    return new_dirty.ID;
}
