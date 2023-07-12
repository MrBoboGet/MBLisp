#pragma once
#include "Value.h"
#include "OpCodes.h"
#include <unordered_map>

#include <MBUtility/StreamReader.h>
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
    struct StackFrame
    {
        //TODO MEGA HACKY 
        //added so that ad hoc invocation of constructors can pop
        //uneccesary arguments and can have the first argument be 
        //the constructed value...
        int PopExtra = 0;


        std::shared_ptr<Scope> StackScope;
        OpCodeExtractor ExecutionPosition;
        std::vector<Value> ArgumentStack;


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
        int FrameTarget = -1;
        bool UnwindingStack = false;
        std::vector<StackFrame> StackFrames;
        std::unordered_map<DynamicVarID,std::vector<Value>> DynamicBindings;
        std::vector<std::vector<DynamicVarID>> BindingStack;
    };
   

    struct ReadTable
    {
        std::unordered_map<char,Value> Mappings;
    };
   

    //TODO kinda hacky, should be temporary, but much more convenient when
    //iterating and prototyping
#define BUILTIN_ARGLIST (Evaluator& AssociatedEvaluator,Ref<Scope> CurrentScope,std::vector<Value>& Arguments)
    class Evaluator
    {
        SymbolID m_CurrentSymbolID = 1;
        SymbolID m_PrimitiveSymbolMax = 0;

        static void Print(Evaluator& AssociatedEvaluator,Value const& ValueToPrint);
        static Value Print BUILTIN_ARGLIST;
        static Value Less BUILTIN_ARGLIST;
        static Value Plus  BUILTIN_ARGLIST;
        static Value CreateList BUILTIN_ARGLIST;


        //classes and generics
        static void p_MergeClasses(std::vector<ClassDefinition*> const& ClassesToMerge,ClassDefinition& NewClass);
        static Value Class BUILTIN_ARGLIST;
        static Value AddMethod BUILTIN_ARGLIST;
        static Value Generic BUILTIN_ARGLIST;
        static Value Dynamic BUILTIN_ARGLIST;



        //builtin containers
        //List
        static Value Index_List BUILTIN_ARGLIST;
        static Value Append_List BUILTIN_ARGLIST;
        static Value Len_List BUILTIN_ARGLIST;
        static Value Flatten_1 BUILTIN_ARGLIST;
        //class instance
        static Value Index_ClassInstance BUILTIN_ARGLIST;
        //Streams
        static Value ReadTerm BUILTIN_ARGLIST;
        static Value Stream_EOF BUILTIN_ARGLIST;
        static Value Stream_PeakByte BUILTIN_ARGLIST;
        static Value Stream_ReadByte BUILTIN_ARGLIST;
        static Value Stream_SkipWhitespace BUILTIN_ARGLIST;

        //Various
        static Value Eq_String BUILTIN_ARGLIST;
        static Value Eq_Symbol BUILTIN_ARGLIST;
        static Value Eq_Int BUILTIN_ARGLIST;
        static Value GenSym BUILTIN_ARGLIST;


        //Environment
        static Value Environment BUILTIN_ARGLIST;
        static Value NewEnvironment BUILTIN_ARGLIST;
        static Value Index_Environment BUILTIN_ARGLIST;
        
        //READING
        
        std::unordered_map<std::string,SymbolID> m_InternedSymbols;
        std::unordered_map<SymbolID,std::string> m_SymbolToString;
        std::shared_ptr<Scope> m_GlobalScope = std::make_shared<Scope>();
        //easiest possible testable variant

        bool p_ValueIsType(ClassID TypeValue,Value const& ValueToInspect);

        void p_Invoke(Value& ObjectToCall,std::vector<Value>& Arguments,std::vector<StackFrame>& CurrentCallStack);
        //The fundamental dispatch loop
        Value p_Eval(ExecutionState& CurrentState);
        Value p_Eval(std::vector<StackFrame> CurrentCallStack);
        Value p_Eval(std::shared_ptr<Scope> CurrentScope,Ref<OpCodeList> OpCodes,IPIndex  Offset = 0);
        //Value p_Eval(std::shared_ptr<Scope> AssociatedScope,FunctionDefinition& FunctionToExecute,std::vector<Value> Arguments);
        Value p_Eval(Ref<Scope> CurrentScope,Value Callable,std::vector<Value> Arguments);

        void p_SkipWhiteSpace(MBUtility::StreamReader& Content);
        

        Value p_Expand(std::shared_ptr<Scope> ExpandScope,Value ValueToExpand);
        Value p_Expand(std::shared_ptr<Scope> ExpandScope,List& ListToExpand);

        //reading
        String p_ReadString(MBUtility::StreamReader& Content);
        Value p_ReadSymbol(MBUtility::StreamReader& Content);
        Int p_ReadInteger(MBUtility::StreamReader& Content);
        List p_ReadList(std::shared_ptr<Scope> AssociatedScope,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        Value p_ReadTerm(std::shared_ptr<Scope> AssociatedScope,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        List p_Read(std::shared_ptr<Scope> AssociatedScope,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue);
        
        SymbolID p_GetSymbolID(std::string const& SymbolString);
        
        bool p_SymbolIsPrimitive(SymbolID IDToCompare);
        void p_InternPrimitiveSymbols();



        template<typename T,typename... Extra>
        void p_AddTypes(std::vector<ClassID>& Types)
        {
            Types.push_back(Value::GetTypeTypeID<T>());
            if constexpr(sizeof...(Extra) > 0)
            {
                p_AddTypes<Extra...>(Types);
            }
        }
    public:
        Evaluator();

        Ref<Scope> CreateDefaultScope();
        template<typename... ArgTypes>
        void AddMethod(std::string const& MethodName,Value Callable)
        {
            std::vector<ClassID> Types;
            p_AddTypes<ArgTypes...>(Types);
            SymbolID GenericSymbol = p_GetSymbolID(MethodName);
            if(m_GlobalScope->TryGet(GenericSymbol) == nullptr)
            {
                 m_GlobalScope->SetVariable(GenericSymbol,GenericFunction());
            }
            GenericFunction& AssociatedFunction = m_GlobalScope->FindVariable(GenericSymbol).GetType<GenericFunction>();
            AssociatedFunction.AddMethod(std::move(Types),std::move(Callable));
        }
        SymbolID GenerateSymbol();

        SymbolID GetSymbolID(std::string const& SymbolString);
        std::string GetSymbolString(SymbolID SymbolToConvert);
        void Eval(std::string_view Content);
        void Eval(std::shared_ptr<Scope>& CurrentScope,std::string_view Content);
    };
}
