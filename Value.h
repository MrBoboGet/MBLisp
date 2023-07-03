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
    typedef uint_least32_t ClassID;
    typedef std::string String;
    class Value;
    typedef std::vector<Value> List;
    template<typename T>
    using Ref = std::shared_ptr<T>;
      

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
    class ClassDefinition;
    class ClassInstance;
    class GenericFunction;



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
            return TypeIn<T,bool,Function,Int,Float,Symbol,List,String,Lambda,Macro,GenericFunction,ClassDefinition,ClassInstance>;
        }
        std::variant<bool,Function,Macro,Int,Float,Symbol,MBUtility::Dynamic<String>,
            std::shared_ptr<Lambda>,std::shared_ptr<List>,std::shared_ptr<GenericFunction>,std::shared_ptr<ClassDefinition>,std::shared_ptr<ClassInstance>> m_Data;


        template<typename T>
        static constexpr bool IsValueType()
        {
            return TypeIn<T,bool,Function,Int,Float,Symbol,Macro>;
        }
        template<typename T>
        static constexpr bool IsReferenceType()
        {
            return TypeIn<T,ClassDefinition,Lambda,GenericFunction,ClassInstance,List>;
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
                else if constexpr(IsReferenceType<T>())
                {
                    return *std::get<std::shared_ptr<T>>(m_Data);
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


        bool IsSameType(Value const& OtherValue) const
        {
            return m_Data.index() == OtherValue.m_Data.index();
        }
        //temporrary implementation
        ClassID GetTypeID() const
        {
            return m_Data.index();
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
                if constexpr(IsReferenceType<T>())
                {
                    return std::holds_alternative<std::shared_ptr<T>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    return std::holds_alternative<MBUtility::Dynamic<String>>(m_Data);
                }
            }
            return false;
        }
       
        template<typename T>
        Ref<T> GetRef()
        {
            return std::get<Ref<T>>(m_Data);
        } 
        template<typename T>
        Ref<T> const GetRef() const
        {
            return std::get<Ref<T>>(m_Data);
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
                else if constexpr(IsReferenceType<T>())
                {
                    m_Data = std::make_shared<T>(std::move(Rhs));
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
                else if constexpr(IsReferenceType<T>())
                {
                    m_Data = std::make_shared<T>(std::move(Rhs));
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
    class ClassDefinition
    {
        public:
        ClassID ID = 0;
        std::vector<ClassID> Types;
        //expression to compile
        std::vector<std::pair<SymbolID,Value>> SlotDefinitions;
        std::shared_ptr<FunctionDefinition> SlotInitializers;
        //constructor is run after slot initializers, which are always run
        std::shared_ptr<FunctionDefinition> Constructor;
    };
    class ClassInstance
    {
        public:
        std::shared_ptr<ClassDefinition> AssociatedClass;
        //binary search to find
        std::vector<std::pair<SymbolID,Value>> Slots;
    };
    class GenericFunction
    {
        std::vector<SymbolID> m_Arguments;
        //specificity is found argument by argument, the first 
        //argument winning, then the second and so on
        //un specified type is specificty 0, the last used alternative
        std::vector<Value> m_Callables;
        std::vector<std::vector<ClassID>> m_CallablesOverrides;
        
        std::vector<std::vector<std::pair<ClassID,size_t>>> m_Specifications;


        //TODO improve, inefficient
        bool p_TypesAreSatisifed(std::vector<ClassID> const& Overrides,std::vector<std::vector<ClassID>> const& ArgumentsClasses);
        std::vector<ClassID> p_GetValueTypes(Value const& ValueToInspect);
    public:
        GenericFunction(std::vector<SymbolID> Arguments);
        void AddMethod(std::vector<ClassID> OverridenTypes,Value Callable);
        //TODO more efficient implementation, the current one is the most naive one
        Value* GetMethod(std::vector<Value>& Arguments);
    };
};
