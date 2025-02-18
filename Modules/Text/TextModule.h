#include "../../Module.h"
#include <MBLSP/TextChanges.h>
#include <MBUtility/StreamReader.h>
#include <MBCC/Tokenizer.h>

namespace MBLisp
{
    class SymbolStore
    {
        std::unordered_map<std::string,Value> m_Contents;
    public:
        Value& operator[](std::string const& String);
        static Value Index BUILTIN_ARGLIST;
    };

    class StreamTokenizer
    {
        Ref<MBUtility::StreamReader> m_Stream;

        typedef decltype(std::declval<MBUtility::StreamReader>().begin()) ItType;
        MBCC::TokenizerBase<ItType> m_Tokenizer;

        size_t m_LastConsumedByteOffset = 0;
    public:
        StreamTokenizer(std::string const& Skip,List const& Tokens);
        StreamTokenizer(StreamTokenizer&&) noexcept = default;
        StreamTokenizer(StreamTokenizer const&) = default;
        void SetStream(Ref<MBUtility::StreamReader> Content);
        static StreamTokenizer Construct(std::string const& Skip,List const& Tokens);
        SymbolStore Peek(Int Depth);
        void ConsumeToken();
    };

    class TextModule : public Module
    {
        static MBLSP::LineIndex CreateLineIndex(String const& Data);
        static MBLSP::LineIndex CreateLineIndex(MBUtility::StreamReader& Data);
        static String JSONEscape(String&);
        static Int GetPosition(MBLSP::LineIndex const& Index, Int Line,Int Col);
        static Int GetLine(MBLSP::LineIndex const& Index, Symbol ByteOffset);
        static Int GetCol(MBLSP::LineIndex const& Index, Symbol ByteOffset);

        static List SplitQuoted(String const& Input,String const& QuoteString,String const& EscapeString);
        static List SplitQuoted_Simple(String const& Input,String const& QuoteString);
        static bool IsWhitespace(String const& Input);
        static String Trim(String const& Input);

        //
        static std::regex Regex(std::string const&);
        static bool Matching(std::regex const& Regex,String const& Text);
        static String Match(std::regex const& Regex,String const& Text);
        static String Substitute(std::regex const& Regex,String const& OriginalString,String const& ReplacementString);
        //
        
        static String GenerateParser(MBUtility::StreamReader& Content,Int k);
    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator);
    };
}
