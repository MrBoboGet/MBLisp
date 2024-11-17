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
        m_Tokenizer.SetParseOffset(m_Stream->Position());
        m_LastConsumedByteOffset = m_Stream->Position();
    }
    SymbolStore StreamTokenizer::Peek(Int Depth)
    {
        SymbolStore ReturnValue;
        auto Result = m_Tokenizer.Peek(Depth);

        ReturnValue["value"] = Result.Value;
        ReturnValue["type"] = Value(Result.Type);
        ReturnValue["position"] = Int(Result.ByteOffset);

        return ReturnValue;
    }
    void StreamTokenizer::ConsumeToken()
    {
        size_t CurrentByteOffset = m_Tokenizer.Peek().ByteOffset;
        CurrentByteOffset += m_Tokenizer.Peek().Value.size();
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
    List TextModule::SplitQuoted(String const& Input,String const& QuoteString,String const& EscapeString)
    {
        List ReturnValue;
        if(Input.size() == 0)
        {
            ReturnValue.push_back(Value(""));
            return ReturnValue;
        }
        if(EscapeString.size() == 0)
        {
            throw std::runtime_error("Invalid escape string: escape string was emtpy");
        }
        size_t ParseOffset = 0;
        while(ParseOffset < Input.size())
        {
            size_t NextQuote = Input.find(QuoteString,ParseOffset);
            size_t InsertOffset = ParseOffset;
            String NewEntry;
            while(NextQuote != Input.npos && NextQuote != 0)
            {
                bool IsEscaped = false;
                int EscapeOffset = NextQuote-1;
                while(EscapeOffset >= 0)
                {
                    if(Input[EscapeOffset] != EscapeString[0])
                    {
                        break;
                    }
                    else
                    {
                        IsEscaped = !IsEscaped;
                    }
                    EscapeOffset--;
                }
                NewEntry.insert(NewEntry.end(),Input.data()+InsertOffset,Input.data()+EscapeOffset+1);
                InsertOffset = NextQuote;
                for(int i = 0; i < ((NextQuote-1)-EscapeOffset)/2;i++)
                {
                    NewEntry += EscapeString[0];
                }
                if(IsEscaped)
                {
                    NewEntry += QuoteString;
                    InsertOffset = NextQuote+QuoteString.size();
                    NextQuote = Input.find(QuoteString,NextQuote+QuoteString.size());
                }
                else
                {
                    break;
                }
            }
            if(NextQuote != QuoteString.npos)
            {
                NewEntry += Input.substr(InsertOffset,NextQuote-InsertOffset);
                ReturnValue.push_back(std::move(NewEntry));
                ParseOffset = NextQuote+QuoteString.size();
                if(ParseOffset == Input.size())
                {
                    ReturnValue.push_back(Value(""));
                }
            }
            else
            {
                NewEntry += Input.substr(InsertOffset,Input.size()-InsertOffset);
                ReturnValue.push_back(std::move(NewEntry));
                break;
            }
        }
        return ReturnValue;
    }
    List TextModule::SplitQuoted_Simple(String const& Input,String const& QuoteString)
    {
        return SplitQuoted(Input,QuoteString,"\\");
    }
    bool TextModule::IsWhitespace(String const& Input)
    {
        bool ReturnValue = false;
        size_t ParseOffset = 0;
        MBParsing::SkipWhitespace(Input.data(),Input.size(),ParseOffset,&ParseOffset);
        if(ParseOffset == Input.size())
        {
            return true;
        }
        return ReturnValue;
    }
    String TextModule::Trim(String const& Input)
    {
        String ReturnValue;
        size_t Offset = 0;
        MBParsing::SkipWhitespace(Input.data(),Input.size(),Offset,&Offset);
        size_t EndOffset = Input.find_last_not_of("\n\t \r");
        EndOffset = EndOffset == Input.npos ? Input.size() : EndOffset+1;
        ReturnValue.append(Input.data()+Offset,Input.data()+EndOffset);
        return ReturnValue;
    }
    std::regex TextModule::Regex(std::string const& regex)
    {
        std::regex ReturnValue = std::regex(regex,std::regex_constants::ECMAScript);
        return ReturnValue;
    }
    bool TextModule::Matching(std::regex const& Regex,String const& Text)
    {
        bool ReturnValue = std::regex_match(Text.begin(),Text.end(),Regex);
        return ReturnValue;
    }
    String TextModule::Match(std::regex const& Regex,String const& Text)
    {
        String ReturnValue;
        std::smatch Matches;
        bool Result = std::regex_search(Text.begin(),Text.end(),Matches,Regex);
        if(Result)
        {
            ReturnValue = Matches.str();
        }
        return ReturnValue;
    }
    String TextModule::Substitute(std::regex const& Regex,String const& OriginalString,String const& ReplacementString)
    {
        String const& TargetString = OriginalString;
        String Result = std::regex_replace(TargetString,Regex,ReplacementString);
        return Result;
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


    //FUNCS

    //

    static String Read_Regex(MBUtility::StreamReader& Stream,std::regex& Regex)
    {
        String ReturnValue;
        auto Begin = Stream.begin();
        auto End = Stream.end();
        std::match_results<decltype(Begin)> Match;
        std::regex_search(Begin,End,Match,Regex,std::regex_constants::match_continuous);
        if(Match.size() > 0)
        {
            ReturnValue = Match[0].str();
            size_t SkipCount = ReturnValue.size();
            Begin.Increment(SkipCount);
            Stream.Consume(Begin,SkipCount);
        }
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
        AssociatedEvaluator.AddGeneric<SplitQuoted_Simple>(ReturnValue,"split-quoted");
        AssociatedEvaluator.AddGeneric<SplitQuoted>(ReturnValue,"split-quoted");
        AssociatedEvaluator.AddGeneric<IsWhitespace>(ReturnValue,"is-whitespace");

        //regex stuff
        AssociatedEvaluator.AddGeneric<Regex>(ReturnValue,"regex");
        AssociatedEvaluator.AddGeneric<Matching>(ReturnValue,"matching");
        AssociatedEvaluator.AddGeneric<Match>(ReturnValue,"match");
        AssociatedEvaluator.AddGeneric<Substitute>(ReturnValue,"substitute");
        //

        //StreamTokenizer
        
        AssociatedEvaluator.AddMethod<SymbolStore,Symbol>("index",SymbolStore::Index);
        AssociatedEvaluator.AddGeneric<&StreamTokenizer::Construct>(ReturnValue,"tokenizer");
        AssociatedEvaluator.AddGeneric<GenerateParser>(ReturnValue,"generate-parser");
        AssociatedEvaluator.AddObjectMethod<&StreamTokenizer::SetStream>(ReturnValue,"set-stream");
        AssociatedEvaluator.AddObjectMethod<&StreamTokenizer::Peek>(ReturnValue,"peek");
        AssociatedEvaluator.AddObjectMethod<&StreamTokenizer::ConsumeToken>(ReturnValue,"consume-token");
        AssociatedEvaluator.AddGeneric<Read_Regex>(ReturnValue,"read-regex");

        AssociatedEvaluator.AddGeneric<Trim>(ReturnValue,"trim");
        
        constexpr bool IsRef = IsTemplateInstantiation<Ref<MBUtility::StreamReader>,Ref>::value;
        return ReturnValue;
    }
}
