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
            auto const& Sym = ValueToEncode.GetType<Symbol>();
            OpCode_PushVar NewCode;
            if(auto It = CurrentState.LocalSymbols.find(Sym.ID); It != CurrentState.LocalSymbols.end())
            {
                NewCode.Local = true;
                NewCode.ID = It->second;
            }
            else
            {
                NewCode.ID = Sym.ID;
            }
            //if(!Sym.SymbolLocation.IsEmpty())
            //{
            //    m_OpcodeLocationInfo[ListToAppend.size()] = LocationInfo(Sym.SymbolLocation,;
            //}
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
    void OpCodeList::p_CreateFuncCall(List const& ListToConvert,std::vector<OpCode>& OutCodes,EncodingState& CurrentState,bool Setting)
    {
        IPIndex CurrentPosition = OutCodes.size();
        p_CreateOpCodes(ListToConvert.front(),OutCodes,CurrentState);
        for(int i = 1; i < ListToConvert.size();i++)
        {
            p_CreateOpCodes(ListToConvert[i],OutCodes,CurrentState);
            CurrentState.ArgumentStackCount += 1;
        }
        CurrentState.ArgumentStackCount -= ListToConvert.size()-1;
        OpCode_CallFunc FunCall(ListToConvert.size()-1,Setting);
        m_OpCodes.push_back(FunCall);

        //Add eventual debug info
        assert(OutCodes.size() > CurrentPosition);
        if(!ListToConvert.GetLocation().IsEmpty())
        {
            m_OpcodeLocationInfo[CurrentPosition] = LocationInfo(ListToConvert.GetLocation(),ListToConvert.GetDepth());
        }
    }
    void OpCodeList::p_CreateOpCodes(List const& ListToConvert, std::vector<OpCode>& ListToAppend,EncodingState& CurrentState)
    {
        if(ListToConvert.empty())
        {
            throw std::runtime_error("Cannot evalaute empty s-expression");
        }
        IPIndex CurrentPosition = ListToAppend.size();
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
                    int CurrentUnwindDepth = CurrentState.UnwindProtectDepth;
                    std::vector<std::pair<SymbolID,IPIndex>> NewUnresolvedSymbols;
                    for(auto const& Pair : CurrentState.UnResolvedGotos) 
                    {
                        if(SymbolToOffsetMap.find(Pair.first) != SymbolToOffsetMap.end())
                        {
                            auto& OpCode = ListToAppend[Pair.second].GetType<OpCode_Goto>();
                            OpCode.NewIP = SymbolToOffsetMap[Pair.first];
                            if(OpCode.NewUnwindSize == CurrentUnwindDepth)
                            {
                                OpCode.NewUnwindSize = -1;
                            }
                            else
                            {
                                OpCode.NewUnwindSize = CurrentUnwindDepth;
                            }
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
                    NewOpcode.NewUnwindSize = CurrentState.UnwindProtectDepth;
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
                    NewLambda.Definition = MakeRef<FunctionDefinition>();
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
                        else if(CurrentSymbol.ID & EnvirSymbol)
                        {
                            if(!(i + 1 < ArgumentList.size()) || !ArgumentList[i+1].IsType<Symbol>())
                            {
                                throw std::runtime_error("&envir parameter requires a symbol as the next value in the parameter list");
                            }
                            NewLambda.Definition->EnvirParameter = ArgumentList[i+1].GetType<Symbol>().ID;
                            i+=1;
                        }
                        else
                        {
                            auto const& Sym = Argument.GetType<Symbol>();
                            NewLambda.Definition->Arguments.push_back(Sym);
                        }
                    }
                    NewLambda.Definition->Instructions = MakeRef<OpCodeList>(ListToConvert,2,*NewLambda.Definition,CurrentState);
                    std::sort(NewLambda.Definition->LocalVars.begin(),NewLambda.Definition->LocalVars.end());

                    OpCode_PushLiteral NewCode;
                    NewCode.Literal = std::move(NewLambda);
                    ListToAppend.push_back(std::move(NewCode));
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::set) || CurrentSymbol == SymbolID(PrimitiveForms::setl))
                {
                    if(ListToConvert.size() != 3)
                    {
                        throw std::runtime_error("Set requires exactly 2 arguments, the symbol to modify, and the new value");   
                    }
                    bool IsLocalSet = CurrentSymbol == SymbolID(PrimitiveForms::setl) && ListToConvert[1].IsType<Symbol>() && CurrentState.InLambda;
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    if (!ListToConvert[1].IsType<List>())
                    {
                        if(!ListToConvert[1].IsType<Symbol>())
                        {
                            throw std::runtime_error("set special form requires either a symbol or function-call as the first argument");
                        }
                        auto const& Sym = ListToConvert[1].GetType<Symbol>();
                        if(CurrentState.LocalSymbols.find(Sym.ID) != CurrentState.LocalSymbols.end())
                        {
                            IsLocalSet = true;
                        }
                        if(!IsLocalSet)
                        {
                            OpCode_PushLiteral LiteralToPush;
                            LiteralToPush.Literal = ListToConvert[1];
                            ListToAppend.push_back(LiteralToPush);
                            ListToAppend.push_back(OpCode_Set());
                        }
                        else
                        {
                            OpCode_PushLiteral LiteralToPush;
                            LiteralToPush.Literal = ListToConvert[1];
                            ListToAppend.push_back(LiteralToPush);
                            OpCode_Set NewOpcode;
                            NewOpcode.LocalSetIndex = CurrentState.GetLocalSymbolIndex(Sym.ID);
                            ListToAppend.push_back(NewOpcode);
                        }
                    }
                    else 
                    {
                        auto& FirstForm = ListToConvert[1].GetType<List>();
                        if(FirstForm.size() == 0)
                        {
                            throw std::runtime_error("empty form is not allowed");
                        }
                        if(FirstForm[0].IsType<Symbol>() && FirstForm[0].GetType<Symbol>().ID < uint_least32_t(PrimitiveForms::LAST))
                        {
                            throw std::runtime_error("set special form requires either a symbol or function-call as the first argument");
                        }
                        ListToAppend.push_back(OpCode_PreSet());
                        p_CreateFuncCall(ListToConvert[1].GetType<List>(), ListToAppend, CurrentState,true);
                    }
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
                    if(ListToConvert.size() == 1 || ListToConvert.size() > 3)
                    {
                        throw std::runtime_error("signal special form requires either 1 or 2 arguments");   
                    }
                    OpCode_Signal SignalCode;
                    if(ListToConvert.size() == 3)
                    {
                        p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                        SignalCode.HasForced = true;
                    }
                    p_CreateOpCodes(ListToConvert[1], ListToAppend, CurrentState);
                    ListToAppend.push_back(SignalCode);
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::unwind_protect))
                {
                    if(ListToConvert.size() != 3)
                    {
                        throw std::runtime_error("signal requires exactly 2 arguments, the protected form, and the form when unwinding");   
                    }
                    IPIndex AddProtectIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Add());
                    CurrentState.UnwindProtectDepth += 1;
                    p_CreateOpCodes(ListToConvert[1], ListToAppend, CurrentState);
                    CurrentState.UnwindProtectDepth -= 1;
                    ListToAppend[AddProtectIndex].GetType<OpCode_UnwindProtect_Add>().UnwindBegin = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Begin());
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    ListToAppend[AddProtectIndex].GetType<OpCode_UnwindProtect_Add>().UnwindEnd = ListToAppend.size();
                    ListToAppend[AddProtectIndex].GetType<OpCode_UnwindProtect_Add>().EndStackCount = CurrentState.ArgumentStackCount;
                    ListToAppend.push_back(OpCode_UnwindProtect_Pop());
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::unwind))
                {
                    if(CurrentState.InSignalHandler == 0)
                    {
                        throw std::runtime_error(
                                "unwind special can only occur within a signal handler");
                    }
                    if(ListToConvert.size() != 1)
                    {
                        throw std::runtime_error("unwind special from takes no arguments");   
                    }
                    CurrentState.UnResolvedUnwinds.push_back(ListToAppend.size());
                    ListToAppend.push_back(OpCode_Unwind(CurrentState.UnwindProtectDepth));
                }
                else if(CurrentSymbol == SymbolID(PrimitiveForms::signal_handlers))
                {
                    if((ListToConvert.size() & 1) != 0)
                    {
                        throw std::runtime_error(
                                "signal requires an even amount of arguments: "
                                "the form to execute, and pairs of handlers");
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
                    IPIndex UnwindProtectIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Add());
                    CurrentState.UnwindProtectDepth += 1;

                    int TargetUnwindDepth = CurrentState.UnwindProtectDepth;
                    CurrentIndex = 2;
                    CurrentState.InSignalHandler += 1;

                    //Add opcdoes for thing to actually execute
                    p_CreateOpCodes(ListToConvert[1],ListToAppend,CurrentState);
                    IPIndex JumpEndIndex = ListToAppend.size();
                    ListToAppend.push_back(OpCode_Goto());
                    std::vector<IPIndex> SignalHandlerDones;
                    while(CurrentIndex < ListToConvert.size())
                    {
                        Handlers[(CurrentIndex-1)/2].HandlerBegin = ListToAppend.size();
                        EncodingState NewState;
                        NewState.TotalLocalSymbolCount = CurrentState.TotalLocalSymbolCount;
                        NewState.LocalSymbols = CurrentState.LocalSymbols;

                        NewState.InSignalHandler += 1;
                        p_CreateOpCodes(ListToConvert[CurrentIndex+1],ListToAppend,NewState);
                        CurrentState.UnResolvedUnwinds.insert(CurrentState.UnResolvedUnwinds.end(),
                                NewState.UnResolvedUnwinds.begin(),NewState.UnResolvedUnwinds.end());
                        if(CurrentState.UnResolvedGotos.size() > 0)
                        {
                            throw std::runtime_error("Unresolved go statements in signal handler");   
                        }
                        SignalHandlerDones.push_back(ListToAppend.size());
                        ListToAppend.push_back(OpCode_SignalHandler_Done());


                        CurrentState.TotalLocalSymbolCount = NewState.TotalLocalSymbolCount;
                        CurrentState.LocalSymbols = NewState.LocalSymbols;
                        CurrentIndex += 2;
                    }
                    CurrentState.InSignalHandler += -1;
                    IPIndex HandlersEnd = ListToAppend.size();
                    for(IPIndex& UnwindIndexes : CurrentState.UnResolvedUnwinds)
                    {
                        auto& CurrentCode = ListToAppend[UnwindIndexes].GetType<OpCode_Unwind>();
                        CurrentCode.HandlersEnd = HandlersEnd;
                        CurrentCode.NewStackSize = CurrentState.ArgumentStackCount;
                        if(CurrentCode.TargetUnwindDepth != TargetUnwindDepth)
                        {
                            CurrentCode.TargetUnwindDepth = TargetUnwindDepth;
                        }
                        else
                        {
                            CurrentCode.TargetUnwindDepth = -1;   
                        }
                    }
                    CurrentState.UnResolvedUnwinds.clear();
                    for(IPIndex& HandlerDoneIndex : SignalHandlerDones)
                    {
                        ListToAppend[HandlerDoneIndex].GetType<OpCode_SignalHandler_Done>().HandlersEnd = HandlersEnd;
                        ListToAppend[HandlerDoneIndex].GetType<OpCode_SignalHandler_Done>().NewStackSize = CurrentState.ArgumentStackCount;
                    }
                    CurrentState.UnwindProtectDepth -= 1;
                    ListToAppend[UnwindProtectIndex].GetType<OpCode_UnwindProtect_Add>().UnwindBegin = ListToAppend.size();
                    ListToAppend[UnwindProtectIndex].GetType<OpCode_UnwindProtect_Add>().EndStackCount = CurrentState.ArgumentStackCount;
                    ListToAppend.push_back(OpCode_UnwindProtect_Begin());
                    ListToAppend.push_back(OpCode_RemoveSignalHandlers());
                    ListToAppend[UnwindProtectIndex].GetType<OpCode_UnwindProtect_Add>().UnwindEnd = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Pop());
                    OpCode_AddSignalHandlers& SignalOpCode = ListToAppend[AddHandlersIndex].GetType<OpCode_AddSignalHandlers>();
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
                    CurrentState.UnwindProtectDepth += 1;
                    ListToAppend.push_back(OpCode_UnwindProtect_Add());
                    p_CreateOpCodes(ListToConvert[2],ListToAppend,CurrentState);
                    CurrentState.UnwindProtectDepth -= 1;
                    IPIndex UnwindBegin = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Begin());
                    ListToAppend.push_back(OpCode_PopBindings());
                    ListToAppend[UnwindIndex].GetType<OpCode_UnwindProtect_Add>().UnwindEnd = ListToAppend.size();
                    ListToAppend.push_back(OpCode_UnwindProtect_Pop());
                    ListToAppend[UnwindIndex].GetType<OpCode_UnwindProtect_Add>().UnwindBegin = UnwindBegin;
                    ListToAppend[UnwindIndex].GetType<OpCode_UnwindProtect_Add>().EndStackCount = CurrentState.ArgumentStackCount;
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
                p_CreateFuncCall(ListToConvert,ListToAppend,CurrentState,false);
            }
        }
        else
        {
            p_CreateFuncCall(ListToConvert, ListToAppend, CurrentState,false);
        }
        assert(ListToAppend.size() > CurrentPosition);
        if(!ListToConvert.GetLocation().IsEmpty())
        {
            m_OpcodeLocationInfo[CurrentPosition] = LocationInfo(ListToConvert.GetLocation(),ListToConvert.GetDepth());
        }
    }
    void OpCodeList::p_WriteProgn(List const& ListToConvert,std::vector<OpCode>& ListToAppend,EncodingState& CurrentState,int Offset)
    {
        IPIndex CurrentPosition = ListToAppend.size();
        for(int i = Offset; i < ListToConvert.size();i++)
        {
            p_CreateOpCodes(ListToConvert[i],ListToAppend,CurrentState);
            if(i+1 < ListToConvert.size())
            {
                ListToAppend.push_back(OpCode_Pop());   
            }
        }
        assert(ListToAppend.size() > CurrentPosition);
        if(!ListToConvert.GetLocation().IsEmpty())
        {
            m_OpcodeLocationInfo[CurrentPosition] = LocationInfo(ListToConvert.GetLocation(),ListToConvert.GetDepth());
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
        p_FillDebugInfo();
    }
    OpCodeList::OpCodeList(Scope& AssociatedScope,Value const& ValueToEncode)
    {
        EncodingState CurrentState;
        CurrentState.TotalLocalSymbolCount = AssociatedScope.TotalLocalSymCount();
        Scope* CurrentScope = &AssociatedScope;
        //TODO check for cycles...
        while(CurrentScope != nullptr)
        {
            for(auto const& Pair : CurrentScope->GetLocalSyms())
            {
                CurrentState.LocalSymbols[Pair.first] = Pair.second + CurrentScope->GetLocalSymBegin();
            }
            if(CurrentScope->ParentCount() > 0)
            {
                CurrentScope = & (*CurrentScope->GetParent(0));
            }
            else
            {
                break;
            }
        }

        p_CreateOpCodes(ValueToEncode,m_OpCodes,CurrentState);
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
        p_FillDebugInfo();
    }
    OpCodeList::OpCodeList(List const& ListToConvert)
    {
        EncodingState CurrentState;
        p_WriteProgn(ListToConvert,m_OpCodes,CurrentState,0);
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
        p_FillDebugInfo();
    }
    void OpCodeList::Append(List const& ListToConvert)
    {
        m_Modified = true;
        IPIndex PrevSize = m_OpCodes.size();
        EncodingState CurrentState;
        p_CreateOpCodes(ListToConvert,m_OpCodes,CurrentState);
        //TODO fix with let/cc
        //m_OpCodes.push_back(OpCode_Pop());
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
        p_FillDebugInfo();
    }
    void OpCodeList::Append(Value const& ListToConvert)
    {
        m_Modified = true;
        EncodingState CurrentState;
        p_CreateOpCodes(ListToConvert,m_OpCodes,CurrentState);
        //TODO fix with let/cc
        //m_OpCodes.push_back(OpCode_Pop());
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
        p_FillDebugInfo();
    }
    //OpCodeList::OpCodeList(List const& ListToConvert,int Offset,bool  InLambda)
    //{
    //    EncodingState CurrentState;
    //    CurrentState.InLambda = InLambda;
    //    p_WriteProgn(ListToConvert,m_OpCodes,CurrentState,Offset);
    //    if(InLambda)
    //    {
    //        for(auto UnresolvedReturn : CurrentState.UnresolvedReturns)
    //        {
    //            auto& CurrentCode = m_OpCodes[UnresolvedReturn].GetType<OpCode_Goto>();
    //            CurrentCode.NewIP = m_OpCodes.size();
    //            CurrentCode.NewUnwindSize = 0;
    //        }
    //    }
    //    if(CurrentState.UnResolvedGotos.size() != 0)
    //    {
    //        throw std::runtime_error("go's to tags without corresponding label detected");
    //    }
    //    p_FillDebugInfo();
    //}
    OpCodeList::OpCodeList(List const& ListToConvert,int Offset,FunctionDefinition& LambdaDef ,
            EncodingState const& ParentState)
    {
        LambdaDef.LocalSymBegin = ParentState.TotalLocalSymbolCount;
        EncodingState CurrentState;
        CurrentState.LocalSymbols = ParentState.LocalSymbols;
        CurrentState.TotalLocalSymbolCount = ParentState.TotalLocalSymbolCount;
        CurrentState.InLambda = true;
        //add lambda symbols
        for(auto const& Arg : LambdaDef.Arguments)
        {
            CurrentState.AddLocalSymbol(Arg.ID);
        }
        if(LambdaDef.RestParameter != 0)
        {
            CurrentState.AddLocalSymbol(LambdaDef.RestParameter);
        }
        if(LambdaDef.EnvirParameter != 0)
        {
            CurrentState.AddLocalSymbol(LambdaDef.EnvirParameter);   
        }
        p_WriteProgn(ListToConvert,m_OpCodes,CurrentState,Offset);
        for(auto UnresolvedReturn : CurrentState.UnresolvedReturns)
        {
            auto& CurrentCode = m_OpCodes[UnresolvedReturn].GetType<OpCode_Goto>();
            CurrentCode.NewIP = m_OpCodes.size();
            CurrentCode.NewUnwindSize = 0;
        }
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }

        //update function state to account for all locally defined symbols
        for(auto const& Sym : CurrentState.LocalSymNames)
        {
            assert(CurrentState.LocalSymbols.find(Sym) != CurrentState.LocalSymbols.end());
            LambdaDef.LocalVars.push_back(std::make_pair( Sym ,CurrentState.LocalSymbols[Sym]-LambdaDef.LocalSymBegin));
        }
        std::sort(LambdaDef.LocalVars.begin(),LambdaDef.LocalVars.end());
        LambdaDef.LocalSymCount = CurrentState.TotalLocalSymbolCount-ParentState.TotalLocalSymbolCount;
        p_FillDebugInfo();
    }
    OpCodeList::OpCodeList(SymbolID ArgID,SymbolID IndexFunc,std::vector<SlotDefinition> const& Initializers)
    {
        for(auto const& Slot : Initializers)
        {
            EncodingState CurrentState;
            p_CreateOpCodes(Slot.DefaultValue,m_OpCodes,CurrentState);

            m_OpCodes.push_back(OpCode_PreSet());

            m_OpCodes.push_back(OpCode_PushVar(IndexFunc));
            m_OpCodes.push_back(OpCode_PushVar(ArgID));
            m_OpCodes.push_back(OpCode_PushLiteral(Value(Symbol(Slot.Symbol))));
            m_OpCodes.push_back(OpCode_CallFunc(2,true));


            m_OpCodes.push_back(OpCode_Pop());
        }
        m_OpCodes.push_back(OpCode_PushVar(ArgID));
        p_FillDebugInfo();
    }
    void OpCodeList::p_FillDebugInfo()
    {
        int Offset = m_DebugInfo.size();
        m_DebugInfo.insert(m_DebugInfo.end(),m_OpCodes.size()-m_DebugInfo.size(),DebugInfo());
        int CurrentDepth = -1; 
        Location CurrentLocation;
        for(size_t i = Offset; i < m_DebugInfo.size();i++)
        {
            if(auto It = m_OpcodeLocationInfo.find(i); It != m_OpcodeLocationInfo.end())
            {
                CurrentLocation = It->second.Loc;
                m_DebugInfo[i].Depth = It->second.Depth;
                CurrentDepth = It->second.Depth+1;
                
            }
            else
            {
                //m_DebugInfo[i].Depth = CurrentDepth;   
            }
            m_DebugInfo[i].Loc = CurrentLocation;
        }
        
        //O(n), walla
        //for(auto const& LocInfo : m_OpcodeLocationInfo)
        //{
        //    m_DebugInfo[LocInfo.first].Depth = LocInfo.second.Depth;
        //    m_DebugInfo[LocInfo.first].Loc = LocInfo.second.Loc;
        //}
        assert(m_DebugInfo.size() == m_OpCodes.size());
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
        assert(NewIP >= 0);
        m_IP = NewIP;
    }
    IPIndex OpCodeExtractor::GetIP() const
    {
        return m_IP;
    }
    IPIndex OpCodeExtractor::OpCodeCount() const
    {
        if(m_AssociatedList == nullptr)
        {
            throw std::runtime_error("Trying to access OpCodeCount with empty OpCodeList");
        }   
        return m_AssociatedList->m_OpCodes.size();
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
    void OpCodeExtractor::SetTrap(Location TrapLocation)
    {
        for(auto& DebugInfo : m_AssociatedList->m_OpcodeLocationInfo)
        {
            if(DebugInfo.second.Loc == TrapLocation)
            {
                m_AssociatedList->m_DebugInfo[DebugInfo.first].Trapped = true;
            }
        }
    }
    void OpCodeExtractor::ClearTraps()
    {
        for(auto& DebugInfo : m_AssociatedList->m_OpcodeLocationInfo)
        {
            m_AssociatedList->m_DebugInfo[DebugInfo.first].Trapped = false;
        }
    }
    void OpCodeExtractor::SetDebugID(int ID)
    {
        m_AssociatedList->m_DebugID = ID;
    }
    bool OpCodeExtractor::Modified()
    {
        bool ReturnValue = m_AssociatedList->m_Modified;
        m_AssociatedList->m_Modified = false;
        return ReturnValue;
    }
    int OpCodeExtractor::GetDebugID()
    {
        return m_AssociatedList->m_DebugID;
    }
    bool OpCodeExtractor::IsTrapped(IPIndex Position)
    {
        if(Position >= m_AssociatedList->m_DebugInfo.size())
        {
            throw std::runtime_error("Trying to query trapped out of bounds");   
        }
        return m_AssociatedList->m_DebugInfo[Position].Trapped;
    }
    int OpCodeExtractor::GetDepth(IPIndex Position) const
    {
        if(Position >= m_AssociatedList->m_DebugInfo.size())
        {
            throw std::runtime_error("Trying to query trapped out of bounds");   
        }
        return m_AssociatedList->m_DebugInfo[Position].Depth;
    }
    Location OpCodeExtractor::GetLocation(IPIndex Position)  const
    {
        if(Position >= m_AssociatedList->m_DebugInfo.size())
        {
            throw std::runtime_error("Trying to query trapped out of bounds");   
        }
        return m_AssociatedList->m_DebugInfo[Position].Loc;
    }
    Symbol OpCodeExtractor::GetName() const
    {
        return m_AssociatedList->m_Name;
    }
}
