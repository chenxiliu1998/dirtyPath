//
// Created by hrc on 18-12-3.
//

#include "utils.h"

std::vector<Value*> GetDirty(Instruction* inst_p, std::vector<int> pos)
{
    std::vector<Value*> p_value_vector;
    unsigned int opnt_cnt = inst_p->getNumOperands();
    //get all operands
    if (pos.size() == 0)
    {
        pos = {};
        for (int i = 0; i < opnt_cnt; i++)
        {
            pos.push_back(i);
        }
    }
    for(int i = 0; i < pos.size(); i++)
    {
        int sym = 0;
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
        //std::cout << " value:" << api_int.roundToDouble();
        double OpValue = api_int.roundToDouble();
        return OpValue;
    } else
    {
        std::cout << "[error]IN GetFigureOpValue, can't get value of " << v << " , please check out if call IfStatic before" << std::endl;
    }
}

double GetDoubleValue(Value* v)
{
    if (ConstantFP *CI = dyn_cast<ConstantFP>(v)) {
        APFloat api_int = CI->getValueAPF();
        double fin = api_int.convertToDouble();
        //float fin = api_int.convertToFloat();
        //std::cout << " value:" << fin;
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
        //std::cout << " value:" << fin;
        //double OpValue = api_int.roundToDouble();
        return fin;
    }
}

std::string GetStringValue(Value* opnd)
{
    std::string result;
    Value *firstop;
    Type* type = opnd->getType();
    if(type->getTypeID() == 15)
    {
        if ( ConstantExpr *pCE = dyn_cast<ConstantExpr>(opnd) )
        {
            firstop = pCE->getOperand(0);
            std::cout << "type : " << firstop->getType()->getTypeID();
            if (GlobalVariable *GV = dyn_cast<GlobalVariable>(firstop))
            {
                Constant *v2 = GV->getInitializer();

                if (ConstantDataArray *CA = dyn_cast<ConstantDataArray>(v2))
                {
                    std::cout << "step 3";
                    //std::cout << "value:" << CA->getAsString().str();
                    result = CA->getAsString().str();
                    return result;
                }
            }
        }
    }
    std::cout << "[error]IN GetStringValue: can't get value of " << firstop->getName().str();
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
        	firstop = pCE->getOperand(0);
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

bool IfSysCall(std::string funcname)
{
    for (auto func_def: vector_func_define)
    {
        if (funcname.compare(func_def.FuncName) == 0)
        {
            return false;
        }
    }
    return true;
}
