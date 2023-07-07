#pragma once
#include "Value.h"
#include "OpCodes.h"
#include <unordered_map>

#include <MBUtility/StreamReader.h>
namespace MBLisp
{
   

    class Evaluator;
    class Scope
    {
        std::shared_ptr<Scope> m_ParentScope = nullptr;
        std::unordered_map<SymbolID,Value> m_Variables;
    public:
        void SetParentScope(std::shared_ptr<Scope> ParentScope);
        Value FindVariable(SymbolID Variable);
        void SetVariable(SymbolID Variable,Value NewValue);
        Value* TryGet(SymbolID Variable);
    };

    struct StackFrame
    {
        std::shared_ptr<Scope> StackScope;
        OpCodeExtractor ExecutionPosition;
        std::vector<Value> ArgumentStack;

        StackFrame(OpCodeExtractor Extractor) : ExecutionPosition(std::move(Extractor))
        {
        }
    };
   

    struct ReadTable
    {
        std::unordered_map<char,Value> Mappings;
    };
    
    class Evaluator
    {
        friend class Scope;
        SymbolID m_CurrentSymbolID = 1;
        SymbolID m_PrimitiveSymbolMax = 0;

        static void Print(Evaluator& AssociatedEvaluator,Value const& ValueToPrint);
        static Value Print(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Less(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Plus(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value CreateList(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);


        //classes and generics
        static void p_MergeClasses(std::vector<ClassDefinition*> const& ClassesToMerge,ClassDefinition& NewClass);
        static Value Class(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value AddMethod(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Generic(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);



        //builtin containers
        //List
        static Value Index_List(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Append_List(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Len_List(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Flatten_1(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        //class instance
        static Value Index_ClassInstance(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        //Streams
        static Value ReadTerm(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Stream_EOF(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Stream_PeakByte(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Stream_ReadByte(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);
        static Value Stream_SkipWhitespace(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);

        //Various
        static Value Eq_String(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments);

        //READING
        
        std::unordered_map<std::string,SymbolID> m_InternedSymbols;
        std::unordered_map<SymbolID,std::string> m_SymbolToString;
        std::shared_ptr<Scope> m_GlobalScope = std::make_shared<Scope>();
        //easiest possible testable variant



        void p_Invoke(Value& ObjectToCall,std::vector<Value>& Arguments,std::vector<StackFrame>& CurrentCallStack);
        //The fundamental dispatch loop
        Value p_Eval(std::vector<StackFrame>& CurrentCallStack);
        Value p_Eval(std::shared_ptr<Scope> CurrentScope,OpCodeList& OpCodes,IPIndex  Offset = 0);
        //Value p_Eval(std::shared_ptr<Scope> AssociatedScope,FunctionDefinition& FunctionToExecute,std::vector<Value> Arguments);
        Value p_Eval(Value Callable,std::vector<Value> Arguments);

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

        
        //TEMP AF
        ReadTable& GetReadTable();
        SymbolID GetSymbolID(std::string const& SymbolString);
        std::string GetSymbolString(SymbolID SymbolToConvert);
        void Eval(std::string_view Content);
        Value Eval(Value Callable,std::vector<Value> Arguments);
    };
}
