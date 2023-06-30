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
    };
    struct Function
    {
        FunctionID ID;
    };
    struct Macro
    {
        FunctionID ID;
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
            if constexpr(std::is_same_v<T,Int>)
            {
                return true;
            }
            else if constexpr(std::is_same_v<T,Float>)
            {
                return true;
            }
            else if constexpr(std::is_same_v<T,List>)
            {
                return true;
            }
            else if constexpr(std::is_same_v<T,Symbol>)
            {
                return true;
            }
            else if constexpr(std::is_same_v<T,String>)
            {
                return true;
            }
            else if constexpr(std::is_same_v<T,bool>)
            {
                return true;
            }
            else if constexpr(std::is_same_v<T,Function>)
            {
                return true;
            }
            return false;
        }
        std::variant<bool,Function,Int,Float,MBUtility::Dynamic<String>,std::shared_ptr<List>,Symbol> m_Data;


        template<typename T>
        static constexpr bool IsValueType()
        {
            return TypeIn<T,bool,Function,Int,Float,Symbol>;
        }
        
        template<typename T>
        T const& p_GetType() const
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr(std::is_same_v<T,Int>)
                {
                    return std::get<Int>(m_Data);
                }
                else if constexpr(std::is_same_v<T,Float>)
                {
                    return std::get<Float>(m_Data);
                }
                else if constexpr(std::is_same_v<T,bool>)
                {
                    return std::get<bool>(m_Data);
                }
                else if constexpr(std::is_same_v<T,List>)
                {
                    return *std::get<std::shared_ptr<List>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,Function>)
                {
                    return std::get<Function>(m_Data);
                }
                else if constexpr(std::is_same_v<T,Symbol>)
                {
                    return std::get<Symbol>(m_Data);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    return *std::get<MBUtility::Dynamic<String>>(m_Data);
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
