#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <string>
#include <variant>
#include <MBUtility/Dynamic.h>
namespace MBLisp
{
    typedef int_least64_t Int;
    typedef double Float;
    typedef int SymbolID;
    typedef int FunctionID;
    typedef int MacroID;
    typedef std::string String;
    class Value;
    typedef std::vector<Value> List;

    enum class ValueType
    {
        List,
        Int,
        Float,
        UserDefined
    };

    struct Symbol
    {
        SymbolID ID;
        Symbol() = default;
        Symbol(SymbolID  NewId)
        {
            ID = NewId;
        }
    };

    class OpCodeList;
    class Scope;
    struct FunctionDefinition
    {
        std::vector<Symbol> Arguments;
        std::shared_ptr<OpCodeList> Instructions;
    };
    //TODO add support for 
    struct Lambda
    {
        std::shared_ptr<FunctionDefinition> Definition;
        std::shared_ptr<Scope> AssociatedScope;
    };
    typedef Value (*BuiltinFuncType)(std::vector<Value>&);
    struct Function
    {
        BuiltinFuncType Func;
    };
    struct Macro
    {
        std::shared_ptr<FunctionDefinition> AssociatedFunction;
    };
    

    template<typename T,typename U,typename... OtherTypes>
    struct i_TypeIn
    {
        static constexpr bool value = std::disjunction<std::is_same<T,U>,i_TypeIn<T,OtherTypes...>>::value;
    };
    template<typename T,typename U>
    struct i_TypeIn<T,U>
    {
        static constexpr bool value = std::is_same<T,U>::value;
    };
    
    template<typename TypeToCheck,typename... OtherType>
    inline constexpr bool TypeIn = i_TypeIn<TypeToCheck,OtherType...>::value;
    class Value
    {
        template<typename T>
        static constexpr bool IsBuiltin()
        {
            return TypeIn<T,bool,Function,Int,Float,Symbol,List,String,Lambda,Macro>;
        }
        std::variant<bool,Function,Macro,Int,Float,MBUtility::Dynamic<String>,std::shared_ptr<Lambda>,std::shared_ptr<List>,Symbol> m_Data;


        template<typename T>
        static constexpr bool IsValueType()
        {
            return TypeIn<T,bool,Function,Int,Float,Symbol,Macro>;
        }
        
        template<typename T>
        T const& p_GetType() const
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr(IsValueType<T>())
                {
                    return std::get<T>(m_Data);
                }
                else if constexpr(std::is_same_v<T,List>)
                {
                    return *std::get<std::shared_ptr<List>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    return *std::get<MBUtility::Dynamic<String>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,Lambda>)
                {
                    return *std::get<std::shared_ptr<Lambda>>(m_Data);
                }
            }
            throw std::runtime_error("Invalid type access: Value was not of type "+std::string(typeid(T).name()));
        }


    public:
        Value& operator=(Value const&) = default;
        Value& operator=(Value&&) = default;
        Value(Value&&) noexcept= default;
        Value(Value const& ) = default;
        Value() = default;


        bool IsSameType(Value const& OtherValue) const
        {
            return m_Data.index() == OtherValue.m_Data.index();
        }
        template<typename T>
        bool IsType() const
        {
            if constexpr(IsValueType<T>())
            {
                return std::holds_alternative<T>(m_Data);
            }
            else
            {
                if constexpr(std::is_same_v<T,List>)
                {
                    return std::holds_alternative<std::shared_ptr<List>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    return std::holds_alternative<MBUtility::Dynamic<String>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,Lambda>)
                {
                    return std::holds_alternative<std::shared_ptr<Lambda>>(m_Data);
                }
            }
            return false;
        }
        template<typename T>
        T& GetType()
        {
            return const_cast<T&>(p_GetType<T>());
        }
        template<typename T>
        T const& GetType() const
        {
            return p_GetType<T>();
        }
        template<typename T>
        Value(T Rhs)
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr(IsValueType<T>())
                {
                    m_Data = std::move(Rhs);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    m_Data = MBUtility::Dynamic(std::move(Rhs));
                }
                else if constexpr(std::is_same_v<T,List>)
                {
                    m_Data = std::make_shared<List>(std::move(Rhs));
                }
                else if constexpr(std::is_same_v<T,Lambda>)
                {
                    m_Data = std::make_shared<Lambda>(std::move(Rhs));
                }
                else
                {
                    static_assert(!std::is_same<T,T>::value,"Initialization of value doesnt take into consideration all builtin types");
                }
            }
            else
            {
                static_assert(!std::is_same<T,T>::value,"Can only initialize value with builtin types");
            }
        }
        template<typename T>
        Value& operator=(T Rhs)
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr(IsValueType<T>())
                {
                    m_Data = std::move(Rhs);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    m_Data = MBUtility::Dynamic(std::move(Rhs));
                }
                else if constexpr(std::is_same_v<T,List>)
                {
                    m_Data = std::make_shared<List>(std::move(Rhs));
                }
                else if constexpr(std::is_same_v<T,Lambda>)
                {
                    m_Data = std::make_shared<Lambda>(std::move(Rhs));
                }
                else
                {
                    static_assert(!std::is_same<T,T>::value,"Assignment of value doesnt take into consideration all builtin types");
                }
            }
            else
            {
                static_assert(!std::is_same<T,T>::value,"Can only assign builtin types");
            }
            return *this;
        }
    };
};
