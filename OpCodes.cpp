#include "OpCodes.h"
#include <unordered_map>
#include <assert.h>
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
        }
        p_CreateOpCodes(ListToConvert.front(),OutCodes,CurrentState);
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
                    ListToAppend[GotoFalseEndPosition].GetType<OpCode_Goto>().ResetStack = false;
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
                    ListToAppend.push_back(OpCode_Goto());
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
                    for(auto const& Argument : ListToConvert[1].GetType<List>())
                    {
                        if(!Argument.IsType<Symbol>())
                        {
                            throw std::runtime_error("Values in lambda argument list has to be symbols");
                        }
                        NewLambda.Definition->Arguments.push_back(Argument.GetType<Symbol>());
                    }
                    NewLambda.Definition->Instructions = std::make_shared<OpCodeList>(ListToConvert,2);
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
                    OpCode_PushLiteral LiteralToPush;
                    LiteralToPush.Literal = ListToConvert[1];
                    ListToAppend.push_back(LiteralToPush);
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
    OpCodeList::OpCodeList(List const& ListToConvert,int Offset)
    {
        EncodingState CurrentState;
        p_WriteProgn(ListToConvert,m_OpCodes,CurrentState,Offset);
        if(CurrentState.UnResolvedGotos.size() != 0)
        {
            throw std::runtime_error("go's to tags without corresponding label detected");
        }
    }

    OpCodeExtractor::OpCodeExtractor(OpCodeList& OpCodes)
    {
        m_AssociatedList = &OpCodes;
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
        return m_IP >= m_AssociatedList->m_OpCodes.size();
    }
}
