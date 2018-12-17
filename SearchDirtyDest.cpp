//
// Created by hrc on 18-12-3.
//

#include "SearchDirtyDest.h"

void SearchDirtyDest()
{
    extern std::vector<DirtyPosition> vector_dirty_postion;
    for (int x = 0; x < vector_module.size(); x++)
    {
        Module* m = vector_module[x];
        for (auto iter1 = m->getFunctionList().begin(); iter1 != m->getFunctionList().end(); iter1++)
        {
            Function &f = *iter1;

            //find func define
            if (f.getBasicBlockList().size() >= 1)
            {
                FuncDefine t = {f.getName().str(), m, &f};
                std::cout << "tmp func define: " << t.FuncName << "  " << f.getBasicBlockList().size() << std::endl;
                vector_func_define.push_back(t);
            }

            for (auto iter2 = f.getBasicBlockList().begin(); iter2 != f.getBasicBlockList().end(); iter2++)
            {
                BasicBlock &bb = *iter2;

                for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++)
                {
                    Instruction &inst = *iter3;
                    std::string op = inst.getOpcodeName();

                    //find call instr
                    if (op.compare("call") == 0)
                    {
                        unsigned int opnt_cnt = inst.getNumOperands();
                        std::string name = inst.getOperand(opnt_cnt-1)->getName();
                        FuncCall tmp ={name, m, &f, &bb, &inst};
                        vector_func_call.push_back(tmp);

                        //find dirty api
                        if (name.compare("send") == 0)
                        {
                            std::vector<int> vector_op = {0};
                            DirtyPosition tmp_dirty_position = {name, m, &f, &bb, &inst, vector_op};
                            vector_dirty_position.push_back(tmp_dirty_position);
                        }
                    }
                } //end instruction
            } //end basic block
        } //end function
    } //end module
}


