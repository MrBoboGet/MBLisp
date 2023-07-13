#include "OpCodes.h"
#include <unordered_map>
#include <assert.h>


#include "Value.h"
namespace MBLisp
{
    OpCodeList::OpCodeList()
    {
           
    }
    void OpCodeList::p_CreateOpCodes(Value const& ValueToEncode, std::vector<OpCode>& ListToAppend,EncodingState& CurrentState)
    {
        if(ValueToEncode.IsType<List>())
        {
            p_CreateOpCodes(ValueToEncode.GetType<List>(),ListToAppend,CurrentState);
        }
        else if( ValueToEncode.IsType<Symbol>())
        {
            OpCode_PushVar NewCode;
            NewCode.ID = ValueToEncode.GetType<Symbol>().ID;
            ListToAppend.push_back(NewCode);
        }
        else
        {
            //push every other kind of literal
            OpCode_PushLiteral NewCode;
            NewCode.Literal = ValueToEncode;
            ListToAppend.push_back(NewCode);
        }
    }
    void OpCodeList::p_CreateFuncCall(List const& ListToConvert,std::vector<OpCode>& OutCodes,EncodingState& CurrentState)
    {
        for(int i = 1; i < ListToConvert.size();i++)
        {
            p_CreateOpCodes(ListToConvert[i],OutCodes,CurrentState);
            CurrentState.ArgumentStackCount += 1;
        }
        p_CreateOpCodes(ListToConvert.front(),OutCodes,CurrentState);
        CurrentState.ArgumentStackCount -= ListToConvert.size()-1;
        OpCode_CallFunc FunCall;
        FunCall.ArgumentCount = ListToConvert.size()-1;
        m_OpCodes.push_back(FunCall);
           
    }
    void OpCodeList::p_CreateOpCodes(List const& ListToConvert, std::vector<OpCode>& ListToAppend,EncodingState& CurrentState)
    {
        if(ListToConvert.empty())
        {
            throw std::runtime_error("Cannot evalaute empty s-expression");
        }
        if(ListToConvert[0].IsType<Symbol>())
        {
            SymbolID CurrentSymbol = ListToConvert[0].GetType<Symbol>().ID;
            if(CurrentSymbol < SymbolID(PrimitiveForms::LAST))
            {
                if(CurrentSymbol == SymbolID(PrimitiveForms::cond))
                {
                    if(ListToConvert.size() != 4)
                    {
                        throw std::runtime_error("Cond requires exactly 3 argumnets: condition, true path, and false path");   
                    }
                    p_CreateOpCodes(ListToConvert[1],ListToAppend,CurrentState);
                    IPIndex JumpToFalsePathPosition = ListToAppend.size();
                    ListToAppend.push_back(OpCode_JumpNotTrue());
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    IPIndex GotoFalseEndPosition = ListToAppend.size();
                    ListToAppend.push_back(OpCode_Goto());
                    IPIndex FalsePathBegin = ListToAppend.size();
                    p_CreateOpCodes(ListToConvert[3],ListToAppend,CurrentState);
                    IPIndex FalsePathEnd = ListToAppend.size();
                    ListToAppend[JumpToFalsePathPosition].GetType<OpCode_JumpNotTrue>().NewIP = FalsePathBegin;
                    ListToAppend[GotoFalseEndPosition].GetType<OpCode_Goto>().NewIP = FalsePathEnd;
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::tagbody))
                {
                    if( (ListToConvert.size() & 1) != 1)
                    {
                        throw std::runtime_error("tagbody requires an even amount of arguments, an arbitrary amount of symbol and body pairs");
                    }
                    int CurrentOffset = 1;
                    std::unordered_map<SymbolID,IPIndex> SymbolToOffsetMap;
                    while(CurrentOffset < ListToConvert.size())
                    {
                        if(!ListToConvert[CurrentOffset].IsType<Symbol>())
                        {
                            throw std::runtime_error("first part of a tagbody pairs has to be a symbol");
                        }
                        SymbolToOffsetMap[ListToConvert[CurrentOffset].GetType<Symbol>().ID] = ListToAppend.size();
                        p_CreateOpCodes(ListToConvert[CurrentOffset+1],ListToAppend,CurrentState);
                        if (CurrentOffset + 2 < ListToConvert.size())
                        {
                            ListToAppend.push_back(OpCode_Pop());
                        }
                        CurrentOffset += 2;
                    }
                    std::vector<std::pair<SymbolID,IPIndex>> NewUnresolvedSymbols;
                    for(auto const& Pair : CurrentState.UnResolvedGotos) 
                    {
                        if(SymbolToOffsetMap.find(Pair.first) != SymbolToOffsetMap.end())
                        {
                            ListToAppend[Pair.second].GetType<OpCode_Goto>().NewIP = SymbolToOffsetMap[Pair.first];
                        }
                        else
                        {
                            NewUnresolvedSymbols.push_back(Pair);
                        }
                    }
                    std::swap(CurrentState.UnResolvedGotos,NewUnresolvedSymbols);
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::go))
                {
                    if(ListToConvert.size() != 2)
                    {
                        throw std::runtime_error("go requiers exactly 1 argument, label of jump");
                    }
                    if(!ListToConvert[1].IsType<Symbol>())
                    {
                        throw std::runtime_error("argument of go special form must be a symbol");
                    }
                    IPIndex GOIndex = ListToAppend.size();
                    SymbolID GoSymbol = ListToConvert[1].GetType<Symbol>().ID;
                    OpCode_Goto NewOpcode;
                    NewOpcode.NewStackSize = CurrentState.ArgumentStackCount;
                    ListToAppend.push_back(NewOpcode);
                    CurrentState.UnResolvedGotos.push_back(std::make_pair(GoSymbol,GOIndex));
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::progn))
                {
                    if(ListToConvert.size() == 1)
                    {
                        throw std::runtime_error("progn requires atleast 1 argument");   
                    }
                    p_WriteProgn(ListToConvert,ListToAppend,CurrentState,1);
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::lambda))
                {
                    if(ListToConvert.size() < 3)
                    {
                        throw std::runtime_error("lambda requires atleast 2 arguments, the lambda arguments and 1 or more body forms");   
                    }
                    if(!ListToConvert[1].IsType<List>())
                    {
                        throw std::runtime_error("lambda arguments has to be specified in a list");
                    }
                    Lambda NewLambda;
                    NewLambda.Definition = std::make_shared<FunctionDefinition>();
                    List const& ArgumentList = ListToConvert[1].GetType<List>();
                    for(int i = 0; i < ArgumentList.size();i++)
                    {
                        Value const& Argument = ArgumentList[i];
                        if(!Argument.IsType<Symbol>())
                        {
                            throw std::runtime_error("Values in lambda argument list has to be symbols");
                        }
                        Symbol const& CurrentSymbol = Argument.GetType<Symbol>().ID;
                        if(CurrentSymbol.ID & RestSymbol)
                        {
                            if(!(i + 1 < ArgumentList.size()) || !ArgumentList[i+1].IsType<Symbol>())
                            {
                                throw std::runtime_error("&rest parameter requires a symbol as the next value in the list");
                            }
                            NewLambda.Definition->RestParameter = ArgumentList[i+1].GetType<Symbol>().ID;
                            i+=1;
                        }
                        else
                        {
                            NewLambda.Definition->Arguments.push_back(Argument.GetType<Symbol>());
                        }
                    }
                    NewLambda.Definition->Instructions = std::make_shared<OpCodeList>(ListToConvert,2,true);
                    OpCode_PushLiteral NewCode;
                    NewCode.Literal = std::move(NewLambda);
                    ListToAppend.push_back(std::move(NewCode));
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::set))
                {
                    if(ListToConvert.size() != 3)
                    {
                        throw std::runtime_error("Set requires exactly 2 arguments, the symbol to modify, and the new value");   
                    }
                    if (!ListToConvert[1].IsType<List>())
                    {
                        OpCode_PushLiteral LiteralToPush;
                        LiteralToPush.Literal = ListToConvert[1];
                        ListToAppend.push_back(LiteralToPush);
                    }
                    else 
                    {
                        p_CreateOpCodes(ListToConvert[1], ListToAppend, CurrentState);
                    }
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    ListToAppend.push_back(OpCode_Set());
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::macro))
                {
                    if(ListToConvert.size() != 2)
                    {
                        throw std::runtime_error("macro requires exactly 1 arguments, function to make macro of");   
                    }
                    p_CreateOpCodes(ListToConvert[1],ListToAppend,CurrentState);
                    ListToAppend.push_back(OpCode_Macro());
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::quote))
                {
                    if(ListToConvert.size() < 2)
                    {
                        throw std::runtime_error("to few arguments for quote special form, quote requires exactly 1 argument");   
                    }
                    else if(ListToConvert.size() > 2)
                    {
                        throw std::runtime_error("to many arguments for quote special form, quote requires exactly 1 argument");   
                    }
                    OpCode_PushLiteral LiteralToPush;
                    LiteralToPush.Literal = ListToConvert[1];
                    ListToAppend.push_back(LiteralToPush);
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::signal))
                {
                    if(ListToConvert.size() != 2)
                    {
                        throw std::runtime_error("signal requires exactly 1 argument, the value to signal");   
                    }
                    p_CreateOpCodes(ListToConvert[1], ListToAppend, CurrentState);
                    ListToAppend.push_back(OpCode_Signal());
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::unwind_protect))
                {
                    if(ListToConvert.size() != 3)
                    {
                        throw std::runtime_error("signal requires exactly 2 arguments, the protected form, and the form when unwinding");   
                    }
                    IPIndex AddProtectIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Add());
                    p_CreateOpCodes(ListToConvert[1], ListToAppend, CurrentState);
                    ListToAppend[AddProtectIndex].GetType<OpCode_UnwindProtect_Add>().UnwindBegin = ListToAppend.size();
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    ListToAppend.push_back(OpCode_UnwindProtect_Pop());
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::unwind))
                {
                    if(CurrentState.InSignalHandler == 0)
                    {
                        throw std::runtime_error("unwind special can only occur within a signal handler");
                    }
                    if(ListToConvert.size() != 1)
                    {
                        throw std::runtime_error("unwind special from takes no arguments");   
                    }
                    CurrentState.UnResolvedUnwinds.push_back(ListToAppend.size());
                    ListToAppend.push_back(OpCode_Unwind());
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::signal_handlers))
                {
                    if((ListToConvert.size() & 1) != 0)
                    {
                        throw std::runtime_error("signal requires exactly an even amount of arguments: the form to execute, and pairs of handlers");
                    }
                    int CurrentIndex = 2;
                    std::vector<OpCode_AddSignalHandlers::SignalHandler> Handlers;
                    while(CurrentIndex < ListToConvert.size())
                    {
                        if(!ListToConvert[CurrentIndex].IsType<List>())
                        {
                            throw std::runtime_error("first part of signal handlers has to be a list, containing form evaluating to a type, and symbol");
                        }
                        List const& HandlerList = ListToConvert[CurrentIndex].GetType<List>();
                        if(HandlerList.size() != 2)
                        {
                            throw std::runtime_error("first part of signal handlers has to be a list, containing form evaluating to a type, and symbol");
                        }
                        if(!HandlerList[1].IsType<Symbol>())
                        {
                            throw std::runtime_error("second part of list to signal handler has to be a symbol");
                        }
                        OpCode_AddSignalHandlers::SignalHandler NewHandler;
                        NewHandler.BoundVariable = HandlerList[1].GetType<Symbol>().ID;
                        Handlers.push_back(NewHandler);
                        p_CreateOpCodes(HandlerList[0],ListToAppend,CurrentState);
                        CurrentIndex += 2;
                    }
                    IPIndex AddHandlersIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_AddSignalHandlers());
                    CurrentIndex = 2;
                    CurrentState.InSignalHandler += 1;

                    //Add opcdoes for thing to actually execute
                    p_CreateOpCodes(ListToConvert[1],ListToAppend,CurrentState);
                    IPIndex JumpEndIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_Goto());
                    while(CurrentIndex < ListToConvert.size())
                    {
                        Handlers[(CurrentIndex-1)/2].HandlerBegin = ListToAppend.size();
                        p_CreateOpCodes(ListToConvert[CurrentIndex+1],ListToAppend,CurrentState);
                        ListToAppend.push_back(OpCode_SignalHandler_Done());
                        CurrentIndex += 2;
                    }
                    CurrentState.InSignalHandler += -1;
                    IPIndex HandlersEnd = ListToAppend.size();
                    for(IPIndex& UnwindIndexes : CurrentState.UnResolvedUnwinds)
                    {
                        ListToAppend[UnwindIndexes].GetType<OpCode_Unwind>().HandlersEnd = HandlersEnd;
                    }
                    ListToAppend.push_back(OpCode_RemoveSignalHandlers());
                    OpCode_AddSignalHandlers& SignalOpCode = ListToAppend[AddHandlersIndex].GetType<OpCode_AddSignalHandlers>();
                    SignalOpCode.HandlersEnd = HandlersEnd;
                    SignalOpCode.Handlers = std::move(Handlers);
                    OpCode_Goto&  GotoEnd = ListToAppend[JumpEndIndex].GetType<OpCode_Goto>();
                    GotoEnd.NewIP = HandlersEnd;
                    GotoEnd.NewStackSize = -1;
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::bind_dynamic))
                {
                    if(ListToConvert.size() != 3)
                    {
                        throw std::runtime_error("bind-dynamic requires exactly 2 arguments: list of binding triplets, form to execute in new binding");
                    }
                    if(!ListToConvert[1].IsType<List>())
                    {
                        throw std::runtime_error("second argument to bind-dynamic has to be a list of binding triplets");
                    }
                    for(auto const& Triplet : ListToConvert[1].GetType<List>())
                    {
                        if(!Triplet.IsType<List>() || Triplet.GetType<List>().size() != 3)
                        {
                            throw std::runtime_error("second argument to bind-dynamic has to be a list of binding triplets");   
                        }
                        for(auto const& TripletValue : Triplet.GetType<List>())
                        {
                            p_CreateOpCodes(TripletValue,ListToAppend,CurrentState);
                        }
                    }
                    ListToAppend.push_back(OpCode_PushBindings(ListToConvert[1].GetType<List>().size()));
                    IPIndex UnwindIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Add());
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    IPIndex UnwindBegin = ListToAppend.size();
                    ListToAppend.push_back(OpCode_PopBindings());
                    ListToAppend.push_back(OpCode_UnwindProtect_Pop());
                    ListToAppend[UnwindIndex].GetType<OpCode_UnwindProtect_Add>().UnwindBegin = UnwindBegin;
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::eval))
                {
                    if(!(ListToConvert.size() == 2 || ListToConvert.size() == 3))
                    {
                        throw std::runtime_error("eval requires either 1 or 2 arguments");
                    }
                    p_CreateOpCodes(ListToConvert[1],ListToAppend,CurrentState);
                    if(ListToConvert.size() == 3)
                    {
                        p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    }
                    ListToAppend.push_back(OpCode_Eval(ListToConvert.size()-1));
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::Return))
                {
                    if(CurrentState.InLambda == 0)
                    {
                        throw  std::runtime_error("return special can only appear within a lambda body");
                    }
                    if(ListToConvert.size() != 2)
                    {
                        throw std::runtime_error("return requires exatly  1 argument,  the value to return");
                    }
                    p_CreateOpCodes(ListToConvert[1],ListToAppend,CurrentState);
                    IPIndex UnresolvedReturn = ListToAppend.size();
                    OpCode_Goto NewOpCode;
                    NewOpCode.NewStackSize = 0;
                    NewOpCode.ReturnTop =  true;
                    CurrentState.UnresolvedReturns.push_back(UnresolvedReturn);
                    ListToAppend.push_back(NewOpCode);
                }
                else
                {
                    assert(false && "OpCode list doesn't cover all cases");   
                }
            }
            else
            {
                p_CreateFuncCall(ListToConvert,ListToAppend,CurrentState);
            }
        }
        else if(ListToConvert[0].IsType<List>())
        {
            p_CreateFuncCall(ListToConvert, ListToAppend, CurrentState);
        }
        else
        {
            throw std::runtime_error("Only symbol or list can be first position of s-expression");
        }
    }
    void OpCodeList::p_WriteProgn(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState,int Offset)
    {
        for(int i = Offset; i < ListToConvert.size();i++)
        {
            p_CreateOpCodes(ListToConvert[i],ListToAppend,CurrentState);
            if(i+1 < ListToConvert.size())
            {
                ListToAppend.push_back(OpCode_Pop());   
            }
        }
    }
    OpCodeList::OpCodeList(Value const& ValueToEncode)
    {
        EncodingState CurrentState;
        p_CreateOpCodes(ValueToEncode,m_OpCodes,CurrentState);
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
    }
    OpCodeList::OpCodeList(List const& ListToConvert)
    {
        EncodingState CurrentState;
        p_WriteProgn(ListToConvert,m_OpCodes,CurrentState,0);
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
    }
    void OpCodeList::Append(List const& ListToConvert)
    {
        EncodingState CurrentState;
        p_CreateOpCodes(ListToConvert,m_OpCodes,CurrentState);
        //TODO fix with let/cc
        //m_OpCodes.push_back(OpCode_Pop());
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
    }
    void OpCodeList::Append(Value const& ListToConvert)
    {
        EncodingState CurrentState;
        p_CreateOpCodes(ListToConvert,m_OpCodes,CurrentState);
        //TODO fix with let/cc
        //m_OpCodes.push_back(OpCode_Pop());
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
    }
    OpCodeList::OpCodeList(List const& ListToConvert,int Offset,bool  InLambda)
    {
        EncodingState CurrentState;
        CurrentState.InLambda = InLambda;
        p_WriteProgn(ListToConvert,m_OpCodes,CurrentState,Offset);
        if(InLambda)
        {
            for(auto UnresolvedReturn : CurrentState.UnresolvedReturns)
            {
                m_OpCodes[UnresolvedReturn].GetType<OpCode_Goto>().NewIP = m_OpCodes.size();
            }
        }
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
    }
    OpCodeList::OpCodeList(SymbolID ArgID,SymbolID IndexFunc,std::vector<SlotDefinition> const& Initializers)
    {
        for(auto const& Slot : Initializers)
        {
            m_OpCodes.push_back(OpCode_PushVar(ArgID));
            m_OpCodes.push_back(OpCode_PushLiteral(Value(Symbol(Slot.Symbol))));
            m_OpCodes.push_back(OpCode_PushVar(IndexFunc));
            m_OpCodes.push_back(OpCode_CallFunc(2));
            EncodingState CurrentState;
            p_CreateOpCodes(Slot.DefaultValue,m_OpCodes,CurrentState);
            m_OpCodes.push_back(OpCode_Set());
            m_OpCodes.push_back(OpCode_Pop());
        }
        m_OpCodes.push_back(OpCode_PushVar(ArgID));
    }

    OpCodeExtractor::OpCodeExtractor(Ref<OpCodeList> OpCodes)
    {
        m_AssociatedList = OpCodes;
    }
    OpCode& OpCodeExtractor::GetCurrentCode()
    {
        return m_AssociatedList->m_OpCodes[m_IP];
    }
    void OpCodeExtractor::SetIP(IPIndex NewIP)
    {
        m_IP = NewIP;
    }
    IPIndex OpCodeExtractor::GetIP()
    {
        return m_IP;
    }
    void OpCodeExtractor::Pop()
    {
        m_IP += 1;
    }
    bool OpCodeExtractor::Finished() const
    {
        return m_AssociatedList == nullptr || m_IP >= m_AssociatedList->m_OpCodes.size();
    }
    void OpCodeExtractor::SetEnd()
    {
        if(!Finished())
        {
            m_IP = m_AssociatedList->m_OpCodes.size();
        }
    }
}
