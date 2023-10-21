#pragma once
#include "Value.h"
namespace MBLisp
{
    class Evaluator;
    class Module
    {
    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator) =  0;
        virtual ~Module(){};
    };

}
