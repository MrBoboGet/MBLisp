#pragma once
#include "../../Module.h"

namespace MBLisp
{
    class DBModule : public Module
    {
    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator) override;
    };
};
