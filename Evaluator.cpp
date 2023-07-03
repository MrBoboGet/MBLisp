#include "Evaluator.h"
#include "assert.h"
#include <MBParsing/MBParsing.h>

#include  <iostream>
namespace MBLisp
{
    //Begin Scope
    void Scope::SetParentScope(std::shared_ptr<Scope> ParentScope)
    {
        m_ParentScope = ParentScope;
    }
    Value Scope::FindVariable(SymbolID Variable)
    {
        Value* ReturnValue = TryGet(Variable);
        if(ReturnValue == nullptr)
        {
            throw std::runtime_error("Couldn't find variable in current scope");
        }
        return *ReturnValue;
    }
    Value* Scope::TryGet(SymbolID Variable)
    {
        Value* ReturnValue = nullptr;
        if(auto VarIt = m_Variables.find(Variable); VarIt != m_Variables.end())
        {
            return &VarIt->second;
        }
        else if(m_ParentScope != nullptr)
        {
            ReturnValue = m_ParentScope->TryGet(Variable);
        }
        return ReturnValue;
    }
    void Scope::SetVariable(SymbolID Variable,Value NewValue)
    {
        m_Variables[Variable] = std::move(NewValue);
    }
    //END Scope



    


    Value Evaluator::Less(std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        if(Arguments.size() != 2)
        {
            throw std::runtime_error("< requires exactly 2 arguments");
        }
        bool ArgsAreSame = Arguments[0].IsSameType(Arguments[1]);
        if(!ArgsAreSame)
        {
            throw std::runtime_error("incompatible types for <");
        }
        if(Arguments[0].IsType<String>())
        {
            ReturnValue = Arguments[0].GetType<String>() < Arguments[1].GetType<String>();
        }
        else if(Arguments[1].IsType<Int>())
        {
            ReturnValue = Arguments[0].GetType<Int>() < Arguments[1].GetType<Int>();
        }
        return  ReturnValue;
    }
    Value Evaluator::CreateList(std::vector<Value>& Arguments)
    {
        Value ReturnValue = List(std::move(Arguments));
        return  ReturnValue;
    }
    Value Evaluator::Plus(std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        if(Arguments.size() != 2)
        {
            throw std::runtime_error("+ requires exactly 2 arguments");
        }
        bool ArgsAreSame = Arguments[0].IsSameType(Arguments[1]);
        if(!ArgsAreSame)
        {
            throw std::runtime_error("incompatible types for +");
        }
        if(Arguments[0].IsType<String>())
        {
            ReturnValue = Arguments[0].GetType<String>() + Arguments[1].GetType<String>();
        }
        else if(Arguments[1].IsType<Int>())
        {
            ReturnValue = Arguments[0].GetType<Int>() + Arguments[1].GetType<Int>();
        }
        return  ReturnValue;
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
            else if(Argument.IsType<Int>())
            {
                std::cout<<Argument.GetType<Int>();
            }
            else if(Argument.IsType<bool>())
            {
                std::cout<<Argument.GetType<bool>();
            }
        }
        std::cout<<std::endl;
        return ReturnValue;
    }
    void Evaluator::p_MergeClasses(std::vector<Ref<ClassDefinition>> const& ClassesToMerge,ClassDefinition& NewClass)
    {
        std::vector<ClassID> NewClasses;
        for(auto const& Class : ClassesToMerge)
        {
            std::vector<ClassID> TempClasses;
            std::swap(NewClasses,TempClasses);
            std::merge(TempClasses.begin(),TempClasses.end(),Class->Types.begin(),Class->Types.end(),std::inserter(TempClasses,TempClasses.begin()));
            std::swap(NewClasses,TempClasses);
        }
        NewClass.Types = std::move(NewClasses);
    }

    //TODO MEGA temporarty
    ClassID i___CurrentClassID = 0;
    Value Evaluator::Class(std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        if(Arguments.size() < 2)
        {
            throw std::runtime_error("to few arguments for class: class requires exactly 2 argument, a list of overrides, and a list of slots");
        }
        if(Arguments.size() > 2)
        {
            throw std::runtime_error("to many arguments for class: class requires exactly 2 argument, a list of overrides, and a list of slots");
        }
        ClassDefinition NewClass;
        if(!Arguments[0].IsType<List>())
        {
            throw std::runtime_error("first argument of class has to be a list of parent types");
        }
        std::vector<std::shared_ptr<ClassDefinition>> Parents;
        for(auto& Value : Arguments[0].GetType<List>())
        {
            if(!Value.IsType<ClassDefinition>())
            {
                throw std::runtime_error("non-ClassDefinition object in list of parent classes in second argument to class");
            }
            Parents.push_back(Value.GetRef<ClassDefinition>());
        }
        p_MergeClasses(Parents,NewClass);
        NewClass.Types.push_back(i___CurrentClassID);
        NewClass.ID = i___CurrentClassID;
        i___CurrentClassID++;
        return std::move(NewClass);
    }
    Value Evaluator::AddMethod(std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        if(Arguments.size() < 3)
        {
            throw std::runtime_error(
                    "to few arguments for addmethod: addmethod requires 3 arguments, the generic to modify, a list of types, and a callable");
        }
        if(Arguments.size() > 3)
        {
            throw std::runtime_error(
                    "to many arguments for addmethod: addmethod requires 3 arguments, the generic to modify, a list of types, and a callable");
        }
        if(!Arguments[0].IsType<GenericFunction>())
        {
            throw std::runtime_error("first argument to addmethod has to be a generic to be modified");
        }
        GenericFunction& GenericToModify = Arguments[0].GetType<GenericFunction>();
        if(!Arguments[1].IsType<List>())
        {
            throw std::runtime_error("second argument to addmethod has to be a list of specified types");
        }
        std::vector<ClassID> OverridenTypes;
        for(auto& Type : Arguments[1].GetType<List>())
        {
            if(!Type.IsType<ClassDefinition>())
            {
                throw std::runtime_error("non-classdefinition in list of class definitions supplied to addmethod");
            }
            OverridenTypes.push_back(Type.GetType<ClassDefinition>().ID);
        }
        GenericToModify.AddMethod(OverridenTypes,Arguments[2]);
        return ReturnValue;
    }
    Value Evaluator::Generic(std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        if(Arguments.size() < 1)
        {
            throw std::runtime_error("to few arguments sfor generic: exactly one argument is required, a list of symbols");
        }
        if(Arguments.size() < 1)
        {
            throw std::runtime_error("to emany arguments for generic: exactly one argument is required, a list of symbols");
        }
        if (!Arguments[0].IsType<List>())
        {
            throw std::runtime_error("argument to generic has to be a list of symbols");
        }
        std::vector<SymbolID> GenericArgs;
        for(auto const& Argument : Arguments[0].GetType<List>())
        {
            if(!Argument.IsType<Symbol>())
            {
                throw std::runtime_error("non-symbol in list of symbols supplied to generic");   
            }
            GenericArgs.push_back(Argument.GetType<Symbol>().ID);
        }
        ReturnValue = std::move(GenericFunction(std::move(GenericArgs)));
        return ReturnValue;
    }
    Value Evaluator::p_Eval(std::shared_ptr<Scope> AssociatedScope,FunctionDefinition& FunctionToExecute,std::vector<Value> Arguments)
    {
        for(int i = 0; i <Arguments.size();i++)
        {
            AssociatedScope->SetVariable(FunctionToExecute.Arguments[i].ID,std::move(Arguments[i]));
        }
        return p_Eval(AssociatedScope,*FunctionToExecute.Instructions);
    }
    void Evaluator::p_Invoke(Value& ObjectToCall,std::vector<Value>& Arguments,std::vector<StackFrame>& CurrentCallStack)
    {
        if(ObjectToCall.IsType<Function>())
        {
            BuiltinFuncType AssociatedFunc = ObjectToCall.GetType<Function>().Func;
            assert(AssociatedFunc != nullptr);
            CurrentCallStack.back().ArgumentStack.push_back(AssociatedFunc(Arguments));
        }
        else if(ObjectToCall.IsType<Lambda>())
        {
            Lambda& AssociatedLambda = ObjectToCall.GetType<Lambda>();
            if(Arguments.size() < AssociatedLambda.Definition->Arguments.size())
            {
                throw std::runtime_error("To few arguments for function call");
            }
            else if(Arguments.size() > AssociatedLambda.Definition->Arguments.size())
            {
                throw std::runtime_error("To many arguments for function call");
            }
            StackFrame NewStackFrame(OpCodeExtractor(*AssociatedLambda.Definition->Instructions));
            NewStackFrame.StackScope = std::make_shared<Scope>();
            NewStackFrame.StackScope->SetParentScope(AssociatedLambda.AssociatedScope);
            for(int i = 0; i < Arguments.size();i++)
            {
                NewStackFrame.StackScope->SetVariable(AssociatedLambda.Definition->Arguments[i].ID,Arguments[i]);   
            }
            CurrentCallStack.push_back(std::move(NewStackFrame));
        }
        else if(ObjectToCall.IsType<ClassDefinition>())
        {
            ClassInstance NewInstance;
            NewInstance.AssociatedClass = ObjectToCall.GetRef<ClassDefinition>();
            CurrentCallStack.back().ArgumentStack.push_back(std::move(NewInstance));
        }
        else if(ObjectToCall.IsType<GenericFunction>())
        {
            GenericFunction& GenericToInvoke = ObjectToCall.GetType<GenericFunction>();
            Value* Callable = GenericToInvoke.GetMethod(Arguments);
            if(Callable == nullptr)
            {
                throw std::runtime_error("No method associated with the argumnet list");   
            }
            p_Invoke(*Callable,Arguments,CurrentCallStack);
        }
        else
        {
            throw std::runtime_error("Cannot invoke object");   
        }
    }
    Value Evaluator::p_Eval(std::shared_ptr<Scope> CurrentScope,OpCodeList& OpCodes,IPIndex Offset)
    {
        Value ReturnValue;
        std::vector<StackFrame> StackFrames = {StackFrame(OpCodeExtractor(OpCodes))};
        StackFrames.back().ExecutionPosition.SetIP(Offset);
        StackFrames.back().StackScope = CurrentScope;
        while(StackFrames.size() != 0)
        {
            StackFrame& CurrentFrame = StackFrames.back();
            if(CurrentFrame.ExecutionPosition.Finished())
            {
                assert(CurrentFrame.ArgumentStack.size() == 1);
                ReturnValue = std::move(CurrentFrame.ArgumentStack.back());
                StackFrames.pop_back();
                if(StackFrames.size() != 0)
                {
                    StackFrames.back().ArgumentStack.push_back(ReturnValue);   
                }
                else
                {
                    return ReturnValue;   
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
                CurrentFrame.ArgumentStack.push_back(CurrentFrame.StackScope->FindVariable(PushCode.ID));
            }
            else if(CurrentCode.IsType<OpCode_PushLiteral>())
            {
                OpCode_PushLiteral& PushCode = CurrentCode.GetType<OpCode_PushLiteral>();
                //copy the literal
                Value LiteralToPush = PushCode.Literal;
                if(LiteralToPush.IsType<Lambda>())
                {
                    LiteralToPush.GetType<Lambda>().AssociatedScope = CurrentFrame.StackScope;
                }
                CurrentFrame.ArgumentStack.push_back(LiteralToPush);
            }
            else if(CurrentCode.IsType<OpCode_Goto>())
            {
                OpCode_Goto& GotoCode = CurrentCode.GetType<OpCode_Goto>();
                CurrentFrame.ExecutionPosition.SetIP(GotoCode.NewIP);
                if(GotoCode.ResetStack)
                {
                    CurrentFrame.ArgumentStack.clear();
                }
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
                Value ValueToExamine = std::move(CurrentFrame.ArgumentStack.back());
                bool Result = ValueToExamine.IsType<bool>() && ValueToExamine.GetType<bool>();
                CurrentFrame.ArgumentStack.pop_back();
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
                //if(!(CurrentFrame.ArgumentStack.back().IsType<Function>() || CurrentFrame.ArgumentStack.back().IsType<Lambda>()))
                //{
                //    throw std::runtime_error("can only invoke objects of type function");
                //}
                OpCode_CallFunc&  CallFuncCode = CurrentCode.GetType<OpCode_CallFunc>();
                Value FunctionToCall = std::move(CurrentFrame.ArgumentStack.back());
                CurrentFrame.ArgumentStack.pop_back();
                std::vector<Value> Arguments;
                for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                {
                    Arguments.push_back(CurrentFrame.ArgumentStack[CurrentFrame.ArgumentStack.size()-CallFuncCode.ArgumentCount+i]);
                }
                for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                {
                    CurrentFrame.ArgumentStack.pop_back();
                }
                p_Invoke(FunctionToCall,Arguments,StackFrames);
            }
            else if(CurrentCode.IsType<OpCode_Macro>())
            {
                assert(CurrentFrame.ArgumentStack.size() >= 1);
                Value ValueToConvert = std::move(CurrentFrame.ArgumentStack.back());
                CurrentFrame.ArgumentStack.pop_back();
                if(!ValueToConvert.IsType<Lambda>())
                {
                    throw std::runtime_error("Can only make macro of function");   
                }
                Macro MacroToCreate;
                MacroToCreate.AssociatedFunction = ValueToConvert.GetType<Lambda>().Definition;
                CurrentFrame.ArgumentStack.push_back(std::move(MacroToCreate));
            }
            else if(CurrentCode.IsType<OpCode_Set>())
            {
                //first value of stack is symbol, second is value
                assert(CurrentFrame.ArgumentStack.size() >= 2);
                Value SymbolToAssign = std::move(*(CurrentFrame.ArgumentStack.end()-2));
                Value AssignedValue = std::move(*(CurrentFrame.ArgumentStack.end()-1));
                if(!SymbolToAssign.IsType<Symbol>())
                {
                    throw std::runtime_error("lhs of set has to be a symbol");
                }
                CurrentFrame.ArgumentStack.pop_back();
                CurrentFrame.ArgumentStack.pop_back();
                CurrentFrame.StackScope->SetVariable(SymbolToAssign.GetType<Symbol>().ID,AssignedValue);
                CurrentFrame.ArgumentStack.push_back(AssignedValue);
            }
        }
        return ReturnValue;
    }
    Value Evaluator::p_Expand(std::shared_ptr<Scope> ExpandScope,Value ValueToExpand)
    {
        if(ValueToExpand.IsType<List>())
        {
            return  p_Expand(ExpandScope,std::move(ValueToExpand.GetType<List>()));
        }
        return ValueToExpand;
    }
    Value Evaluator::p_Expand(std::shared_ptr<Scope> ExpandScope,List ListToExpand)
    {
        Value ReturnValue;
        if(ListToExpand.size() == 0)
        {
            return ListToExpand;
        }
        if(ListToExpand[0].IsType<Symbol>())
        {
            Symbol& HeadSymbol = ListToExpand[0].GetType<Symbol>();
            if(!p_SymbolIsPrimitive(HeadSymbol.ID))
            {
                if(auto  VarIt = ExpandScope->TryGet(HeadSymbol.ID); VarIt != nullptr && VarIt->IsType<Macro>())
                {
                    Macro& AssociatedMacro = VarIt->GetType<Macro>();
                    std::vector<Value> Arguments;
                    for(int i = 1; i < ListToExpand.size();i++)
                    {
                        Arguments.push_back(ListToExpand[i]);
                    }
                    return p_Expand(ExpandScope,p_Eval(ExpandScope,*AssociatedMacro.AssociatedFunction,std::move(Arguments)));
                }
            }
        }
        //head wasn't macro, try to expand children
        List NewList;
        NewList.push_back(ListToExpand[0]);
        for(int i = 1; i < ListToExpand.size();i++)
        {
            if(ListToExpand[i].IsType<List>())
            {
                NewList.push_back(p_Expand(ExpandScope,std::move(ListToExpand[i].GetType<List>())));
            }
            else
            {
                NewList.push_back(ListToExpand[i]);
            }
        }
        return NewList;
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
    Value Evaluator::p_ReadSymbol(std::string_view& Content)
    {
        Value ReturnValue;
        std::string SymbolString;
        while(Content.size() != 0 &&
                ((Content[0] >= 'A' && Content[0] <= 'Z') || (Content[0] >= 'a' && Content[0] <= 'z') || Content[0] == '<' || Content[0] == '+'))
        {
            SymbolString += Content[0];
            Content = Content.substr(1);
        }
        if (SymbolString == "")
        {
            throw std::runtime_error(std::string("Error reading symbol: first character invalid") + Content[0]);
        }
        else if(SymbolString == "true")
        {
            ReturnValue = true;
        }
        else if(SymbolString == "false")
        {
            ReturnValue = false;
        }
        else
        {
            ReturnValue = Symbol(p_GetSymbolID(SymbolString));
        }
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
        return IDToCompare < m_PrimitiveSymbolMax;
    }
    void Evaluator::p_InternPrimitiveSymbols()
    {
        for(auto const& String : {"cond","tagbody","go","set","lambda","progn","quote","macro"})
        {
            p_GetSymbolID(String);
        }
        for(auto const& Pair : std::vector<std::pair<std::string,BuiltinFuncType>>{
                    {"print",Print},
                    {"+",Plus},
                    {"<",Less},
                    {"list",CreateList},
                    {"class",Class},
                    {"addmethod",AddMethod},
                    {"generic",Generic},
                })
        {
            Function NewBuiltin;
            NewBuiltin.Func = Pair.second;
            m_GlobalScope->SetVariable(p_GetSymbolID(Pair.first),NewBuiltin);
        }
        m_PrimitiveSymbolMax = m_CurrentSymbolID;
    }
    Evaluator::Evaluator()
    {
        p_InternPrimitiveSymbols();
    }
    void Evaluator::Eval(std::string_view Content)
    {
        std::shared_ptr<Scope> CurrentScope = std::make_shared<Scope>();
        CurrentScope->SetParentScope(m_GlobalScope);
        OpCodeList OpCodes;
        while(Content.size() != 0)
        {
            IPIndex InstructionToExecute = OpCodes.Size();
            Value NewTerm = p_Expand(CurrentScope,p_ReadTerm(Content));
            p_SkipWhiteSpace(Content);
            if(NewTerm.IsType<List>())
            {
                OpCodes.Append(NewTerm.GetType<List>());
            }
            else
            {
                List ListToEncode = {std::move(NewTerm)};
                OpCodes.Append(NewTerm.GetType<List>());
            }
            p_Eval(CurrentScope,OpCodes,InstructionToExecute);
        }
    }
}
