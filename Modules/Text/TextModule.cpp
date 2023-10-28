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
    String TextModule::JSONEscape(String& StringToEscape)
    {
        String ReturnValue;
        for(char Character : StringToEscape)
        {
            if(Character == '\\')
            {
                ReturnValue += "\\\\";   
            }
            else if(Character <= 31)
            {
                if(Character == '\n')
                {
                    ReturnValue += "\\n";
                }
                else if(Character == '\t')
                {
                    ReturnValue += "\\t";
                }
                else if(Character == '\r')
                {
                    ReturnValue += "\\r";
                }
            }
            else if(Character == '"')
            {
                ReturnValue += "\\\"";   
            }
            else
            {
                ReturnValue += Character;
            }
        }
        return ReturnValue;
    }
    Int TextModule::GetPosition(MBLSP::LineIndex const& Index, Int Line,Int Col)
    {
        Int ReturnValue;
        MBLSP::Position Position;
        if(Col == -1)
        {
            Col = 0;   
        }
        Position.character = Col;
        Position.line = Line;
        ReturnValue = Index.PositionToByteOffset(Position);
        return ReturnValue;
    }
    Int TextModule::GetLine(MBLSP::LineIndex const& Index, Symbol ByteOffset)
    {
        MBLSP::Position BytePosition = Index.ByteOffsetToPosition(ByteOffset.SymbolLocation.Position);
        return BytePosition.line;
    }
    Int TextModule::GetCol(MBLSP::LineIndex const& Index, Symbol ByteOffset)
    {
        MBLSP::Position BytePosition = Index.ByteOffsetToPosition(ByteOffset.SymbolLocation.Position);
        return BytePosition.character;
    }
    Ref<Scope> TextModule::GetModuleScope(Evaluator& AssociatedEvaluator) 
    {
        Ref<Scope> ReturnValue = MakeRef<Scope>();
        AssociatedEvaluator.AddGeneric<static_cast<MBLSP::LineIndex(*)(String const&)>(CreateLineIndex)>(ReturnValue,"line-index");
        AssociatedEvaluator.AddGeneric<static_cast<MBLSP::LineIndex(*)(MBUtility::StreamReader&)>(CreateLineIndex)>(ReturnValue,"line-index");
        AssociatedEvaluator.AddGeneric<GetPosition>(ReturnValue,"get-byte-position");
        AssociatedEvaluator.AddGeneric<GetLine>(ReturnValue,"get-line");
        AssociatedEvaluator.AddGeneric<GetCol>(ReturnValue,"get-col");
        AssociatedEvaluator.AddGeneric<JSONEscape>(ReturnValue,"json-escape");

        return ReturnValue;
    }
}
