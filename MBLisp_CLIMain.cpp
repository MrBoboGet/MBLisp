#include "Evaluator.h"
#include "MBUtility/MBFiles.h"
int main(int argc,const char** argv)
{
    if(argc == 1)
    {
        return 0;   
    }
    std::string FileContent = MBUtility::ReadWholeFile(argv[1]);
    MBLisp::Evaluator Evaluator;
    Evaluator.Eval(FileContent);
}
