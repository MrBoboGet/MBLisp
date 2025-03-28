#pragma once
#include "Value.h"
namespace MBLisp
{
    typedef uint_least32_t IPIndex;
    //we go with two 
    struct OpCode_PushVar
    {
        SymbolID ID;
        bool Local = false;
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
    struct OpCode_PushLambda
    {
        OpCode_PushLambda()
        {
                
        }
        OpCode_PushLambda(Lambda LiteralToUse)
        {
            Literal = LiteralToUse;
        }
        Lambda Literal;
    };
    struct OpCode_Set
    {
        int_least32_t LocalSetIndex = -1;
    };
    struct OpCode_PreSet
    {
    };
    struct OpCode_Pop
    {
           
    };
    struct OpCode_Macro
    {
           
    };
    //kinda overloaded, should probably split into separate parts
    struct OpCode_Goto
    {
        IPIndex NewIP = -1;
        int NewStackSize =  -1;
        int NewUnwindSize = -1;
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
        bool Setting = false;
        OpCode_CallFunc(int ArgCount,bool IsSetting)
        {
            ArgumentCount = ArgCount;   
            Setting = IsSetting;
        }
    };
    //signals and unwind stuff
    struct OpCode_Unwind
    {
        OpCode_Unwind(int CurrentUnwindProtectCount)
        {
            TargetUnwindDepth = CurrentUnwindProtectCount;
        }
        IPIndex HandlersEnd = -1;
        int NewStackSize =  -1;
        int TargetUnwindDepth = -1;
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
        int NewStackSize =  -1;
        int UnwindDepth =  -1;
    };
    struct OpCode_RemoveSignalHandlers
    {
    };
    struct OpCode_UnwindProtect_Add
    {
        IPIndex UnwindBegin = -1;
        IPIndex UnwindEnd = -1;
        int EndStackCount = -1;
    };
    struct OpCode_UnwindProtect_Begin
    {
           
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
        setl,
        LAST,
    };
    struct OpCode
    {
        std::variant<
            OpCode_PushVar,
            OpCode_Macro,
            OpCode_Set,
            OpCode_PreSet,
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
            OpCode_UnwindProtect_Begin,
            OpCode_UnwindProtect_Pop,
            //let stuff
            OpCode_PushBindings,
            OpCode_PopBindings,
            OpCode_Eval,
            OpCode_PushLambda
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
        Symbol m_Name;
        std::vector<OpCode> m_OpCodes;
        bool m_Modified = false;
        struct LocationInfo
        {
            Location Loc;
            int Depth = -1;
            LocationInfo()
            {
                   
            }
            LocationInfo(Location NewLoc,int Depth)
            {
                Loc = NewLoc;
                this->Depth = Depth;
            }
        };
        std::unordered_map<IPIndex,LocationInfo> m_OpcodeLocationInfo;

        struct DebugInfo
        {
            bool Trapped = false;
            int Depth = -1;
            Location Loc;
        };
        std::vector<DebugInfo> m_DebugInfo;

        int m_DebugID = 0;
        struct EncodingState
        {
            std::vector<std::vector<std::pair<SymbolID,IPIndex>>> UnResolvedGotos;
            int InSignalHandler = 0;
            int UnwindProtectDepth = 0;
            int ArgumentStackCount = 0;
            bool InLambda = 0;
            std::vector<IPIndex> UnResolvedUnwinds;
            std::vector<IPIndex> UnresolvedReturns;
            //
            std::unordered_map<SymbolID,int> LocalSymbols;
            std::vector<SymbolID> LocalSymNames;
            int TotalLocalSymbolCount = 0;

            int GetLocalSymbolIndex(SymbolID Symbol)
            {
                int ReturnValue = 0;
                auto It = LocalSymbols.find(Symbol);
                if(It != LocalSymbols.end())
                {
                    return It->second;   
                }
                else
                {
                    int NewIndex = TotalLocalSymbolCount;
                    TotalLocalSymbolCount++;
                    LocalSymbols[Symbol] = NewIndex;
                    LocalSymNames.push_back(Symbol);
                    return NewIndex;
                }
                return ReturnValue;
            }
            void AddLocalSymbol(SymbolID Symbol)
            {
                int NewIndex = TotalLocalSymbolCount;
                LocalSymNames.push_back(Symbol);
                TotalLocalSymbolCount++;
                LocalSymbols[Symbol] = NewIndex;
            }
        };
        void p_CreateOpCodes(Value const& ValueToEncode,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState);
        void p_CreateFuncCall(List const& ValueToEncode,std::vector<OpCode>& OutCodes,EncodingState& CurrentState,bool Setting);
        void p_CreateOpCodes(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState);

        void p_WriteProgn(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState,int Offset);

        void p_FillDebugInfo();
        public:
        OpCodeList();
        OpCodeList(Value const& ValueToEncode);
        OpCodeList(Scope& CurrentScope,Value const& ValueToEncode);


        OpCodeList(List const& ListToConvert);
        //OpCodeList(List const& ListToConvert,int Offset,bool InLambda);
        OpCodeList(List const& ListToConvert,int Offset,FunctionDefinition& LamdaDef ,EncodingState const& ParentState);
        OpCodeList(SymbolID ArgID,SymbolID IndexFunc,std::vector<SlotDefinition> const& Initializers);
        void Append(List const& ListToConvert);
        void Append(Value const& ListToConvert);
        void SetName(Symbol NewName)
        {
            m_Name = NewName;   
        }
        IPIndex Size()
        {
            return m_OpCodes.size();   
        }
    };
    class OpCodeExtractor
    {
        Ref<OpCodeList> m_AssociatedList = MakeRef<OpCodeList>();
        IPIndex m_IP = 0;
    public:
        OpCodeExtractor()
        {
               
        }
        OpCodeExtractor(Ref<OpCodeList> OpCodes);
        OpCode& GetCurrentCode();
        void SetIP(IPIndex NewIP);
        IPIndex GetIP() const;
        IPIndex OpCodeCount() const;
        void Pop();
        bool Finished() const;
        void SetEnd();
        
        //Debug stuff
        void SetTrap(Location TrapLocation);
        void ClearTraps();
        void SetDebugID(int ID);
        int GetDebugID();
        bool Modified();
        bool IsTrapped(IPIndex Position);
        int GetDepth(IPIndex Position) const;
        Location GetLocation(IPIndex Position) const;
        Symbol GetName() const;
    };
}

