#include "Evaluator.h"
#include "MBUtility/MBFiles.h"
#include <filesystem>
#include <iostream>
#include <MBSystem/MBSystem.h>
#include <MBUnicode/MBUnicode.h>
int main(int argc,const char** argv)
{
    if(argc == 1)
    {
        return 0;   
    }
    std::string FileContent = MBUtility::ReadWholeFile(argv[1]);
    MBLisp::Evaluator Evaluator;
    std::shared_ptr<MBLisp::Scope> EvalScope = std::make_shared<MBLisp::Scope>();
    //source standard library if it exists
    if(std::filesystem::exists(MBSystem::GetUserHomeDirectory()/".mblisp/libs/std/index.lisp"))
    {
        std::string StandardLibrary = MBUtility::ReadWholeFile(
                MBUnicode::PathToUTF8(MBSystem::GetUserHomeDirectory()/".mblisp/libs/std/index.lisp"));
        Evaluator.Eval(EvalScope,StandardLibrary);
    }
    Evaluator.Eval(EvalScope,FileContent);
}
