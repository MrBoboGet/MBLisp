#include "Evaluator.h"
#include <filesystem>
#include <iostream>
#include <MBUnicode/MBUnicode.h>
#include <MBSystem/MBSystem.h>

#include <MBUtility/MBStrings.h>
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
        int ArgOffset = 2;
        std::string SourceFile = argv[1];
        std::vector<std::string> LispArgv;
        if(argc >= 3 && std::strcmp(argv[1],"-m") == 0)
        {
            ArgOffset = 3;
            std::string TargetFile = argv[2];
            bool MultiPart = false;
            for(auto& Character : TargetFile)
            {
                if(Character == '.')
                {
                    Character = '/';   
                    MultiPart = true;
                }
            }
            TargetFile += ".lisp";
            auto LibDir = MBSystem::GetUserHomeDirectory()/".mblisp/libs";
            std::string NewSourcefile;
            if(MultiPart)
            {
                if(std::filesystem::exists(LibDir/TargetFile))
                {
                    NewSourcefile = MBUnicode::PathToUTF8(LibDir/TargetFile);
                }
            }
            else
            {
                auto DirIt = std::filesystem::directory_iterator(LibDir);
                for(auto const& Entry : DirIt)
                {
                    if(Entry.is_directory())
                    {
                        auto SuggestedFile = Entry.path()/TargetFile;
                        if(std::filesystem::exists(SuggestedFile))
                        {
                            NewSourcefile = MBUnicode::PathToUTF8(SuggestedFile);
                            break;
                        }
                    }
                }
            }
            if(NewSourcefile.empty())
            {
                std::cout<<"Unable to find module with name '"<<argv[2]<<"'"<<std::endl;
                std::exit(1);
            }
            SourceFile = std::move(NewSourcefile);
        }
        for(int i = ArgOffset; i < argc; i++)
        {
            LispArgv.push_back(argv[i]);   
        }
        Evaluator->SetArgv(LispArgv);
        Evaluator->LoadStd();
        Evaluator->Eval(SourceFile);
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
    try
    {
        //Evaluator->Join();
    }
    catch(...)
    {

    }
}
