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
        StreamTokenizer(StreamTokenizer const&) noexcept = default;
        void SetStream(Ref<MBUtility::StreamReader> Content);
        static StreamTokenizer Construct(std::string const& Skip,List const& Tokens);
        SymbolStore Peek(int Depth);
        void ConsumeToken(int Depth);
    };

    class TextModule : public Module
    {
        static MBLSP::LineIndex CreateLineIndex(String const& Data);
        static MBLSP::LineIndex CreateLineIndex(MBUtility::StreamReader& Data);
        static String JSONEscape(String&);
        static Int GetPosition(MBLSP::LineIndex const& Index, Int Line,Int Col);
        static Int GetLine(MBLSP::LineIndex const& Index, Symbol ByteOffset);
        static Int GetCol(MBLSP::LineIndex const& Index, Symbol ByteOffset);


    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator);
    };
}
