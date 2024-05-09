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
    class ScopeModule : public Module
    {
        Ref<Scope> m_Module;
    public:
        ScopeModule(Ref<Scope> Scope)
        {
            m_Module = Scope;
        }
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator)  override
        {
            return m_Module;
        }
    };
}
