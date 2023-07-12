#include "Evaluator.h"
#include "assert.h"
#include <MBParsing/MBParsing.h>

#include  <iostream>
#include <iterator>
namespace MBLisp
{



    


    Value Evaluator::Less BUILTIN_ARGLIST
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
    Value Evaluator::CreateList BUILTIN_ARGLIST
    {
        Value ReturnValue = List(std::move(Arguments));
        return  ReturnValue;
    }
    Value Evaluator::Plus BUILTIN_ARGLIST
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
    Value Evaluator::Print BUILTIN_ARGLIST
    {
        Value ReturnValue;
        for(auto const& Argument : Arguments)
        {
            Print(AssociatedEvaluator,Argument);
            std::cout<<" ";
        }
        std::cout<<std::endl;
        if(Arguments.size() != 0)
        {
            ReturnValue = Arguments.back();
        }
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
    Value Evaluator::Class BUILTIN_ARGLIST
    {
        Value ReturnValue;
        if(Arguments.size() < 2)
        {
            throw std::runtime_error("to few arguments for class: class requires atleast 2 argument, a list of overrides, and a list of slots");
        }
        if(Arguments.size() > 3)
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
            if(Value::TypeIsBuiltin(Value.GetType<ClassDefinition>().ID))
            {
                throw std::runtime_error("cannot inherit from builtin class");
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

        if(Arguments.size() == 3)
        {
            NewClass.Constructor = std::make_shared<Value>(Arguments[2]);   
        }
        
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
    Value Evaluator::AddMethod BUILTIN_ARGLIST
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
    Value Evaluator::Generic BUILTIN_ARGLIST
    {
        return GenericFunction();
    }
    std::atomic<DynamicVarID> g__CurrentDynamicVarID{1};
    Value Evaluator::Dynamic BUILTIN_ARGLIST
    {
        DynamicVariable ReturnValue;
        if(Arguments.size() != 1)
        {
            throw std::runtime_error("dynamic requires exacty 1 argument, the initial value for the dynamic variable");   
        }
        ReturnValue.DefaultValue = std::move(Arguments[0]);
        ReturnValue.ID = g__CurrentDynamicVarID.load();
        g__CurrentDynamicVarID.fetch_add(1);
        return ReturnValue;
    }
    Value Evaluator::ReadTerm BUILTIN_ARGLIST
    {
        Value ReturnValue;
        if(Arguments.size() != 1)
        {
            throw std::runtime_error("Read requires exatly one argument: input stream to read");   
        }
        if(!Arguments[0].IsType<MBUtility::StreamReader>())
        {
            throw std::runtime_error("Invalid argument for read-term, argument has to be stream");   
        }
        MBUtility::StreamReader& Reader = Arguments[0].GetType<MBUtility::StreamReader>();
        ReadTable& Table = CurrentScope->FindVariable(AssociatedEvaluator.GetSymbolID("*READTABLE*")).GetType<ReadTable>();
        std::shared_ptr<Scope> NewScope = std::make_shared<Scope>();
        ReturnValue = AssociatedEvaluator.p_ReadTerm(NewScope,Table,Reader,Arguments[0]);
        return ReturnValue;
    }
    Value Evaluator::Stream_EOF BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<MBUtility::StreamReader>().EOFReached();
    }
    Value Evaluator::Stream_PeakByte BUILTIN_ARGLIST
    {
        return String(1,Arguments[0].GetType<MBUtility::StreamReader>().PeekByte());
    }
    Value Evaluator::Stream_ReadByte BUILTIN_ARGLIST
    {
        return String(1,Arguments[0].GetType<MBUtility::StreamReader>().ReadByte());
    }
    Value Evaluator::Stream_SkipWhitespace BUILTIN_ARGLIST
    {
        AssociatedEvaluator.p_SkipWhiteSpace(Arguments[0].GetType<MBUtility::StreamReader>());
        return false;
    }
    Value Evaluator::Index_List BUILTIN_ARGLIST
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
    Value Evaluator::Append_List BUILTIN_ARGLIST
    {
        List& AssociatdList = Arguments[0].GetType<List>();
        for(int i = 1; i < Arguments.size();i++)
        {
            AssociatdList.push_back(Arguments[i]);
        }
        return AssociatdList;
    }
    Value Evaluator::Len_List BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<List>().size();
    }
    Value Evaluator::Flatten_1 BUILTIN_ARGLIST
    {
        List ReturnValue;
        for(auto& Argument : Arguments)
        {
            if(Argument.IsType<List>())
            {
                for(auto& SubArgument : Argument.GetType<List>())
                {
                    ReturnValue.push_back(std::move(SubArgument));
                }
            }
            else
            {
                ReturnValue.push_back(std::move(Argument));
            }
        }
        return ReturnValue;
    }
    Value Evaluator::Eq_String BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<String>() == Arguments[1].GetType<String>();
    }
    Value Evaluator::Eq_Symbol BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Symbol>().ID == Arguments[1].GetType<Symbol>().ID;
    }
    Value Evaluator::Eq_Int BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Int>() == Arguments[1].GetType<Int>();
    }
    Value Evaluator::GenSym BUILTIN_ARGLIST
    {
        return Symbol(AssociatedEvaluator.GenerateSymbol());
    }
    Value Evaluator::Environment BUILTIN_ARGLIST
    {
        return CurrentScope;
    }
    Value Evaluator::NewEnvironment BUILTIN_ARGLIST
    {
        Ref<Scope> NewScope = std::make_shared<Scope>();
        NewScope->SetParentScope(AssociatedEvaluator.m_GlobalScope);
        return NewScope;
    }
    Value Evaluator::Index_Environment BUILTIN_ARGLIST
    {
        Scope& AssociatedScope = Arguments[0].GetType<Scope>();
        SymbolID SymbolIndex = Arguments[1].GetType<Symbol>().ID;
        Value* ReturnValue = AssociatedScope.TryGet(SymbolIndex);
        if(ReturnValue == nullptr)
        {
            AssociatedScope.SetVariable(SymbolIndex,Value());
            ReturnValue = AssociatedScope.TryGet(SymbolIndex);
            assert(ReturnValue != nullptr);
        }
        if(!ReturnValue->IsType<Value>())
        {
            ReturnValue->MakeRef();
        }
        return *ReturnValue;
    }
    Value Evaluator::AddReaderCharacter BUILTIN_ARGLIST
    {
        if(Arguments.size() != 3)
        {
            throw std::runtime_error("add-reader-character requires exactly 3 arguments");   
        }
        if(!Arguments[1].IsType<String>() || Arguments[1].GetType<String>().size() != 1)
        {
            throw std::runtime_error("second argument to add-reader-character must be string of size exactly 1");   
        }
        Value ReturnValue = false;
        ReadTable& TableToModify = Arguments[0].GetType<ReadTable>();
        TableToModify.Mappings[Arguments[1].GetType<String>()[0]] = std::move(Arguments[2]);
        return ReturnValue;
    }
    Value Evaluator::RemoveReaderCharacter BUILTIN_ARGLIST
    {
        if(Arguments.size() != 2)
        {
            throw std::runtime_error("remove-reader-character requires exactly 2 arguments");   
        }
        if(!Arguments[1].IsType<String>() || Arguments[1].GetType<String>().size() != 1)
        {
            throw std::runtime_error("second argument to remove-reader-character must be string of size exactly 1");   
        }
        Value ReturnValue = false;
        ReadTable& TableToModify = Arguments[0].GetType<ReadTable>();
        TableToModify.Mappings.erase(TableToModify.Mappings.find(Arguments[1].GetType<String>()[0]));
        return ReturnValue;
    }
    Value Evaluator::AddCharacterExpander BUILTIN_ARGLIST
    {
        if(Arguments.size() != 3)
        {
            throw std::runtime_error("add-reader-character requires exactly 3 arguments");   
        }


        return false;
    }
    Value Evaluator::RemoveCharacterExpander BUILTIN_ARGLIST
    {
        if(Arguments.size() != 3)
        {
            throw std::runtime_error("add-reader-character requires exactly 3 arguments");   
        }

        return false;
    }
    SymbolID Evaluator::GenerateSymbol()
    {
        SymbolID NewSymbol = m_CurrentSymbolID;
        m_CurrentSymbolID++;
        std::string SymbolString = "g:"+std::to_string(NewSymbol);
        while(m_InternedSymbols.find(SymbolString) != m_InternedSymbols.end())
        {
            SymbolString += "_";
        }
        m_SymbolToString[NewSymbol] = SymbolString;
        m_InternedSymbols[SymbolString] = NewSymbol;
        return NewSymbol;
    }
    Value Evaluator::Index_ClassInstance BUILTIN_ARGLIST
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
    ///Value Evaluator::p_Eval(std::shared_ptr<Scope> AssociatedScope,FunctionDefinition& FunctionToExecute,std::vector<Value> Arguments)
    ///{
    ///    if(Arguments.size() < FunctionToExecute.Arguments.size())
    ///    {
    ///        throw std::runtime_error("To few arguments for function call");   
    ///    }
    ///    for(int i = 0; i <FunctionToExecute.Arguments.size();i++)
    ///    {
    ///        AssociatedScope->SetVariable(FunctionToExecute.Arguments[i].ID,std::move(Arguments[i]));
    ///    }
    ///    return p_Eval(AssociatedScope,*FunctionToExecute.Instructions);
    ///}
    Value Evaluator::p_Eval(Ref<Scope> CurrentScope,Value Callable,std::vector<Value> Arguments)
    {
        std::vector<StackFrame> CurrentCallStack = {StackFrame(OpCodeExtractor())};
        CurrentCallStack.back().StackScope = CurrentScope;
        p_Invoke(Callable,Arguments,CurrentCallStack);
        return p_Eval(std::move(CurrentCallStack));
    }
    void Evaluator::p_Invoke(Value& ObjectToCall,std::vector<Value>& Arguments,std::vector<StackFrame>& CurrentCallStack)
    {
        if(ObjectToCall.IsType<Function>())
        {
            BuiltinFuncType AssociatedFunc = ObjectToCall.GetType<Function>().Func;
            assert(AssociatedFunc != nullptr);
            CurrentCallStack.back().ArgumentStack.push_back(AssociatedFunc(*this,CurrentCallStack.back().StackScope,Arguments));
        }
        else if(ObjectToCall.IsType<Lambda>())
        {
            Lambda& AssociatedLambda = ObjectToCall.GetType<Lambda>();
            if(Arguments.size() < AssociatedLambda.Definition->Arguments.size())
            {
                throw std::runtime_error("To few arguments for function call");
            }
            if(AssociatedLambda.Definition->RestParameter == 0)
            {
                if(Arguments.size() > AssociatedLambda.Definition->Arguments.size())
                {
                    throw std::runtime_error("To many arguments for function call");
                }
            }
            StackFrame NewStackFrame(OpCodeExtractor(AssociatedLambda.Definition->Instructions));
            NewStackFrame.StackScope = std::make_shared<Scope>();
            NewStackFrame.StackScope->SetParentScope(AssociatedLambda.AssociatedScope);
            for(int i = 0; i < AssociatedLambda.Definition->Arguments.size();i++)
            {
                NewStackFrame.StackScope->OverrideVariable(AssociatedLambda.Definition->Arguments[i].ID,Arguments[i]);   
            }
            if(AssociatedLambda.Definition->RestParameter != 0)
            {
                List RestList;
                for(int i = AssociatedLambda.Definition->Arguments.size();i < Arguments.size();i++)
                {
                    RestList.push_back(Arguments[i]);
                }
                NewStackFrame.StackScope->OverrideVariable(AssociatedLambda.Definition->RestParameter,std::move(RestList));
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
            StackFrame NewStackFrame(OpCodeExtractor(NewInstance->AssociatedClass->SlotInitializers->Instructions));
            NewStackFrame.StackScope = std::make_shared<Scope>();
            NewStackFrame.StackScope->SetParentScope(m_GlobalScope);
            Value NewValue = NewInstance;
            NewStackFrame.StackScope->OverrideVariable(p_GetSymbolID("INIT"),NewValue);
            if(NewInstance->AssociatedClass->Constructor != nullptr)
            {
                std::vector<Value> Args = {NewValue};
                for(auto& Arg : Arguments)
                {
                    Args.push_back(Arg);
                }
                p_Invoke(*NewInstance->AssociatedClass->Constructor,Args,CurrentCallStack);
                CurrentCallStack.back().PopExtra = 1;
            }
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
    Value Evaluator::p_Eval(std::vector<StackFrame> CurrentCallStack)
    {
        ExecutionState NewState;
        NewState.StackFrames = std::move(CurrentCallStack);
        return p_Eval(NewState);
    }
    Value Evaluator::p_Eval(ExecutionState& CurrentState)
    {
        Value ReturnValue;
        auto& StackFrames = CurrentState.StackFrames;
        while(StackFrames.size() != 0)
        {

            if(CurrentState.UnwindingStack)
            {
                assert(CurrentState.StackFrames.size() > 0);
                if(CurrentState.FrameTarget == StackFrames.size() -1)
                {
                    CurrentState.UnwindingStack = false;
                    CurrentState.FrameTarget = -1; 
                }
                else
                {
                    if (CurrentState.StackFrames.back().ActiveUnwindProtectorsBegin.size() == 0)
                    {
                        CurrentState.StackFrames.pop_back();
                        continue;
                    }
                    else if (CurrentState.StackFrames.back().Unwinding == false)
                    {
                        CurrentState.StackFrames.back().ExecutionPosition.SetIP(CurrentState.StackFrames.back().ActiveUnwindProtectorsBegin.back());
                        CurrentState.StackFrames.back().Unwinding = true;
                    }
                }
            }


            StackFrame& CurrentFrame = CurrentState.StackFrames.back();
            if(CurrentFrame.ExecutionPosition.Finished())
            {
                assert(CurrentFrame.ArgumentStack.size() == 1);
                assert(CurrentFrame.ActiveSignalHandlers.size() == 0);
                assert(CurrentFrame.ActiveUnwindProtectorsBegin.size() == 0);
                assert(CurrentFrame.SignalHandlerBunchSize.size() == 0);
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
            if(CurrentFrame.PopExtra != 0)
            {
                assert(CurrentFrame.ArgumentStack.size() >= CurrentFrame.PopExtra);
                for(int i = 0; i < CurrentFrame.PopExtra;i++)
                {
                    CurrentFrame.ArgumentStack.pop_back();
                }
                CurrentFrame.PopExtra = 0;
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
                Value VarToPush = CurrentFrame.StackScope->FindVariable(PushCode.ID);
                if(!VarToPush.IsType<DynamicVariable>())
                {
                    CurrentFrame.ArgumentStack.push_back(std::move(VarToPush));
                }
                else
                {
                    DynamicVariable const& DynamicToPush = VarToPush.GetType<DynamicVariable>();
                    VarToPush = DynamicToPush.DefaultValue;
                    if(auto DynIt = CurrentState.DynamicBindings.find(DynamicToPush.ID); DynIt != CurrentState.DynamicBindings.end())
                    {
                        if(DynIt->second.size() > 0)
                        {
                            VarToPush = DynIt->second.back();
                        }
                    }
                    CurrentFrame.ArgumentStack.push_back(std::move(VarToPush));
                }
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
                Macro MacroToCreate;
                MacroToCreate.Callable = std::make_shared<Value>(std::move(ValueToConvert));
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
                else if(SymbolToAssign.IsType<DynamicVariable>())
                {
                    DynamicVariable& AssociatedVariable = SymbolToAssign.GetType<DynamicVariable>();
                    if(auto It = CurrentState.DynamicBindings.find(AssociatedVariable.ID); It != CurrentState.DynamicBindings.end()
                            && It->second.size() != 0)
                    {
                        It->second.back() = std::move(AssignedValue);
                    }
                    else
                    {
                        AssociatedVariable.DefaultValue = std::move(AssignedValue);   
                    }
                }
                else
                {
                    SymbolToAssign = AssignedValue;   
                }
                CurrentFrame.ArgumentStack.push_back(AssignedValue);
            }
            else if(CurrentCode.IsType<OpCode_Signal>())
            {
                //should probably not allow signals when unwinding stack...
                assert(CurrentFrame.ArgumentStack.size() > 0);
                Value SignalValue = std::move(CurrentFrame.ArgumentStack.back());
                CurrentFrame.ArgumentStack.pop_back();
                //the signal form returns a value in the current frame, which
                int CurrentFrameIndex = CurrentState.StackFrames.size()-1;
                bool SignalFound = false;
                for(int i = CurrentFrameIndex; i >= 0; i--)
                {
                    if (SignalFound) break;
                    auto& CandidateFrame = CurrentState.StackFrames[i];
                    for(int j = int(CandidateFrame.ActiveSignalHandlers.size())-1;j >= 0;j--)
                    {
                        auto const& Handler = CandidateFrame.ActiveSignalHandlers[j];
                        if(p_ValueIsType(Handler.HandledType,SignalValue))
                        {
                            //add frame to stack
                            StackFrame NewFrame = StackFrame(CandidateFrame.ExecutionPosition);
                            NewFrame.StackScope = CandidateFrame.StackScope;
                            NewFrame.ExecutionPosition.SetIP(Handler.SignalBegin);
                            NewFrame.SignalFrameIndex = i;
                            NewFrame.StackScope->OverrideVariable(Handler.BoundValue,std::move(SignalValue));
                            StackFrames.push_back(std::move(NewFrame));
                            SignalFound = true;
                            break;
                        }
                    }
                }
                if(!SignalFound)
                {
                    CurrentFrame.ArgumentStack.push_back(false);
                }
            }
            else if(CurrentCode.IsType<OpCode_SignalHandler_Done>())
            {
                CurrentFrame.ExecutionPosition.SetEnd();
            } 
            else if(CurrentCode.IsType<OpCode_AddSignalHandlers>())
            {
                OpCode_AddSignalHandlers const& AddSignalsCode = CurrentCode.GetType<OpCode_AddSignalHandlers>();
                assert(CurrentFrame.ArgumentStack.size() >= AddSignalsCode.Handlers.size());
                int StackOffset = CurrentFrame.ArgumentStack.size()-AddSignalsCode.Handlers.size();
                for(auto const& NewHandler : AddSignalsCode.Handlers)
                {
                    Value const& TypeValue = CurrentFrame.ArgumentStack[StackOffset];
                    if(!TypeValue.IsType<ClassDefinition>())
                    {
                        throw std::runtime_error("signal handler value specifier evaluated to non class type");
                    }
                    SignalHandler NewSignalHandler;
                    NewSignalHandler.HandledType = TypeValue.GetType<ClassDefinition>().ID;
                    NewSignalHandler.BoundValue = NewHandler.BoundVariable;
                    NewSignalHandler.SignalBegin = NewHandler.HandlerBegin;
                    CurrentFrame.ActiveSignalHandlers.push_back(NewSignalHandler);
                    StackOffset++;
                }
                CurrentFrame.ArgumentStack.resize(CurrentFrame.ArgumentStack.size() - AddSignalsCode.Handlers.size());
                CurrentFrame.SignalHandlerBunchSize.push_back(AddSignalsCode.Handlers.size());
            }
            else if(CurrentCode.IsType<OpCode_RemoveSignalHandlers>())
            {
                assert(CurrentFrame.SignalHandlerBunchSize.size() > 0);
                assert(CurrentFrame.ActiveSignalHandlers.size() > 0);
                int SignalsToRemove = CurrentFrame.SignalHandlerBunchSize.back();
                CurrentFrame.SignalHandlerBunchSize.pop_back();
                CurrentFrame.ActiveSignalHandlers.resize(CurrentFrame.ActiveSignalHandlers.size()-SignalsToRemove);
                assert(CurrentFrame.SignalHandlerBunchSize.size() > 0 || CurrentFrame.ActiveSignalHandlers.size() == 0);
            }
            else if(CurrentCode.IsType<OpCode_UnwindProtect_Add>())
            {
                CurrentFrame.ActiveUnwindProtectorsBegin.push_back(CurrentCode.GetType<OpCode_UnwindProtect_Add>().UnwindBegin);
            }
            else if(CurrentCode.IsType<OpCode_UnwindProtect_Pop>())
            {
                assert(CurrentFrame.ActiveUnwindProtectorsBegin.size() > 0);
                CurrentFrame.ActiveUnwindProtectorsBegin.pop_back();
                if(CurrentState.UnwindingStack)
                {
                    if(CurrentFrame.ActiveUnwindProtectorsBegin.size() > 0)
                    {
                        CurrentFrame.ExecutionPosition.SetIP(CurrentFrame.ActiveUnwindProtectorsBegin.back());
                    }
                }
                CurrentFrame.Unwinding = false;
            }
            else if(CurrentCode.IsType<OpCode_Unwind>())
            {
                assert(CurrentFrame.SignalFrameIndex != -1);
                CurrentState.UnwindingStack = true;
                CurrentState.FrameTarget = CurrentFrame.SignalFrameIndex;
                StackFrames[CurrentFrame.SignalFrameIndex].ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_Unwind>().HandlersEnd);
                StackFrames[CurrentFrame.SignalFrameIndex].ArgumentStack.push_back(false);
            }
            else if(CurrentCode.IsType<OpCode_PushBindings>())
            {
                OpCode_PushBindings const& PushBindings = CurrentCode.GetType<OpCode_PushBindings>();
                assert(CurrentFrame.ArgumentStack.size() >= PushBindings.BindCount);
                std::vector<Value> Arguments;
                Arguments.insert(Arguments.end(),std::make_move_iterator(CurrentFrame.ArgumentStack.end()-PushBindings.BindCount*3),
                    std::make_move_iterator(CurrentFrame.ArgumentStack.end()));
                CurrentFrame.ArgumentStack.resize(CurrentFrame.ArgumentStack.size()-PushBindings.BindCount*3);
                std::vector<DynamicVarID> ModifiedBindings;
                std::vector<Value> NewValues;
                for(int i = 0; i < PushBindings.BindCount;i++)
                {
                    if(!Arguments[i].IsType<Scope>())
                    {
                        //TODO make this signal  instead
                        throw std::runtime_error("first part of binding triplet has to be a scope");
                    }   
                    Scope& ScopeToModify = Arguments[i].GetType<Scope>();
                    if(!Arguments[i+1].IsType<Symbol>())
                    {
                        //TODO make this signal  instead
                        throw std::runtime_error("second part of binding triplet has to be a symbol");
                    }   
                    SymbolID IDToModify = Arguments[i+1].GetType<Symbol>().ID;
                    Value* VariableToInspect = ScopeToModify.TryGet(IDToModify);
                    if(VariableToInspect == nullptr)
                    {
                        throw std::runtime_error("couldn't find dynamic variable  in scope");
                    }
                    if(!VariableToInspect->IsType<DynamicVariable>())
                    {
                        throw std::runtime_error("variable was not a dynamic variable");
                    }
                    ModifiedBindings.push_back(VariableToInspect->GetType<DynamicVariable>().ID);
                    NewValues.push_back(std::move(Arguments[i+2]));
                }
                for(int i = 0; i < ModifiedBindings.size();i++)
                {
                    CurrentState.DynamicBindings[ModifiedBindings[i]].push_back(std::move(NewValues[i]));
                }
                CurrentState.BindingStack.push_back(std::move(ModifiedBindings));
            }
            else if(CurrentCode.IsType<OpCode_PopBindings>())
            {
                assert(CurrentState.BindingStack.size()  != 0);
                auto BindingsToPop = std::move(CurrentState.BindingStack.back());
                CurrentState.BindingStack.pop_back();
                for(auto ID : BindingsToPop)
                {
                    assert(CurrentState.DynamicBindings[ID].size() !=  0);
                    CurrentState.DynamicBindings[ID].pop_back();
                }
            }
            else if(CurrentCode.IsType<OpCode_Eval>())
            {
                assert(CurrentFrame.ArgumentStack.size() >= 1);
                OpCode_Eval const& EvalCode = CurrentCode.GetType<OpCode_Eval>();
                Ref<Scope> ScopeToUse = CurrentFrame.StackScope;
                if(EvalCode.ArgCount == 2)
                {
                    Value ScopeValue = std::move(CurrentFrame.ArgumentStack.back());
                    CurrentFrame.ArgumentStack.pop_back();
                    if(!ScopeValue.IsType<Scope>())
                    {
                        throw std::runtime_error("argument to eval not of type environment");
                    }
                    ScopeToUse = ScopeValue.GetRef<Scope>();
                }
                Value ValueToEvaluate = CurrentFrame.ArgumentStack.back();
                CurrentFrame.ArgumentStack.pop_back();
                ValueToEvaluate = p_Expand(ScopeToUse,std::move(ValueToEvaluate));
                Ref<OpCodeList> NewOpcodes = std::make_shared<OpCodeList>(ValueToEvaluate);
                StackFrame NewFrame = StackFrame(OpCodeExtractor(NewOpcodes));
                NewFrame.StackScope = ScopeToUse;
                CurrentState.StackFrames.push_back(std::move(NewFrame));
            }
            else
            {
                assert(false && "p_Eval doesnt cover all opcode cases");
            }
        }
        return ReturnValue;

    }
    bool Evaluator::p_ValueIsType(ClassID TypeValue,Value const& ValueToInspect)
    {
        bool ReturnValue = false;
        if(ValueToInspect.IsType<ClassInstance>())
        {
            std::vector<ClassID> const& Types = ValueToInspect.GetType<ClassInstance>().AssociatedClass->Types;
            auto TypeIt = std::lower_bound(Types.begin(),Types.end(),TypeValue);
            if(TypeIt != Types.end() && *TypeIt == TypeValue)
            {
                return true;
            }
        }
        else
        {
            return TypeValue == ValueToInspect.GetTypeID();
        }
        return ReturnValue;
    }
    Value Evaluator::p_Eval(std::shared_ptr<Scope> CurrentScope,Ref<OpCodeList> OpCodes,IPIndex Offset)
    {
        std::vector<StackFrame> StackFrames = {StackFrame(OpCodeExtractor(OpCodes))};
        StackFrames.back().ExecutionPosition.SetIP(Offset);
        StackFrames.back().StackScope = CurrentScope;
        return p_Eval(std::move(StackFrames));
    }
    Value Evaluator::p_Expand(std::shared_ptr<Scope> ExpandScope,Value ValueToExpand)
    {
        if(ValueToExpand.IsType<List>())
        {
            return  p_Expand(ExpandScope,ValueToExpand.GetType<List>());
        }
        return ValueToExpand;
    }
    Value Evaluator::p_Expand(std::shared_ptr<Scope> ExpandScope,List& ListToExpand)
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
                    return p_Expand(ExpandScope,p_Eval(ExpandScope,*AssociatedMacro.Callable,std::move(Arguments)));
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
                NewList.push_back(p_Expand(ExpandScope,ListToExpand[i].GetType<List>()));
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
        //TODO improve...
        std::string SymbolString = Content.ReadWhile([](char CharToRead)
                {
                    return (CharToRead > ' ' && CharToRead < 0x7f) && CharToRead != '"' && CharToRead != '(' && CharToRead != ')';
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
                    return (NextChar >= '0' && NextChar <= '9') || NextChar == '-';
                });
        ReturnValue = std::stoi(NumberString);
        return ReturnValue;
    }
    List Evaluator::p_ReadList(std::shared_ptr<Scope> AssociatedScope,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue)
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
            ReturnValue.push_back(p_ReadTerm(AssociatedScope,Table,Content,StreamValue));
            p_SkipWhiteSpace(Content);
        }
        return ReturnValue;
    }
    Value Evaluator::p_ReadTerm(std::shared_ptr<Scope> AssociatedScope,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue)
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
            ReturnValue = p_ReadList(AssociatedScope,Table,Content,StreamValue);
        }
        else if(Content.PeekByte() == '"')
        {
            ReturnValue = p_ReadString(Content);
        }
        else if( (NextChar >= '0' && NextChar <= '9') || NextChar == '-')
        {
            ReturnValue = p_ReadInteger(Content);
        }
        else if(auto ReaderIt = Table.Mappings.find(NextChar); ReaderIt != Table.Mappings.end())
        {
            Content.ReadByte();
            ReturnValue = p_Eval(AssociatedScope,ReaderIt->second,{StreamValue});
        }
        else
        {
            ReturnValue = p_ReadSymbol(Content);
        }
        return ReturnValue;
    }
    List Evaluator::p_Read(std::shared_ptr<Scope> AssociatedScope,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue)
    {
        List ReturnValue;
        while(!Content.EOFReached())
        {
            ReturnValue.push_back(p_ReadTerm(AssociatedScope,Table,Content,StreamValue));
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
            if(SymbolString == "&rest")
            {
                ReturnValue |= RestSymbol;
            }
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
        for(auto const& String : {"cond",
                                  "tagbody",
                                  "go",
                                  "set",
                                  "lambda",
                                  "progn",
                                  "quote",
                                  "macro",
                                  "signal-handlers",
                                  "signal",
                                  "unwind",
                                  "unwind-protect",
                                  "bind-dynamic",
                                  "eval",
                                  })
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
                    {"read-term",ReadTerm},
                    {"flatten-1",Flatten_1},
                    {"gensym",GenSym},
                    {"dynamic",Dynamic},
                    {"environment",Environment},
                    {"new-environment",NewEnvironment},
                })
        {
            Function NewBuiltin;
            NewBuiltin.Func = Pair.second;
            m_GlobalScope->SetVariable(p_GetSymbolID(Pair.first),NewBuiltin);
        }
        m_PrimitiveSymbolMax = m_CurrentSymbolID;

        //primitive types
        m_GlobalScope->SetVariable(p_GetSymbolID("list_t"),ClassDefinition(Value::GetTypeTypeID<List>()));
        m_GlobalScope->SetVariable(p_GetSymbolID("int_t"),ClassDefinition(Value::GetTypeTypeID<Int>()));
        m_GlobalScope->SetVariable(p_GetSymbolID("float_t"),ClassDefinition(Value::GetTypeTypeID<Float>()));
        m_GlobalScope->SetVariable(p_GetSymbolID("symbol_t"),ClassDefinition(Value::GetTypeTypeID<Symbol>()));
        m_GlobalScope->SetVariable(p_GetSymbolID("string_t"),ClassDefinition(Value::GetTypeTypeID<String>()));
        
        //list
        AddMethod<List>("append",Append_List);
        AddMethod<List>("index",Index_List);
        AddMethod<List>("len",Len_List);
        //class
        AddMethod<ClassInstance>("index",Index_ClassInstance);

        //scope
        AddMethod<Scope>("index",Index_Environment);
        //comparisons
        AddMethod<String,String>("eq",Eq_String);
        AddMethod<Symbol,Symbol>("eq",Eq_Symbol);
        AddMethod<Int,Int>("eq",Eq_Int);

        //streams
        AddMethod<MBUtility::StreamReader>("eof",Stream_EOF);
        AddMethod<MBUtility::StreamReader>("peek-byte",Stream_PeakByte);
        AddMethod<MBUtility::StreamReader>("read-byte",Stream_ReadByte);
        AddMethod<MBUtility::StreamReader>("skip-whitespace",Stream_SkipWhitespace);

        
        m_GlobalScope->SetVariable(p_GetSymbolID("*READTABLE*"),Value::MakeExternal(ReadTable()));
        //Readtables
        AddMethod<ReadTable,String>("add-reader-character",AddReaderCharacter);
        AddMethod<ReadTable,String>("remove-reader-character",RemoveReaderCharacter);
        AddMethod<ReadTable,String>("add-character-expander",AddCharacterExpander);
        AddMethod<ReadTable,String>("remove-character-expander",RemoveCharacterExpander);
    }
    Evaluator::Evaluator()
    {
        p_InternPrimitiveSymbols();
    }
    void Evaluator::Eval(std::shared_ptr<Scope> CurrentScope,std::string_view Content)
    {
        Ref<OpCodeList> OpCodes = std::make_shared<OpCodeList>();
        Value ReaderValue = Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBUtility::IndeterminateStringStream>(Content)));
        MBUtility::StreamReader& Reader = ReaderValue.GetType<MBUtility::StreamReader>();
        assert(ReaderValue.IsType<MBUtility::StreamReader>());
        ReadTable& Table = CurrentScope->FindVariable(p_GetSymbolID("*READTABLE*")).GetType<ReadTable>();
        while(!Reader.EOFReached())
        {
            IPIndex InstructionToExecute = OpCodes->Size();
            Value NewTerm = p_Expand(CurrentScope,p_ReadTerm(CurrentScope,Table,Reader,ReaderValue));
            p_SkipWhiteSpace(Reader);
            if(NewTerm.IsType<List>())
            {
                OpCodes->Append(NewTerm.GetType<List>());
            }
            else
            {
                List ListToEncode = {std::move(NewTerm)};
                OpCodes->Append(ListToEncode);
            }
            p_Eval(CurrentScope,OpCodes,InstructionToExecute);
        }

    }
    void Evaluator::Eval(std::string_view Content)
    {
        std::shared_ptr<Scope> CurrentScope = CreateDefaultScope();
        Eval(CurrentScope,Content);
    }
    Ref<Scope> Evaluator::CreateDefaultScope()
    {
        Ref<Scope> ReturnValue = std::make_shared<Scope>();
        ReturnValue->SetParentScope(m_GlobalScope);
        ReturnValue->SetVariable(p_GetSymbolID("*READTABLE*"),Value::MakeExternal(
                    ReadTable(m_GlobalScope->FindVariable(p_GetSymbolID("*READTABLE*")).GetType<ReadTable>())));
        return ReturnValue;
    }
}
