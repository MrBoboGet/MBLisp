#include "Value.h"
#include <algorithm>
#include "assert.h"
namespace MBLisp
{
       
    //Begin Scope
    void Scope::SetParentScope(Ref<Scope> NewScope)
    {
        //ghetto clear
        while(m_ParentScope.size() > 0)
        {
            m_ParentScope.pop_back();   
        }
        ParentScope NewParent;
        NewParent.AssociatedScope = NewScope;
        m_ParentScope.push_back(std::move(NewParent));
    }
    void Scope::AddParentScope(Ref<Scope> NewScope)
    {
        ParentScope NewParent;
        NewParent.AssociatedScope = NewScope;
        m_ParentScope.push_back(NewParent);
    }
    void Scope::SetShadowingParent(Ref<Scope> NewScope)
    {
        //ghetto clear
        while(m_ParentScope.size() > 0)
        {
            m_ParentScope.pop_back();   
        }
        ParentScope NewParent;
        NewParent.AssociatedScope = NewScope;
        NewParent.Shadowing = true;
        m_ParentScope.push_back(std::move(NewParent));
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
        else if(m_ParentScope.size() != 0)
        {
            for(auto& Parent : m_ParentScope)
            {
                ReturnValue = Parent.AssociatedScope->TryGet(Variable);
                if(ReturnValue != nullptr)
                {
                    break;   
                }
            }
        }
        return ReturnValue;
    }
    void Scope::Clear()
    {
        m_Variables.clear();
    }
    void Scope::SetVariable(SymbolID Variable,Value NewValue)
    {
        for(auto& Parent : m_ParentScope)  ///m_ParentScope.AssociatedScope != nullptr && !m_ParentScope.Shadowing)
        {
            if(Parent.Shadowing) 
                continue;
            if(auto It = Parent.AssociatedScope->TryGet(Variable); It != nullptr)
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
    bool GenericFunction::p_TypesAreSatisifed(std::vector<ClassID> const& Overrides,Value const* Begin, Value const* End)
    {
        bool ReturnValue = true;
        if(Overrides.size() > End-Begin)
        {
            return false;   
        }
        for(int i = 0; i < Overrides.size();i++)
        {
            if(!p_TypeIsSatisfied(Overrides[i],*(Begin+i)))
            {
                return false;   
            }
        }

        return ReturnValue;
    }
    Value* GenericFunction::GetMethod(Value* Begin,Value* End)
    {
        Value* ReturnValue = nullptr;
        if(m_Specifications.size() == 0)
        {
            return ReturnValue;   
        }
        for(int i = int(m_Specifications[0].size())-1; i >= 0; i--)
        {
            if(p_TypesAreSatisifed(m_CallablesOverrides[m_Specifications[0][i].second],Begin,End))
            {
                return &m_Callables[m_Specifications[0][i].second];
            }
        }
        return ReturnValue;
    }
}
