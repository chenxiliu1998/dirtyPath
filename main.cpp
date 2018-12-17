#include "SearchDirtyDest.h"
#include "test.h"
#include "Analyse.h"
#include "head.h"
#include "Environment.h"

//global para
unique_ptr<Module> g_array_ptr[10];
vector<Module*> g_vector_module;
vector<Function*> vector_function;
vector<BasicBlock*> vector_bb;
vector<Instruction*> vector_instr;
vector<DirtyPosition> g_vector_dirty_position;
vector<FuncCall> g_vector_func_call;
vector<FuncDefine> g_vector_func_define;
vector<DirtyPath> g_vector_dirty_path;
vector<Environment> vector_environment;


int main(int argc, char *argv[]) {

    //cout << g_vector_dirty_position.size() << endl;
    if (argc < 2)
    {
        errs() << "Expected an argument - IR file name\n";
        exit(1);
    }
    LLVMContext con;
    LLVMContext &Context = con;
    SMDiagnostic err;
    SMDiagnostic &Err = err;

    // 按照顺序依次读取参数指定文件的Module,赋值给g_vector_module(一个文件生成一个Module)
    //init g_vector_module
    for(int i = 1; i < argc; i++)
    {
        g_array_ptr[i-1] = parseIRFile(argv[i], Err, Context);
        Module* m = g_array_ptr[i-1].get();
        g_vector_module.push_back(m);
    }
    cout << "number of modules : " << g_vector_module.size()<< endl;
    cout << "*****************************" << endl;

    //search dirty dest and func define
    SearchDirtyDest();
    StartAnalyse();
    test();

    //free memory
    for(int i = 0; i < 10; i++)
        g_array_ptr[i] = nullptr;
    return 0;
}
