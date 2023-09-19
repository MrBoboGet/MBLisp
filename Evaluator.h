#pragma once
#include "Value.h"
#include "OpCodes.h"
#include <unordered_map>

#include <MBUtility/StreamReader.h>
#include <MBUtility/MBInterfaces.h>

#include <MBUtility/MBVector.h>


#include <thread>
#include <mutex>
#include <condition_variable>

#include "Threading.h"
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
    typedef FuncArgVector StackArgVector;
    struct StackFrame
    {
        //TODO MEGA HACKY 
        //added so that ad hoc invocation of constructors can pop
        //uneccesary arguments and can have the first argument be 
        //the constructed value...
        int PopExtra = 0;


        std::shared_ptr<Scope> StackScope;
        OpCodeExtractor ExecutionPosition;
        StackArgVector ArgumentStack;


        //signal/unwind stuff
        int SignalFrameIndex = -1;
        bool Unwinding = false;
        std::vector<SignalHandler> ActiveSignalHandlers;
        std::vector<int> SignalHandlerBunchSize;
        std::vector<IPIndex> ActiveUnwindProtectorsBegin;


        StackFrame(OpCodeExtractor Extractor) : ExecutionPosition(std::move(Extractor))
        {
        }
    };

    struct ExecutionState
    {
        //-1 means last, other value entails being in a signal
        ThreadID ThreadID = 0;
        int FrameTarget = -1;
        bool UnwindingStack = false;
        bool UnwindForced = false;
        std::vector<StackFrame> StackFrames;
        std::unordered_map<DynamicVarID,std::vector<Value>> DynamicBindings;
        std::vector<std::vector<DynamicVarID>> BindingStack;
    };
   

    struct ReadTable
    {
        std::unordered_map<char,Value> Mappings;
        std::vector<std::pair<char,Value>> ExpandMappings;
    };
   


    class Evaluator;
    class Module
    {
    public:
        virtual Ref<Scope> GetModuleScope() =  0;
        virtual void SetEvaluator(Evaluator* AssociatedEvaluator) = 0;
    };


    class UncaughtSignal : public std::exception 
    {
    public:
        Value ThrownValue;
        const char* what() const noexcept 
        {
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
#define BUILTIN_ARGLIST (Evaluator& AssociatedEvaluator,Ref<Scope> CurrentScope,FuncArgVector& Arguments)
    class Evaluator
    {
        SymbolID m_CurrentSymbolID = 1;
        SymbolID m_PrimitiveSymbolMax = 0;

        static void Print(Evaluator& AssociatedEvaluator,Value const& ValueToPrint);
        static Value Print BUILTIN_ARGLIST;
        static Value Less BUILTIN_ARGLIST;
        static Value Sort BUILTIN_ARGLIST;
        static Value Plus  BUILTIN_ARGLIST;
        static Value CreateList BUILTIN_ARGLIST;
        static Value CreateDict BUILTIN_ARGLIST;


        //classes and generics
        static void p_MergeClasses(std::vector<ClassDefinition*> const& ClassesToMerge,ClassDefinition& NewClass);
        static Value Class BUILTIN_ARGLIST;
        static Value AddMethod BUILTIN_ARGLIST;
        static Value Generic BUILTIN_ARGLIST;
        static Value Dynamic BUILTIN_ARGLIST;

        //Operators
        static Value Less_String BUILTIN_ARGLIST;
        static Value Less_Int BUILTIN_ARGLIST;
        static Value Less_Symbol BUILTIN_ARGLIST;
        //builtin containers
        //List
        static Value Index_List BUILTIN_ARGLIST;
        static Value Append_List BUILTIN_ARGLIST;
        static Value Len_List BUILTIN_ARGLIST;
        static Value Flatten_1 BUILTIN_ARGLIST;
        //class instance
        static Value Index_ClassInstance BUILTIN_ARGLIST;
        //Streams
        static Value Expand BUILTIN_ARGLIST;
        static Value Stream_ReadTerm BUILTIN_ARGLIST;
        static Value Stream_ReadString BUILTIN_ARGLIST;
        static Value Stream_ReadNumber BUILTIN_ARGLIST;
        static Value Stream_ReadBytes BUILTIN_ARGLIST;
        static Value Stream_ReadLine BUILTIN_ARGLIST;
        static Value Stream_EOF BUILTIN_ARGLIST;
        static Value Stream_PeakByte BUILTIN_ARGLIST;
        static Value Stream_ReadByte BUILTIN_ARGLIST;
        static Value Stream_SkipWhitespace BUILTIN_ARGLIST;

        //Various
        static Value Eq_String BUILTIN_ARGLIST;
        static Value Eq_Symbol BUILTIN_ARGLIST;
        static Value Eq_Int BUILTIN_ARGLIST;
        static Value Eq_Type BUILTIN_ARGLIST;
        static Value Eq_Any BUILTIN_ARGLIST;
        static Value Minus_Int BUILTIN_ARGLIST;

        static Value In_String BUILTIN_ARGLIST;
        static Value In_Environment BUILTIN_ARGLIST;
        static Value Str_Symbol BUILTIN_ARGLIST;
        static Value Str_Int BUILTIN_ARGLIST;
        static Value Str_Bool BUILTIN_ARGLIST;
        static Value Str_Null BUILTIN_ARGLIST;
        static Value Str_Float BUILTIN_ARGLIST;
        static Value Symbol_String BUILTIN_ARGLIST;
        static Value Symbol_SymbolInt BUILTIN_ARGLIST;
        static Value GenSym BUILTIN_ARGLIST;


        //Dict
        static Value Index_Dict BUILTIN_ARGLIST;
        static Value Keys_Dict BUILTIN_ARGLIST;
        static Value In_Dict BUILTIN_ARGLIST;


        
        //String manip
        static Value Split_String BUILTIN_ARGLIST;
        static Value Len_String BUILTIN_ARGLIST;

        //Environment
        static Value Environment BUILTIN_ARGLIST;
        static Value NewEnvironment BUILTIN_ARGLIST;
        static Value Index_Environment BUILTIN_ARGLIST;
        static Value SetVar_Environment BUILTIN_ARGLIST;
        static Value Shadow_Environment BUILTIN_ARGLIST;
        static Value SetParent_Environment BUILTIN_ARGLIST;
        static Value Clear_Environment BUILTIN_ARGLIST;
       
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
        static Value UserHomeDir BUILTIN_ARGLIST;
        static Value ListDir BUILTIN_ARGLIST;
        static Value IsDirectory BUILTIN_ARGLIST;
        
        //IO
        static Value Write_OutStream BUILTIN_ARGLIST;
        static Value Flush_OutStream BUILTIN_ARGLIST;
        static Value OutStream_String BUILTIN_ARGLIST;
        static Value InStream_String BUILTIN_ARGLIST;
        static Value Open_URI BUILTIN_ARGLIST;


        //special stuff
        static Value GetInternalModule BUILTIN_ARGLIST;
        static Value InternalModules BUILTIN_ARGLIST;
        static Value IsSpecial_Symbol BUILTIN_ARGLIST;
        static Value Position_Symbol BUILTIN_ARGLIST;
        static Value SetName_Macro BUILTIN_ARGLIST;
        static Value SetName_Lambda BUILTIN_ARGLIST;
        static Value SetName_Generic BUILTIN_ARGLIST;
        static Value SetName_ClassDefinition BUILTIN_ARGLIST;
        static Value Name_Macro BUILTIN_ARGLIST;
        static Value Name_Lambda BUILTIN_ARGLIST;
        static Value Name_Generic BUILTIN_ARGLIST;
        static Value Name_ClassDefinition BUILTIN_ARGLIST;

        //Threading
        ThreadingState m_ThreadingState;

        static Value Thread BUILTIN_ARGLIST;
        static Value This_Thread BUILTIN_ARGLIST;
        static Value Sleep BUILTIN_ARGLIST;




        std::unordered_map<std::string,std::unique_ptr<Module>> m_BuiltinModules;
        std::unordered_map<std::string,SymbolID> m_InternedSymbols;
        std::unordered_map<SymbolID,std::string> m_SymbolToString;
        std::unordered_map<ClassID,Value> m_BuiltinTypeDefinitions;

        template<typename T>
        void p_RegisterBuiltinClass(std::string const& Name)
        {
            m_GlobalScope->SetVariable(p_GetSymbolID(Name),ClassDefinition(Value::GetTypeTypeID<T>()));
            m_BuiltinTypeDefinitions[Value::GetTypeTypeID<T>()] = m_GlobalScope->FindVariable(p_GetSymbolID(Name));
        }
        std::shared_ptr<Scope> m_GlobalScope = std::make_shared<Scope>();
        //easiest possible testable variant

        
        bool p_ValueIsType(ClassID TypeValue,Value const& ValueToInspect);

        void p_Invoke(Value& ObjectToCall,FuncArgVector& Arguments,ExecutionState& CurrentState);
        void p_EmitSignal(ExecutionState& State,Value SignalToEmit,bool ForceUnwind);
        //The fundamental dispatch loop
        Value p_Eval(ExecutionState& CurrentState);
        Value p_Eval(std::vector<StackFrame> CurrentCallStack);
        Value p_Eval(std::shared_ptr<Scope> CurrentScope,Ref<OpCodeList> OpCodes,IPIndex  Offset = 0);
        //Value p_Eval(std::shared_ptr<Scope> AssociatedScope,FunctionDefinition& FunctionToExecute,std::vector<Value> Arguments);

        void p_SkipWhiteSpace(MBUtility::StreamReader& Content);
        

        Value p_Expand(std::shared_ptr<Scope> ExpandScope,Value ValueToExpand);
        Value p_Expand(std::shared_ptr<Scope> ExpandScope,List const& ListToExpand);

        //reading
        String p_ReadString(MBUtility::StreamReader& Content);
        Value p_ReadSymbol(Ref<Scope> ReadScope, SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content);
        Int p_ReadInteger(MBUtility::StreamReader& Content);
        List p_ReadList(std::shared_ptr<Scope> AssociatedScope,SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        Value p_ReadTerm(std::shared_ptr<Scope> AssociatedScope,SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        List p_Read(std::shared_ptr<Scope> AssociatedScope,SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        
        SymbolID p_GetSymbolID(std::string const& SymbolString);
        
        bool p_SymbolIsPrimitive(SymbolID IDToCompare);
        void p_InternPrimitiveSymbols();
        void p_LoadModules();




        template<typename T,typename... Extra>
        void p_AddTypes(std::vector<ClassID>& Types)
        {
            if constexpr(std::is_same_v<T,Any>)
            {
                Types.push_back(0);
            }
            else
            {
                Types.push_back(Value::GetTypeTypeID<T>());
            }
            if constexpr(sizeof...(Extra) > 0)
            {
                p_AddTypes<Extra...>(Types);
            }
        }


        void p_LoadFile(Ref<Scope> AssocatedScope,std::filesystem::path const& LoadFilePath);


        template<typename ClassToConvert, Value (ClassToConvert::*MemberMethod)()>
        static Value p_MemberConverter BUILTIN_ARGLIST
        {
            return (Arguments[0].GetType<ClassToConvert>().*MemberMethod)();
        }
    public:
        Evaluator();

        Evaluator(Evaluator&&) = delete;
        Evaluator(Evaluator const&) = delete;
        Evaluator& operator==(Evaluator const&) = delete;

        Ref<Scope> CreateDefaultScope();

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
        SymbolID GenerateSymbol();

        SymbolID GetSymbolID(std::string const& SymbolString);
        std::string GetSymbolString(SymbolID SymbolToConvert);

        void LoadStd();
        void Eval(std::filesystem::path const& SourceFile);
        void Repl();
        Value Eval(Ref<Scope> CurrentScope,Value Callable,FuncArgVector Arguments);
    };
}
