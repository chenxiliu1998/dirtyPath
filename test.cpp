//
// Created by hrc on 18-12-3.
//
#include "test.h"
void printpred(int, int);

void test()
{

    std::cout << "*************************************dirty_api size: " << vector_dirty_position.size() <<std::endl;
    for(auto e: vector_dirty_position)
    {
        std::cout << "FuncName: " << e.FuncName << std::endl;
        std::cout << "ModuleID: " << e.ModuleP-> getName().str()<< std::endl;
    }
    std::cout << "************************************func_define size: " << vector_func_define.size() << std::endl;
    for(auto e: vector_func_define)
    {
        std::cout << "FuncName: " << e.FuncName << std::endl;
        std::cout << "ModuleID: " << e.ModuleP->getName().str() << std::endl;
    }
    std::cout << "*************************************func_call size: " << vector_func_call.size() << std::endl;
    for (auto e: vector_func_call)
    {
        std::cout << "FuncName: " << e.FuncName << std::endl;
        std::cout << "ModuleID: " << e.ModuleP->getName().str() << std::endl;
    }



    for(auto e: vector_dirty_path)
    {

        std::cout << "+++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "============node number: " << e.VectorNode.size() << std::endl;
        for (auto x: e.VectorNode)
        {

            std::cout << "ID: " << x.ID << std::endl;
            std::cout << "pred node: ";
            for (auto i: x.PredNodeID)
                std::cout << i << " ";
            std::cout << "succ node: ";
            for (auto i: x.SuccNodeID)
                std::cout << i << " ";
            std::cout << std::endl;
            std::cout << "FlowInDirty: ";
            for (auto i: x.DirtyInID)
                std::cout << i << " ";
            std::cout << std::endl;
            std::cout << "FlowOutDirty: ";
            for (auto i: x.DirtyOutID)
                std::cout << i << " ";
            std::cout << std::endl;

            std::cout << "Instruction " << x.InstrP << " ";

            std::cout << x.InstrP->getOpcodeName();
            Instruction &inst = *(x.InstrP);
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
            std::cout << std::endl;

        }

        std::cout << "============dirty nubmer: " << e.VectorDirty.size() << std::endl;
        for(auto x: e.VectorDirty)
        {
            std::cout << "ID: " << x.ID << std::endl;
            if (x.ValueP)
                std::cout << x.ValueP->getType()->getTypeID() << std::endl;
        }
    }


    for (int i = 0; i < vector_dirty_path.size(); i++)
    {
        //std::cout << "--------------------------------------------------------instrcutions: " << std::endl;
        //printpred(i, 0);
        std::cout << "*************************System call: " << std::endl;
        if (vector_dirty_path[i].VectorAPI.size() != 0)
        {
            for (auto a = --vector_dirty_path[i].VectorAPI.end(); a != vector_dirty_path[i].VectorAPI.begin(); a--)
                std::cout << *a << std::endl;
            std::cout << vector_dirty_path[i].VectorAPI.front() << std::endl;
        }
        std::cout << "*************************dirty source int: " << vector_dirty_path[i].VectorInt.size() << std::endl;
        for (auto a: vector_dirty_path[i].VectorInt)
            std::cout << "valuep: " << vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetIntValue(vector_dirty_path[i].VectorDirty[a].ValueP) << std::endl;
        std::cout << "*************************dirty source float: " << vector_dirty_path[i].VectorFloat.size() << std::endl;
        for (auto a: vector_dirty_path[i].VectorFloat)
            std::cout << vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetFloatValue(vector_dirty_path[i].VectorDirty[a].ValueP) << std::endl;
        std::cout << "*************************dirty source double: " << vector_dirty_path[i].VectorDouble.size() << std::endl;
        for (auto a: vector_dirty_path[i].VectorDouble)
            std::cout << vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetDoubleValue(vector_dirty_path[i].VectorDirty[a].ValueP) << std::endl;
        std::cout << "*************************dirty source string: " << vector_dirty_path[i].VectorString.size() << std::endl;
        for (auto a: vector_dirty_path[i].VectorString)
            std::cout << "valuep: " << vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetStringValue(vector_dirty_path[i].VectorDirty[a].ValueP) << std::endl;
    }


    std::cout << "--------------------------------------------------------------all dump" << std::endl;
    for (auto m: vector_module) {
        for (auto iter1 = m->getFunctionList().begin();
             iter1 != m->getFunctionList().end(); iter1++) {
            Function &f = *iter1;
            std::cout << "Function: " << f.getName().str() << std::endl;
            std::cout << "---arg number: " << f.arg_size() << std::endl;
            std::cout << "---bb number: " << f.getBasicBlockList().size() << std::endl;
            for (auto iter2 = f.getBasicBlockList().begin();
                 iter2 != f.getBasicBlockList().end(); iter2++) {
                BasicBlock &bb = *iter2;
                std::cout << "......BasicBlock: " << &bb << std::endl;
                for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++) {
                    Instruction &inst = *iter3;
                    std::cout << ".........Instruction " << &inst << " : " << inst.getOpcodeName();
                    unsigned int i = 0;
                    unsigned int opnt_cnt = inst.getNumOperands();
                    for (; i < opnt_cnt; ++i) {
                        Value *opnd = inst.getOperand(i);
                        std::string o;
                        //raw_string_ostream os(o);
                        //opnd->print(os);
                        //opnd->printAsOperand(os, true, m);
                        if (opnd->hasName()) {
                            o = opnd->getName();
                            std::cout << " " << o << ",";
                        } else {
                            std::cout << " ptr" << opnd << ",";
                        }
                        //std::cout << "meta data: " << opnd->isUsedByMetadata();
                    }
                    std::cout << std::endl;
                }
            }
        }
    }

}

void printpred(int i, int j)
{
    Node x = vector_dirty_path[i].VectorNode[j];
    std::cout << "Instruction " << x.InstrP << " ";

    std::cout << x.InstrP->getOpcodeName();
    Instruction &inst = *(x.InstrP);
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
    std::cout << std::endl;

    for (auto k: x.PredNodeID)
        printpred(i, k);
}
