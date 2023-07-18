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
        int NewStackSize =  -1;
        bool ReturnTop = false;
    };
    struct OpCode_JumpNotTrue
    {
        IPIndex NewIP;
    };
    class OpCode;
    struct OpCode_HandleSignal
    {
        std::shared_ptr<std::vector<OpCode>> SignalCode;
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
    //signals and unwind stuff
    struct OpCode_Unwind
    {
        IPIndex HandlersEnd = -1;
    };
    struct OpCode_Signal
    {
        bool HasForced = false;
    };
    //as many values on the stack, denoting type, as signal handlers pushed here
    struct OpCode_AddSignalHandlers
    {
        struct SignalHandler
        {
            SymbolID BoundVariable = -1;
            IPIndex HandlerBegin = -1;
        };
        std::vector<SignalHandler> Handlers;
    };
    struct OpCode_SignalHandler_Done
    {
        IPIndex HandlersEnd = -1;
    };
    struct OpCode_RemoveSignalHandlers
    {
    };
    struct OpCode_UnwindProtect_Add
    {
        IPIndex UnwindBegin = -1;
    };
    struct OpCode_UnwindProtect_Pop
    {
    };
    //let stuff
    //BindCount amount of triples, oscope, symbol and value
    struct OpCode_PushBindings
    {
        int BindCount = 0;
        OpCode_PushBindings() = default;
        OpCode_PushBindings(int NewBindCount)
        {
            BindCount = NewBindCount;
        }
    };
    struct OpCode_PopBindings
    {
    };
    //eval
    struct OpCode_Eval
    {
        OpCode_Eval() = default;
        int ArgCount = 1;
        OpCode_Eval(int Arguments)
        {
            ArgCount = Arguments;   
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
        //
        signal_handlers,
        signal,
        unwind,
        unwind_protect,
        bind_dynamic,
        eval,
        Return,
        LAST,
    };
    struct OpCode
    {
        std::variant<
            OpCode_PushVar,
            OpCode_Macro,
            OpCode_Set,
            OpCode_PushLiteral,
            OpCode_Pop,
            OpCode_Goto,
            OpCode_JumpNotTrue,
            OpCode_CallFunc,
            //
            OpCode_Unwind,
            OpCode_Signal,
            OpCode_AddSignalHandlers,
            OpCode_SignalHandler_Done,
            OpCode_RemoveSignalHandlers,
            OpCode_UnwindProtect_Add,
            OpCode_UnwindProtect_Pop,
            //let stuff
            OpCode_PushBindings,
            OpCode_PopBindings,
            OpCode_Eval
                > m_Data;
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
            int InSignalHandler = 0;
            int ArgumentStackCount = 0;
            bool InLambda = 0;
            std::vector<IPIndex> UnResolvedUnwinds;
            std::vector<IPIndex> UnresolvedReturns;
        };
        void p_CreateOpCodes(Value const& ValueToEncode,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState);
        void p_CreateFuncCall(List const& ValueToEncode,std::vector<OpCode>& OutCodes,EncodingState& CurrentState);
        void p_CreateOpCodes(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState);

        void p_WriteProgn(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState,int Offset);
        public:
        OpCodeList();
        OpCodeList(Value const& ValueToEncode);
        OpCodeList(List const& ListToConvert);
        OpCodeList(List const& ListToConvert,int Offset,bool InLambda);
        OpCodeList(SymbolID ArgID,SymbolID IndexFunc,std::vector<SlotDefinition> const& Initializers);
        void Append(List const& ListToConvert);
        void Append(Value const& ListToConvert);
        IPIndex Size()
        {
            return m_OpCodes.size();   
        }
    };
    class OpCodeExtractor
    {
        Ref<OpCodeList> m_AssociatedList = nullptr;
        IPIndex m_IP = 0;
    public:
        OpCodeExtractor()
        {
               
        }
        OpCodeExtractor(Ref<OpCodeList> OpCodes);
        OpCode& GetCurrentCode();
        void SetIP(IPIndex NewIP);
        IPIndex GetIP();
        void Pop();
        bool Finished() const;
        void SetEnd();
    };
}

