#include "Value.h"
#include <algorithm>
#include "assert.h"
namespace MBLisp
{
       
    //Begin Scope
    void Scope::SetParentScope(std::shared_ptr<Scope> ParentScope)
    {
        m_ParentScope = ParentScope;
    }
    Value Scope::FindVariable(SymbolID Variable)
    {
        Value* ReturnValue = TryGet(Variable);
        if(ReturnValue == nullptr)
        {
            throw LookupError(Variable);
        }
        return *ReturnValue;
    }
    Value* Scope::TryGet(SymbolID Variable)
    {
        Value* ReturnValue = nullptr;
        if(auto VarIt = m_Variables.find(Variable); VarIt != m_Variables.end())
        {
            return &VarIt->second;
        }
        else if(m_ParentScope != nullptr)
        {
            ReturnValue = m_ParentScope->TryGet(Variable);
        }
        return ReturnValue;
    }
    void Scope::SetVariable(SymbolID Variable,Value NewValue)
    {
        if(m_ParentScope != nullptr)
        {
            if(auto It = m_ParentScope->TryGet(Variable); It != nullptr)
            {
                *It = std::move(NewValue);
                return;
            }
        }
        m_Variables[Variable] = std::move(NewValue);
    }
    void Scope::OverrideVariable(SymbolID Variable,Value NewValue)
    {
        m_Variables[Variable] = std::move(NewValue);
    }
    //END Scope
    ClassID ClassIdentificator::m_CurrentID = 1<<ExternalClassBit;


    //GenericFunction::GenericFunction(std::vector<SymbolID> Arguments)
    //{
    //    m_Arguments = std::move(Arguments);
    //    for(int i = 0; i < m_Arguments.size();i++)
    //    {
    //        m_Specifications.push_back(std::vector<std::pair<ClassID,size_t>>());
    //    }
    //}
    void GenericFunction::AddMethod(std::vector<ClassID> OverridenTypes,Value Callable)
    {
        size_t NewIndex = m_Callables.size(); 
        m_Callables.push_back(Callable);
        for(int i = 0; i < OverridenTypes.size();i++)
        {
            if(m_Specifications.size() <= i)
            {
                m_Specifications.emplace_back();   
            }
            m_Specifications[i].push_back(std::make_pair(OverridenTypes[i],NewIndex));
            std::sort(m_Specifications[i].begin(),m_Specifications[i].end(),[](std::pair<ClassID,size_t> const& Lhs,std::pair<ClassID,size_t> const& Rhs)
                    {
                        return Lhs.first < Rhs.first;
                    });
        }
        m_CallablesOverrides.push_back(std::move(OverridenTypes));

    }
    bool GenericFunction::p_TypeIsSatisfied(ClassID Override,Value const& Arg)
    {
        bool ReturnValue = true;
        if(Override == 0)
        {
            return true;
        }
        if(Arg.IsType<ClassInstance>())
        {
            auto const& Types = Arg.GetType<ClassInstance>().AssociatedClass->Types;
            if(auto It = std::lower_bound(Types.begin(),Types.end(),Override);
                    !(It != Types.end() && *It == Override))
            {
                return false;
            }
        }
        else
        {
            return Override == Arg.GetTypeID();
        }
        return ReturnValue;
    }
    bool GenericFunction::p_TypesAreSatisifed(std::vector<ClassID> const& Overrides,std::vector<Value> const& ArgumentsClasses)
    {
        bool ReturnValue = true;
        if(Overrides.size() > ArgumentsClasses.size())
        {
            return false;   
        }
        for(int i = 0; i < Overrides.size();i++)
        {
            if(!p_TypeIsSatisfied(Overrides[i],ArgumentsClasses[i]))
            {
                return false;   
            }
        }

        return ReturnValue;
    }
    Value* GenericFunction::GetMethod(std::vector<Value>& Arguments)
    {
        Value* ReturnValue = nullptr;
        if(m_Specifications.size() == 0)
        {
            return ReturnValue;   
        }
        for(int i = int(m_Specifications[0].size())-1; i >= 0; i--)
        {
            if(p_TypesAreSatisifed(m_CallablesOverrides[m_Specifications[0][i].second],Arguments))
            {
                return &m_Callables[m_Specifications[0][i].second];
            }
        }
        return ReturnValue;
    }
}
