#pragma once
#include "Value.h"
namespace MBLisp
{
    typedef uint_least32_t IPIndex;
    //we go with two 
    struct OpCode_PushVar
    {
        SymbolID ID;
        OpCode_PushVar()
        {
               
        }
        OpCode_PushVar(SymbolID NewID)
        {
            ID = NewID;   
        }
    };
    struct OpCode_PushLiteral
    {
        OpCode_PushLiteral()
        {
                
        }
        OpCode_PushLiteral(Value LiteralToUse)
        {
            Literal = LiteralToUse;
        }
        Value Literal;
    };
    struct OpCode_Set
    {
    };
    struct OpCode_Pop
    {
           
    };
    struct OpCode_Macro
    {
           
    };
    struct OpCode_Goto
    {
        IPIndex NewIP = -1;
        bool ResetStack = true;
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
        OpCode_CallFunc() {};
        OpCode_CallFunc(int ArgCount)
        {
            ArgumentCount = ArgCount;   
        }
    };
    enum class PrimitiveForms
    {
        cond = 1,
        tagbody,
        go,
        set,
        lambda,
        progn,
        quote,
        macro,//should probably replace with function call semantics
        LAST
    };
    struct OpCode
    {
        std::variant<OpCode_PushVar,OpCode_Macro,OpCode_Set,OpCode_PushLiteral,OpCode_Pop,OpCode_Goto,OpCode_JumpNotTrue,OpCode_Jump,OpCode_CallFunc> m_Data;
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

        void p_WriteProgn(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState,int Offset);
        public:
        OpCodeList();
        OpCodeList(List const& ListToConvert);
        OpCodeList(List const& ListToConvert,int Offset);
        OpCodeList(SymbolID ArgID,SymbolID IndexFunc,std::vector<SlotDefinition> const& Initializers);
        void Append(List const& ListToConvert);
        IPIndex Size()
        {
            return m_OpCodes.size();   
        }
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
}

