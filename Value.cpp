#include "Value.h"
#include <algorithm>
#include "assert.h"
namespace MBLisp
{
       
    ClassID ClassIdentificator::m_CurrentID = 1<<ExternalClassBit;


    GenericFunction::GenericFunction(std::vector<SymbolID> Arguments)
    {
        m_Arguments = std::move(Arguments);
        for(int i = 0; i < m_Arguments.size();i++)
        {
            m_Specifications.push_back(std::vector<std::pair<ClassID,size_t>>());
        }
    }
    void GenericFunction::AddMethod(std::vector<ClassID> OverridenTypes,Value Callable)
    {
        if(OverridenTypes.size() > m_Arguments.size())
        {
            throw std::runtime_error("Cannot override more types than method contains");
        }
        size_t NewIndex = m_Callables.size(); 
        m_Callables.push_back(Callable);
        for(int i = 0; i < OverridenTypes.size();i++)
        {
            m_Specifications[i].push_back(std::make_pair(OverridenTypes[i],NewIndex));
            std::sort(m_Specifications[i].begin(),m_Specifications[i].end(),[](std::pair<ClassID,size_t> const& Lhs,std::pair<ClassID,size_t> const& Rhs)
                    {
                        return Lhs.first < Rhs.first;
                    });
        }
        m_CallablesOverrides.push_back(std::move(OverridenTypes));

    }
    bool GenericFunction::p_TypesAreSatisifed(std::vector<ClassID> const& Overrides,std::vector<std::vector<ClassID>> const& ArgumentsClasses)
    {
        bool ReturnValue = true;
        assert(Overrides.size() <= ArgumentsClasses.size());
        for(int i = 0; i < Overrides.size();i++)
        {
            if(auto It = std::lower_bound(ArgumentsClasses[i].begin(),ArgumentsClasses[i].end(),Overrides[i]);
                    !(It != ArgumentsClasses[i].end() && *It == Overrides[i]))
            {
                return false;
            }
        }

        return ReturnValue;
    }
    std::vector<ClassID> GenericFunction::p_GetValueTypes(Value const& ValueToInspect)
    {
        if(ValueToInspect.IsType<ClassInstance>())
        {
            return ValueToInspect.GetType<ClassInstance>().AssociatedClass->Types;
        }
        else
        {
            return {ValueToInspect.GetTypeID()};   
        }
    }
    Value* GenericFunction::GetMethod(std::vector<Value>& Arguments)
    {
        Value* ReturnValue = nullptr;
        std::vector<std::vector<ClassID>> ArgumentClasses;
        for(auto const& Argument : Arguments)
        {
            ArgumentClasses.push_back(p_GetValueTypes(Argument));   
        }
        for(int i = int(m_Specifications[0].size())-1; i >= 0; i--)
        {
            if(p_TypesAreSatisifed(m_CallablesOverrides[m_Specifications[0][i].second],ArgumentClasses))
            {
                return &m_Callables[m_Specifications[0][i].second];
            }
        }
        return ReturnValue;
    }
}
