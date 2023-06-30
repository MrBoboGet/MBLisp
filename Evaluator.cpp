#include "Evaluator.h"
#include "assert.h"
#include <MBParsing/MBParsing.h>

#include  <iostream>
namespace MBLisp
{
    //Begin Scope
    Value Scope::FindVariable(SymbolID Variable)
    {
        Value ReturnValue;
        if(auto const& VarIt = m_Variables.find(Variable); VarIt != m_Variables.end())
        {
            return VarIt->second;
        }
        else if(auto const& EvalIt = m_AssociatedEvaluator->m_Builtins.find(Variable); EvalIt  != m_AssociatedEvaluator->m_Builtins.end())
        {
            Function NewFuncRef;
            NewFuncRef.ID = Variable;
            ReturnValue = NewFuncRef;
            return ReturnValue;
        }
        throw std::runtime_error("No variable with name \""+m_AssociatedEvaluator->m_SymbolToString[Variable]+"\" in current scope");
    }
    void Scope::SetVariable(SymbolID Variable,Value NewValue)
    {
        m_Variables[Variable] = std::move(NewValue);
    }
    //END Scope



    
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
                    p_CreateOpCodes(ListToConvert,ListToAppend,CurrentState);

                    std::vector<OpCode> TruePath;
                    p_CreateOpCodes(ListToConvert[2],TruePath,CurrentState);
                    std::vector<OpCode> FalsePath;
                    p_CreateOpCodes(ListToConvert[3],FalsePath,CurrentState);
                    IPIndex TruePathEnd = ListToAppend.size()+2+TruePath.size()+1;
                    IPIndex FalsePathEnd = TruePathEnd+FalsePath.size();
                    OpCode_JumpNotTrue CondJump;
                    CondJump.NewIP = TruePathEnd;
                    OpCode_Jump UncondJump;
                    UncondJump.NewIP = FalsePathEnd;
                    ListToAppend.push_back(CondJump);
                    ListToAppend.insert(ListToAppend.end(),TruePath.begin(),TruePath.end());
                    ListToAppend.push_back(UncondJump);
                    ListToAppend.insert(ListToAppend.end(),FalsePath.begin(),FalsePath.end());
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
                    if(ListToConvert[1].IsType<Symbol>())
                    {
                        throw std::runtime_error("argument of go special form must be a symbol");
                    }
                    IPIndex GOIndex = ListToAppend.size();
                    SymbolID GoSymbol = ListToConvert[1].GetType<Symbol>().ID;
                    ListToAppend.push_back(OpCode_Goto());
                    CurrentState.UnResolvedGotos.push_back(std::make_pair(GOIndex,GoSymbol));
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
               
        }
    }
    OpCodeList::OpCodeList(List const& ListToConvert)
    {
        EncodingState CurrentState;
        for(auto const& SubList : ListToConvert)
        {
            p_CreateOpCodes(SubList,m_OpCodes,CurrentState);
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


    Value Evaluator::Print(std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        for(auto const& Argument : Arguments)
        {
            if(Argument.IsType<String>())
            {
                std::cout<<'"'<<Argument.GetType<String>()<<'"';
            }
        }

        return ReturnValue;
    }
    void Evaluator::p_Eval(OpCodeList& OpCodes)
    {
        std::vector<StackFrame> StackFrames = {StackFrame(OpCodeExtractor(OpCodes),this)};
        while(StackFrames.size() != 0)
        {
            StackFrame& CurrentFrame = StackFrames.back();
            if(CurrentFrame.ExecutionPosition.Finished())
            {
                assert(CurrentFrame.ArgumentStack.size() == 1);
                Value ReturnValue = std::move(CurrentFrame.ArgumentStack.back());
                StackFrames.pop_back();
                if(StackFrames.size() != 0)
                {
                    StackFrames.back().ArgumentStack.push_back(ReturnValue);   
                }
                continue;
            }
            OpCode& CurrentCode = CurrentFrame.ExecutionPosition.GetCurrentCode();
            CurrentFrame.ExecutionPosition.Pop();
            if(CurrentCode.IsType<OpCode_Pop>())
            {
                assert(CurrentFrame.ArgumentStack.size() != 0);
                CurrentFrame.ArgumentStack.pop_back();
            }
            else if(CurrentCode.IsType<OpCode_PushVar>())
            {
                OpCode_PushVar& PushCode = CurrentCode.GetType<OpCode_PushVar>();
                CurrentFrame.ArgumentStack.push_back(CurrentFrame.StackScope.FindVariable(PushCode.ID));
            }
            else if(CurrentCode.IsType<OpCode_PushLiteral>())
            {
                OpCode_PushLiteral& PushCode = CurrentCode.GetType<OpCode_PushLiteral>();
                CurrentFrame.ArgumentStack.push_back(PushCode.Literal);
            }
            else if(CurrentCode.IsType<OpCode_Goto>())
            {
                CurrentFrame.ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_Goto>().NewIP);
            }
            else if(CurrentCode.IsType<OpCode_Jump>())
            {
                //examine the top of the stack, if true, jump
                CurrentFrame.ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_Jump>().NewIP);
            }
            else if(CurrentCode.IsType<OpCode_JumpNotTrue>())
            {
                //examine the top of the stack, if true, jump
                assert(CurrentFrame.ArgumentStack.size() != 0);
                Value& ValueToExamine = CurrentFrame.ArgumentStack.back();
                bool Result = ValueToExamine.IsType<bool>() && ValueToExamine.GetType<bool>();
                if(!Result)
                {
                    CurrentFrame.ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_JumpNotTrue>().NewIP);
                }
            }
            else if(CurrentCode.IsType<OpCode_CallFunc>())
            {
                //last value is the function to call, rest is arguments
                //creates new stack frame
                assert(CurrentFrame.ArgumentStack.size() != 0);
                if(!CurrentFrame.ArgumentStack.back().IsType<Function>())
                {
                    throw std::runtime_error("can only invoke objects of type Function");
                }
                OpCode_CallFunc&  CallFuncCode = CurrentCode.GetType<OpCode_CallFunc>();
                Function FunctionToCall = CurrentFrame.ArgumentStack.back().GetType<Function>();
                CurrentFrame.ArgumentStack.pop_back();
                std::vector<Value> Arguments;
                for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                {
                    Arguments.push_back(CurrentFrame.ArgumentStack[CurrentFrame.ArgumentStack.size()-CallFuncCode.ArgumentCount+i]);
                }
                for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                {
                    Arguments.pop_back();
                }
                BuiltinFuncType AssociatedFunc = m_Builtins[FunctionToCall.ID];
                assert(AssociatedFunc != nullptr);
                AssociatedFunc(Arguments);
            }
            else if(CurrentCode.IsType<OpCode_Set>())
            {
                //first value of stack is symbol, second is value
                assert(CurrentFrame.ArgumentStack.size() == 2);
                Value& SymbolToAssign = CurrentFrame.ArgumentStack[0];
                Value& AssignedValue = CurrentFrame.ArgumentStack[1];
                if(!SymbolToAssign.IsType<Symbol>())
                {
                    throw std::runtime_error("lhs of set has to be a symbol");
                }
                CurrentFrame.StackScope.SetVariable(SymbolToAssign.GetType<Symbol>().ID,AssignedValue);
            }
        }
    }

    void Evaluator::p_SkipWhiteSpace(std::string_view& Content)
    {
        if(Content.size() == 0)
        {
            return;   
        }
        size_t NextOffset = Content.find_first_not_of(" \r\t\n\v");
        Content = Content.substr(std::min(NextOffset,Content.size()));
    }
    //TODO escaping
    String Evaluator::p_ReadString(std::string_view& Content)
    {
        String ReturnValue;
        Content = Content.substr(1);
        bool StringFinished = false;
        while(Content.size() != 0)
        {
            if(Content[0] == '"')
            {
                StringFinished = true;
                Content = Content.substr(1);
                break;
            }
            ReturnValue += Content[0];
            Content = Content.substr(1);
        }
        return ReturnValue;
    }
    Symbol Evaluator::p_ReadSymbol(std::string_view& Content)
    {
        Symbol ReturnValue;
        std::string SymbolString;
        while(Content.size() != 0 &&
                ((Content[0] >= 'A' && Content[0] <= 'Z') || Content[0] >= 'a' && Content[0] <= 'z'))
        {
            SymbolString += Content[0];
            Content = Content.substr(1);
        }
        ReturnValue.ID = p_GetSymbolID(SymbolString);
        return ReturnValue;
    }
    Int Evaluator::p_ReadInteger(std::string_view& Content)
    {
        Int ReturnValue;
        std::string NumberString;
        while(Content.size() != 0 && Content[0] >= '0' && Content[0] <= '9')
        {
            NumberString += Content[0];
            Content = Content.substr(1);
        }
        ReturnValue = std::stoi(NumberString);
        return ReturnValue;
    }
    List Evaluator::p_ReadList(std::string_view& Content)
    {
        List ReturnValue;
        Content = Content.substr(1);
        p_SkipWhiteSpace(Content);
        if(Content.size() == 0)
        {
            throw std::runtime_error("Error reading list: no corresponding )");   
        }
        while(Content.size() != 0)
        {
            if(Content[0] == ')')
            {
                Content = Content.substr(1);   
                break;
            }
            ReturnValue.push_back(p_ReadTerm(Content));
            p_SkipWhiteSpace(Content);
        }

        return ReturnValue;
    }
    Value Evaluator::p_ReadTerm(std::string_view& Content)
    {
        Value ReturnValue;
        p_SkipWhiteSpace(Content);
        if(Content.size() == 0)
        {
            throw std::runtime_error("Error reading term: end of input");   
        }
        if(Content[0] == '(')
        {
            ReturnValue = p_ReadList(Content);
        }
        else if(Content[0] == '"')
        {
            ReturnValue = p_ReadString(Content);
        }
        else if( Content[0] >= '0' && Content[0] <= '9')
        {
            ReturnValue = p_ReadInteger(Content);
        }
        else
        {
            ReturnValue = p_ReadSymbol(Content);
        }
        return ReturnValue;
    }
    List Evaluator::p_Read(std::string_view Content)
    {
        List ReturnValue;
        while(Content.size() != 0)
        {
            ReturnValue.push_back(p_ReadTerm(Content));
            p_SkipWhiteSpace(Content);
        }
        return ReturnValue;
    }
    SymbolID Evaluator::p_GetSymbolID(std::string const& SymbolString)
    {
        SymbolID& ReturnValue = m_InternedSymbols[SymbolString];
        if(ReturnValue == 0)
        {
            ReturnValue = m_CurrentSymbolID;   
            m_CurrentSymbolID++;
            m_SymbolToString[ReturnValue] = SymbolString;
        }
        return ReturnValue;
    }
    bool Evaluator::p_SymbolIsPrimitive(SymbolID IDToCompare)
    {
        return IDToCompare < SymbolID(PrimitiveForms::LAST);
    }
    void Evaluator::p_InternPrimitiveSymbols()
    {
        for(auto const& String : {"cond","tagbody","go","set"})
        {
            p_GetSymbolID(String);
        }
        for(auto const& Pair : std::vector<std::pair<std::string,BuiltinFuncType>>{{"print",Print}})
        {
            m_Builtins[p_GetSymbolID(Pair.first)] = Pair.second;
        }
    }
    Evaluator::Evaluator()
    {
        p_InternPrimitiveSymbols();
    }
    void Evaluator::Eval(std::string_view Content)
    {
        List ParsedContent = p_Read(Content);
        m_OpCodes = OpCodeList(ParsedContent);
        p_Eval(m_OpCodes);
    }
}
