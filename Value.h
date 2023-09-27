#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <string>
#include <variant>
#include <MBUtility/Dynamic.h>
#include <functional>

#include <assert.h>

#include <MBUtility/MBVector.h>

#include <mutex>
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
    typedef uint_least32_t ThreadID;
    typedef std::string String;
    class Value;
    typedef std::vector<Value> List;
    typedef MBUtility::MBVector<Value,4,size_t,std::allocator<Value>> FuncArgVector;
    //typedef std::vector<Value> FuncArgVector;
    //typedef std::unordered_map<Value,Value> Dict;
    
    struct Any{};

    typedef std::unique_ptr<std::lock_guard<std::recursive_mutex>>  BuiltinLock;
    
    inline constexpr uint_least32_t RestSymbol = 1<<30;
    inline constexpr uint_least32_t EnvirSymbol = 1<<28;
    inline constexpr uint_least32_t GeneratedSymbol = 1<<29;

    class RefContent
    {
    protected:
        int m_RefCount = 1;
        RefContent()
        {
               
        }
        mutable std::unique_ptr<std::recursive_mutex> m_ReferenceMutex = nullptr;
    public:
        BuiltinLock GetLock() const
        {
            if(m_ReferenceMutex == nullptr)
            {
                m_ReferenceMutex = std::make_unique<std::recursive_mutex>();
            }
            return std::make_unique<std::lock_guard<std::recursive_mutex>>(*m_ReferenceMutex);
        }
        ClassID StoredClass = 0;
        int RefCount()
        {
            return m_RefCount;
        }
        ClassID GetTypeID()
        {
            return StoredClass;   
        }
        void Increment()
        {
            m_RefCount += 1;
        }
        void Decrement()
        {
            m_RefCount -= 1;
        }


        //no error checking, done higher up in the call stack
        template<typename T>
        T& GetContent();
        template<typename T>
        T const& GetContent() const;
        virtual ~RefContent() { }
    };

    template<typename T>
    class RefContent_Specialised : public RefContent
    {
    public:
        T Content;
        template<typename... Args>
        RefContent_Specialised(Args&&... Arguments)
            : Content(std::forward<Args>(Arguments)...)
        {
               
        }
        virtual ~RefContent_Specialised()
        {
               
        }
    };
    template<typename T>
    T& RefContent::GetContent()
    {
        return static_cast<RefContent_Specialised<T>&>(*this).Content;
    }
    template<typename T>
    T const& RefContent::GetContent() const
    {
        return static_cast<RefContent_Specialised<T> const&>(*this).Content;
    }
    class RefBase
    {
        protected:
        RefContent* m_AllocatedContent = nullptr;
        RefBase()
        {
               
        }
        public:
        BuiltinLock GetLock() const
        {
            if(m_AllocatedContent == nullptr)
            {
                throw std::runtime_error("Cannot aquire lock of null RefBase");
            }
            return m_AllocatedContent->GetLock();
        }
        ClassID GetTypeID() const
        {
            if(m_AllocatedContent == nullptr)
            {
                throw std::runtime_error("Trying to get TypeID from null  RefBase");   
            }
            return m_AllocatedContent->GetTypeID();
        }

        template<typename T>
        T& GetType()
        {
            if(m_AllocatedContent == nullptr)
            {
                throw std::runtime_error("Trying to access type from null RefBase");   
            }
            return m_AllocatedContent->GetContent<T>();
        }
        template<typename T>
        T const& GetType() const
        {
            if(m_AllocatedContent == nullptr)
            {
                throw std::runtime_error("Trying to access type from null RefBase");   
            }
            return m_AllocatedContent->GetContent<T>();
        }
        
        bool operator==(RefBase const& Rhs) const
        {
            return m_AllocatedContent == Rhs.m_AllocatedContent;   
        }
        bool operator!=(RefBase const& Rhs) const
        {
            return m_AllocatedContent != Rhs.m_AllocatedContent;   
        }
        
        RefBase(RefBase const& OtherRef)
        {
            m_AllocatedContent = OtherRef.m_AllocatedContent;
            if(m_AllocatedContent != nullptr)
            {
                m_AllocatedContent->Increment();
            }
        }
        RefBase(RefBase&& OtherRef) noexcept
        {
            m_AllocatedContent = OtherRef.m_AllocatedContent;
            OtherRef.m_AllocatedContent = nullptr;
        }
        RefBase& operator=(RefBase ObjectToCopy)
        {
            m_AllocatedContent = ObjectToCopy.m_AllocatedContent;
            ObjectToCopy.m_AllocatedContent = nullptr;
            return *this;
        }
        ~RefBase()
        {
            if(m_AllocatedContent != nullptr)
            {
                m_AllocatedContent->Decrement();   
                if(m_AllocatedContent->RefCount() == 0)
                {
                    delete m_AllocatedContent;   
                }
            }
        }
    };

    template<typename T>
    class Ref : public RefBase
    {
    private:
        //template<typename A,typename... Args> 
        //friend Ref<A> MakeRef(Args... Arguments);
    public:
        explicit Ref(RefContent* Content)
        {
            m_AllocatedContent = Content;
        }
        explicit Ref(RefBase& Content)
        {
            (RefBase&)*this = Content;
        }
        bool operator==(nullptr_t Rhs) const
        {
            return m_AllocatedContent == nullptr;
        }
        bool operator!=(nullptr_t Rhs) const
        {
            return m_AllocatedContent != nullptr;
        }
        T* operator->()
        {
            return &m_AllocatedContent->GetContent<T>();
        }
        T const* operator->() const
        {
            return &m_AllocatedContent->GetContent<T>();
        }
        T& operator*()
        {
            return m_AllocatedContent->GetContent<T>();
        }
        T const& operator*() const
        {
            return m_AllocatedContent->GetContent<T>();
        }
        T* get()
        {
            return &m_AllocatedContent->GetContent<T>();
        }
        T const* get() const
        {
            return &m_AllocatedContent->GetContent<T>();
        }
        Ref(nullptr_t Null)
        {
        }
        Ref()
        {
        }
        //template<typename... Args>
        //Ref(Args&&... Arguments)
        //{
        //    m_AllocatedContent = new RefContent_Specialised<T>(std::forward<Args>(Arguments)...);
        //}
    };

    
    enum class ValueType
    {
        List,
        Int,
        Float,
        UserDefined
    };


    struct Location
    {
        PositionType Position = -1;
        SymbolID URI = -1;
    };
    struct Symbol
    {
        SymbolID ID;
        Location SymbolLocation;

        Symbol() = default;
        Symbol(SymbolID  NewId)
        {
            ID = NewId;
        }
        bool operator==(Symbol Rhs) const
        {
            return ID == Rhs.ID;   
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
        SymbolID EnvirParameter = 0;
        Ref<OpCodeList> Instructions;
    };
    //TODO add support for 
    struct Lambda
    {
        Ref<FunctionDefinition> Definition;
        Ref<Scope> AssociatedScope;
        Symbol Name;
    };
    //class Setter
    //{
    //public:
    //    virtual void Set(Value const& ValueToSet) = 0;
    //    virtual ~Setter(){};
    //};
    class CallContext;
    typedef Value (*BuiltinFuncType)(CallContext&,FuncArgVector&);
    struct Function
    {
        BuiltinFuncType Func;
        Function(){};
        Function(BuiltinFuncType FuncToSet)
        {
            Func = FuncToSet;   
        }
        bool operator==(Function OtherFunc) const
        {
            return Func == OtherFunc.Func;   
        }
    };
    struct Macro
    {
        Ref<Value> Callable;
        Symbol Name;
        bool operator==(Macro const& OtherMacro) const
        {
            return (RefBase const&)Callable == (RefBase const&) OtherMacro.Callable;   
        }
    };

    struct ThreadHandle
    {
        ThreadID ID = -1;
        bool operator==(ThreadHandle Rhs) const
        {
            return ID == Rhs.ID;   
        }
        bool operator!=(ThreadHandle Rhs) const
        {
            return !(*this == Rhs);
        }
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

    template<typename T,typename U,typename... OtherTypes>
    struct i_MaxSize
    {
        static constexpr size_t value = sizeof(T) > i_MaxSize<U,OtherTypes...>::value ? sizeof(T) : i_MaxSize<U,OtherTypes...>::value;
    };
    template<typename T,typename U>
    struct i_MaxSize<T,U>
    {
        static constexpr size_t value = sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);
    };

    template<typename T,typename U,typename... OtherTypes>
    struct i_MaxAlign
    {
        static constexpr size_t value = alignof(T) > i_MaxAlign<U,OtherTypes...>::value ? alignof(T) : i_MaxAlign<U,OtherTypes...>::value;
    };
    template<typename T,typename U>
    struct i_MaxAlign<T,U>
    {
        static constexpr size_t value = alignof(T) > alignof(U) ? alignof(T) : alignof(U);
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
        ExternalValue()
        {
               
        }
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
        template<typename Class,typename... ArgTypes>
        static ExternalValue Emplace(ArgTypes&&... Args)
        {
            ExternalValue ReturnValue;
            ReturnValue.m_ExternalData = new Class(std::forward<ArgTypes>(Args)...);
            ReturnValue.m_Deleter = [](void* DataToDelete){delete static_cast<Class*>(DataToDelete);};
            ReturnValue.m_ClassID = GetClassID<Class>();
            return ReturnValue;
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
  

    
    class Null 
    { 
    private:
        char PlaceHolder = 0;
    public:
        bool operator==(Null ) const
        {
            return true;   
        }
        bool operator!=(Null) const
        {
            return false;   
        }
    };
    template<typename T,typename... Args> Ref<T> MakeRef(Args... Arguments);

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
   
        //typedef std::variant<Null,bool,Function,Int,Float,Symbol,ThreadHandle,Null> ValueTypes; 

        typedef std::variant<Null,bool,Function,Int,Float,Symbol,ThreadHandle> ValueTypes;
        typedef std::variant<Null,bool,Function,Int,Float,Symbol,ThreadHandle,
            String,
            Macro,
            Lambda,
            List,
            std::unordered_map<Value,Value,Value_Hasher>,
            GenericFunction,
            ClassDefinition,
            ClassInstance,
            Value,
            DynamicVariable,
            Scope> BuiltinTypes;

        template<typename T>
        static constexpr bool IsValueType()
        {
            return TypeIn<T,bool,Function,Int,Float,Symbol,ThreadHandle,Null>;
        }
        template<typename T>
        static constexpr bool IsReferenceType()
        {
            return TypeIn<T,ClassDefinition,DynamicVariable,Macro,Lambda,GenericFunction,ClassInstance,List,Scope,String,
                std::unordered_map<Value,Value,Value_Hasher>>;
        }
        template<typename T>
        static constexpr bool IsBuiltin()
        {
            return IsValueType<T>() || IsReferenceType<T>();
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
        //template<typename VariantType, std::size_t Index = 0>
        //static constexpr std::size_t MaxAlign() 
        //{
        //    //if(std::variant_size_v<VariantType> > Index)
        //    //{
        //    //    return 0;
        //    //}
        //    //size_t ReturnValue = std::max(alignof(decltype(std::get<Index>(VariantType()))),MaxAlign<VariantType,Index+1>());
        //    return 1;
        //} 
        //template<typename VariantType, std::size_t Index = 0>
        //static constexpr std::size_t MaxSize() 
        //{
        //    //if(std::variant_size_v<VariantType> > Index)
        //    //{
        //    //    return 0;
        //    //}
        //    //size_t ReturnValue = std::max(sizeof(decltype(std::get<Index>(VariantType()))),MaxAlign<VariantType,Index+1>());
        //    return 1;
        //} 
        class ValueVariant
        {
            alignas(i_MaxAlign<bool,Function,Int,Float,Symbol,ThreadHandle,Null>::value) char m_Content[i_MaxSize<bool,Function,Int,Float,Symbol,ThreadHandle,Null>::value];
            uint_least8_t m_BuiltinClassID = 1u;//Null type
            //Null,bool,Function,Int,Float,Symbol,ThreadHandle
            static constexpr char m_BuiltinTypeSize[] = {0,sizeof(Null),sizeof(bool),sizeof(Function),sizeof(Int),sizeof(Float),sizeof(Symbol),sizeof(ThreadHandle)};
             
            ///static constexpr int p_BuiltinByteSize(uint_least8_t ClassID) 
            ///{
            ///    return (ClassID >> 4)& 
            ///}
            bool p_BuiltinStored() const
            {
                return !(m_BuiltinClassID & 1u<<7);
            }
            template<typename T>
            T& p_GetType()
            {
                return *std::launder(reinterpret_cast<T*>(m_Content));
            }
            template<typename T>
            T const& p_GetType() const
            {
                return *std::launder(reinterpret_cast<T const*>(m_Content));
            }
            public:
            ValueVariant()
            {
                new (m_Content) Null();
            }
            ValueVariant(RefBase&& ObjectToCopy)
            {
                m_BuiltinClassID = 1u<<7;
                new (m_Content)RefBase(std::move(ObjectToCopy));
            }
            template<typename T,typename = std::enable_if_t<IsValueType<T>(),T>>
            ValueVariant(T ValueType)
            {
                m_BuiltinClassID = VariantIndex<BuiltinTypes,T>()+1;
                new (m_Content)T(ValueType);
            }
            ValueVariant(ValueVariant const& OtherVariant)
            {
                if(OtherVariant.p_BuiltinStored())
                {
                    std::memcpy( (char*)this,(char const*)&OtherVariant,sizeof(ValueVariant));
                }
                else
                {
                    m_BuiltinClassID = 1<<7u;
                    new(m_Content)RefBase(OtherVariant.p_GetType<RefBase>());
                }
            }
            ValueVariant(ValueVariant&& OtherVariant) noexcept
            { 
                if(OtherVariant.p_BuiltinStored())
                {
                    std::memcpy( (char*)this,(char const*)&OtherVariant,sizeof(ValueVariant));
                }
                else
                {
                    m_BuiltinClassID = 1<<7u;
                    new(m_Content)RefBase(std::move(OtherVariant.p_GetType<RefBase>()));
                }
            }
            ValueVariant& operator=(ValueVariant VariantToCopy)
            {
                if(!p_BuiltinStored())
                {
                    std::launder<RefBase>(reinterpret_cast<RefBase*>(m_Content))->~RefBase();
                }
                if(VariantToCopy.p_BuiltinStored())
                {
                    std::memcpy( (char*)this,(char const*)&VariantToCopy,sizeof(ValueVariant));
                }
                else
                {
                    m_BuiltinClassID = 1<<7u;
                    new(m_Content)RefBase(std::move(VariantToCopy.p_GetType<RefBase>()));
                }
                return *this;
            }
            ~ValueVariant()
            {
                if(!p_BuiltinStored())
                {
                    std::launder<RefBase>(reinterpret_cast<RefBase*>(m_Content))->~RefBase();
                }
            }
            bool operator==(ValueVariant const& VariantToCompare) const
            {
                bool ReturnValue = false;
                if(m_BuiltinClassID != VariantToCompare.m_BuiltinClassID)
                {
                    return false;
                }
                else if(p_BuiltinStored())
                {
                    if(m_BuiltinClassID == GetTypeTypeID<Symbol>())
                    {
                        return p_GetType<Symbol>() == VariantToCompare.p_GetType<Symbol>();
                    }
                    int BytesToCompare = m_BuiltinTypeSize[m_BuiltinClassID & (~(1u<<7))];
                    return std::memcmp(m_Content,VariantToCompare.m_Content,BytesToCompare) == 0;;
                }
                else
                {
                    RefBase const& ThisRef = p_GetType<RefBase>();
                    RefBase const& OtherRef = VariantToCompare.p_GetType<RefBase>();
                    if(ThisRef.GetTypeID() !=  OtherRef.GetTypeID())
                    {
                        return false;   
                    }
                    if(ThisRef.GetTypeID() == GetTypeTypeID<String>())
                    {
                        return ThisRef.GetType<String>() == OtherRef.GetType<String>();
                    }
                    return ThisRef == OtherRef;
                }
                return ReturnValue;
            }
            bool operator!=(ValueVariant const& VariantToCompare) const
            {
                return !(*this == VariantToCompare);
            }
            ClassID GetTypeID() const
            {
                if(p_BuiltinStored())
                {
                    return m_BuiltinClassID;
                }
                else
                {
                    return p_GetType<RefBase>().GetTypeID();
                }
            }
            bool IsReference() const
            {
                return !p_BuiltinStored();
            }
            template<typename T>
            T const& GetType() const
            {
                if constexpr(IsValueType<T>())
                {
                    if(!(p_BuiltinStored() || VariantIndex<BuiltinTypes,T>()+1 != m_BuiltinClassID))
                    {
                        throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
                    }
                    return p_GetType<T>();
                }
                else if constexpr(std::is_same_v<T,RefBase>)
                {
                    if(p_BuiltinStored())
                    {
                        throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
                    }
                    return p_GetType<RefBase>();
                }
                else
                {
                    static_assert(!std::is_same_v<T,T>,"Only builtin types and RefBase can be stored in a ValueVariant");
                    return p_GetType<T>();
                }
            }
            template<typename T>
            T& GetType()
            {
                if constexpr(IsValueType<T>())
                {
                    if(!(p_BuiltinStored() || VariantIndex<BuiltinTypes,T>()+1 != m_BuiltinClassID))
                    {
                        throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
                    }
                    return p_GetType<T>();
                }
                else if constexpr(std::is_same_v<T,RefBase>)
                {
                    if(p_BuiltinStored())
                    {
                        throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
                    }
                    return p_GetType<RefBase>();
                }
                else
                {
                    static_assert(!std::is_same_v<T,T>,"Only builtin types and RefBase can be stored in a ValueVariant");   
                    return p_GetType<T>();
                }
            }
        };

        ValueVariant m_Data;


        
        template<typename T>
        T const& p_GetType() const
        {
            if(m_Data.GetTypeID() != GetTypeTypeID<T>())
            {
               throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
            }
            if constexpr(IsValueType<T>())
            {
                return m_Data.GetType<T>();
            }
            else
            {
                return m_Data.GetType<RefBase>().GetType<T>();
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

        Value& operator=(Value const& ValueToCopy) = default;

        bool operator==(Value const& OtherValue) const
        {
            return m_Data == OtherValue.m_Data;
        }
        bool operator!=(Value const& OtherValue) const
        {
            return(!(*this == OtherValue));
        }
        template<typename T> 
        static constexpr ClassID GetTypeTypeID()
        {
            if(std::is_same_v<T,ClassInstance>)
            {
                return 1<<UserClassBit;   
            }
            else if constexpr(std::is_same_v<T,Any>)
            {
                return 0;   
            }
            else if constexpr(IsBuiltin<T>())
            {
                return VariantIndex<BuiltinTypes,T>()+1;
            }
            else
            {
                return GetClassID<T>();   
            }
        }
        bool IsBuiltin() const
        {
            return TypeIsBuiltin(GetTypeID());
        }
        static bool TypeIsBuiltin(ClassID ID)
        {
            return !(ID & (1u<<ExternalClassBit | 1u<<UserClassBit));
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
            return m_Data.GetTypeID() == GetTypeTypeID<T>();
        }
        template<typename T>
        Ref<T> GetRef()
        {
            static_assert(!IsValueType<T>(),"Can only get reference to refernce type");
            if(!IsType<T>())
            {
                throw std::runtime_error("Error getting reference to type in value: type of \""+std::string(typeid(T).name())+"\" not stored");
            }
            return Ref<T>(m_Data.GetType<RefBase>());
        } 
        template<typename T>
        Ref<T> const GetRef() const
        {
            static_assert(!IsValueType<T>(),"Can only get reference to refernce type");
            if(!IsType<T>())
            {
                throw std::runtime_error("Error getting reference to type in value: type of \""+std::string(typeid(T).name())+"\" not stored");
            }
            return Ref<T>(m_Data.GetType<RefBase>());
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
                m_Data = ValueVariant(Function(Rhs));
            }
            else if constexpr (std::is_same_v<T,const char*>)
            {
                m_Data = ValueVariant(MakeRef<String>(Rhs));
            }
            else if constexpr(std::is_same_v<T,bool>)
            {
                m_Data = ValueVariant(Rhs);
            }
            else if constexpr (std::is_integral_v<T>)
            {
                m_Data = ValueVariant(Int(Rhs));
            }
            else if constexpr(IsBuiltin<T>())
            {
                if constexpr(IsValueType<T>())
                {
                    m_Data = ValueVariant(std::move(Rhs));
                }
                else if constexpr(IsReferenceType<T>())
                {
                    m_Data = ValueVariant(MakeRef<T>(std::move(Rhs)));
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
        BuiltinLock GetLock() const
        {
            if(!m_Data.IsReference())
            {
                throw std::runtime_error("Can only aquire lock reference type");   
            }
            return m_Data.GetType<RefBase>().GetLock();
        }
        template<typename T>
        Value(Ref<T> Rhs)
        {
            m_Data = ValueVariant(std::move(Rhs));
        }


        template<typename T>
        static Value MakeExternal(T ExternalObject)
        {
            Value ReturnValue;
            ReturnValue.m_Data = ValueVariant(MakeRef<T>(std::move(ExternalObject)));
            return ReturnValue;
        }
        template<typename T,typename... ArgTypes>
        static Value EmplaceExternal(ArgTypes&&... Args)
        {
            Value ReturnValue;
            ReturnValue.m_Data = ValueVariant(MakeRef<T>(std::forward<ArgTypes>(Args)...));
            return ReturnValue;
        }
        template<typename T>
        Value& operator=(T Rhs)
        {
            if constexpr(IsBuiltin<T>())
            {
                if constexpr(IsValueType<T>())
                {
                    m_Data = ValueVariant(std::move(Rhs));
                }
                else
                {
                    m_Data = MakeRef<T>(std::move(Rhs));
                }
            }
            else
            {
                static_assert(!std::is_same<T,T>::value,"Can only assign builtin types");
            }
            return *this;
        }
    };
    template<typename T,typename... Args>
    Ref<T> MakeRef(Args... Arguments)
    {
        RefContent* NewContent =  new RefContent_Specialised<T>(std::forward<Args>(Arguments)...);
        NewContent->StoredClass = Value::GetTypeTypeID<T>();
        return Ref<T>(NewContent);
    }

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
        Symbol Name;
        ClassID ID = 0;
        std::vector<ClassID> Types;
        //expression to compile
        std::vector<SlotDefinition> SlotDefinitions;
        Ref<FunctionDefinition> SlotInitializers;
        //constructor is optionally run after slot initializers, which are always run
        Ref<Value> Constructor;
    };
    class ClassInstance
    {
        public:
        Ref<ClassDefinition> AssociatedClass;
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
        bool p_TypesAreSatisifed(std::vector<ClassID> const& Overrides,Value const* Begin, Value const* End);
        bool p_TypeIsSatisfied(ClassID Override,Value const& Arg);
    public:
        Symbol Name;
        void AddMethod(std::vector<ClassID> OverridenTypes,Value Callable);
        //TODO more efficient implementation, the current one is the most naive one
        Value* GetMethod(Value* Begin,Value* End);
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
        if(IsType<ClassInstance>())
        {
            ReturnValue = GetType<ClassInstance>().AssociatedClass->ID;
        }
        else
        {
            return m_Data.GetTypeID();
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
        struct ParentScope
        {
            Ref<Scope> AssociatedScope;   
            bool Shadowing = false;
        };
        MBUtility::MBVector<ParentScope,4> m_ParentScope;
        std::unordered_map<SymbolID,Value> m_Variables;
    public:
        void SetParentScope(Ref<Scope> ParentScope);
        void AddParentScope(Ref<Scope> ParentScope);
        void SetShadowingParent(Ref<Scope> ParentScope);
        Value FindVariable(SymbolID Variable);
        void SetVariable(SymbolID Variable,Value NewValue);
        void OverrideVariable(SymbolID Variable,Value NewValue);
        Value* TryGet(SymbolID Variable);
        void Clear();
    };

};
