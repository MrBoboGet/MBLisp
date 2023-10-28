#include "../../Module.h"
#include <MBLSP/TextChanges.h>
#include <MBUtility/StreamReader.h>

namespace MBLisp
{
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
