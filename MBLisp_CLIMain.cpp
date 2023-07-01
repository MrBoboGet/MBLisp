#include "Evaluator.h"
#include "MBUtility/MBFiles.h"
#include <filesystem>
#include <iostream>
int main(int argc,const char** argv)
{
    if(argc == 1)
    {
        return 0;   
    }
    std::cout << std::filesystem::current_path() << std::endl;
    std::string FileContent = MBUtility::ReadWholeFile(argv[1]);
    MBLisp::Evaluator Evaluator;
    Evaluator.Eval(FileContent);
}
