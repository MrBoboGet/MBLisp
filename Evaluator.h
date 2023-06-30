#pragma once
#include "Value.h"
#include <unordered_map>
namespace MBLisp
{
    typedef uint_least32_t IPIndex;
    //we go with two 
    struct OpCode_PushVar
    {
        SymbolID ID;
    };
    struct OpCode_PushLiteral
    {
        Value Literal;
    };
    struct OpCode_Set
    {

    };
    struct OpCode_Pop
    {
           
    };
    struct OpCode_Goto
    {
        IPIndex NewIP;
    };
    struct OpCode_Jump
    {
        IPIndex NewIP;
    };
    struct OpCode_JumpNotTrue
    {
        IPIndex NewIP;
    };
    struct OpCode_CallFunc
    {
        //determines the amount of vales to take from the stack when calling the function
        int ArgumentCount = 0;
    };
    struct OpCode
    {
        std::variant<OpCode_PushVar,OpCode_Set,OpCode_PushLiteral,OpCode_Pop,OpCode_Goto,OpCode_JumpNotTrue,OpCode_Jump,OpCode_CallFunc> m_Data;
    public:
        OpCode() = default;
        OpCode(OpCode const&) = default;
        OpCode(OpCode&&) noexcept = default;
        OpCode& operator=(OpCode const&) = default;
        OpCode& operator=(OpCode&&) = default;

        template<typename T>
        OpCode(T NewValue)
        {
            m_Data = NewValue;
        }
        template<typename T>
        bool IsType() const
        {
            return std::holds_alternative<T>(m_Data);
        }
        template<typename T>
        T& GetType()
        {
            return std::get<T>(m_Data);
        }
        template<typename T>
        T const& GetType() const
        {
            return std::get<T>(m_Data);
        }
        template<typename T>
        OpCode& operator=(T NewValue)
        {
            m_Data = std::move(NewValue);
            return *this;
        }
    };


    class OpCodeList
    {
        friend class OpCodeExtractor;
        
        std::vector<OpCode> m_OpCodes;

        struct EncodingState
        {
            std::vector<std::pair<SymbolID,IPIndex>> UnResolvedGotos;
        };
        
        void p_CreateOpCodes(Value const& ValueToEncode,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState);
        void p_CreateFuncCall(List const& ValueToEncode,std::vector<OpCode>& OutCodes,EncodingState& CurrentState);
        void p_CreateOpCodes(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState);
        public:
        OpCodeList();
        OpCodeList(List const& ListToConvert);
    };
    class OpCodeExtractor
    {
        OpCodeList* m_AssociatedList = nullptr;
        IPIndex m_IP = 0;
    public:
        OpCodeExtractor(OpCodeList& OpCodes);
        OpCode& GetCurrentCode();
        void SetIP(IPIndex NewIP);
        IPIndex GetIP();
        void Pop();
        bool Finished() const;
    };
   

    class Evaluator;
    class Scope
    {
        Evaluator* m_AssociatedEvaluator;
        std::unordered_map<SymbolID,Value> m_Variables;
    public:
        Scope(Evaluator* AssoicatedEvaluator)
        {
            m_AssociatedEvaluator = AssoicatedEvaluator;
        }
        Value FindVariable(SymbolID Variable);
        void SetVariable(SymbolID Variable,Value NewValue);
    };

    struct StackFrame
    {
        Scope StackScope;
        OpCodeExtractor ExecutionPosition;
        std::vector<Value> ArgumentStack;
        StackFrame(OpCodeExtractor Extractor,Evaluator* AssociatedEvaluator) : ExecutionPosition(std::move(Extractor)),StackScope(AssociatedEvaluator)
        {
        }
    };
    
    enum class PrimitiveForms
    {
        cond = 1,
        tagbody,
        go,
        set,
        LAST
    };
    
    class Evaluator
    {
        friend class Scope;
        SymbolID m_CurrentSymbolID = 1;

        typedef Value (*BuiltinFuncType)(std::vector<Value>&);
        static Value Print(std::vector<Value>& Arguments);
        std::unordered_map<FunctionID,BuiltinFuncType> m_Builtins;

        std::unordered_map<std::string,SymbolID> m_InternedSymbols;
        std::unordered_map<SymbolID,std::string> m_SymbolToString;
        

        //easiest possible testable variant
        OpCodeList m_OpCodes;

        //The fundamental dispatch loop
        void p_Eval(OpCodeList& OpCodes);

        void p_SkipWhiteSpace(std::string_view& Content);

        String p_ReadString(std::string_view& Content);
        Symbol p_ReadSymbol(std::string_view& Content);
        Int p_ReadInteger(std::string_view& Content);
        List p_ReadList(std::string_view& Content);
        Value p_ReadTerm(std::string_view& Content);
        List p_Read(std::string_view Content);
        
        SymbolID p_GetSymbolID(std::string const& SymbolString);
        
        bool p_SymbolIsPrimitive(SymbolID IDToCompare);
        void p_InternPrimitiveSymbols();
    public:
        Evaluator();
        void Eval(std::string_view Content);
    };
}
