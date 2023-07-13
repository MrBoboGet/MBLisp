#include "Evaluator.h"
#include "MBUtility/MBFiles.h"
#include <filesystem>
#include <iostream>
#include <MBUnicode/MBUnicode.h>
int main(int argc,const char** argv)
{
    if(argc == 1)
    {
        return 0;   
    }
    std::string FileContent = argv[1];
    MBLisp::Evaluator Evaluator;
    try
    {
        Evaluator.LoadStd();
        Evaluator.Eval(FileContent);
    }
    catch(MBLisp::LookupError const& e)
    {
        std::cout<<e.what()<<": "<<Evaluator.GetSymbolString(e.GetSymbol())<<std::endl;
    }
    catch(std::exception const& e)
    {
        std::cout<<e.what()<<std::endl;
    }
}
