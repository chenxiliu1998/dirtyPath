//
// Created by hrc on 18-12-3.
//

#include "Analyse.h"

void StartAnalyse()
{
    if (g_vector_dirty_position.empty())
    {
        cout << "[ATTENTION]no dirty call in this project" << endl;
        exit(0);
    }
    cout << "[*]IN StartAnalyse" << endl;
    for (int i = 0; i < g_vector_dirty_position.size(); i++) // 遍历污点位置
    {
        DirtyAnalyse(i);
    }
}

// 分析污点位置
void DirtyAnalyse(int pos) {
    cout << "[*]IN DirtyAnalyse" << endl;
    vector<Environment> vector_environment;
    vector<Environment> vector_environment_new;
    DirtyPosition dirty_position = g_vector_dirty_position[pos]; // 得到dirty_position

    // 新建污点路径并添加入g_vector_dirty_path,由于依次分析,故index与g_vector_dirty_position对应
    DirtyPath dirty_path;
    g_vector_dirty_path.push_back(dirty_path);

    g_vector_dirty_path[pos].VectorAPI.push_back(dirty_position.FuncName);

    // 新建节点
    Node tmp_node;
    tmp_node.ID = g_vector_dirty_path[pos].VectorNode.size(); // =0
    // 将污点位置的位置信息赋给node
    tmp_node.ModuleP = dirty_position.ModuleP;
    tmp_node.FuncP = dirty_position.FuncP;
    tmp_node.BBP = dirty_position.BBP;
    tmp_node.InstrP = dirty_position.InstrP;
    tmp_node.SuccNodeID = {-1}; // 后续节点为空

    vector<Value*> p_value_vector = GetDirty(tmp_node.InstrP, {0, 1}); //? vector_op
    // 遍历关键参数,初始化污点Dirty加入当前传播路径g_vector_dirty_path[post]的VectorDirty,并为污点建立env
    for (int i = 0; i < p_value_vector.size(); i++) // 遍历关键参数
    {
        Value* value_p = p_value_vector[i]; // 得到参数
        // 新建污点, 加入当前传播路径的VectorDirty
        Dirty dirty_t;
        dirty_t.ValueP = value_p;
        if (value_p->hasName())
            dirty_t.Name = value_p->getName().str();
        dirty_t.FlowInNodeID.push_back(tmp_node.ID); // 将上文新建的节点作为流入节点
        dirty_t.ID = g_vector_dirty_path[pos].VectorDirty.size(); //? =i
        g_vector_dirty_path[pos].VectorDirty.push_back(dirty_t);

        tmp_node.DirtyInID.push_back(dirty_t.ID); //? 回指
        // 新建env
        Environment env = Environment(pos, tmp_node.ModuleP, tmp_node.FuncP, tmp_node.BBP, tmp_node.InstrP, tmp_node.ID, dirty_t.ID, -1);
        env.last_sys = dirty_position.FuncName;
        env.last_sys_insr = tmp_node.InstrP;
        vector_environment.push_back(env);
    }
    g_vector_dirty_path[pos].VectorNode.push_back(tmp_node); // 当前传播路径节点

    while(!vector_environment.empty())
    {
        //analyse
        for (Environment env: vector_environment)
        {
            vector<int> pred_node = env.GetPredNode();

            //for each in pred node: classify -> update dirty
            for (auto j: pred_node)
            {
                Node t_pred_node = g_vector_dirty_path[pos].VectorNode[j];

                string pred_op_name;
                int t_class = env.classify(j);
                vector<Value *> tmp_value_p = {};
                switch (t_class)
                {
                    case RETURN: //call xxx, xxx, xxx, func
                    {
                        int op_num = t_pred_node.InstrP->getNumOperands();
                        string func_name = t_pred_node.InstrP->getOperand(op_num-1)->getName();

                        //func为系统函数
                        if (IfSysCall(func_name))
                        {
                            //加入dirty_path的VectorAPI
                            g_vector_dirty_path[pos].VectorAPI.push_back(func_name);
                            //获取除函数名以外的参数
                            tmp_value_p = GetDirty(t_pred_node.InstrP, {});
                            tmp_value_p.pop_back();
                        } else
                        {
                            //如果不是系统函数,
                            int visit_flag = 0;
                            for (auto itera = env.visit_name.begin(); itera != env.visit_name.end(); itera++)
                            {
                                string n = *itera;
                                if (n.compare(func_name) == 0)
                                {
                                    visit_flag = 1;
                                    break;
                                }
                            }
                            if (visit_flag == 1) //visit过了，获取函数参数，这时的参数位置由env中的op_pos决定
                            {
                                tmp_value_p = GetDirty(t_pred_node.InstrP, {env.op_pos});
                            } else //没有visit过，需要进入,所以这里的输入dirty为空
                            {
                                Value *null_v = NULL;
                                tmp_value_p.push_back(null_v);
                            }
                        }
                        break;
                    }
                    case NORMAL: //获取所有参数，插入新的dirty
                    {
                        pred_op_name = t_pred_node.InstrP->getOpcodeName();
                        if (pred_op_name.compare("store") == 0)
                        {
                            tmp_value_p = GetDirty(t_pred_node.InstrP, {0});
                        }
                        else
                            tmp_value_p = GetDirty(t_pred_node.InstrP, {});
                        break;
                    }
                    case CALL: //alloc
                    {
                        //如果是CALL，说明当前一条指令为call，这个不需要用dirty找，什么都不用做
                        Value *null_value_p = NULL;
                        tmp_value_p.push_back(null_value_p);
                        break;
                    }

                    case GETELEMENTPTR: //getelementptr
                    {
                        tmp_value_p = GetDirty(t_pred_node.InstrP, {0});
                        //dirty == instrp
                        if ((unsigned long)g_vector_dirty_path[pos].VectorDirty[env.DirtyID].ValueP == (unsigned long)g_vector_dirty_path[pos].VectorNode[j].InstrP)
                        {
                            tmp_value_p = GetDirty(t_pred_node.InstrP, {0});
                        }
                        else if (g_vector_dirty_path[pos].VectorDirty[env.DirtyID].ValueP == tmp_value_p[0])
                        {
                            Value* value_p = (Value*)g_vector_dirty_path[pos].VectorNode[j].InstrP;
                            tmp_value_p.push_back(value_p);
                        }
                        break;
                    }
                } //end switch

                //根据classify的结果处理dirty &&更新env
                vector<int> t_dirty_in;

                for (Value *v: tmp_value_p)
                {
                    Dirty new_dirty;
                    new_dirty.ID = g_vector_dirty_path[pos].VectorDirty.size();
                    if (v != NULL && v->hasName())
                        new_dirty.Name = v->getName();
                    new_dirty.ValueP = v;
                    new_dirty.FlowInNodeID.push_back(t_pred_node.ID);
                    g_vector_dirty_path[pos].VectorDirty.push_back(new_dirty);
                    t_dirty_in.push_back(new_dirty.ID);

                    //update env
                    env.DirtyID = new_dirty.ID; //update dirty id
                    env.CurrentNodeID = t_pred_node.ID; //update current_node
                    //cout << "new current node ID: " << t_pred_node.ID << endl;
                    env.update(); //按照新的instruction更新env
                    vector_environment_new.push_back(env); //将每一个dirty更新后的env放到新的env向量中
                } //end for value
                g_vector_dirty_path[pos].VectorNode[t_pred_node.ID].DirtyInID = t_dirty_in;
            } //end for each pred node
        } //end for each vector_env
        vector_environment = vector_environment_new;
        cout << "vector environment new is: " << vector_environment.size() << endl;
        vector_environment_new = {};
    }
}
