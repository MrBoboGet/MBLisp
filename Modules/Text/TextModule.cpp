#include "TextModule.h"
#include "../../Evaluator.h"

#include <MBCC/MBCC.h>
#include <MBCC/Compilers/MBLisp.h>

#include <MBUtility/StreamUtils.h>

namespace MBLisp
{

    //
    Value& SymbolStore::operator[](std::string const& String)
    {
        return m_Contents[String];
    }
    Value SymbolStore::Index BUILTIN_ARGLIST
    {
        auto& Store = Arguments[0].GetType<SymbolStore>();
        auto const& AssociatedString = Context.GetEvaluator().GetSymbolString(Arguments[1].GetType<Symbol>().ID);
        if(Context.IsSetting())
        {
            Store[AssociatedString] = Context.GetSetValue();
            return Value();
        }
        return Store[AssociatedString];
    }
    //
   
    //
    StreamTokenizer::StreamTokenizer(std::string const& Skip,List const& Tokens)
    {
        std::vector<std::string> Regexes;
        for(auto const& Regex : Tokens)
        {
            if(!Regex.IsType<String>())
            {
                throw std::runtime_error("StreamTokenizer needs list of regexes");
            }
            Regexes.emplace_back(Regex.GetType<String>());
        }
        m_Tokenizer.SetRegexes(Skip,Regexes);
    }
    StreamTokenizer StreamTokenizer::Construct(std::string const& Skip,List const& Tokens)
    {
        return  StreamTokenizer(Skip,Tokens);
    }
    void StreamTokenizer::SetStream(Ref<MBUtility::StreamReader> Content)
    {
        m_Stream = Content;
        m_Tokenizer.SetData(m_Stream->begin(),m_Stream->end());
    }
    SymbolStore StreamTokenizer::Peek(Int Depth)
    {
        SymbolStore ReturnValue;
        auto Result = m_Tokenizer.Peek(Depth);

        ReturnValue["value"] = Result.Value;
        ReturnValue["type"] = Value(Result.Type);

        return ReturnValue;
    }
    void StreamTokenizer::ConsumeToken()
    {
        size_t CurrentByteOffset = m_Tokenizer.Peek().ByteOffset;
        m_Stream->Consume(m_Tokenizer.GetFront(),CurrentByteOffset-m_LastConsumedByteOffset);
        m_LastConsumedByteOffset = CurrentByteOffset;
        m_Tokenizer.ConsumeToken();
    }

    //
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
    String TextModule::GenerateParser(MBUtility::StreamReader& Content,Int k)
    {
        String ReturnValue;
        std::string Contents = MBUtility::ReadAll( static_cast<MBUtility::IndeterminateInputStream&>(Content));
        MBCC::MBCCDefinitions Definitions = MBCC::MBCCDefinitions::ParseDefinitions(Contents.data(),Contents.size(),0);
        MBCC::GrammarOptions Options;
        Options.k = k;
        MBCC::ParserInfo Info = MBCC::ParserCompilerHandler::CreateParserInfo(std::move(Definitions),std::move(Options));
        MBCC::LispParser Parser;
        MBUtility::MBStringOutputStream OutStream(ReturnValue);
        Parser.WriteParser(Info.Grammar,Info.LOOK,OutStream);
        return ReturnValue;
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

        //StreamTokenizer
        
        AssociatedEvaluator.AddMethod<SymbolStore,Symbol>("index",SymbolStore::Index);
        AssociatedEvaluator.AddGeneric<&StreamTokenizer::Construct>(ReturnValue,"tokenizer");
        AssociatedEvaluator.AddGeneric<GenerateParser>(ReturnValue,"generate-parser");
        AssociatedEvaluator.AddObjectMethod<&StreamTokenizer::SetStream>(ReturnValue,"set-stream");
        AssociatedEvaluator.AddObjectMethod<&StreamTokenizer::Peek>(ReturnValue,"peek");
        AssociatedEvaluator.AddObjectMethod<&StreamTokenizer::ConsumeToken>(ReturnValue,"consume-token");
        
        constexpr bool IsRef = IsTemplateInstantiation<Ref<MBUtility::StreamReader>,Ref>::value;
        return ReturnValue;
    }
}
