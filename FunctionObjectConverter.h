#pragma once
#include "Value.h"
#include <type_traits>

namespace MBLisp
{
    

    template<typename... Types>
    struct First 
    {
        typedef void type;
    };
    template<typename Front,typename... Types>
    struct First<Front,Types...>
    {
        typedef Front type;
    };

    template<typename... T>
    using First_v = typename First<T...>::type;


    template<int N,typename... Types>
    struct Nth 
    {
        typedef void type;
    };
    template<typename Head,typename... Tail>
    struct Nth<0,Head,Tail...>
    {
        typedef Head type;
    };
    template<int N,typename Head,typename... Tail>
    struct Nth<N,Head,Tail...>
    {
        typedef typename Nth<N-1,Tail...>::type type;
    };

    template<size_t N,typename... T>
    using Nth_v = typename Nth<N,T...>::type;

    template<typename T>
    Value ToBuiltin(T&& RegularValue)
    {
        if constexpr(std::is_same_v<T,Value>())
        {
            return RegularValue;
        }
        else if constexpr(Value::IsBuiltin<T>())
        {
            return Value(std::forward<T>(RegularValue));
        }
        else if constexpr(IsTemplateInstantiation<T,Ref>::value)
        {
            return Value(std::forward<T>(RegularValue));
        }
        else
        {
            return Value::EmplaceExternal<T>(std::move(RegularValue));
        }
    }

    template<typename T>
    using FromBuiltin_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template<typename T>
    auto FromBuiltin(Value& Val)
    {
        typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type Type;
        if constexpr(IsTemplateInstantiation<T,Ref>::value)
        {
            return Val.GetRef<Type>();
        }
        else if constexpr(std::is_same_v<Type,Value>)
        {
            return Val;
        }
        else
        {
            return Val.GetType<Type>();
        }
    }
    
    template<typename T>
    class FunctionObjectConverter : public Invocable
    {
        T m_Data;

        template<typename ReturnType,typename ObjectType,typename... TotalArgTypes,typename... SuppliedArgTypes>
        Value p_Invoke(FuncArgVector& Args,ReturnType (ObjectType::* Func)(TotalArgTypes...),SuppliedArgTypes&&... SuppliedArgs)
        {
            if constexpr(sizeof...(SuppliedArgTypes) == sizeof...(TotalArgTypes))
            {
                if constexpr(std::is_same_v<ReturnType,void>)
                {
                    m_Data(std::forward<SuppliedArgTypes>(SuppliedArgs)...);
                    return Value();
                }
                else
                {
                    return ToBuiltin(m_Data(std::forward<SuppliedArgTypes>(SuppliedArgs)...));
                }
            }
            else
            {
                if(sizeof...(SuppliedArgs) >= Args.size())
                {
                    throw std::runtime_error("Insufficient amount of arguments supplied");
                }
                return p_Invoke(Args,Func,std::forward<SuppliedArgTypes>(SuppliedArgs)...,FromBuiltin<Nth_v<sizeof...(SuppliedArgTypes),TotalArgTypes...>>(Args[sizeof...(SuppliedArgs)]));
            }
        }
    public:
        FunctionObjectConverter(T Data) : m_Data(Data){}

        virtual Value operator() BUILTIN_ARGLIST
        {
            return p_Invoke(Arguments,&T::operator());
        }
    };
}
