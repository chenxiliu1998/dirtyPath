//
// Created by hrc on 18-12-3.
//
#include "test.h"
void printpred(int, int);

void test()
{

    cout << "*************************************dirty_api size: " << g_vector_dirty_position.size() << endl;
    for(auto e: g_vector_dirty_position)
    {
        cout << "FuncName: " << e.FuncName << endl;
        cout << "ModuleID: " << e.ModuleP-> getName().str()<< endl;
    }
    cout << "************************************func_define size: " << g_vector_func_define.size() << endl;
    for(auto e: g_vector_func_define)
    {
        cout << "FuncName: " << e.FuncName << endl;
        cout << "ModuleID: " << e.ModuleP->getName().str() << endl;
    }
    cout << "*************************************func_call size: " << g_vector_func_call.size() << endl;
    for (auto e: g_vector_func_call)
    {
        cout << "FuncName: " << e.FuncName << endl;
        cout << "ModuleID: " << e.ModuleP->getName().str() << endl;
    }



    for(auto e: g_vector_dirty_path)
    {

        cout << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "============node number: " << e.VectorNode.size() << endl;
        for (auto x: e.VectorNode)
        {

            cout << "ID: " << x.ID << endl;
            cout << "pred node: ";
            for (auto i: x.PredNodeID)
                cout << i << " ";
            cout << "succ node: ";
            for (auto i: x.SuccNodeID)
                cout << i << " ";
            cout << endl;
            cout << "FlowInDirty: ";
            for (auto i: x.DirtyInID)
                cout << i << " ";
            cout << endl;
            cout << "FlowOutDirty: ";
            for (auto i: x.DirtyOutID)
                cout << i << " ";
            cout << endl;

            cout << "Instruction " << x.InstrP << " ";

            cout << x.InstrP->getOpcodeName();
            Instruction &inst = *(x.InstrP);
            unsigned int opnt_cnt = inst.getNumOperands();

            for (int i = 0; i < opnt_cnt; ++i)
            {
                Value *opnd = inst.getOperand(i);
                string o;
                if (opnd->hasName()) {
                    o = opnd->getName();
                    cout << " " << o << ",";
                } else {
                    cout << " ptr" << opnd << ",";
                }
            }
            cout << endl;
            cout << endl;

        }

        cout << "============dirty nubmer: " << e.VectorDirty.size() << endl;
        for(auto x: e.VectorDirty)
        {
            cout << "ID: " << x.ID << endl;
            if (x.ValueP)
                cout << x.ValueP->getType()->getTypeID() << endl;
        }
    }


    for (int i = 0; i < g_vector_dirty_path.size(); i++)
    {
        //cout << "--------------------------------------------------------instrcutions: " << endl;
        //printpred(i, 0);
        cout << "*************************System call: " << endl;
        if (g_vector_dirty_path[i].VectorAPI.size() != 0)
        {
            for (auto a = --g_vector_dirty_path[i].VectorAPI.end(); a != g_vector_dirty_path[i].VectorAPI.begin(); a--)
                cout << *a << endl;
            cout << g_vector_dirty_path[i].VectorAPI.front() << endl;
        }
        cout << "*************************dirty source int: " << g_vector_dirty_path[i].VectorInt.size() << endl;
        for (auto a: g_vector_dirty_path[i].VectorInt)
            cout << "valuep: " << g_vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetIntValue(g_vector_dirty_path[i].VectorDirty[a].ValueP) << endl;
        cout << "*************************dirty source float: " << g_vector_dirty_path[i].VectorFloat.size() << endl;
        for (auto a: g_vector_dirty_path[i].VectorFloat)
            cout << g_vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetFloatValue(g_vector_dirty_path[i].VectorDirty[a].ValueP) << endl;
        cout << "*************************dirty source double: " << g_vector_dirty_path[i].VectorDouble.size() << endl;
        for (auto a: g_vector_dirty_path[i].VectorDouble)
            cout << g_vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetDoubleValue(g_vector_dirty_path[i].VectorDirty[a].ValueP) << endl;
        cout << "*************************dirty source string: " << g_vector_dirty_path[i].VectorString.size() << endl;
        for (auto a: g_vector_dirty_path[i].VectorString)
            cout << "valuep: " << g_vector_dirty_path[i].VectorDirty[a].ValueP << " = " << GetStringValue(g_vector_dirty_path[i].VectorDirty[a].ValueP) << endl;
    }


    cout << "--------------------------------------------------------------all dump" << endl;
    for (auto m: g_vector_module) {
        for (auto iter1 = m->getFunctionList().begin();
             iter1 != m->getFunctionList().end(); iter1++) {
            Function &f = *iter1;
            cout << "Function: " << f.getName().str() << endl;
            cout << "---arg number: " << f.arg_size() << endl;
            cout << "---bb number: " << f.getBasicBlockList().size() << endl;
            for (auto iter2 = f.getBasicBlockList().begin();
                 iter2 != f.getBasicBlockList().end(); iter2++) {
                BasicBlock &bb = *iter2;
                cout << "......BasicBlock: " << &bb << endl;
                for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++) {
                    Instruction &inst = *iter3;
                    cout << ".........Instruction " << &inst << " : " << inst.getOpcodeName();
                    unsigned int i = 0;
                    unsigned int opnt_cnt = inst.getNumOperands();
                    for (; i < opnt_cnt; ++i) {
                        Value *opnd = inst.getOperand(i);
                        string o;
                        //raw_string_ostream os(o);
                        //opnd->print(os);
                        //opnd->printAsOperand(os, true, m);
                        if (opnd->hasName()) {
                            o = opnd->getName();
                            cout << " " << o << ",";
                        } else {
                            cout << " ptr" << opnd << ",";
                        }
                        //cout << "meta data: " << opnd->isUsedByMetadata();
                    }
                    cout << endl;
                }
            }
        }
    }

}

void printpred(int i, int j)
{
    Node x = g_vector_dirty_path[i].VectorNode[j];
    cout << "Instruction " << x.InstrP << " ";

    cout << x.InstrP->getOpcodeName();
    Instruction &inst = *(x.InstrP);
    unsigned int opnt_cnt = inst.getNumOperands();

    for (int i = 0; i < opnt_cnt; ++i)
    {
        Value *opnd = inst.getOperand(i);
        string o;
        if (opnd->hasName()) {
            o = opnd->getName();
            cout << " " << o << ",";
        } else {
            cout << " ptr" << opnd << ",";
        }
    }
    cout << endl;
    cout << endl;

    for (auto k: x.PredNodeID)
        printpred(i, k);
}
