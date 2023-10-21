#include "../../Module.h"
#include <MBLSP/TextChanges.h>
#include <MBUtility/StreamReader.h>

namespace MBLisp
{
    class TextModule : public Module
    {
        static MBLSP::LineIndex CreateLineIndex(String const& Data);
        static MBLSP::LineIndex CreateLineIndex(MBUtility::StreamReader& Data);
        static Int GetPosition(MBLSP::LineIndex const& Index, Int Line,Int Col);

    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator);
    };
}
