#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <string>
#include <variant>
#include <MBUtility/Dynamic.h>
#include <functional>

namespace MBLisp
{
    typedef int_least64_t Int;
    typedef double Float;
    typedef uint_least32_t SymbolID;
    typedef int_least32_t PositionType;
    typedef int FunctionID;
    typedef int MacroID;
    typedef uint_least32_t ClassID;
    typedef uint_least32_t DynamicVarID;
    typedef std::string String;
    class Value;
    typedef std::vector<Value> List;
    //typedef std::unordered_map<Value,Value> Dict;
    template<typename T>
    using Ref = std::shared_ptr<T>;
    struct Any{};
    
    inline constexpr uint_least32_t RestSymbol = 1<<30;
    
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
        PositionType Position = -1;

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
        SymbolID RestParameter = 0;
        std::shared_ptr<OpCodeList> Instructions;
    };
    //TODO add support for 
    struct Lambda
    {
        std::shared_ptr<FunctionDefinition> Definition;
        std::shared_ptr<Scope> AssociatedScope;
        std::string Name;
    };
    class Evaluator;
    typedef Value (*BuiltinFuncType)(Evaluator&,Ref<Scope>,std::vector<Value>&);
    struct Function
    {
        BuiltinFuncType Func;
        Function(){};
        Function(BuiltinFuncType FuncToSet)
        {
            Func = FuncToSet;   
        }
    };
    struct Macro
    {
        std::shared_ptr<Value> Callable;
        std::string Name;
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




    inline constexpr int ExternalClassBit = 30;
    inline constexpr int UserClassBit = 31;
    class ClassIdentificator
    {
        template<typename T>
        friend ClassID GetClassID();
        static ClassID m_CurrentID;
    };

    template<typename T>
    ClassID GetClassID()
    {
        static ClassID ID = ++ClassIdentificator::m_CurrentID;
        return ID;
    }


    class DynamicVariable;

    class ExternalValue
    {
        ClassID m_ClassID = 0;
        void* m_ExternalData = nullptr;
        std::function<void(void*)> m_Deleter;
    public:
        ExternalValue(ExternalValue const&) = delete;
        ExternalValue& operator=(ExternalValue const&) = delete;
        ExternalValue(ExternalValue&& OtherValue) noexcept
        {
            m_ClassID = OtherValue.m_ClassID;   
            m_ExternalData = OtherValue.m_ExternalData;
            OtherValue.m_ExternalData = nullptr;
            OtherValue.m_ClassID = 0;
        }
    
        template<typename T>
        explicit ExternalValue(T ValueToStore)
        {
            m_ExternalData = new T(std::move(ValueToStore));
            m_Deleter = [](void* DataToDelete){delete static_cast<T*>(DataToDelete);};
            m_ClassID = GetClassID<T>();
        }
        ~ExternalValue()
        {
            if(m_ExternalData != nullptr)
            {
                m_Deleter(m_ExternalData);   
            }
        }

        ClassID GetTypeID()
        {
            return m_ClassID;   
        }
        template<typename T>
        bool IsType() const
        {
            return m_ClassID == GetClassID<T>();
        }
        template<typename T>
        T& GetType()
        {
            if(!IsType<T>())
            {
                throw std::runtime_error("Invalid type access for ExternalValue: "+std::string(typeid(T).name()));
            }
            return *static_cast<T*>(m_ExternalData);
        }
        template<typename T>
        T const& GetType() const
        {
            if(!IsType<T>())
            {
                throw std::runtime_error("Invalid type access for ExternalValue: "+std::string(typeid(T).name()));
            }
            return *static_cast<T const*>(m_ExternalData);
        }
    };
    
    class Null { };
    template<typename TypeToCheck,typename... OtherType>
    inline constexpr bool TypeIn = i_TypeIn<TypeToCheck,OtherType...>::value;
    class Value
    {
public:
        struct Value_Hasher
        {
            std::size_t operator()(Value const& ValueToHash) const
            {
                std::size_t ReturnValue = 0;
                if(ValueToHash.IsType<String>())
                {
                    ReturnValue = std::hash<std::string>()(ValueToHash.GetType<String>());
                }
                else if(ValueToHash.IsType<Int>())
                {
                    ReturnValue = std::hash<Int>()(ValueToHash.GetType<Int>());
                }
                else if(ValueToHash.IsType<bool>())
                {
                    ReturnValue = std::hash<bool>()(ValueToHash.GetType<bool>());
                }
                else if(ValueToHash.IsType<Symbol>())
                {
                    ReturnValue = std::hash<SymbolID>()(ValueToHash.GetType<Symbol>().ID);
                }
                else
                {
                    throw std::runtime_error("Can only hash value types");   
                }
                return ReturnValue;
            };
        };
        typedef std::variant<Null,bool,Function,Macro,Int,Float,Symbol,MBUtility::Dynamic<String>,
            Ref<Lambda>,
            Ref<List>,
            Ref<std::unordered_map<Value,Value,Value_Hasher>>,
            Ref<GenericFunction>,
            Ref<ClassDefinition>,
            Ref<ClassInstance>,
            Ref<Value>,
            Ref<DynamicVariable>,
            Ref<Scope>,
            Ref<ExternalValue>> DataStorage;
        DataStorage m_Data;


        template<typename T>
        static constexpr bool IsValueType()
        {
            return TypeIn<T,bool,Function,Int,Float,Symbol,Macro,Null>;
        }
        template<typename T>
        static constexpr bool IsReferenceType()
        {
            return TypeIn<T,ClassDefinition,DynamicVariable,Lambda,GenericFunction,ClassInstance,List,Value,Scope
                ,std::unordered_map<Value,Value,Value_Hasher>>;
        }
        template<typename T>
        static constexpr bool IsBuiltin()
        {
            return IsValueType<T>() || IsReferenceType<T>() || std::is_same_v<T,String>;
        }
        
        template<typename T>
        T const& p_GetType() const
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr (std::is_same_v<T, Value>)
                {
                    return *std::get<Ref<Value>>(m_Data);
                }
                if (std::holds_alternative<Ref<Value>>(m_Data))
                {
                    return std::get<Ref<Value>>(m_Data)->p_GetType<T>();
                }
                if constexpr(IsValueType<T>())
                {
                    return std::get<T>(m_Data);
                }
                else if constexpr(IsReferenceType<T>())
                {
                    return *std::get<Ref<T>>(m_Data);
                }
                else if constexpr(std::is_same_v<T,String>)
                {
                    return *std::get<MBUtility::Dynamic<String>>(m_Data);
                }
            }
            else
            {
                if constexpr (std::is_same_v<ExternalValue, T>)
                {
                    return *std::get<Ref<ExternalValue>>(m_Data);
                }
                if(IsType<Value>())
                {
                    return std::get<Ref<Value>>(m_Data)->GetType<T>();
                }
                else
                {
                    return std::get<Ref<ExternalValue>>(m_Data)->GetType<T>();
                }
            }
            throw std::runtime_error("Invalid type access: Value was not of type "+std::string(typeid(T).name()));
        }
        template<typename VariantType, typename T, std::size_t Index = 0>
        static constexpr std::size_t VariantIndex() 
        {
            static_assert(std::variant_size_v<VariantType> > Index, "Type not found in variant");
            if constexpr (Index == std::variant_size_v<VariantType>) 
            {
                return Index;
            } 
            else if constexpr (std::is_same_v<std::variant_alternative_t<Index, VariantType>,T>) 
            {
                return Index;
            } 
            else 
            {
                return VariantIndex<VariantType, T, Index + 1>();
            }
        } 

        template<template<class...> class Template, class Instance>
        struct IsTemplateInstance_t : std::false_type {};
        template<class... TemplateArgs, template<class...> class Template>
        struct IsTemplateInstance_t<Template, Template<TemplateArgs...> > : std::true_type {};
        
        template<template<class...> class T,typename U>
        static constexpr bool IsTemplateInstance = IsTemplateInstance_t<T,U>::value;
    public:
        Value& operator=(Value&&) = default;
        Value(Value&&) noexcept= default;
        Value(Value const& ) = default;
        Value() = default;

        Value& operator=(Value const& ValueToCopy)
        {
            //reference, assign this value 
            DataStorage* StorageToAssign = &m_Data;
            Value const* PointerToCopy = &ValueToCopy;
            if(ValueToCopy.IsType<Value>())
            {
                PointerToCopy = &ValueToCopy.GetType<Value>();
            }
            if(IsType<Value>())
            {
                StorageToAssign = &GetType<Value>().m_Data;
            }
            *StorageToAssign = PointerToCopy->m_Data;
            return *this;
        }
    

        bool operator==(Value const& OtherValue) const
        {
            bool ReturnValue = false;
            if(!IsSameType(OtherValue))
            {
                return false;
            }
            if(IsType<Float>())
            {
                return GetType<Float>() == OtherValue.GetType<Float>();
            }
            else if(IsType<Int>())
            {
                return GetType<Int>() == OtherValue.GetType<Int>();
            }
            else if(IsType<Symbol>())
            {
                return GetType<Symbol>().ID == OtherValue.GetType<Symbol>().ID;
            }
            else if(IsType<bool>())
            {
                return GetType<bool>() == OtherValue.GetType<bool>();
            }
            else if(IsType<String>())
            {
                return GetType<String>() == OtherValue.GetType<String>();
            }
            else if(IsType<ClassInstance>())
            {
                return GetRef<ClassInstance>().get() == OtherValue.GetRef<ClassInstance>().get();
            }
            else if(IsType<ClassDefinition>())
            {
                return GetRef<ClassDefinition>().get() == OtherValue.GetRef<ClassDefinition>().get();
            }
            return ReturnValue;
        }
        
        bool IsBuiltin()
        {
            return TypeIsBuiltin(GetTypeID());
        } 
        static constexpr bool TypeIsBuiltin(ClassID IDToInspect)
        {
            return IDToInspect <= std::variant_size_v<DataStorage>;
        } 
        template<typename T> 
        static constexpr ClassID GetTypeTypeID()
        {
            if(std::is_same_v<T,ClassInstance>)
            {
                return 1<<UserClassBit;   
            }
            if constexpr(IsBuiltin<T>())
            {
                if constexpr (std::is_same_v<T, String>)
                {
                    return VariantIndex<DataStorage, MBUtility::Dynamic<String>>()+1;
                }
                else if constexpr(IsValueType<T>())
                {
                    return VariantIndex<DataStorage,T>()+1;
                }
                else
                {
                    return VariantIndex<DataStorage,Ref<T>>()+1;
                }

            }
            else
            {
                return GetClassID<T>();   
            }
        }
        
        bool IsSameType(Value const& OtherValue) const
        {
            return GetTypeID() == OtherValue.GetTypeID();
        }
        //temporrary implementation
        ClassID GetTypeID() const;
        template<typename T>
        bool IsType() const
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr (std::is_same_v<T, Value>)
                {
                    return std::holds_alternative < Ref<Value>>(m_Data);
                }
                if (std::holds_alternative <Ref<Value>>(m_Data))
                {
                    return std::get<Ref<Value>>(m_Data)->IsType<T>();
                }
                if constexpr(IsValueType<T>())
                {
                    return std::holds_alternative<T>(m_Data);
                }
                else
                {
                    if constexpr(IsReferenceType<T>())
                    {
                        return std::holds_alternative<Ref<T>>(m_Data);
                    }
                    else if constexpr(std::is_same_v<T,String>)
                    {
                        return std::holds_alternative<MBUtility::Dynamic<String>>(m_Data);
                    }
                }
            }
            else
            {
                return GetType<ExternalValue>().IsType<T>();
            }
            return false;
        }
        void MakeRef()
        {
            if(IsType<Value>())
            {
                return;   
            }
            Ref<Value> NewValue = std::make_shared<Value>();
            NewValue->m_Data = std::move(m_Data);
            m_Data = NewValue;
        }
        template<typename T>
        Ref<T> GetRef()
        {
            static_assert(IsReferenceType<T>(),"Can only get reference to value type");
            if(std::holds_alternative<Ref<Value>>(m_Data))
            {
                return std::get<Ref<Value>>(m_Data)->GetRef<T>();
            }
            else
            {
                return std::get<Ref<T>>(m_Data);
            }
        } 
        template<typename T>
        Ref<T> const GetRef() const
        {
            static_assert(IsReferenceType<T>(),"Can only get reference to value type");
            if(std::holds_alternative<Ref<Value>>(m_Data))
            {
                return std::get<Ref<Value>>(m_Data)->GetRef<T>();
            }
            else
            {
                return std::get<Ref<T>>(m_Data);
            }
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
            if constexpr (std::is_same_v<T,BuiltinFuncType>)
            {
                m_Data = Function(Rhs);
            }
            else if constexpr (std::is_same_v<T,const char*>)
            {
                m_Data = String(Rhs);
            }
            else if constexpr(std::is_same_v<T,bool>)
            {
                m_Data = Rhs;
            }
            else if constexpr (std::is_integral_v<T>)
            {
                m_Data = Int(Rhs);
            }
            else if constexpr(IsBuiltin<T>())
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
        Value(Ref<T> Rhs)
        {
            if constexpr(IsReferenceType<T>())
            {
                m_Data = Rhs;
            }
            else
            {
                static_assert(!std::is_same<T,T>::value,"Can only initialize refernce type with reference");
            }
        }


        template<typename T>
        static Value MakeExternal(T ExternalObject)
        {
            Value ReturnValue;
            ReturnValue.m_Data = std::make_shared<ExternalValue>(std::move(ExternalObject));
            return ReturnValue;
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
                else if(IsType<Value>())
                {
                    GetType<Value>() = std::move(Rhs);
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
    typedef std::unordered_map<Value,Value,Value::Value_Hasher> Dict;
    struct SlotDefinition
    {
        SymbolID Symbol = 0;
        int Order = 0;
        Value DefaultValue;

        bool operator<(SlotDefinition const& rhs) const
        {
            return Symbol < rhs.Symbol;   
        }
    };
    class ClassDefinition
    {
        public:
        ClassDefinition() = default;

        explicit ClassDefinition(ClassID NewID)
        {
            ID = NewID;
        }
        ClassID ID = 0;
        std::vector<ClassID> Types;
        //expression to compile
        std::vector<SlotDefinition> SlotDefinitions;
        Ref<FunctionDefinition> SlotInitializers;
        //constructor is optionally run after slot initializers, which are always run
        std::shared_ptr<Value> Constructor;
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
        std::string Name;
        void AddMethod(std::vector<ClassID> OverridenTypes,Value Callable);
        //TODO more efficient implementation, the current one is the most naive one
        Value* GetMethod(std::vector<Value>& Arguments);
    };

    class DynamicVariable
    {
        public:
        Value DefaultValue;
        DynamicVarID ID = 0;
    };





    inline ClassID Value::GetTypeID() const
    {
        ClassID ReturnValue = 0;
        if(IsType<Value>())
        {
            ReturnValue = GetType<Value>().GetTypeID();
        }
        else if(std::holds_alternative<Ref<ExternalValue>>(m_Data))
        {
            ReturnValue = std::get<Ref<ExternalValue>>(m_Data)->GetTypeID();
        }
        else if(std::holds_alternative<Ref<ClassInstance>>(m_Data))
        {
            ReturnValue = GetType<ClassInstance>().AssociatedClass->ID;
        }
        else
        {
            return m_Data.index()+1;
        }
        return ReturnValue;
    }
    class LookupError : public std::exception
    {
        std::string m_ErrorString = "Couldn't find variable in current scope";
        SymbolID m_Symbol = -1;
    public:
        LookupError(SymbolID LookedUpSymbol)
        {
            m_Symbol = LookedUpSymbol;   
        }
        SymbolID GetSymbol() const
        {
            return m_Symbol;
        }
        const char* what() const noexcept override
        {
            return m_ErrorString.c_str();
        }
    };

    class Scope
    {
        std::shared_ptr<Scope> m_ParentScope = nullptr;
        std::unordered_map<SymbolID,Value> m_Variables;
    public:
        void SetParentScope(std::shared_ptr<Scope> ParentScope);
        Value FindVariable(SymbolID Variable);
        void SetVariable(SymbolID Variable,Value NewValue);
        void OverrideVariable(SymbolID Variable,Value NewValue);
        Value* TryGet(SymbolID Variable);
    };

};
