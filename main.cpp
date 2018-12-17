#include "SearchDirtyDest.h"
#include "test.h"
#include "Analyse.h"
#include "head.h"
#include "Environment.h"
using namespace llvm;

//global para
std::unique_ptr<Module> array_ptr[10];
std::vector<Module*> vector_module;
std::vector<Function*> vector_function;
std::vector<BasicBlock*> vector_bb;
std::vector<Instruction*> vector_instr;
std::vector<DirtyPosition> vector_dirty_position;
std::vector<FuncCall> vector_func_call;
std::vector<FuncDefine> vector_func_define;
std::vector<DirtyPath> vector_dirty_path;
std::vector<Environment> vector_environment;


int main(int argc, char *argv[]) {

    //std::cout << vector_dirty_position.size() <<std::endl;
    if (argc < 2)
    {
        errs() << "Expected an argument - IR file name\n";
        exit(1);
    }
    LLVMContext con;
    LLVMContext &Context = con;
    SMDiagnostic err;
    SMDiagnostic &Err = err;

    //init vector_module
    for(int i = 1; i < argc; i++)
    {
        array_ptr[i-1] = parseIRFile(argv[i], Err, Context);
        Module* m = array_ptr[i-1].get();
        vector_module.push_back(m);
    }
    std::cout << "number of modules : " << vector_module.size()<< std::endl;
    std::cout << "*****************************" << std::endl;

    //search dirty dest and func define
    SearchDirtyDest();
    StartAnalyse();
    test();

    //free memory
    for(int i = 0; i < 10; i++)
        array_ptr[i] = nullptr;
    return 0;
}