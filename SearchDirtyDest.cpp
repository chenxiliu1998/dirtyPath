//
// Created by hrc on 18-12-3.
//

#include "SearchDirtyDest.h"

// 通过g_vector_module遍历分析模块,赋值g_vector_[func_define|func_call|dirty_position]
void SearchDirtyDest()
{
    extern vector<DirtyPosition> vector_dirty_postion;
    for (int x = 0; x < g_vector_module.size(); x++) // 遍历各个Module
    {
        Module* m = g_vector_module[x];
        for (auto iter1 = m->getFunctionList().begin(); iter1 != m->getFunctionList().end(); iter1++) // 遍历Module中的Function
        {
            Function &f = *iter1;

            // 当为函数定义时,将该函数添加进g_vector_func_define
            //find func define
            if (f.getBasicBlockList().size() >= 1) //? 函数调用时无BB,定义时才有BB
            {
                FuncDefine t = {f.getName().str(), m, &f};
                cout << "tmp func define: " << t.FuncName << "  " << f.getBasicBlockList().size() << endl;
                g_vector_func_define.push_back(t);
            }

            for (auto iter2 = f.getBasicBlockList().begin(); iter2 != f.getBasicBlockList().end(); iter2++) // 遍历Function中的BasicBlock
            {
                BasicBlock &bb = *iter2;

                for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++) // 遍历BB中的指令
                {
                    Instruction &inst = *iter3;
                    string op = inst.getOpcodeName(); // 操作码

                    //find call instr
                    if (op.compare("call") == 0)
                    {
                        // 将call的函数添加进g_vector_func_call
                        unsigned int opnt_cnt = inst.getNumOperands(); // 获取call指令的参数数量
                        string name = inst.getOperand(opnt_cnt-1)->getName(); // 最后一个参数,即为call的函数
                        FuncCall tmp = {name, m, &f, &bb, &inst};
                        g_vector_func_call.push_back(tmp);

                        // 若call函数为关键函数,则添加一个污点位置进g_vector_dirty_position
                        //find dirty api
                        if (name.compare("send") == 0)
                        {
                            vector<int> vector_op = {0}; //? 关键参数为第一个
                            DirtyPosition tmp_dirty_position = {name, m, &f, &bb, &inst, vector_op};
                            g_vector_dirty_position.push_back(tmp_dirty_position);
                        }
                    }
                } //end instruction
            } //end basic block
        } //end function
    } //end module
}
