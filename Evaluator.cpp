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



    


    Value Evaluator::Less(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
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
    Value Evaluator::CreateList(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
    {
        Value ReturnValue = List(std::move(Arguments));
        return  ReturnValue;
    }
    Value Evaluator::Plus(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
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
    void Evaluator::Print(Evaluator& AssociatedEvaluator,Value const& ValueToPrint)
    {
        if(ValueToPrint.IsType<String>())
        {
            std::cout<<'"'<<ValueToPrint.GetType<String>()<<'"';
        }
        else if(ValueToPrint.IsType<Int>())
        {
            std::cout<<ValueToPrint.GetType<Int>();
        }
        else if(ValueToPrint.IsType<bool>())
        {
            std::cout<<ValueToPrint.GetType<bool>();
        }
        else if(ValueToPrint.IsType<bool>())
        {
            std::cout<<bool(ValueToPrint.GetType<bool>());
        }
        else if(ValueToPrint.IsType<Symbol>())
        {
            std::cout<<AssociatedEvaluator.GetSymbolString(ValueToPrint.GetType<Symbol>().ID);
        }
        else if(ValueToPrint.IsType<List>())
        {
            std::cout<<"(";
            auto const& ListToPrint = ValueToPrint.GetType<List>();
            for(int i = 0; i < ListToPrint.size();i++)
            {
                Print(AssociatedEvaluator,ListToPrint[i]);
                if(i + 1 < ListToPrint.size())
                {
                    std::cout<<" ";
                }
            }
            std::cout<< ")";
        }
    }
    Value Evaluator::Print(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        for(auto const& Argument : Arguments)
        {
            Print(AssociatedEvaluator,Argument);
        }
        std::cout<<std::endl;
        return ReturnValue;
    }
    void Evaluator::p_MergeClasses(std::vector<ClassDefinition*> const& ClassesToMerge,ClassDefinition& NewClass)
    {
        std::vector<ClassID> NewClasses;
        std::vector<SlotDefinition> NewSlots;
        for(auto const& Class : ClassesToMerge)
        {
            std::vector<ClassID> TempClasses;
            std::swap(NewClasses,TempClasses);
            std::merge(TempClasses.begin(),TempClasses.end(),Class->Types.begin(),Class->Types.end(),std::inserter(TempClasses,TempClasses.begin()));
            std::swap(NewClasses,TempClasses);

            std::vector<SlotDefinition> TempSlots;
            std::swap(NewSlots,TempSlots);
            std::merge(TempSlots.begin(),TempSlots.end(),Class->SlotDefinitions.begin(),Class->SlotDefinitions.end(),std::inserter(TempSlots,TempSlots.begin()));
            std::swap(NewSlots, TempSlots);
        }
        NewClass.Types = std::move(NewClasses);
        NewClass.SlotDefinitions = std::move(NewSlots);
    }

    //TODO MEGA temporary
    ClassID i___CurrentClassID = 1u<<UserClassBit;
    Value Evaluator::Class(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
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
        std::vector<ClassDefinition*> Parents;
        for(auto& Value : Arguments[0].GetType<List>())
        {
            if(!Value.IsType<ClassDefinition>())
            {
                throw std::runtime_error("non-ClassDefinition object in list of parent classes in second argument to class");
            }
            Parents.push_back(Value.GetRef<ClassDefinition>().get());
        }
        ClassDefinition TemporaryClass;//slots get added here
        if(!Arguments[1].IsType<List>())
        {
             throw std::runtime_error("second argument to class has to be a list of slot definitions");
        }
        for(auto const& Slot : Arguments[1].GetType<List>())
        {
            if(!Slot.IsType<List>() || Slot.GetType<List>().size() != 2 || !(Slot.GetType<List>()[0].IsType<Symbol>()))
            {
                throw std::runtime_error(
                        "error in class: slot definition has to be a list containing a symbol as the first element, "
                        "and an arbitrary form "
                        "as the second");
            }
            SlotDefinition NewSlot;
            NewSlot.DefaultValue = Slot.GetType<List>()[1];
            NewSlot.Symbol = Slot.GetType<List>()[0].GetType<Symbol>().ID;
            TemporaryClass.SlotDefinitions.push_back(NewSlot);
        }
        if (Parents.size() == 0)
        {
            TemporaryClass.Types.push_back(1u << UserClassBit);
        }
        Parents.push_back(&TemporaryClass);
        p_MergeClasses(Parents,NewClass);
        Ref<FunctionDefinition> SlotInitializers = std::make_shared<FunctionDefinition>();
        SlotInitializers->Arguments = {AssociatedEvaluator.GetSymbolID("INIT")};
        SlotInitializers->Instructions = std::make_shared<OpCodeList>(OpCodeList(AssociatedEvaluator.GetSymbolID("INIT"),AssociatedEvaluator.GetSymbolID("index"),
                NewClass.SlotDefinitions));
        NewClass.SlotInitializers = SlotInitializers;
        i___CurrentClassID++;
        NewClass.Types.push_back(i___CurrentClassID);
        NewClass.ID = i___CurrentClassID;
        return std::move(NewClass);
    }
    Value Evaluator::AddMethod(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
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
    Value Evaluator::Generic(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
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
    Value Evaluator::Index_List(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
    {
        assert(Arguments.size() >= 2 && Arguments[0].IsType<List>());
        List& AssociatedList = Arguments[0].GetType<List>();
        if(!Arguments[1].IsType<Int>())
        {
            throw std::runtime_error("Can only index list type with integer");
        }
        Int Index = Arguments[1].GetType<Int>();
        if(Index >= AssociatedList.size() || Index < 0)
        {
            throw std::runtime_error("Index out of range when indexing list"); 
        }
        Value& AssociatedValue = AssociatedList[Index];
        if(!AssociatedValue.IsType<Value>())
        {
            AssociatedValue.MakeRef();
        }
        return AssociatedValue;
    }
    Value Evaluator::Index_ClassInstance(Evaluator& AssociatedEvaluator,std::vector<Value>& Arguments)
    {
        Value ReturnValue;
        assert(Arguments.size() >= 2 && Arguments[0].IsType<ClassInstance>());
        ClassInstance& AssociatedInstance = Arguments[0].GetType<ClassInstance>();
        if(!Arguments[1].IsType<Symbol>())
        {
            throw std::runtime_error("Can only index class instance with symbol type");
        }
        Symbol& IndexSymbol = Arguments[1].GetType<Symbol>();
        auto SymbolIt = std::lower_bound(AssociatedInstance.Slots.begin(),AssociatedInstance.Slots.end(),IndexSymbol.ID,
                [](std::pair<SymbolID,Value> const& lhs,SymbolID rhs)
                {
                    return lhs.first < rhs;
                });
        if(SymbolIt == AssociatedInstance.Slots.end() || SymbolIt->first != IndexSymbol.ID)
        {
            throw std::runtime_error("Couldn't find symbol when indexing class instance");
        }
        if(!SymbolIt->second.IsType<Value>())
        {
            SymbolIt->second.MakeRef();
        }
        return SymbolIt->second;
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
            CurrentCallStack.back().ArgumentStack.push_back(AssociatedFunc(*this,Arguments));
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
            Ref<ClassInstance> NewInstance = std::make_shared<ClassInstance>();
            NewInstance->AssociatedClass = ObjectToCall.GetRef<ClassDefinition>();

            for(auto const& Slot : NewInstance->AssociatedClass->SlotDefinitions)
            {
                NewInstance->Slots.push_back(std::make_pair(Slot.Symbol,Value()));
            }
            StackFrame NewStackFrame(OpCodeExtractor(*NewInstance->AssociatedClass->SlotInitializers->Instructions));
            NewStackFrame.StackScope = std::make_shared<Scope>();
            NewStackFrame.StackScope->SetParentScope(m_GlobalScope);
            Value NewValue = NewInstance;
            NewStackFrame.StackScope->SetVariable(p_GetSymbolID("INIT"),NewValue);
            CurrentCallStack.push_back(std::move(NewStackFrame));
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
                CurrentFrame.ArgumentStack.pop_back();
                CurrentFrame.ArgumentStack.pop_back();
                if(SymbolToAssign.IsType<Symbol>())
                {
                    CurrentFrame.StackScope->SetVariable(SymbolToAssign.GetType<Symbol>().ID,AssignedValue);
                }
                else
                {
                    SymbolToAssign = AssignedValue;   
                }
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

    void Evaluator::p_SkipWhiteSpace(MBUtility::StreamReader& Content)
    {
        Content.SkipWhile([](char NextChar)
                {
                    return NextChar == ' ' || NextChar == '\r' || NextChar == '\t' || NextChar == '\n' || NextChar == '\v';
                });
    }
    //TODO escaping
    String Evaluator::p_ReadString(MBUtility::StreamReader& Content)
    {
        String ReturnValue;
        Content.ReadByte();
        bool StringFinished = false;
        ReturnValue = Content.ReadWhile([](char NextChar)
                {
                    return NextChar != '"';
                });
        if(Content.EOFReached())
        {
            throw std::runtime_error("missing \" for string literal");
        }
        Content.ReadByte();
        return ReturnValue;
    }
    Value Evaluator::p_ReadSymbol(MBUtility::StreamReader& Content)
    {
        Value ReturnValue;
        std::string SymbolString = Content.ReadWhile([](char CharToRead)
                {
                    return (CharToRead >= 'A' && CharToRead <= 'Z') || (CharToRead >= 'a' && CharToRead <= 'z') || CharToRead == '<' || CharToRead == '+' 
                    || CharToRead == '-';
                });
        if (SymbolString == "")
        {
            throw std::runtime_error("Error reading symbol: first character invalid");
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
    Int Evaluator::p_ReadInteger(MBUtility::StreamReader& Content)
    {
        Int ReturnValue;
        std::string NumberString = Content.ReadWhile([](char NextChar)
                {
                    return NextChar >= '0' && NextChar <= '9';
                });
        ReturnValue = std::stoi(NumberString);
        return ReturnValue;
    }
    List Evaluator::p_ReadList(MBUtility::StreamReader& Content)
    {
        List ReturnValue;
        Content.ReadByte();
        p_SkipWhiteSpace(Content);
        if(Content.EOFReached())
        {
            throw std::runtime_error("Error reading list: no corresponding )");   
        }
        while(!Content.EOFReached())
        {
            if(Content.PeekByte() == ')')
            {
                Content.ReadByte();
                break;
            }
            ReturnValue.push_back(p_ReadTerm(Content));
            p_SkipWhiteSpace(Content);
        }
        return ReturnValue;
    }
    Value Evaluator::p_ReadTerm(MBUtility::StreamReader& Content)
    {
        Value ReturnValue;
        p_SkipWhiteSpace(Content);
        if(Content.EOFReached())
        {
            throw std::runtime_error("Error reading term: end of input");   
        }
        char NextChar = Content.PeekByte();
        if(Content.PeekByte() == '(')
        {
            ReturnValue = p_ReadList(Content);
        }
        else if(Content.PeekByte() == '"')
        {
            ReturnValue = p_ReadString(Content);
        }
        else if( NextChar >= '0' && NextChar <= '9')
        {
            ReturnValue = p_ReadInteger(Content);
        }
        else
        {
            ReturnValue = p_ReadSymbol(Content);
        }
        return ReturnValue;
    }
    List Evaluator::p_Read(MBUtility::StreamReader& Content)
    {
        List ReturnValue;
        while(!Content.EOFReached())
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
    std::string Evaluator::GetSymbolString(SymbolID SymbolToConvert)
    {
        return m_SymbolToString[SymbolToConvert];
    }
    SymbolID Evaluator::GetSymbolID(std::string const& SymbolString)
    {
        return p_GetSymbolID(SymbolString);
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

        //default generics
        GenericFunction IndexGeneric = GenericFunction({p_GetSymbolID("container"),p_GetSymbolID("asdad")});
        IndexGeneric.AddMethod({Value::GetBuiltinTypeID<List>()},Function(Index_List));
        IndexGeneric.AddMethod({1u<<UserClassBit},Function(Index_ClassInstance));

        m_GlobalScope->SetVariable(p_GetSymbolID("index"),Value(std::make_shared<GenericFunction>(std::move(IndexGeneric))));
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
        MBUtility::StreamReader Reader = MBUtility::StreamReader(std::make_unique<MBUtility::IndeterminateStringStream>(Content));
        while(!Reader.EOFReached())
        {
            IPIndex InstructionToExecute = OpCodes.Size();
            Value NewTerm = p_Expand(CurrentScope,p_ReadTerm(Reader));
            p_SkipWhiteSpace(Reader);
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
