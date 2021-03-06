//
// Created by hrc on 18-12-3.
//

#include "utils.h"

// 通过指令指针inst_p和参数位置pos,获得关键函数的污点变量指针
vector<Value*> GetDirty(Instruction* inst_p, vector<int> pos)
{
    vector<Value*> p_value_vector;
    unsigned int opnt_cnt = inst_p->getNumOperands(); // 获取操作数数量
    //get all operands
    if (pos.size() == 0)
    {
        pos = {};
        for (int i = 0; i < opnt_cnt; i++) //? 最后一个不是call的函数嘛
        {
            pos.push_back(i);
        }
    }
    for(int i = 0; i < pos.size(); i++)
    {
        int sym = 0; //? why sym
        Value *opnd = inst_p->getOperand(pos[i]);
        for (auto j: p_value_vector)
            if (j == opnd)
            {
                sym = 1;
                break;
            }
        if (sym == 0)
            p_value_vector.push_back(opnd);

    }
    return p_value_vector;
}


double GetIntValue(Value *v)
{
    if (ConstantInt *CI = dyn_cast<ConstantInt>(v))
    {
        APInt api_int = CI->getValue();
        //cout << " value:" << api_int.roundToDouble();
        double OpValue = api_int.roundToDouble();
        return OpValue;
    } else
    {
        cout << "[error]IN GetFigureOpValue, can't get value of " << v << " , please check out if call IfStatic before" << endl;
    }
}

double GetDoubleValue(Value* v)
{
    if (ConstantFP *CI = dyn_cast<ConstantFP>(v)) {
        APFloat api_int = CI->getValueAPF();
        double fin = api_int.convertToDouble();
        //float fin = api_int.convertToFloat();
        //cout << " value:" << fin;
        //double OpValue = api_int.roundToDouble();
        return fin;
    }
}

float GetFloatValue(Value* v)
{
    if (ConstantFP *CI = dyn_cast<ConstantFP>(v)) {
        APFloat api_int = CI->getValueAPF();
        //double fin = api_int.convertToDouble();
        float fin = api_int.convertToFloat();
        //cout << " value:" << fin;
        //double OpValue = api_int.roundToDouble();
        return fin;
    }
}

string GetStringValue(Value* opnd)
{
    string result;
    Value *firstop;
    Type* type = opnd->getType();
    if(type->getTypeID() == 15)
    {
        if ( ConstantExpr *pCE = dyn_cast<ConstantExpr>(opnd) )
        {
            firstop = pCE->getOperand(0);
            cout << "type : " << firstop->getType()->getTypeID();
            if (GlobalVariable *GV = dyn_cast<GlobalVariable>(firstop))
            {
                Constant *v2 = GV->getInitializer();

                if (ConstantDataArray *CA = dyn_cast<ConstantDataArray>(v2))
                {
                    cout << "step 3";
                    //cout << "value:" << CA->getAsString().str();
                    result = CA->getAsString().str();
                    return result;
                }
            }
        }
    }
    cout << "[error]IN GetStringValue: can't get value of " << firstop->getName().str();
    return result;
}

int IfStatic(Value *v){
    int class_type = -1;
    if (ConstantInt *CI = dyn_cast<ConstantInt>(v)) {
        if(v->getType()->isIntegerTy(8)){
            class_type = CHAR;
        }
        else
            class_type = INT;
        //return true;
    }
    if (ConstantFP *CI = dyn_cast<ConstantFP>(v)) {
        if(v->getType()->getTypeID() == 2){
            class_type = FLOAT;
        }
        if(v->getType()->getTypeID() == 3){
            class_type = DOUBLE;
        }
    }
    Type * type = v->getType();
    if(type->getTypeID() == 15){
        if ( ConstantExpr *pCE = dyn_cast<ConstantExpr>(v) ){
            Value *firstop = pCE->getOperand(0);
            if (GlobalVariable *GV = dyn_cast<GlobalVariable>(firstop))
            {
                Constant *v2 = GV->getInitializer();
                if (ConstantDataArray *CA = dyn_cast<ConstantDataArray>(v2))
                {
            		class_type = STRING;
            	}
	    } 
        }
    }

    return class_type;
}

bool IfSysCall(string funcname)
{
    for (auto func_def: g_vector_func_define)
    {
        if (funcname.compare(func_def.FuncName) == 0)
        {
            return false;
        }
    }
    return true;
}
