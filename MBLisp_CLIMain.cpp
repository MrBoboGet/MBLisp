#include "Evaluator.h"
#include <filesystem>
#include <iostream>
#include <MBUnicode/MBUnicode.h>
using namespace std::chrono;
int main(int argc,const char** argv)
{
    std::shared_ptr<MBLisp::Evaluator> Evaluator = MBLisp::Evaluator::CreateEvaluator();
    try
    {
        if(argc == 1)
        {
            Evaluator->Repl();
        }
        std::string FileContent = argv[1];
        Evaluator->LoadStd();
        Evaluator->Eval(FileContent);
    }
    catch(MBLisp::LookupError const& e)
    {
        std::cout<<e.what()<<": "<<Evaluator->GetSymbolString(e.GetSymbol())<<std::endl;
    }
    catch (MBLisp::UncaughtSignal& e)
    {
        std::cout<<"Uncaught signal:";
        Evaluator->Eval(e.AssociatedScope, e.AssociatedScope->FindVariable(Evaluator->GetSymbolID("print")), {e.ThrownValue});
    }
    catch(std::exception const& e)
    {
        std::cout<<e.what()<<std::endl;
    }
}
