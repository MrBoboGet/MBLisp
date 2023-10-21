#include "TextModule.h"
#include "../../Evaluator.h"

namespace MBLisp
{
    MBLSP::LineIndex TextModule::CreateLineIndex(String const& Data)
    {
        return MBLSP::LineIndex(Data);
    }
    MBLSP::LineIndex TextModule::CreateLineIndex(MBUtility::StreamReader& Data)
    {
        //easiest implementation, could be faster, but reasonable as one often needs to memory map the file
        //either way when using a line index
        std::string TotalData;
        constexpr size_t ReadSize = 4096;
        char ReadBuffer[ReadSize];
        while(true)
        {
            size_t NewBytes = Data.Read(ReadBuffer,ReadSize);
            TotalData.insert(TotalData.end(),ReadBuffer,ReadBuffer+NewBytes);
            if(NewBytes < ReadSize)
            {
                break;   
            }
        }
        return MBLSP::LineIndex(TotalData);
    }
    Int TextModule::GetPosition(MBLSP::LineIndex const& Index, Int Line,Int Col)
    {
        Int ReturnValue;
        MBLSP::Position Position;
        Position.character = Col;
        Position.line = Line;
        ReturnValue = Index.PositionToByteOffset(Position);
        return ReturnValue;
    }
    Ref<Scope> TextModule::GetModuleScope(Evaluator& AssociatedEvaluator) 
    {
        Ref<Scope> ReturnValue = MakeRef<Scope>();
        AssociatedEvaluator.AddGeneric<static_cast<MBLSP::LineIndex(*)(String const&)>(CreateLineIndex)>(ReturnValue,"line-index");
        AssociatedEvaluator.AddGeneric<static_cast<MBLSP::LineIndex(*)(MBUtility::StreamReader&)>(CreateLineIndex)>(ReturnValue,"line-index");
        AssociatedEvaluator.AddGeneric<GetPosition>(ReturnValue,"get-byte-position");

        return ReturnValue;
    }
}
