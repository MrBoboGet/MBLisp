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
#include <MBUtility/Iterator.h>

#include <mutex>

#include <algorithm>
namespace MBLisp
{
    template<typename T,template <typename> class R>
    struct IsTemplateInstantiation : std::false_type { };
    template<typename T,template <typename> class R>
    struct IsTemplateInstantiation<R<T>,R> : std::true_type { 
        typedef T type;
    };


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
    //Defined in Evaluator.h
    class CallContext;

#define BUILTIN_ARGLIST (CallContext& Context ,FuncArgVector& Arguments)

    struct Location
    {
        PositionType Position = -1;
        SymbolID URI = -1;
        bool operator==(Location Rhs) const
        {
            return std::tie(Position,URI) == std::tie(Rhs.Position,Rhs.URI);   
        }
        bool IsEmpty() const
        {
            return Position == -1 && URI == -1;   
        }
    };

    struct LocationHasher
    {
        size_t operator()(Location ObjectToHash) const
        {
            size_t FirstValue = std::hash<PositionType>()(ObjectToHash.Position);
            size_t SecondValue = std::hash<SymbolID>()(ObjectToHash.URI);
            //seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            FirstValue ^= SecondValue+0x9e3779b9+(FirstValue<<6)+(FirstValue>>2);
            return FirstValue;
        }
    };
    template<typename T>
    class ListImpl : public std::vector<T>
    {
        Location m_Location;
        int m_Depth = -1;
public:
        ListImpl(std::initializer_list<T> Elems) : std::vector<T>(Elems)
        {
               
        }
        ListImpl(){};
        void SetLocation(Location NewLoc)
        {
            m_Location = NewLoc;
        }
        Location GetLocation() const
        {
            return m_Location;   
        }
        void SetDepth(int Depth)
        {
            m_Depth = Depth;
        }
        int GetDepth() const
        {
            return m_Depth;   
        }
    };


    typedef ListImpl<Value> List;
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
        bool m_RecursiveLocked = false;
    public:
        BuiltinLock GetLock() const
        {
            if(m_ReferenceMutex == nullptr)
            {
                m_ReferenceMutex = std::make_unique<std::recursive_mutex>();
            }
            return std::make_unique<std::lock_guard<std::recursive_mutex>>(*m_ReferenceMutex);
        }

        void LockRecursionLock()
        {
            if(m_RecursiveLocked)
            {
                throw std::runtime_error("Error accessing variable: recursion detected");
            }
            m_RecursiveLocked = true;
        }
        void UnlockRecursionLock()
        {
            m_RecursiveLocked = false;
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
    void RefDestructor(T& ContentToDelete)
    {
           
    }
    class Scope;
    template<>
    inline void RefDestructor<Scope>(Scope& ScopeToDelete);
   


    template<typename T>
    class RefContent_Specialised_Base : public RefContent
    {
    public:
        T* ContentRef = nullptr;
    };

    template<typename T>
    class RefContent_Specialised : public RefContent_Specialised_Base<T>
    {
    public:
        T Content;
        template<typename... Args>
        RefContent_Specialised(Args&&... Arguments)
            : Content(std::forward<Args>(Arguments)...)
        {
            RefContent_Specialised_Base<T>::ContentRef = &Content;
        }
        virtual ~RefContent_Specialised()
        {
            RefDestructor(Content);
        }
    };
    template<typename T,typename V>
    class RefContent_Specialised_Ptr : public RefContent_Specialised_Base<T>
    {
    public:
        V Ref;
        RefContent_Specialised_Ptr(T* Ptr,V OwningRef)
            : Ref(std::move(OwningRef))
        {
            RefContent_Specialised_Base<T>::ContentRef = Ptr;
        }
        virtual ~RefContent_Specialised_Ptr()
        {

        }
    };

    template<typename T>
    T& RefContent::GetContent()
    {
        return *static_cast<RefContent_Specialised_Base<T>&>(*this).ContentRef;
    }
    template<typename T>
    T const& RefContent::GetContent() const
    {
        return *static_cast<RefContent_Specialised_Base<T> const&>(*this).Content;
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
        void LockRecursionLock() const
        {
            if(m_AllocatedContent == nullptr)
            {
                throw std::runtime_error("Cannot aquire recursion lock of null RefBase");
            }
            m_AllocatedContent->LockRecursionLock();
        }
        void UnlockRecursionLock() const
        {
            if(m_AllocatedContent == nullptr)
            {
                throw std::runtime_error("Cannot aquire recursion lock of null RefBase");
            }
            m_AllocatedContent->UnlockRecursionLock();
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
            std::swap(m_AllocatedContent,ObjectToCopy.m_AllocatedContent);
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
        int RefCount() const
        {
            return m_AllocatedContent->RefCount();   
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
        bool operator==(Ref const& Rhs)  const
        {
            return static_cast<RefBase const&>(*this) == static_cast<RefBase const&>(Rhs);
        }
        bool operator!=(Ref const& Rhs)  const
        {
            return static_cast<RefBase const&>(*this) != static_cast<RefBase const&>(Rhs);
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
        std::shared_ptr<T> GetSharedPtr() 
        {
            T* Ptr = &*(*this);
            return std::shared_ptr<T>(Ptr,[RefOwner=*this](T* ){});
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


    struct Symbol
    {
        SymbolID ID = -1;
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
    class ClassDefinition;
    class ClassInstance;
    class GenericFunction;



    struct FunctionDefinition
    {
        std::vector<Symbol> Arguments;
        MBUtility::MBVector<std::pair<SymbolID,int>,4> LocalVars;
        int LocalSymCount = 0;
        int LocalSymBegin = 0;
        SymbolID RestParameter = 0;
        SymbolID EnvirParameter = 0;
        Ref<OpCodeList> Instructions;
    };
    struct Lambda
    {
        Ref<FunctionDefinition> Definition;
        Ref<Scope> AssociatedScope;
        Symbol Name;
    };
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

    template<typename... Types>
    class TypeList { };
    class PolymorphicContainer
    {
        friend class GenericFunction;
        void* m_Data = nullptr;
        std::vector<std::pair<ClassID,void*>> m_Types;

        std::function<void(void*)> m_Deleter;

        template<typename T,typename P>
        void p_AddTypes(T* DataPointer)
        {
            m_Types.push_back(std::pair<ClassID,void*>(GetClassID<P>(),static_cast<P*>(DataPointer)));
        }
        template<typename T,typename P1,typename P2,typename... P3>
        void p_AddTypes(T* DataPointer)
        {
            p_AddTypes<T,P1>(DataPointer);
            p_AddTypes<T,P2>(DataPointer);
            if constexpr( sizeof...(P3) > 0)
            {
                p_AddTypes<T,P3...>(DataPointer);
            }
        }
    public:

        PolymorphicContainer() = delete;
        PolymorphicContainer& operator=(PolymorphicContainer const&) = delete;
        PolymorphicContainer(PolymorphicContainer&&) = delete;
        PolymorphicContainer(PolymorphicContainer const&) = delete;
        ~PolymorphicContainer()
        {
            m_Deleter(m_Data);   
        }

        template<typename T>
        bool IsType() const
        {
            ClassID ID = GetClassID<T>();
            auto It = std::lower_bound(m_Types.begin(),m_Types.end(),ID,[](std::pair<ClassID,void*> const& lhs,ClassID rhs)
                    {
                        return lhs.first < rhs;
                    });
            return It != m_Types.end() && It->first == ID;
        }

        template<typename T,typename...  PolyTypes,typename... ArgTypes>
        //Kinda hacky...
        PolymorphicContainer(TypeList<T> Type,TypeList<PolyTypes...> Poly,ArgTypes&&... Args)
        {
            T* NewData = new T(std::forward<ArgTypes>(Args)...);
            p_AddTypes<T,T,PolyTypes...>(NewData);
            std::sort(m_Types.begin(),m_Types.end(), [](std::pair<ClassID,void*> const& lhs,std::pair<ClassID,void*> const& rhs)
                    {
                        return lhs.first < rhs.first;
                    } );
            m_Deleter = [](void* ptr){delete reinterpret_cast<T*>(ptr);};
        }
        template<typename T>
        T const& GetType() const
        {
            ClassID ID = GetClassID<T>();
            auto It = std::lower_bound(m_Types.begin(),m_Types.end(),ID,[](std::pair<ClassID,void*> const& lhs,ClassID rhs)
                    {
                        return lhs.first < rhs;
                    });
            if(It != m_Types.end() && It->first == ID)
            {
                return *reinterpret_cast<T const*>(It->second);
            }
            throw std::runtime_error("Error acessing type in PolymorphicContainer: object of type " + std::string(typeid(T).name()) + " not stored");
        }
    };

    
    class Invocable
    {
    public:
        virtual Value operator() BUILTIN_ARGLIST = 0;
    };

    class FunctionObject 
    {
        std::unique_ptr<Invocable> m_Invocable;
    public:
        FunctionObject(){};
        FunctionObject(std::unique_ptr<Invocable> Invocable) : m_Invocable(std::move(Invocable)) { }
        inline Value operator() BUILTIN_ARGLIST;
    };


    class DynamicVariable;

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
    template<typename T,typename... Args> Ref<T> MakeRef(Args&&... Arguments);

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
            std::unordered_map<Value,Value,Value::Value_Hasher>,
            GenericFunction,
            ClassDefinition,
            ClassInstance,
            FunctionObject,
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
                   FunctionObject,
                std::unordered_map<Value,Value,Value::Value_Hasher>>;
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
        class ValueVariant
        {
            alignas(i_MaxAlign<bool,Function,Int,Float,Symbol,ThreadHandle,Null>::value) char m_Content[i_MaxSize<bool,Function,Int,Float,Symbol,ThreadHandle,Null>::value];
            uint_least8_t m_BuiltinClassID = 1u;//Null type
            //Null,bool,Function,Int,Float,Symbol,ThreadHandle
            static constexpr char m_BuiltinTypeSize[] = {0,sizeof(Null),sizeof(bool),sizeof(Function),sizeof(Int),sizeof(Float),sizeof(Symbol),sizeof(ThreadHandle)};
             
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
            if(m_Data.GetTypeID() != GetTypeTypeID<T>() && !IsType<PolymorphicContainer>())
            {
               throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
            }
            if constexpr(IsValueType<T>())
            {
                return m_Data.GetType<T>();
            }
            else
            {
                RefBase const& StoredRef = m_Data.GetType<RefBase>();
                if constexpr(!std::is_same_v<T,PolymorphicContainer>)
                {
                    if(StoredRef.GetTypeID() == GetClassID<PolymorphicContainer>())
                    {
                        return StoredRef.GetType<PolymorphicContainer>().GetType<T>();
                    }
                }
                if constexpr(!std::is_abstract<T>::value)
                {
                    return StoredRef.GetType<T>();
                }
                throw std::runtime_error("Invalid type dereference: type of \""+ std::string(typeid(T).name())+ "\" not stored");
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
            if constexpr(!std::is_same_v<T,PolymorphicContainer>)
            {
                if(IsType<PolymorphicContainer>())
                {
                    RefBase const& StoredRef = m_Data.GetType<RefBase>();
                    return StoredRef.GetType<PolymorphicContainer>().IsType<T>();
                }
            }
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
            if(IsType<PolymorphicContainer>())
            {
                RefContent* NewContent = new RefContent_Specialised_Ptr<T,Value>(&GetType<T>(),*this);
                NewContent->StoredClass = Value::GetTypeTypeID<T>();
                return Ref<T>(NewContent);
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
            if(IsType<PolymorphicContainer>())
            {
                RefContent* NewContent = new RefContent_Specialised_Ptr<T,Value>(&GetType<T>(),*this);
                NewContent->StoredClass = Value::GetTypeTypeID<T>();
                return Ref<T>(NewContent);
            }
            return Ref<T>(m_Data.GetType<RefBase>());
        } 

        template<typename T>
        std::shared_ptr<T> GetSharedPtr() 
        {
            if(!IsType<T>())
            {
                throw std::runtime_error("Invalid type dereference when creating shared_ptr for Value");
            }
            else if(IsValueType<T>())
            {
                throw std::runtime_error("Cannot create shared_ptr from value with a value type");
            }
            T* Ptr = &GetType<T>();
            return std::shared_ptr<T>(Ptr,[RefOwner=*this](T* ){});
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
        template<typename T,typename... PolyTypes,typename... ArgTypes>
        static Value EmplacePolymorphic(ArgTypes&&... Args)
        {
            Value ReturnValue;
            ReturnValue.m_Data = ValueVariant(MakeRef<PolymorphicContainer>(TypeList<T>(),TypeList<PolyTypes...>(),std::forward<ArgTypes>(Args)...));
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

    class ValueRecursionLock
    {
        Value& m_Val;
    public:
        ValueRecursionLock(Value& Val);
        ~ValueRecursionLock();
    };

    template<typename T,typename... Args>
    Ref<T> MakeRef(Args&&... Arguments)
    {
        RefContent* NewContent =  new RefContent_Specialised<T>(std::forward<Args>(Arguments)...);
        NewContent->StoredClass = Value::GetTypeTypeID<T>();
        return Ref<T>(NewContent);
    }

    template<typename T>
    Ref<T> FromShared(std::shared_ptr<T> Ptr)
    {
        RefContent* NewContent = new RefContent_Specialised_Ptr<T,std::shared_ptr<T>>(Ptr.get(),Ptr);
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
            Types.push_back(NewID);
        }
        Symbol Name;
        ClassID ID = 0;
        std::vector<ClassID> Types;
        //expression to compile
        std::vector<SlotDefinition> SlotDefinitions;
        Ref<FunctionDefinition> SlotInitializers;
        Ref<Scope> Envir;

        //constructor is optionally run after slot initializers, which are always run
        Ref<Value> Constructor;
    };
    class ClassInstance
    {
        public:
        Ref<ClassDefinition> AssociatedClass;
        //binary search to find
        std::vector<std::pair<SymbolID,Value>> Slots;

        Value* TryGet(SymbolID ID)
        {
            auto It = std::lower_bound(Slots.begin(),Slots.end(),ID,[](std::pair<SymbolID,Value> const& lhs,SymbolID rhs)
                    {
                        return lhs.first < rhs;
                    });
            if(It != Slots.end() && It->first == ID)
            {
                return &It->second;
            }
            return nullptr;
        }
    };
    class GenericFunction
    {
        //specificity is found argument by argument, the first 
        //argument winning, then the second and so on
        //un specified type is specificty 0, the last used alternative
        std::vector<Value> m_Callables;
        std::vector<std::vector<ClassID>> m_CallablesOverrides;
        std::vector<std::vector<std::pair<ClassID,size_t>>> m_Specifications;

        bool m_EmptyInitialised = false;
        Value m_EmptyCallable;

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



    struct ScopeStackframeInfo
    {
        bool IsStackScope = false;
    };
    struct GenericParent
    {
        virtual Value* TryGetVariable(SymbolID ID) = 0;
    };
    class Scope
    {
        struct ParentScope
        {
            Ref<Scope> AssociatedScope;   
            bool Shadowing = false;
        };


        struct GenericParentScope
        {
            std::unique_ptr<GenericParent> Parent;
            bool Shadowing = false;
        };

        MBUtility::MBVector<ParentScope,4> m_ParentScope;
        MBUtility::MBVector<GenericParentScope,1> m_GenericParents;
        std::unordered_map<SymbolID,Value> m_Variables;

        MBUtility::MBVector<Value,4> m_LocalVars;
        MBUtility::MBVector<std::pair<SymbolID,int>,4> m_LocalVarsNames;
        int m_LocalSymBegin = 0;

        ScopeStackframeInfo  m_StackFrameInfo;
    public:
        Scope() = default;
        Scope(FunctionDefinition const& FunctionDef)
        {
            m_LocalVars.resize(FunctionDef.LocalSymCount);
            m_LocalSymBegin = FunctionDef.LocalSymBegin;
            m_LocalVarsNames = FunctionDef.LocalVars;
        }


        //parent stuff

        void AddGenericParent(std::unique_ptr<GenericParent> Parent,bool Shadowing = false);
        
        void SetParentScope(Ref<Scope> ParentScope);
        void AddParentScope(Ref<Scope> ParentScope);
        void SetShadowingParent(Ref<Scope> ParentScope);
        int ParentCount() const
        {
            return m_ParentScope.size();   
        }
        Ref<Scope> GetParent(int Index)
        {
            if(Index < 0 || Index >= m_ParentScope.size())
            {
                throw std::runtime_error("Parent index out range in GetParent for scope");   
            }
            return m_ParentScope[Index].AssociatedScope;
        }

        //variables reflection

        //very niche, used to construct opcodes when eval in a specific scope
        std::vector<std::pair<SymbolID,int>> GetLocalSyms() const;
        int GetLocalSymBegin() const
        {
            return m_LocalSymBegin;   
        }
        int TotalLocalSymCount() const
        {
            return m_LocalSymBegin + m_LocalVars.size();   
        }
        size_t VarCount() const
        {
            return m_Variables.size();   
        }
        std::vector<SymbolID> Vars() const;

        auto begin()
        {
            return m_Variables.begin();
        }
        auto end()
        {
            return m_Variables.end();
        }

        //setters
        void SetVariable(SymbolID Variable,Value NewValue);
        void OverrideVariable(SymbolID Variable,Value NewValue);
        void SetLocalVariable(int Index,Value NewValue);
        void Clear();
        //assigns as direct offset in current scope
        void SetLocalDirect(int Index,Value NewValue);
        //getters
        Value FindVariable(SymbolID Variable);
        Value* GetOrCreate(SymbolID Variable);
        Value* TryGet(SymbolID Variable);
        Value* TryGetNonShadowing(SymbolID Variable);

        Value& GetLocal(SymbolID Variable);
        Value* TryGetLocalByID(SymbolID Variable);

        


        //deprecated
        ScopeStackframeInfo& GetStackInfo()
        {
            return m_StackFrameInfo;   
        }


        //kinda hacky, might consider doing proper garbage collecting...
    };
    template<>
    inline void RefDestructor<Scope>(Scope& ScopeToDelete)
    {
    }

    inline Value FunctionObject::operator() BUILTIN_ARGLIST
    {
        if(m_Invocable == nullptr)
        {
            throw std::runtime_error("Invalid function object: m_Data was null");
        }
        return (*m_Invocable)(Context,Arguments);
    }

};
