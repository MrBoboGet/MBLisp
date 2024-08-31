#pragma once
#include "Value.h"
#include "OpCodes.h"
#include <unordered_map>

#include <MBUtility/StreamReader.h>
#include <MBUtility/MBInterfaces.h>

#include <MBUtility/MBVector.h>



#include "Threading.h"
#include "DebugInternals.h"

#include "Module.h"

#include "FunctionObjectConverter.h"
namespace MBLisp
{
   


    struct SignalHandler
    {
        ClassID HandledType = -1;
        SymbolID BoundValue = 0;
        IPIndex SignalBegin = -1;
    };
    struct UnwindProtector
    {
        IPIndex UnwindBegin = -1;
    };
    struct NonLocalGotoInfo
    {
        IPIndex ReturnAdress = -1;
        int TargetUnwindDepth = -1;
        int StackSize = -1;
    };
    typedef FuncArgVector StackArgVector;

    struct UnwindProtectInfo
    {
        IPIndex Begin = -1;
        IPIndex End = -1;
        int EndStackCount = -1;
    };
    struct StackFrame
    {
        //TODO MEGA HACKY 
        //added so that ad hoc invocation of constructors can pop
        //uneccesary arguments and can have the first argument be 
        //the constructed value...
        int PopExtra = 0;


        Ref<Scope> StackScope;
        OpCodeExtractor ExecutionPosition;
        StackArgVector ArgumentStack;


        //signal/unwind stuff
        int SignalFrameIndex = -1;
        bool Unwinding = false;
        int ProtectDepth = 0;
        std::vector<SignalHandler> ActiveSignalHandlers;
        std::vector<int> SignalHandlerBunchSize;
        std::vector<UnwindProtectInfo> ActiveUnwindProtectors;
        std::vector<NonLocalGotoInfo> StoredGotos;


        //GC Stuff
        
        //This is set to true whenever a 
        //lambda or (environmnent) is called. Poping a scope with its adress
        // taken requries special care in order to ensure that we don't leak commonly occuring cycles.
        bool ScopeRetrieved = false;


        StackFrame(OpCodeExtractor Extractor) : ExecutionPosition(std::move(Extractor))
        {
        }
        //in order to bootstrap the first stack frame
        StackFrame()
        {
               
        }
    };

    class Evaluator;
    struct ExecutionState;
    class CallContext
    {
        friend class Evaluator;
        ExecutionState* m_CurrentState = nullptr;
        Evaluator*  m_AssociatedEvaluator = nullptr;
        Value m_SetValue;
        bool m_IsSetting = false;
        bool m_ThreadPaused = false;
    public:
        Evaluator& GetEvaluator();
        //kinda sussy
        ExecutionState& GetState();
        bool IsSetting();
        Value const& GetSetValue();
        Value GetVariable(std::string const& VarName);
        void PauseThread();
        bool IsMultiThreaded();
    };
    struct ExecutionState
    {
        friend class Evaluator;
        friend class CallContext;
        friend class DebugState;
        private:
        //-1 means last, other value entails being in a signal
        ThreadID AssociatedThread = 0;
        int FrameTarget = -1;
        bool UnwindingStack = false;
        bool UnwindForced = false;

        //Debug Stuff
        int TraphandlerIndex = -1;
        //these property are added mostly for efficiency,
        //reducing the amount of parameters to builtin parameters
        //and allowing for seamles setting/getting. Does however 
        //depend on the single threadeadness use of ExecutionState
        CallContext CurrentCallContext;
      
        void PopFrame();
        
        std::vector<StackFrame> StackFrames;
        std::unordered_map<DynamicVarID,std::vector<Value>> DynamicBindings;
        std::vector<std::vector<DynamicVarID>> BindingStack;

    public:

        bool InTrapHandler()
        {
            return TraphandlerIndex != -1 && TraphandlerIndex <= StackFrames.size();
        }
        Scope& GetCurrentScope()
        {
            return *StackFrames.back().StackScope;
        }
        int StackSize()
        {
            return StackFrames.size();   
        }
        Ref<Scope> GetScopeRef()
        {
            return StackFrames.back().StackScope;
        }
    };
   

    struct ReadTable
    {
        std::unordered_map<char,Value> Mappings;
        std::vector<std::pair<char,Value>> ExpandMappings;
    };
   


    class UncaughtSignal : public std::exception 
    {
    public:
        Value ThrownValue;
        Ref<Scope> AssociatedScope;
        const char* what() const noexcept 
        {
            if(ThrownValue.IsType<String>())
            {
                return ThrownValue.GetType<String>().c_str();
            }
            return "Uncaught signal";   
        }
    };
    class InvalidCharacter : public std::exception 
    {
    public:
        std::string Message;
        const char* what() const noexcept 
        {
            return Message.c_str();
        }
    };



    


    //TODO kinda hacky, should be temporary, but much more convenient when
    //iterating and prototyping
    class Evaluator
    {

        struct StackTrace
        {
            struct Frame 
            {
                Symbol Name;
                Location FrameLocation;
            };
            std::vector<Frame> Frames;
            String Message;

            StackTrace(ExecutionState& CurrentState,String const& Message);
        };
        String GetString(StackTrace const& Trace);


        SymbolID m_CurrentSymbolID = 1;
        SymbolID m_PrimitiveSymbolMax = 0;
        
        friend class CallContext;
        
        static void Print(Evaluator& AssociatedEvaluator,Value const& ValueToPrint);
        static Value Print BUILTIN_ARGLIST;
        static Value Less BUILTIN_ARGLIST;
        static Value Sort BUILTIN_ARGLIST;
        static Value Plus  BUILTIN_ARGLIST;
        static Int Plus_Int (Int Lhs,Int Rhs);
        static String Plus_String (String& Lhs,String& Rhs);
        static Value CreateList BUILTIN_ARGLIST;
        static bool InsertElements(List& Lhs,List& Rhs);
        static Value CreateDict BUILTIN_ARGLIST;


        //classes and generics
        static void p_MergeClasses(std::vector<ClassDefinition*> const& ClassesToMerge,ClassDefinition& NewClass);
        static Value Class BUILTIN_ARGLIST;
        static Value AddMethod BUILTIN_ARGLIST;
        static Value Generic BUILTIN_ARGLIST;
        static Value ConstructDynamicVariable(Value DefaultValue);
        static Value Dynamic BUILTIN_ARGLIST;

        //Operators
        static Value Less_String BUILTIN_ARGLIST;
        static Value Less_Int BUILTIN_ARGLIST;
        static Value Less_Symbol BUILTIN_ARGLIST;
        //builtin containers
        //List
        static Value Index_List BUILTIN_ARGLIST;
        static Value Append_List BUILTIN_ARGLIST;
        static Value Back_List BUILTIN_ARGLIST;
        static Value Len_List BUILTIN_ARGLIST;
        static Value Flatten_1 BUILTIN_ARGLIST;
        static Value Pop_List(List& ListToModify);
        static bool Reverse_List(List& ListToModify);
        //class instance
        static Value Index_ClassInstance BUILTIN_ARGLIST;
        static List Slots_ClassInstance(ClassInstance& InstanceToInspect);


        static Value Expand BUILTIN_ARGLIST;

        //Streams
        static Value Stream_ReadTerm BUILTIN_ARGLIST;
        static Value Stream_Symbol BUILTIN_ARGLIST;
        static Value Stream_ReadString BUILTIN_ARGLIST;
        static Value Stream_ReadNumber BUILTIN_ARGLIST;
        static Value Stream_ReadBytes BUILTIN_ARGLIST;
        static Value Stream_ReadLine BUILTIN_ARGLIST;
        static Value Stream_Position BUILTIN_ARGLIST;
        static Value Stream_ReadUntil BUILTIN_ARGLIST;
        static Value Stream_EOF BUILTIN_ARGLIST;
        static Value Stream_PeakByte BUILTIN_ARGLIST;
        static Value Stream_ReadByte BUILTIN_ARGLIST;
        static Value Stream_SkipWhitespace BUILTIN_ARGLIST;
        static bool Seek_Stream(MBUtility::MBSearchableInputStream& InStream,Int Offset);
        static Int Offset_Stream(MBUtility::MBSearchableInputStream& InStream);
        //Blocking
        
        //Non-blocking

        //Various
        static Value Eq_String BUILTIN_ARGLIST;
        static Value Eq_Symbol BUILTIN_ARGLIST;
        static Value Eq_Int BUILTIN_ARGLIST;
        static Value Eq_Bool BUILTIN_ARGLIST;
        static Value Eq_Null BUILTIN_ARGLIST;
        static Value Eq_Type BUILTIN_ARGLIST;
        static bool Is_Type(ClassDefinition& Lhs,ClassDefinition& Rhs);
        static Value Eq_Any BUILTIN_ARGLIST;
        static bool Eq_ThreadHandle(ThreadHandle  lhs,ThreadHandle rhs);
        static Value Minus_Int BUILTIN_ARGLIST;

        static Value In_String BUILTIN_ARGLIST;
        static Value In_Environment BUILTIN_ARGLIST;
        static Value Str_Symbol BUILTIN_ARGLIST;
        static Value Str_Int BUILTIN_ARGLIST;
        static String Str_String(String& Input);
        static String Str_ThreadHandle(ThreadHandle Input);
        static Int Int_ThreadHandle(ThreadHandle Input);
        static Value Str_Bool BUILTIN_ARGLIST;
        static Value Str_Null BUILTIN_ARGLIST;
        static Value Str_Float BUILTIN_ARGLIST;
        static Value Str_StackTrace BUILTIN_ARGLIST;
        static Value Symbol_String BUILTIN_ARGLIST;
        static Value Symbol_SymbolInt BUILTIN_ARGLIST;
        static Value GenSym BUILTIN_ARGLIST;

        static Value Int_Str BUILTIN_ARGLIST;
        static Value IndexOf_StrStr BUILTIN_ARGLIST;
        static Value Substr BUILTIN_ARGLIST;

        //Dict
        static Value Index_Dict BUILTIN_ARGLIST;
        static Value Keys_Dict BUILTIN_ARGLIST;
        static Value In_Dict BUILTIN_ARGLIST;
        static Int Len_Dict(Dict& DictToInspect);


        
        //String manip
        static Value Split_String BUILTIN_ARGLIST;
        static Value Len_String BUILTIN_ARGLIST;
        static Value Append_String BUILTIN_ARGLIST;

        //Environment
        static Value Environment BUILTIN_ARGLIST;
        static Value NewEnvironment BUILTIN_ARGLIST;
        static Value Index_Environment BUILTIN_ARGLIST;
        static Value SetVar_Environment BUILTIN_ARGLIST;
        static Value Shadow_Environment BUILTIN_ARGLIST;
        static Value SetParent_Environment BUILTIN_ARGLIST;
        static Value AddParent_Environment BUILTIN_ARGLIST;
        static Value Clear_Environment BUILTIN_ARGLIST;
        static List Vars (Scope& Environemnt);
        static List AllVars (Scope& Environemnt);
       
        //Readtable
        static Value AddReaderCharacter BUILTIN_ARGLIST;
        static Value RemoveReaderCharacter BUILTIN_ARGLIST;
        static Value AddCharacterExpander BUILTIN_ARGLIST;
        static Value RemoveCharacterExpander BUILTIN_ARGLIST;
       
        //
        static Value Load BUILTIN_ARGLIST;
        static Value Type BUILTIN_ARGLIST;

        //filesystem stuff
        static Value Exists BUILTIN_ARGLIST;
        static Value Cwd BUILTIN_ARGLIST;
        static Value ParentPath BUILTIN_ARGLIST;
        static Value PathID BUILTIN_ARGLIST;
        static Value Canonical BUILTIN_ARGLIST;
        static Value UserHomeDir BUILTIN_ARGLIST;
        static Value ListDir BUILTIN_ARGLIST;
        static Value IsDirectory BUILTIN_ARGLIST;
        static Value IsFile BUILTIN_ARGLIST;
        
        //IO
        static Value Write_OutStream BUILTIN_ARGLIST;
        static Value Flush_OutStream BUILTIN_ARGLIST;
        static Value OutStream_String BUILTIN_ARGLIST;
        static Value InStream_String BUILTIN_ARGLIST;
        static Value Open_URI BUILTIN_ARGLIST;
        static bool Flush(MBUtility::MBOctetOutputStream& OutStream);
        static bool Close(MBUtility::MBFileOutputStream& OutStream);


        //special stuff
        static Value GetInternalModule BUILTIN_ARGLIST;
        static Value InternalModules BUILTIN_ARGLIST;
        static Value IsSpecial_Symbol BUILTIN_ARGLIST;
        static Value Position_Symbol BUILTIN_ARGLIST;
        static Value URI_Symbol BUILTIN_ARGLIST;

        static Value SetName_Macro BUILTIN_ARGLIST;
        static Value SetName_Lambda BUILTIN_ARGLIST;
        static Value SetName_Generic BUILTIN_ARGLIST;
        static Value SetName_ClassDefinition BUILTIN_ARGLIST;
        static Value Name_Macro BUILTIN_ARGLIST;
        static Value Name_Lambda BUILTIN_ARGLIST;
        static Value Name_Generic BUILTIN_ARGLIST;
        static Value Name_ClassDefinition BUILTIN_ARGLIST;
        static Value Applicable BUILTIN_ARGLIST;



        static Value TestTest BUILTIN_ARGLIST;

        //Threading
        ThreadingState m_ThreadingState;

        struct LispStackFrame
        {
            Ref<Scope> StackScope;
            Symbol Name;
            Location Position;
            Int Depth = -1;
        };
        static Value Thread BUILTIN_ARGLIST;
        static Value This_Thread BUILTIN_ARGLIST;
        static Value Sleep BUILTIN_ARGLIST;
        static Value Pause BUILTIN_ARGLIST;
        static Value Remove BUILTIN_ARGLIST;
        static Value Resume BUILTIN_ARGLIST;
        static Value ActiveThreads BUILTIN_ARGLIST;
        static Value GetStackFrames BUILTIN_ARGLIST;
        static Value StackCount BUILTIN_ARGLIST;
        static ThreadHandle Thread_Handle(Int ID);
        static Ref<Scope> GetScope(LispStackFrame& StackeFrame);
        static Symbol GetName_Stackframe(LispStackFrame& StackeFrame);
        static Symbol GetLocation_StackFrame(LispStackFrame& StackeFrame);
        static Int GetDepth_StackFrame(LispStackFrame& StackeFrame);


        //Debugging
        DebugState m_DebugState;


        std::unordered_map<std::string,std::unique_ptr<Module>> m_BuiltinModules;
        std::unordered_map<std::string,Ref<Scope>> m_LoadedModules;

        std::unordered_map<std::string,SymbolID> m_InternedSymbols;
        std::unordered_map<SymbolID,std::string> m_SymbolToString;
        std::unordered_map<ClassID,Value> m_BuiltinTypeDefinitions;

        template<typename T>
        void p_RegisterBuiltinClass(std::string const& Name)
        {
            auto SymID = p_GetSymbolID(Name);
            auto ClassDef = ClassDefinition(Value::GetTypeTypeID<T>());
            ClassDef.Name.ID = SymID;
            m_GlobalScope->SetVariable(SymID,std::move(ClassDef));
            m_BuiltinTypeDefinitions[Value::GetTypeTypeID<T>()] = m_GlobalScope->FindVariable(SymID);
        }
        template<typename Original,typename New>
        void p_RegisterBuiltinClass()
        {
            m_BuiltinTypeDefinitions[Value::GetTypeTypeID<New>()] = m_BuiltinTypeDefinitions[Value::GetTypeTypeID<Original>()];
        }
        Ref<Scope> m_GlobalScope = MakeRef<Scope>();
        //easiest possible testable variant

        class ContinueUnwind{};
        
        bool p_ValueIsType(ClassID TypeValue,Value const& ValueToInspect);
        std::string p_TypeString(Value const& ValueToInspect);
        Value p_GetDynamicValue(Scope& AssociatedScope,ExecutionState& CurrentState,std::string const& VariableName);

        void p_Invoke(Value& ObjectToCall,FuncArgVector& Arguments,ExecutionState& CurrentState,bool Setting = false,bool IsTrapHandler = false);
        void p_InvokeTrapHandler(ExecutionState& State);
        void p_EmitSignal(ExecutionState& State,Value SignalToEmit,bool ForceUnwind);
        //The fundamental dispatch loop
        //Return index is the stack frame index where the value of the previous call should be returned instead of continuing evaluating 
        //further down the stack.
        //As a dummy stack is always present of bootstraping scopes, so is the lowest value this can take 1
        Value p_Eval(ExecutionState& CurrentState,int ReturnIndex);
        Value p_Eval(ExecutionState& CurrentState,Ref<OpCodeList> OpCodes,IPIndex  Offset = 0);
        //Value p_Eval(Ref<Scope> AssociatedScope,FunctionDefinition& FunctionToExecute,std::vector<Value> Arguments);

        void p_SkipWhiteSpace(MBUtility::StreamReader& Content);
        

        Value p_Expand(ExecutionState&  CurrentState,Scope& Namespace,Value ValueToExpand);
        Value p_Expand(ExecutionState&  CurrentState,Scope& Namespace,List const& ListToExpand);
        Value p_GetExecutableTerm(ExecutionState&  CurrentState,Scope& Namespace,Value ValueToExpand);

        //reading
        String p_ReadString(MBUtility::StreamReader& Content);
        Value p_ReadSymbol(ExecutionState&  CurrentState, SymbolID URI,Ref<ReadTable>& Table,MBUtility::StreamReader& Content);
        Int p_ReadInteger(MBUtility::StreamReader& Content);
        List p_ReadList(ExecutionState&  CurrentState,SymbolID URI,Ref<ReadTable>& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        Value p_ReadTerm(ExecutionState&  CurrentState,SymbolID URI,Ref<ReadTable>& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        
        SymbolID p_GetSymbolID(std::string const& SymbolString);
        
        bool p_SymbolIsPrimitive(SymbolID IDToCompare);
        void p_InternPrimitiveSymbols();
        void p_LoadModules();



        template<typename T>
        void p_AddType(std::vector<ClassID>& Types)
        {
            typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type Type;
            typedef IsTemplateInstantiation<Type,Ref> IsRefType;
            if constexpr(std::is_same_v<Type,Any>)
            {
                Types.push_back(0);
            }
            else if constexpr(std::is_same_v<Type,Value>)
            {
                Types.push_back(0);
            }
            else if constexpr(IsRefType::value)
            {
                //Types.push_back(0);
                p_AddType<typename IsRefType::type>(Types);
            }
            else
            {
                Types.push_back(Value::GetTypeTypeID<Type>());
            }
        }
        template<typename T,typename... Extra>
        void p_AddTypes(std::vector<ClassID>& Types)
        {
            p_AddType<T>(Types);
            if constexpr(sizeof...(Extra) > 0)
            {
                p_AddTypes<Extra...>(Types);
            }
        }


        //returns the read-table
        Value p_LoadFile(ExecutionState& CurrentState,std::filesystem::path const& LoadFilePath,bool KeepReadtable = false);


        template<typename ClassToConvert, Value (ClassToConvert::*MemberMethod)()>
        static Value p_MemberConverter BUILTIN_ARGLIST
        {
            return (Arguments[0].GetType<ClassToConvert>().*MemberMethod)();
        }
        struct i_Empty{};
        template <int CurrentIndex,int TargetIndex,typename... Types>
        struct i_TypeExtractor 
        { 
        };
        template <int CurrentIndex,int TargetIndex,typename T>
        struct i_TypeExtractor<CurrentIndex,TargetIndex,T>
        { 
            typedef T type;
        };
        //template <int CurrentIndex,int TargetIndex,typename T>
        //struct i_TypeExtractor<CurrentIndex,TargetIndex,Ref<T>>
        //{ 
        //    typedef T type;
        //};
        template <int CurrentIndex,int TargetIndex,typename T,typename... Rest>
        struct i_TypeExtractor<CurrentIndex,TargetIndex,T,Rest...> : 
        std::conditional_t<CurrentIndex == TargetIndex,
            i_TypeExtractor<0,0,T>, 
            i_TypeExtractor<CurrentIndex+1,TargetIndex,Rest...>>
        {

        };



        template <
            typename ObjectType,
            typename ReturnType,
            typename... TotalArgTypes,
            typename... SuppliedArgTypes>
        static ReturnType p_InvokeMemberMethod(
                ObjectType& InvokingObject,
                ReturnType (ObjectType::*MemberMethod)(TotalArgTypes...),
                FuncArgVector& LispArgs,
                SuppliedArgTypes&&... Args)
        {
            if constexpr(sizeof...(TotalArgTypes) == sizeof...(SuppliedArgTypes))
            {
                return (InvokingObject.*MemberMethod)(std::forward<SuppliedArgTypes>(Args)...);
            }
            else
            {
                //+1 beccause the first argument  is always the type of the invoking object
                if(sizeof...(SuppliedArgTypes) + 1 >= LispArgs.size())
                {
                    throw std::runtime_error("Insufficient arguments supplied");   
                }
                typedef typename std::remove_cv<
                        typename std::remove_reference<typename i_TypeExtractor<0,sizeof...(SuppliedArgTypes),TotalArgTypes...>::type>::type>::type 
                        ArgType;
                typedef IsTemplateInstantiation<ArgType,Ref> IsRefType;
                if constexpr(IsRefType::value)
                {
                    return p_InvokeMemberMethod(InvokingObject,MemberMethod,LispArgs,std::forward<SuppliedArgTypes>(Args)...,
                        LispArgs[sizeof...(SuppliedArgTypes)+1].GetRef<typename IsRefType::type>());
                }
                else
                {
                    return p_InvokeMemberMethod(InvokingObject,MemberMethod,LispArgs,std::forward<SuppliedArgTypes>(Args)...,
                        LispArgs[sizeof...(SuppliedArgTypes)+1].GetType<ArgType>());
                }
            }
        }

        template<typename ObjectType,typename ReturnType,auto Func>
        static Value p_MemberMethodConverter BUILTIN_ARGLIST
        {
            ObjectType& InvokingObject = Arguments[0].GetType<ObjectType>();
            if constexpr(std::is_same_v<ReturnType,Value> || Value::IsBuiltin<ReturnType>() || IsTemplateInstantiation<ReturnType,Ref>::value)
            {
                return p_InvokeMemberMethod(InvokingObject,Func,Arguments);   
            }
            if constexpr(!std::is_same_v<ReturnType,void>)
            {
                return Value::EmplaceExternal<ReturnType>(p_InvokeMemberMethod(InvokingObject,Func,Arguments));
            }
            else
            {
               p_InvokeMemberMethod(InvokingObject,Func,Arguments);
               return Value();
            }
        }

        template <
            typename ReturnType,
            typename... TotalArgTypes,
            typename... SuppliedArgTypes>
        static ReturnType p_InvokeFunction(
                ReturnType (*Function)(TotalArgTypes...),
                FuncArgVector& LispArgs,
                SuppliedArgTypes&&... Args)
        {
            if constexpr(sizeof...(TotalArgTypes) == sizeof...(SuppliedArgTypes))
            {
                if constexpr(!std::is_same_v<ReturnType,void>)
                {
                    return Function(std::forward<SuppliedArgTypes>(Args)...);
                }
                else
                {
                    Function(std::forward<SuppliedArgTypes>(Args)...);
                }
            }
            else
            {
                //+1 beccause the first argument  is always the type of the invoking object
                if(sizeof...(SuppliedArgTypes) >= LispArgs.size())
                {
                    throw std::runtime_error("Insufficient arguments supplied");   
                }

                typedef typename std::remove_cv<
                            typename std::remove_reference<typename i_TypeExtractor<0,sizeof...(SuppliedArgTypes),TotalArgTypes...>::type>::type>::type ArgType;
                typedef IsTemplateInstantiation<ArgType,Ref> IsRefType;
                if constexpr(std::is_same_v<Value,ArgType>)
                {
                    return p_InvokeFunction(Function,LispArgs,std::forward<SuppliedArgTypes>(Args)...,
                        LispArgs[sizeof...(SuppliedArgTypes)]);
                }
                else if constexpr(IsRefType::value)
                {
                    return p_InvokeFunction(Function,LispArgs,std::forward<SuppliedArgTypes>(Args)...,
                        LispArgs[sizeof...(SuppliedArgTypes)].GetRef<ArgType>());
                }
                else
                {
                    return p_InvokeFunction(Function,LispArgs,std::forward<SuppliedArgTypes>(Args)...,
                        LispArgs[sizeof...(SuppliedArgTypes)].GetType<ArgType>());
                }
            }
        }

        template<typename ReturnType,auto Func>
        static Value p_FunctionConverter BUILTIN_ARGLIST
        {
            if constexpr(std::is_same_v<ReturnType,void>)
            {
                p_InvokeFunction(Func,Arguments);
                return Value();
            }
            else if constexpr(std::is_same_v<ReturnType,Value> || Value::IsBuiltin<ReturnType>() || IsTemplateInstantiation<ReturnType,Ref>::value)
            {
                return p_InvokeFunction(Func,Arguments);   
            }
            else
            {
                if constexpr(!std::is_same_v<ReturnType,void>)
                {
                    return Value::EmplaceExternal<ReturnType>(p_InvokeFunction(Func,Arguments));
                }
                else
                {
                    p_InvokeFunction(Func,Arguments);
                    return Value();
                }
            }
        }


        template<auto Func,typename ClassType,typename ReturnType,typename... ArgTypes>
        void AddObjectMethod(Ref<Scope>& ScopeToModify,std::string const& MethodName,ReturnType (ClassType::*MemberMethod)(ArgTypes...))
        {
            static_assert(std::is_same_v<decltype(Func),decltype(MemberMethod)>,"Func and supplied func has to be of the same argument");
            std::vector<ClassID> Types;
            p_AddType<ClassType>(Types);
            if constexpr(sizeof...(ArgTypes) > 0)
            {
                p_AddTypes<ArgTypes...>(Types);
            }
            SymbolID GenericSymbol = p_GetSymbolID(MethodName);
            if(ScopeToModify->TryGet(GenericSymbol) == nullptr)
            {
                 ScopeToModify->SetVariable(GenericSymbol,GenericFunction());
            }
            GenericFunction& AssociatedFunction = ScopeToModify->FindVariable(GenericSymbol).GetType<GenericFunction>();
            AssociatedFunction.Name = p_GetSymbolID(MethodName);
            AssociatedFunction.AddMethod(std::move(Types),Value(p_MemberMethodConverter<ClassType,ReturnType,Func>));
        }


        template<auto Func,typename ReturnType,typename... ArgTypes>
        void AddGeneric(Ref<Scope>& ScopeToModify,std::string const& MethodName,ReturnType (*MemberMethod)(ArgTypes...))
        {
            //static_assert(std::is_same_v<decltype(Func),decltype(MemberMethod)>,"Func and supplied func has to be of the same argument");
            std::vector<ClassID> Types;
            if constexpr(sizeof...(ArgTypes) > 0)
            {
                p_AddTypes<ArgTypes...>(Types);
            }
            SymbolID GenericSymbol = p_GetSymbolID(MethodName);
            if(ScopeToModify->TryGet(GenericSymbol) == nullptr)
            {
                 ScopeToModify->SetVariable(GenericSymbol,GenericFunction());
            }
            GenericFunction& AssociatedFunction = ScopeToModify->FindVariable(GenericSymbol).GetType<GenericFunction>();
            AssociatedFunction.Name = p_GetSymbolID(MethodName);
            AssociatedFunction.AddMethod(std::move(Types),Value(p_FunctionConverter<ReturnType,Func>));
        }

        template<typename T,typename ReturnType,typename ObjectType,typename... ArgTypes>
        void p_AddFunctionObject(Ref<Scope>& ScopeToModify,std::string const& MethodName, ReturnType(ObjectType::* Func)(ArgTypes...),T Callable)
        {
            std::vector<ClassID> Types;
            if constexpr(sizeof...(ArgTypes) > 0)
            {
                p_AddTypes<ArgTypes...>(Types);
            }
            SymbolID GenericSymbol = p_GetSymbolID(MethodName);
            if(ScopeToModify->TryGet(GenericSymbol) == nullptr)
            {
                 ScopeToModify->SetVariable(GenericSymbol,GenericFunction());
            }
            GenericFunction& AssociatedFunction = ScopeToModify->FindVariable(GenericSymbol).GetType<GenericFunction>();
            AssociatedFunction.Name = p_GetSymbolID(MethodName);
            AssociatedFunction.AddMethod(std::move(Types),Value( FunctionObject( std::make_unique<FunctionObjectConverter<T>>(std::move(Callable)))));
        }
		

        static SymbolID p_PathURI(Evaluator& AssociatedEvaluator,std::string const& Path);
        static SymbolID p_PathURI(Evaluator& AssociatedEvaluator,std::filesystem::path const& Path);
    public:
        template<auto  Func>
        void AddGeneric(std::string const& MethodName)
        {
            AddGeneric<Func>(m_GlobalScope,MethodName);
        }
        template<auto  Func>
        void AddGeneric(Ref<Scope>& ScopeToModify,std::string const& FunctionName)
        {
            AddGeneric<Func>(ScopeToModify,FunctionName,Func);
        }

        template<auto  Func>
        void AddObjectMethod(std::string const& MethodName)
        {
            AddObjectMethod<Func>(m_GlobalScope,MethodName);
        }
        template<auto  Func>
        void AddObjectMethod(Ref<Scope>& ScopeToModify,std::string const& MethodName)
        {
            AddObjectMethod<Func>(ScopeToModify,MethodName,Func);
        }


        template<typename T>
        void AddFunctionObject(Ref<Scope>& ScopeToModify,std::string const& MethodName,T Object)
        {
            p_AddFunctionObject(ScopeToModify,MethodName,&T::operator(),std::move(Object));
        }
        template<typename  T>
        void AddFunctionObject(std::string const& MethodName,T Object)
        {
            AddFunctionObject(m_GlobalScope,MethodName,std::move(Object));
        }


        Evaluator();

        Evaluator(Evaluator&&) = delete;
        Evaluator(Evaluator const&) = delete;
        Evaluator& operator==(Evaluator const&) = delete;

        Ref<Scope> CreateDefaultScope();
        //is this sussy or not...
        DebugState&  GetDebugState();

        template<typename... ArgTypes>
        void AddMethod(Ref<Scope> ScopeToModify,std::string const& MethodName,Value Callable)
        {
            std::vector<ClassID> Types;
            p_AddTypes<ArgTypes...>(Types);
            SymbolID GenericSymbol = p_GetSymbolID(MethodName);
            if(ScopeToModify->TryGet(GenericSymbol) == nullptr)
            {
                 ScopeToModify->SetVariable(GenericSymbol,GenericFunction());
            }
            GenericFunction& AssociatedFunction = ScopeToModify->FindVariable(GenericSymbol).GetType<GenericFunction>();
            AssociatedFunction.Name = p_GetSymbolID(MethodName);
            AssociatedFunction.AddMethod(std::move(Types),std::move(Callable));
        }
        template<typename... ArgTypes>
        void AddMethod(std::string const& MethodName,Value Callable)
        {
            AddMethod<ArgTypes...>(m_GlobalScope,MethodName,std::move(Callable));
        }

        //template<typename ClassType,typename... ArgTypes>
        //void AddMemberMethod(Ref<Scope> ScopeToModify,std::string const& MethodName,Value (ClassType::*MemberMethod)(ArgTypes...))
        //{

        //}
        template<typename ClassType,Value (ClassType::*MemberMethod)()>
        void AddMemberMethod(Ref<Scope> ScopeToModify,std::string const& MethodName)
        {
            AddMethod<ClassType>(ScopeToModify,MethodName,p_MemberConverter<ClassType,MemberMethod>);
        }
        template<typename ClassType,Value (ClassType::*MemberMethod)()>
        void AddMemberMethod(std::string const& MethodName)
        {
            AddMethod<ClassType>(m_GlobalScope,MethodName,p_MemberConverter<ClassType,MemberMethod>);
        }

        class TestClass
        {
            String m_InteralString =  "UwU";
            public:
            String TestTest(String Test1,Int Test2)
            {
                return Test1+std::to_string(Test2)+" "+m_InteralString;
            }
        };
        static String GenericTest(Int Hello,String World)
        {
            return "Hello world! "+std::to_string(Hello)+World;   
        }

        SymbolID GenerateSymbol();

        SymbolID GetSymbolID(std::string const& SymbolString);
        std::string GetSymbolString(SymbolID SymbolToConvert);

        void LoadStd();
        void Eval(std::filesystem::path const& SourceFile);
        void Repl();
        Value Eval(ExecutionState& CurrentState,Value Callable,FuncArgVector Arguments);
        Value Eval(Ref<Scope> AssociatedScope,Value Callable,FuncArgVector Arguments);
        Value Eval(Value Callable,FuncArgVector Arguments);
        void Unwind(ExecutionState& CurrentState,int TargetIndex);

        void AddInternalModule(std::string const& Name,Ref<Scope> ModuleScope);
    };
}
