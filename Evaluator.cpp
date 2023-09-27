
#include "Evaluator.h"
#include <MBUtility/StreamReader.h>
#include "MBLSP/LSP_Structs.h"
#include "assert.h"
#include <MBParsing/MBParsing.h>

#include  <iostream>
#include <iterator>

#include <MBUtility/MBFiles.h>
#include <MBSystem/MBSystem.h>
#include <MBUnicode/MBUnicode.h>
#include <MBUtility/MBStrings.h>

#include <atomic>

#include "Modules/LSP/LSPModule.h"

#include <MBUtility/FileStreams.h>

#include <iostream>


#include "Threading.h"
namespace MBLisp
{
    //CallContext
    Evaluator& CallContext::GetEvaluator()
    {
        assert(m_AssociatedEvaluator != nullptr);
        return *m_AssociatedEvaluator;
    }
    ExecutionState& CallContext::GetState()
    {
        return *m_CurrentState;
    }
    Ref<Scope> CallContext::GetCurrentScope()
    {
        assert(m_CurrentState != nullptr);
        return m_CurrentState->StackFrames.back().StackScope;
    }
    bool CallContext::IsSetting()
    {
        return m_IsSetting;
    }
    Value const& CallContext::GetSetValue()
    {
        return m_SetValue;
    }
    void CallContext::PauseThread()
    {
        m_ThreadPaused = true;
        m_AssociatedEvaluator->m_ThreadingState.Pause(m_CurrentState->ThreadID);
    }
    bool CallContext::IsMultiThreaded()
    {
        return m_AssociatedEvaluator->m_ThreadingState.MultipleThreadsActive();
    }
    //


    //BEGIN ThreadingState

    


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
        else if(Arguments[1].IsType<Symbol>())
        {
            ReturnValue = Arguments[0].GetType<Symbol>().SymbolLocation.Position < Arguments[1].GetType<Symbol>().SymbolLocation.Position;
        }
        else if(Arguments[1].IsType<Symbol>())
        {
            ReturnValue = Arguments[0].GetType<Symbol>().SymbolLocation.Position < Arguments[1].GetType<Symbol>().SymbolLocation.Position;
        }
        return  ReturnValue;
    }
    Value Evaluator::Less_String BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<String>() < Arguments[1].GetType<String>();
    }
    Value Evaluator::Less_Int BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Int>() < Arguments[1].GetType<Int>();
    }
    Value Evaluator::Less_Symbol BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Symbol>().ID < Arguments[1].GetType<Symbol>().ID;
    }
    Value Evaluator::Sort BUILTIN_ARGLIST
    {
        Ref<List> AssociatedList = Arguments[0].GetRef<List>();
        Value LessFunc = Context.GetCurrentScope()->FindVariable(Context.GetEvaluator().p_GetSymbolID("<"));
        std::sort(AssociatedList->begin(),AssociatedList->end(),[&](Value const& lhs,Value const& rhs)
                {
                    FuncArgVector Args = {lhs,rhs};
                    auto Result = Context.GetEvaluator().Eval(Context.GetCurrentScope() ,LessFunc,std::move(Args));
                    return Result.IsType<bool>() && Result.GetType<bool>();
                });
        return AssociatedList;
    }
    Value Evaluator::CreateList BUILTIN_ARGLIST
    {
        Ref<List> ReturnValue = MakeRef<List>();
        for(auto& Elem : Arguments)
        {
            ReturnValue->push_back(Elem);
        }
        //Value ReturnValue = List(std::move(Arguments));
        return  ReturnValue;
    }
    Value Evaluator::CreateDict BUILTIN_ARGLIST
    {
        return Dict();
    }
    Value Evaluator::Index_Dict BUILTIN_ARGLIST
    {
        if(!Context.IsSetting())
        {
            return Arguments[0].GetType<Dict>()[Arguments[1]];
        }
        else
        {
            Arguments[0].GetType<Dict>()[Arguments[1]] = Context.GetSetValue();
            return Context.GetSetValue();
        }
        return Value(Null());
    }
    Value Evaluator::Keys_Dict BUILTIN_ARGLIST
    {
        List ReturnValue;
        for(auto const& Pairs : Arguments[0].GetType<Dict>())
        {
            ReturnValue.push_back(Pairs.first);
        }
        return ReturnValue;
    }
    Value Evaluator::In_Dict BUILTIN_ARGLIST
    {
        return  Arguments[1].GetType<Dict>().find(Arguments[0]) != Arguments[1].GetType<Dict>().end();
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
        else if(ValueToPrint.IsType<Null>())
        {
            std::cout<<"null";
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
        else if(ValueToPrint.IsType<Dict>())
        {
            std::cout<<"{";
            auto const& DictToPrint = ValueToPrint.GetType<Dict>();
            int i = 0;
            for(auto const& Element : DictToPrint)
            {
                Print(AssociatedEvaluator,Element.first);
                std::cout<<": ";
                Print(AssociatedEvaluator,Element.second);
                if(i + 1 < DictToPrint.size())
                {
                    std::cout<<" ";
                }
                i  += 1;
            }
            std::cout<< "}";
        }
    }
    Value Evaluator::Print BUILTIN_ARGLIST
    {
        Value ReturnValue;
        for(auto const& Argument : Arguments)
        {
            Print(Context.GetEvaluator(),Argument);
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
        std::sort(TemporaryClass.SlotDefinitions.begin(), TemporaryClass.SlotDefinitions.end());
        if (Parents.size() == 0)
        {
            TemporaryClass.Types.push_back(1u << UserClassBit);
        }
        Parents.push_back(&TemporaryClass);
        p_MergeClasses(Parents,NewClass);

        if(Arguments.size() == 3)
        {
            NewClass.Constructor = MakeRef<Value>(Arguments[2]);   
        }
        
        Ref<FunctionDefinition> SlotInitializers = MakeRef<FunctionDefinition>();
        SlotInitializers->Arguments = {Context.GetEvaluator().GetSymbolID("INIT")};
        SlotInitializers->Instructions = MakeRef<OpCodeList>(OpCodeList(Context.GetEvaluator().GetSymbolID("INIT"),Context.GetEvaluator().GetSymbolID("index"),
                NewClass.SlotDefinitions));
        NewClass.SlotInitializers = SlotInitializers;
        i___CurrentClassID++;
        NewClass.Types.push_back(i___CurrentClassID);
        NewClass.ID = i___CurrentClassID;
        return NewClass;
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
    Value Evaluator::Expand BUILTIN_ARGLIST
    {
        return Context.GetEvaluator().p_Expand(Context.GetCurrentScope(),Arguments[0]);
    }
    Value Evaluator::Stream_ReadTerm BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
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
        ReadTable& Table = Context.GetCurrentScope()->FindVariable(Context.GetEvaluator().GetSymbolID("*READTABLE*")).GetType<ReadTable>();
        SymbolID URI = Context.GetEvaluator().p_GetSymbolID(Context.GetCurrentScope()->FindVariable(Context.GetEvaluator().p_GetSymbolID("load-filepath")).GetType<String>());
        //TODO think through this functionality again, how needed is it for ":" expander, and could it be implemented in a more clean way
        Ref<Scope> NewScope = Context.GetState().StackFrames.front().StackScope;
        ReturnValue = Context.GetEvaluator().p_ReadTerm(NewScope,URI,Table,Reader,Arguments[0]);
        return ReturnValue;
    }
    Value Evaluator::Stream_ReadString BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        return Context.GetEvaluator().p_ReadString(Arguments[0].GetType<MBUtility::StreamReader>());
    }
    Value Evaluator::Stream_ReadNumber BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        Value ReturnValue;
        std::string NumberString = Arguments[0].GetType<MBUtility::StreamReader>().ReadWhile([]
                (char NextChar)
                {
                    return ((NextChar >= '0' && NextChar <= '9') || NextChar == '-');
                });
        return std::stoi(NumberString);
    }
    Value Evaluator::Stream_ReadBytes BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        int BytesToRead = Arguments[1].GetType<Int>();
        if(BytesToRead < 0)
        {
            throw std::runtime_error("Can only read a positive amount of bytes");   
        }
        int ReadBytes = 0;
        std::string ReturnValue = Arguments[0].GetType<MBUtility::StreamReader>().ReadWhile([&](char NextByte)
                {
                    bool ReturnValue = ReadBytes < BytesToRead;
                    ReadBytes += 1;
                    return ReturnValue;
                });
        return ReturnValue;
    }
    Value Evaluator::Stream_ReadLine BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        auto& Reader = Arguments[0].GetType<MBUtility::StreamReader>();
        std::string ReturnValue = Reader.ReadWhile([]
                (char NextByte)
                {
                    return NextByte != '\n';
                });
        if(!Reader.EOFReached())
        {
            Reader.ReadByte();
        }
        if(ReturnValue.size() > 0 && ReturnValue.back() == '\r')
        {
            ReturnValue.resize(ReturnValue.size()-1);
        }
        return ReturnValue;
    }
    Value Evaluator::Stream_Position BUILTIN_ARGLIST
    {
        auto const& Reader = Arguments[0].GetType<MBUtility::StreamReader>();
        return  Int(Reader.Position());
    }
    Value Evaluator::Stream_ReadUntil BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        auto& Reader = Arguments[0].GetType<MBUtility::StreamReader>();
        String const& SearchedString = Arguments[1].GetType<String>();
        std::string ReturnValue;
        size_t EarliestPossibleOffset = 0;
        //kinda trivial implementation, might be 
        //possible to optimise for longer string, but most likely not so necccessary
        Reader.DoWhile([&]
                (char NextByte)
                {
                    bool Result = true;
                    if(ReturnValue.find(SearchedString,EarliestPossibleOffset) != std::string::npos)
                    {
                        Result = false;
                    }
                    else
                    {
                        if(ReturnValue.size() >= SearchedString.size())
                        {
                            EarliestPossibleOffset = (ReturnValue.size()-SearchedString.size())+1;
                        }
                    }
                    return Result;
                },
                [&](char CurrentByte){ReturnValue += CurrentByte;}
                );
        if(EarliestPossibleOffset ==  (ReturnValue.size() - SearchedString.size()))
        {
            ReturnValue.resize(ReturnValue.size()-SearchedString.size());
        }
        else 
        {
            throw std::runtime_error("Error in read-until: searched string \""+SearchedString+"\" was not found");
        }
        return ReturnValue;
    }
    Value Evaluator::Stream_EOF BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        return Arguments[0].GetType<MBUtility::StreamReader>().EOFReached();
    }
    Value Evaluator::Stream_PeakByte BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        return String(1,Arguments[0].GetType<MBUtility::StreamReader>().PeekByte());
    }
    Value Evaluator::Stream_ReadByte BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        return String(1,Arguments[0].GetType<MBUtility::StreamReader>().ReadByte());
    }
    Value Evaluator::Stream_SkipWhitespace BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        Context.GetEvaluator().p_SkipWhiteSpace(Arguments[0].GetType<MBUtility::StreamReader>());
        return false;
    }
    Value Evaluator::Index_List BUILTIN_ARGLIST
    {
        assert(Arguments.size() >= 2 && Arguments[0].IsType<List>() && Arguments[0].GetRef<List>() != nullptr);
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
        assert(!AssociatedValue.IsType<List>() || AssociatedValue.GetRef<List>() != nullptr);
        if(Context.IsSetting())
        {
            AssociatedValue = Context.GetSetValue();
        }
        assert(!AssociatedValue.IsType<List>() || AssociatedValue.GetRef<List>() != nullptr);
        return AssociatedValue;
    }
    Value Evaluator::Append_List BUILTIN_ARGLIST
    {
        assert(!Arguments[0].IsType<List>() || Arguments[0].GetRef<List>() != nullptr);
        Ref<List> AssociatdList = Arguments[0].GetRef<List>();
        for(int i = 1; i < Arguments.size();i++)
        {
            AssociatdList->push_back(Arguments[i]);
            assert(!Arguments[i].IsType<List>() || Arguments[i].GetRef<List>() != nullptr);
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
                    ReturnValue.push_back(SubArgument);
                }
            }
            else
            {
                ReturnValue.push_back(Argument);
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
    Value Evaluator::Eq_Bool BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<bool>() == Arguments[1].GetType<bool>();
    }
    Value Evaluator::Eq_Type BUILTIN_ARGLIST
    {
        return (Arguments[0].GetType<ClassDefinition>().ID == Arguments[1].GetType<ClassDefinition>().ID)
            || Arguments[0].GetType<ClassDefinition>().ID == 0 || Arguments[1].GetType<ClassDefinition>().ID == 0;
    }
    Value Evaluator::Eq_Any BUILTIN_ARGLIST
    {
        return false;
    }
    Value Evaluator::Eq_Null BUILTIN_ARGLIST
    {
        return true;
    }
    Value Evaluator::Minus_Int BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Int>()-Arguments[1].GetType<Int>();
    }
    Value Evaluator::Split_String BUILTIN_ARGLIST
    {
        auto Result = MBUtility::Split(Arguments[0].GetType<String>(),Arguments[1].GetType<String>());
        List ReturnValue;
        for(auto const& String : Result)
        {
            ReturnValue.push_back(String);
        }
        return ReturnValue;
    }
    Value Evaluator::Len_String BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<String>().size();
    }
    Value Evaluator::In_String BUILTIN_ARGLIST
    {
        return Arguments[1].GetType<String>().find(Arguments[0].GetType<String>()) != std::string::npos;
    }
    Value Evaluator::In_Environment BUILTIN_ARGLIST
    {
        Scope& Envir = Arguments[1].GetType<Scope>();
        Symbol& SymbolToCheck = Arguments[0].GetType<Symbol>();
        return Envir.TryGet(SymbolToCheck.ID) != nullptr;
    }
    Value Evaluator::Str_Symbol BUILTIN_ARGLIST
    {
        return Context.GetEvaluator().GetSymbolString(Arguments[0].GetType<Symbol>().ID);
    }
    Value Evaluator::Str_Int BUILTIN_ARGLIST
    {
        return std::to_string(Arguments[0].GetType<Int>());
    }
    Value Evaluator::Int_Str BUILTIN_ARGLIST
    {
        return  std::stoi(Arguments[0].GetType<String>());
    }
    Value Evaluator::IndexOf_StrStr BUILTIN_ARGLIST
    {
        auto Result = Arguments[1].GetType<String>().find(Arguments[0].GetType<String>());
        if(Result == String::npos)
        {
            Result = -1;   
        }
        return Int(Result);
    }
    Value Evaluator::Substr BUILTIN_ARGLIST
    {
        String const& OriginalString = Arguments[0].GetType<String>();
        int Offset = Arguments[1].GetType<Int>();
        auto Length = String::npos;
        if(Arguments.size() > 2)
        {
            Length = Arguments[2].GetType<Int>();
        }
        return OriginalString.substr(Offset,Length);
    }
    Value Evaluator::Str_Bool BUILTIN_ARGLIST
    {
        if(Arguments[0].GetType<bool>())
        {
            return "true";   
        }
        else 
        {
            return "false";   
        }
    }
    Value Evaluator::Str_Null BUILTIN_ARGLIST
    {
        return "null";
    }
    Value Evaluator::Str_Float BUILTIN_ARGLIST
    {
        return std::to_string(Arguments[0].GetType<Float>());
    }
    Value Evaluator::Symbol_String BUILTIN_ARGLIST
    {
        return Symbol(Context.GetEvaluator().GetSymbolID(Arguments[0].GetType<String>()));
    }
    Value Evaluator::Symbol_SymbolInt BUILTIN_ARGLIST
    {
        Symbol ReturnValue;
        ReturnValue.ID = Arguments[0].GetType<Symbol>().ID;
        ReturnValue.SymbolLocation.Position = Arguments[1].GetType<Int>();
        return ReturnValue;
    }
    Value Evaluator::GenSym BUILTIN_ARGLIST
    {
        return Symbol(Context.GetEvaluator().GenerateSymbol());
    }
    Value Evaluator::Environment BUILTIN_ARGLIST
    {
        return Context.GetCurrentScope();
    }
    Value Evaluator::NewEnvironment BUILTIN_ARGLIST
    {
        Ref<Scope> NewScope = MakeRef<Scope>();
        NewScope->SetShadowingParent(Context.GetEvaluator().m_GlobalScope);
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
        if(Context.IsSetting())
        {
            *ReturnValue = Context.GetSetValue();
        }
        return *ReturnValue;
    }
    Value Evaluator::SetVar_Environment BUILTIN_ARGLIST
    {
        Arguments[0].GetType<Scope>().SetVariable(Arguments[1].GetType<Symbol>().ID,Arguments[2]);
        return Value();
    }
    Value Evaluator::Shadow_Environment BUILTIN_ARGLIST
    {
        Scope& AssociatedScope = Arguments[0].GetType<Scope>();
        SymbolID SymbolIndex = Arguments[1].GetType<Symbol>().ID;
        Value* ReturnValue = AssociatedScope.TryGet(SymbolIndex);
        if(ReturnValue == nullptr)
        {
            AssociatedScope.OverrideVariable(SymbolIndex,Value());
            ReturnValue = AssociatedScope.TryGet(SymbolIndex);
            assert(ReturnValue != nullptr);
        }
        if(Context.IsSetting())
        {
            *ReturnValue = Context.GetSetValue();
        }
        return *ReturnValue;
    }
    Value Evaluator::SetParent_Environment BUILTIN_ARGLIST
    {
        Scope& ScopeToModify = Arguments[0].GetType<Scope>();
        ScopeToModify.SetParentScope(Arguments[1].GetRef<Scope>());
        return Value();
    }
    Value Evaluator::AddParent_Environment BUILTIN_ARGLIST
    {
        Scope& ScopeToModify = Arguments[0].GetType<Scope>();
        ScopeToModify.AddParentScope(Arguments[1].GetRef<Scope>());
        return Value();
    }
    Value Evaluator::Clear_Environment BUILTIN_ARGLIST
    {
        Arguments[0].GetType<Scope>().Clear();
        return true;
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
        TableToModify.Mappings[Arguments[1].GetType<String>()[0]] = Arguments[2];
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
            throw std::runtime_error("add-character-expander requires exactly 3 arguments");   
        }
        if (!Arguments[1].IsType<String>() || Arguments[1].GetType<String>().size() != 1)
        {
            throw std::runtime_error("second argument to add-character-expander must be string of size exactly 1");
        }
        Value ReturnValue = false;
        ReadTable& TableToModify = Arguments[0].GetType<ReadTable>();
        TableToModify.ExpandMappings.push_back(std::make_pair(Arguments[1].GetType<String>()[0],Arguments[2]));
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
        NewSymbol |= GeneratedSymbol;
        //std::string SymbolString = "g:"+std::to_string(NewSymbol);
        //while(m_InternedSymbols.find(SymbolString) != m_InternedSymbols.end())
        //{
        //    SymbolString += "_";
        //}
        //m_SymbolToString[NewSymbol] = SymbolString;
        //m_InternedSymbols[SymbolString] = NewSymbol;
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
        if(Context.IsSetting())
        {
            SymbolIt->second = Context.GetSetValue();
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
    Value Evaluator::Eval(Ref<Scope> CurrentScope,Value Callable,FuncArgVector Arguments)
    {
        std::vector<StackFrame> CurrentCallStack = {StackFrame(OpCodeExtractor())};
        CurrentCallStack.back().StackScope = CurrentScope;
        ExecutionState NewState;
        NewState.StackFrames = std::move(CurrentCallStack);
        p_Invoke(Callable,Arguments,NewState);
        return p_Eval(NewState);
    }
    void Evaluator::p_EmitSignal(ExecutionState& CurrentState,Value SignalValue,bool ForceUnwind)
    {
        //the signal form returns a value in the current frame, which
        int CurrentFrameIndex = CurrentState.StackFrames.size()-1;
        auto& CurrentFrame = CurrentState.StackFrames.back();
        bool SignalFound = false;
        auto& StackFrames = CurrentState.StackFrames;
        for(int i = CurrentFrameIndex; i >= 0; i--)
        {
            if(CurrentFrame.SignalFrameIndex != -1 && i == CurrentFrameIndex)
            {
                continue;   
            }
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
                    if(ForceUnwind)
                    {
                        CurrentState.UnwindForced = true;
                    }
                    break;
                }
            }
        }
        if(!SignalFound)
        {
            CurrentFrame.ArgumentStack.push_back(false);
            if(ForceUnwind)
            {
                UncaughtSignal Exception;
                Exception.ThrownValue = SignalValue;
                throw Exception;
            }
        }
    }
    void Evaluator::p_Invoke(Value& ObjectToCall,FuncArgVector& Arguments,ExecutionState& CurrentState,bool Setting)
    {
        auto& CurrentCallStack = CurrentState.StackFrames;
        if(ObjectToCall.IsType<Function>())
        {
            BuiltinFuncType AssociatedFunc = ObjectToCall.GetType<Function>().Func;
            assert(AssociatedFunc != nullptr);

            for(auto& Arg : Arguments)
            {
                assert(!Arg.IsType<List>() || Arg.GetRef<List>() != nullptr);
            }
            try
            {
                CurrentState.CurrentCallContext.m_IsSetting = Setting;
                CurrentCallStack.back().ArgumentStack.push_back(AssociatedFunc(CurrentState.CurrentCallContext,Arguments));
                if(CurrentState.CurrentCallContext.m_ThreadPaused)
                {
                    m_ThreadingState.Resume(CurrentState.ThreadID);
                }
            }
            catch(std::exception const& e)
            {
                p_EmitSignal(CurrentState,e.what(),true);
            }
            //for(auto& Arg : Arguments)
            //{
            //    assert(!Arg.IsType<List>() || Arg.GetRef<List>() != nullptr);
            //}
            assert(CurrentCallStack.back().ArgumentStack.size() == 0 || !CurrentCallStack.back().ArgumentStack.back().IsType<List>() || CurrentCallStack.back().ArgumentStack.back().GetRef<List>() != nullptr);
        }
        else if(ObjectToCall.IsType<Lambda>())
        {
            Lambda& AssociatedLambda = ObjectToCall.GetType<Lambda>();
            if(Arguments.size() < AssociatedLambda.Definition->Arguments.size())
            {
                //throw std::runtime_error("To few arguments for function call with function \""+AssociatedLambda.Name+"\"");
                p_EmitSignal(CurrentState,"To few arguments for function call with function \""+GetSymbolString(AssociatedLambda.Name.ID)+"\"",true);
                return;
            }
            if(AssociatedLambda.Definition->RestParameter == 0)
            {
                if(Arguments.size() > AssociatedLambda.Definition->Arguments.size())
                {
                    //throw std::runtime_error("To many arguments for function call \""+AssociatedLambda.Name+"\"");
                    p_EmitSignal(CurrentState,"To many arguments for function call \""+GetSymbolString(AssociatedLambda.Name.ID)+"\"",true);
                    return;
                }
            }
            StackFrame NewStackFrame(OpCodeExtractor(AssociatedLambda.Definition->Instructions));
            NewStackFrame.StackScope = MakeRef<Scope>();
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
            if(AssociatedLambda.Definition->EnvirParameter != 0)
            {
                NewStackFrame.StackScope->OverrideVariable(AssociatedLambda.Definition->EnvirParameter,CurrentCallStack.back().StackScope);
            }
            CurrentCallStack.push_back(std::move(NewStackFrame));
        }
        else if(ObjectToCall.IsType<ClassDefinition>())
        {
            Ref<ClassInstance> NewInstance = MakeRef<ClassInstance>();
            NewInstance->AssociatedClass = ObjectToCall.GetRef<ClassDefinition>();

            for(auto const& Slot : NewInstance->AssociatedClass->SlotDefinitions)
            {
                NewInstance->Slots.push_back(std::make_pair(Slot.Symbol,Value()));
            }
            StackFrame NewStackFrame(OpCodeExtractor(NewInstance->AssociatedClass->SlotInitializers->Instructions));
            NewStackFrame.StackScope = MakeRef<Scope>();
            NewStackFrame.StackScope->SetParentScope(m_GlobalScope);
            Value NewValue = NewInstance;
            NewStackFrame.StackScope->OverrideVariable(p_GetSymbolID("INIT"),NewValue);
            if(NewInstance->AssociatedClass->Constructor != nullptr)
            {
                FuncArgVector Args = {NewValue};
                for(auto& Arg : Arguments)
                {
                    Args.push_back(Arg);
                }
                p_Invoke(*NewInstance->AssociatedClass->Constructor,Args,CurrentState);
                CurrentCallStack.back().PopExtra = 1;
            }
            CurrentCallStack.push_back(std::move(NewStackFrame));
        }
        else if(ObjectToCall.IsType<GenericFunction>())
        {
            GenericFunction& GenericToInvoke = ObjectToCall.GetType<GenericFunction>();
            Value* Callable = GenericToInvoke.GetMethod(Arguments.data(),Arguments.data()+Arguments.size());
            if(Callable == nullptr)
            {
                //throw std::runtime_error("No method associated with the argument list for generic \""+GenericToInvoke.Name+"\"");   
                p_EmitSignal(CurrentState,"No method associated with the argument list for generic \""+GetSymbolString(GenericToInvoke.Name.ID)+"\"",true);
                return;
            }
            p_Invoke(*Callable,Arguments,CurrentState,Setting);
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
        CurrentState.CurrentCallContext.m_AssociatedEvaluator = this;
        CurrentState.CurrentCallContext.m_CurrentState = &CurrentState;
        auto& StackFrames = CurrentState.StackFrames;
        while(StackFrames.size() != 0)
        {
            if(m_ThreadingState.MultipleThreadsActive())
            {
                m_ThreadingState.WaitForTurn(CurrentState.ThreadID);
            }
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
                Value* VarPointer = CurrentFrame.StackScope->TryGet(PushCode.ID);
                if(VarPointer == nullptr)
                {
                    p_EmitSignal(CurrentState,"Error finding variable with name \""+GetSymbolString(PushCode.ID)+"\"",true);
                    continue;
                }
                //assert(!VarPointer->IsType<List>() || VarPointer->GetRef<List>() != nullptr);
                if(!VarPointer->IsType<DynamicVariable>())
                {
                    CurrentFrame.ArgumentStack.push_back(*VarPointer);
                }
                else
                {
                    DynamicVariable const& DynamicToPush = VarPointer->GetType<DynamicVariable>();
                    Value const* VarToPushPointer = &DynamicToPush.DefaultValue;
                    if(auto DynIt = CurrentState.DynamicBindings.find(DynamicToPush.ID); DynIt != CurrentState.DynamicBindings.end())
                    {
                        if(DynIt->second.size() > 0)
                        {
                            VarToPushPointer = &DynIt->second.back();
                        }
                    }
                    CurrentFrame.ArgumentStack.push_back(*VarToPushPointer);
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
                Value ValueToReturn;
                if(GotoCode.ReturnTop)
                {
                    assert(CurrentFrame.ArgumentStack.size() != 0);
                    ValueToReturn = CurrentFrame.ArgumentStack.back();
                }
                if(GotoCode.NewStackSize != -1)
                {
                    assert(CurrentFrame.ArgumentStack.size() >= GotoCode.NewStackSize);
                    CurrentFrame.ArgumentStack.resize(GotoCode.NewStackSize);
                }
                if(GotoCode.ReturnTop)
                {
                    CurrentFrame.ArgumentStack.push_back(std::move(ValueToReturn));
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
                FuncArgVector Arguments;
                for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                {
                    Arguments.push_back(std::move(CurrentFrame.ArgumentStack[CurrentFrame.ArgumentStack.size()-CallFuncCode.ArgumentCount+i]));
                }
                CurrentFrame.ArgumentStack.resize(CurrentFrame.ArgumentStack.size()-CallFuncCode.ArgumentCount);
                //for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                //{
                //    CurrentFrame.ArgumentStack.pop_back();
                //}
                p_Invoke(FunctionToCall,Arguments,CurrentState,CallFuncCode.Setting);
            }
            else if(CurrentCode.IsType<OpCode_Macro>())
            {
                assert(CurrentFrame.ArgumentStack.size() >= 1);
                Value ValueToConvert = std::move(CurrentFrame.ArgumentStack.back());
                CurrentFrame.ArgumentStack.pop_back();
                Macro MacroToCreate;
                MacroToCreate.Callable = MakeRef<Value>(std::move(ValueToConvert));
                CurrentFrame.ArgumentStack.push_back(std::move(MacroToCreate));
            }
            else if(CurrentCode.IsType<OpCode_PreSet>())
            {
                assert(CurrentFrame.ArgumentStack.size() >= 1);
                CurrentState.CurrentCallContext.m_SetValue = std::move(CurrentFrame.ArgumentStack.back());
                CurrentFrame.ArgumentStack.pop_back();
            }
            else if(CurrentCode.IsType<OpCode_Set>())
            {
                //first value of stack is symbol, second is value
                assert(CurrentFrame.ArgumentStack.size() >= 2);
                Value SymbolToAssign = std::move(*(CurrentFrame.ArgumentStack.end()-1));
                Value AssignedValue = std::move(*(CurrentFrame.ArgumentStack.end()-2));
                CurrentFrame.ArgumentStack.pop_back();
                CurrentFrame.ArgumentStack.pop_back();
                assert(!SymbolToAssign.IsType<List>() || SymbolToAssign.GetRef<List>() != nullptr);
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
                        It->second.back() = AssignedValue;
                    }
                    else
                    {
                        AssociatedVariable.DefaultValue = AssignedValue;
                    }
                }
                else
                {
                    SymbolToAssign = AssignedValue;   
                }
                assert(!SymbolToAssign.IsType<List>() || SymbolToAssign.GetRef<List>() != nullptr);
                assert(!AssignedValue.IsType<List>() || AssignedValue.GetRef<List>() != nullptr);
                CurrentFrame.ArgumentStack.push_back(AssignedValue);
            }
            else if(CurrentCode.IsType<OpCode_Signal>())
            {
                //should probably not allow signals when unwinding stack...
                assert(CurrentFrame.ArgumentStack.size() > 0);
                Value SignalValue = std::move(CurrentFrame.ArgumentStack.back());
                CurrentFrame.ArgumentStack.pop_back();
                bool IsForced = false;
                if(CurrentCode.GetType<OpCode_Signal>().HasForced)
                {
                    Value IsForcedValue = std::move(CurrentFrame.ArgumentStack.back());
                    CurrentFrame.ArgumentStack.pop_back();
                    if(IsForcedValue.IsType<bool>() && IsForcedValue.GetType<bool>())
                    {
                        IsForced = true;
                    }
                }
                p_EmitSignal(CurrentState,SignalValue,IsForced);
            }
            else if(CurrentCode.IsType<OpCode_SignalHandler_Done>())
            {
                if(!CurrentState.UnwindForced)
                {
                    CurrentFrame.ExecutionPosition.SetEnd();
                }
                else
                {
                    assert(CurrentFrame.SignalFrameIndex != -1);
                    CurrentState.UnwindForced = false;
                    CurrentState.UnwindingStack = true;
                    CurrentState.FrameTarget = CurrentFrame.SignalFrameIndex;
                    StackFrames[CurrentFrame.SignalFrameIndex].ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_SignalHandler_Done>().HandlersEnd);
                    StackFrames[CurrentFrame.SignalFrameIndex].ArgumentStack.resize(CurrentCode.GetType<OpCode_SignalHandler_Done>().NewStackSize+1);
                }
            } 
            else if(CurrentCode.IsType<OpCode_AddSignalHandlers>())
            {
                OpCode_AddSignalHandlers const& AddSignalsCode = CurrentCode.GetType<OpCode_AddSignalHandlers>();
                assert(CurrentFrame.ArgumentStack.size() >= AddSignalsCode.Handlers.size());
                int StackOffset = CurrentFrame.ArgumentStack.size()-AddSignalsCode.Handlers.size();
                std::vector<SignalHandler> NewHandlers;
                for(auto const& NewHandler : AddSignalsCode.Handlers)
                {
                    Value const& TypeValue = CurrentFrame.ArgumentStack[StackOffset];
                    if(!TypeValue.IsType<ClassDefinition>())
                    {
                        //throw std::runtime_error("signal handler value specifier evaluated to non class type");
                        p_EmitSignal(CurrentState,"signal handler value specifier evaluated to non class type",true);
                        continue;
                    }
                    SignalHandler NewSignalHandler;
                    NewSignalHandler.HandledType = TypeValue.GetType<ClassDefinition>().ID;
                    NewSignalHandler.BoundValue = NewHandler.BoundVariable;
                    NewSignalHandler.SignalBegin = NewHandler.HandlerBegin;
                    NewHandlers.push_back(std::move(NewSignalHandler));
                    StackOffset++;
                }
                std::reverse(NewHandlers.begin(),NewHandlers.end());
                CurrentFrame.ActiveSignalHandlers.insert(CurrentFrame.ActiveSignalHandlers.end(),NewHandlers.begin(),NewHandlers.end());
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
                CurrentState.UnwindForced = false;
                CurrentState.UnwindingStack = true;
                CurrentState.FrameTarget = CurrentFrame.SignalFrameIndex;
                StackFrames[CurrentFrame.SignalFrameIndex].ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_Unwind>().HandlersEnd);
                StackFrames[CurrentFrame.SignalFrameIndex].ArgumentStack.resize(CurrentCode.GetType<OpCode_Unwind>().NewStackSize+1);
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
                        //throw std::runtime_error("first part of binding triplet has to be a scope");
                        p_EmitSignal(CurrentState,"first part of binding triplet has to be a scope",true);
                        continue;
                    }   
                    Scope& ScopeToModify = Arguments[i].GetType<Scope>();
                    if(!Arguments[i+1].IsType<Symbol>())
                    {
                        //throw std::runtime_error("second part of binding triplet has to be a symbol");
                        p_EmitSignal(CurrentState,"second part of binding triplet has to be a symbol",true);
                        continue;
                    }   
                    SymbolID IDToModify = Arguments[i+1].GetType<Symbol>().ID;
                    Value* VariableToInspect = ScopeToModify.TryGet(IDToModify);
                    if(VariableToInspect == nullptr)
                    {
                        //throw std::runtime_error("couldn't find dynamic variable  in scope");
                        p_EmitSignal(CurrentState,"couldn't find dynamic variable  in scope",true);
                        continue;
                    }
                    if(!VariableToInspect->IsType<DynamicVariable>())
                    {
                        //throw std::runtime_error("variable was not a dynamic variable");
                        p_EmitSignal(CurrentState,"variable was not a dynamic variable",true);
                        continue;
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
                        //throw std::runtime_error("argument to eval not of type environment");
                        p_EmitSignal(CurrentState,"argument to eval not of type environment",true);
                        continue;
                    }
                    ScopeToUse = ScopeValue.GetRef<Scope>();
                }
                Value ValueToEvaluate = CurrentFrame.ArgumentStack.back();
                CurrentFrame.ArgumentStack.pop_back();
                ValueToEvaluate = p_Expand(ScopeToUse,std::move(ValueToEvaluate));
                Ref<OpCodeList> NewOpcodes = MakeRef<OpCodeList>(ValueToEvaluate);
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
        if(TypeValue == 0) 
        {
            return true;
        }
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
    Value Evaluator::p_Eval(Ref<Scope> CurrentScope,Ref<OpCodeList> OpCodes,IPIndex Offset)
    {
        std::vector<StackFrame> StackFrames = {StackFrame(OpCodeExtractor(OpCodes))};
        StackFrames.back().ExecutionPosition.SetIP(Offset);
        StackFrames.back().StackScope = CurrentScope;
        return p_Eval(std::move(StackFrames));
    }
    Value Evaluator::p_Expand(Ref<Scope> ExpandScope,Value ValueToExpand)
    {
        if(ValueToExpand.IsType<List>())
        {
            return  p_Expand(ExpandScope,ValueToExpand.GetType<List>());
        }
        return ValueToExpand;
    }
    Value Evaluator::p_Expand(Ref<Scope> ExpandScope,List const& ListToExpand)
    {
        Value ReturnValue;
        if(ListToExpand.size() == 0)
        {
            return ListToExpand;
        }
        if(ListToExpand[0].IsType<Symbol>())
        {
            Symbol const& HeadSymbol = ListToExpand[0].GetType<Symbol>();
            if(!p_SymbolIsPrimitive(HeadSymbol.ID))
            {
                if(auto  VarIt = ExpandScope->TryGet(HeadSymbol.ID); VarIt != nullptr && VarIt->IsType<Macro>())
                {
                    Macro& AssociatedMacro = VarIt->GetType<Macro>();
                    FuncArgVector Arguments;
                    for(int i = 1; i < ListToExpand.size();i++)
                    {
                        Arguments.push_back(ListToExpand[i]);
                    }
                    return p_Expand(ExpandScope,Eval(ExpandScope,*AssociatedMacro.Callable,std::move(Arguments)));
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
    int h_EscapeCount(std::string_view Content)
    {
        int EscapeCount = 0;
        for(int i = int(Content.size())-1;i >= 0;i--)
        {
            if(Content[i] == '\\')
            {
                EscapeCount += 1;   
            }
            else
            {
                break;   
            }
        }
        return EscapeCount;
    }
    String Evaluator::p_ReadString(MBUtility::StreamReader& Content)
    {
        String ReturnValue;
        p_SkipWhiteSpace(Content);
        Content.ReadByte();
        bool StringFinished = false;
        while(!StringFinished)
        {
            ReturnValue += Content.ReadWhile([](char NextChar)
                    {
                        return NextChar != '"';
                    });
            if(Content.EOFReached())
            {
                throw std::runtime_error("missing \" for string literal");
            }
            if(int EscapeCount = h_EscapeCount(ReturnValue); EscapeCount != 0)
            {
                ReturnValue.resize(ReturnValue.size() - (EscapeCount-(EscapeCount/2)));
                if(EscapeCount % 2 != 0)
                {
                    ReturnValue += Content.ReadByte();
                }
                else
                {
                    Content.ReadByte();
                    break;
                }
            }
            else
            {
                Content.ReadByte();
                break;
            }
        }
        return ReturnValue;
    }
    Value Evaluator::p_ReadSymbol(Ref<Scope> ReadScope,SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content)
    {
        Value ReturnValue;
        size_t Position = Content.Position();
        //TODO improve...
        std::string SymbolString = Content.ReadWhile([](char CharToRead)
                {
                    return (CharToRead > ' ' && CharToRead < 0x7f) && CharToRead != '"' && CharToRead != '(' && CharToRead != ')';
                });
        if (SymbolString == "")
        {
            InvalidCharacter Exception;
            Exception.Message = "Error reading symbol: first character invalid: ";
            Exception.Message += Content.PeekByte();
            throw Exception;
        }
        else if(SymbolString == "true")
        {
            ReturnValue = true;
        }
        else if(SymbolString == "false")
        {
            ReturnValue = false;
        }
        else if(SymbolString == "null")
        {
            ReturnValue = Null();
        }
        else
        {
            Symbol NewSymbol = Symbol(p_GetSymbolID(SymbolString));
            NewSymbol.SymbolLocation.Position = Position;
            NewSymbol.SymbolLocation.URI = URI;
            ReturnValue = NewSymbol;
            for(auto const& ExpandPairs : Table.ExpandMappings)
            {
                if(SymbolString.find(ExpandPairs.first) != std::string::npos)
                {
                    ReturnValue = Eval(ReadScope,ExpandPairs.second,{ReturnValue});
                    break;
                }
            }
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
    List Evaluator::p_ReadList(Ref<Scope> AssociatedScope,SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue)
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
            ReturnValue.push_back(p_ReadTerm(AssociatedScope,URI,Table,Content,StreamValue));
            p_SkipWhiteSpace(Content);
        }
        return ReturnValue;
    }
    Value Evaluator::p_ReadTerm(Ref<Scope> AssociatedScope,SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue)
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
            ReturnValue = p_ReadList(AssociatedScope,URI,Table,Content,StreamValue);
            assert(ReturnValue.GetRef<List>() != nullptr);
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
            ReturnValue = Eval(AssociatedScope,ReaderIt->second,{StreamValue});
        }
        else
        {
            ReturnValue = p_ReadSymbol(AssociatedScope,URI,Table,Content);
        }
        return ReturnValue;
    }
    List Evaluator::p_Read(Ref<Scope> AssociatedScope, SymbolID URI,ReadTable const& Table,MBUtility::StreamReader& Content,Value& StreamValue)
    {
        List ReturnValue;
        while(!Content.EOFReached())
        {
            ReturnValue.push_back(p_ReadTerm(AssociatedScope,URI,Table,Content,StreamValue));
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
            else if(SymbolString == "&envir")
            {
                ReturnValue |= EnvirSymbol;
            }
            m_SymbolToString[ReturnValue] = SymbolString;
        }
        return ReturnValue;
    }
    std::string Evaluator::GetSymbolString(SymbolID SymbolToConvert)
    {
        if(SymbolToConvert & GeneratedSymbol)
        {
            return "g:"+std::to_string(SymbolToConvert  & ~GeneratedSymbol);
        }
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
    Value Evaluator::Type BUILTIN_ARGLIST
    {
        if(Arguments.size() != 1)
        {
            throw std::runtime_error("type requires exactly 1 argument: value to return the type of");   
        }
        Value ReturnValue;
        if(!(Arguments[0].IsType<ClassDefinition>() && Arguments[0].IsType<ClassInstance>()))
        {
            if(Arguments[0].IsBuiltin())
            {
                return Context.GetEvaluator().m_BuiltinTypeDefinitions[Arguments[0].GetTypeID()];
            }
            else
            {
                return ClassDefinition(Arguments[0].GetTypeID());
            }
        }
        else if(Arguments[0].IsType<ClassDefinition>())
        {
            return Arguments[0];   
        }
        else if(Arguments[0].IsType<ClassInstance>())
        {
            return Arguments[0].GetType<ClassInstance>().AssociatedClass;
        }
        return ReturnValue;
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
                                  "return",
                                  })
        {
            p_GetSymbolID(String);
        }
        for(auto const& Pair : std::vector<std::pair<std::string,BuiltinFuncType>>{
                    {"print",Print},
                    {"+",Plus},
                    {"list",CreateList},
                    {"class",Class},
                    {"addmethod",AddMethod},
                    {"generic",Generic},
                    {"read-term",Stream_ReadTerm},
                    {"flatten-1",Flatten_1},
                    {"gensym",GenSym},
                    {"dynamic",Dynamic},
                    {"environment",Environment},
                    {"new-environment",NewEnvironment},
                    {"expand",Expand},
                    {"type",Type},
                    {"load",Load},

                    //fs stuff, should probably make this into a module
                    {"is-directory",IsDirectory},
                    {"exists",Exists},
                    {"cwd",Cwd},
                    {"parent-path",ParentPath},
                    {"user-home-dir",UserHomeDir},
                    {"list-dir",ListDir},
                    {"is-directory",IsDirectory},
                    //
                    {"dict",CreateDict},
                    //
                    {"get-internal-module",GetInternalModule},
                    {"internal-modules",InternalModules},

                    //threading
                    {"lock",[] BUILTIN_ARGLIST -> Value
                        {
                            return Value::EmplaceExternal<Lock>();
                        }},
                    {"thread",Thread},
                    {"this-thread",This_Thread},
                })
        {
            Function NewBuiltin;
            NewBuiltin.Func = Pair.second;
            m_GlobalScope->SetVariable(p_GetSymbolID(Pair.first),NewBuiltin);
        }
        m_PrimitiveSymbolMax = m_CurrentSymbolID;

        //primitive types
        p_RegisterBuiltinClass<List>("list_t");
        p_RegisterBuiltinClass<Int>("int_t");
        p_RegisterBuiltinClass<Float>("float_t");
        p_RegisterBuiltinClass<Symbol>("symbol_t");
        p_RegisterBuiltinClass<String>("string_t");
        p_RegisterBuiltinClass<bool>("bool_t");
        p_RegisterBuiltinClass<Dict>("dict_t");
        p_RegisterBuiltinClass<Null>("null_t");
        p_RegisterBuiltinClass<Function>("function_t");
        p_RegisterBuiltinClass<Lambda>("lambda_t");
        p_RegisterBuiltinClass<Macro>("macro_t");
        p_RegisterBuiltinClass<GenericFunction>("generic_t");
        p_RegisterBuiltinClass<ClassDefinition>("type_t");
        p_RegisterBuiltinClass<Any>("any_t");
        p_RegisterBuiltinClass<ThreadHandle>("thread_t");
        
        //list
        AddMethod<List>("append",Append_List);
        AddMethod<List,Int>("index",Index_List);
        AddMethod<List>("len",Len_List);
        AddMethod<List>("sort",Sort);
        //class
        AddMethod<ClassInstance>("index",Index_ClassInstance);

        //scope
        AddMethod<Scope,Symbol>("index",Index_Environment);
        AddMethod<Scope,Symbol,Any>("set-var",SetVar_Environment);
        AddMethod<Scope,Scope>("set-parent",SetParent_Environment);
        AddMethod<Scope,Scope>("add-parent",AddParent_Environment);
        AddMethod<Scope,Symbol>("shadow",Shadow_Environment);
        //comparisons
        AddMethod<String,String>("eq",Eq_String);
        AddMethod<Symbol,Symbol>("eq",Eq_Symbol);
        AddMethod<bool,bool>("eq",Eq_Bool);
        AddMethod<Int,Int>("eq",Eq_Int);
        AddMethod<ClassDefinition,ClassDefinition>("eq",Eq_Type);
        AddMethod<Any,Any>("eq",Eq_Any);
        AddMethod<Null,Null>("eq",Eq_Null);
        AddMethod<Int,Int>("minus",Minus_Int);

        //streams
        AddMethod<MBUtility::StreamReader>("eof",Stream_EOF);
        AddMethod<MBUtility::StreamReader>("peek-byte",Stream_PeakByte);
        AddMethod<MBUtility::StreamReader>("read-byte",Stream_ReadByte);
        AddMethod<MBUtility::StreamReader>("skip-whitespace",Stream_SkipWhitespace);
        AddMethod<MBUtility::StreamReader,Int>("read-bytes",Stream_ReadBytes);
        AddMethod<MBUtility::StreamReader>("read-string",Stream_ReadString);
        AddMethod<MBUtility::StreamReader>("read-number",Stream_ReadNumber);
        AddMethod<MBUtility::StreamReader>("read-line",Stream_ReadLine);
        AddMethod<MBUtility::StreamReader,String>("read-until",Stream_ReadUntil);
        AddMethod<String,std::unique_ptr<MBUtility::MBOctetOutputStream>>("write",Write_OutStream);
        AddMethod<String>("out-stream",OutStream_String);
        AddMethod<String>("in-stream",InStream_String);
        AddMethod<String>("open",Open_URI);
        AddMethod<String,String>("open",Open_URI);

        //Dict
        AddMethod<Dict,Any>("index",Index_Dict);
        AddMethod<Any,Dict>("in",In_Dict);
        AddMethod<Dict>("keys",Keys_Dict);
        
        //Strings
        AddMethod<String,String>("split",Split_String);
        AddMethod<String,String>("in",In_String);
        AddMethod<String>("len",Len_String);
        AddMethod<Symbol,Scope>("in",In_Environment);
        AddMethod<Scope>("clear",Clear_Environment);
        AddMethod<Symbol>("str",Str_Symbol);
        AddMethod<bool>("str",Str_Bool);
        AddMethod<Null>("str",Str_Null);
        AddMethod<Float>("str",Str_Float);
        AddMethod<Int>("str",Str_Int);
        AddMethod<String>("int",Int_Str);
        AddMethod<String,Int>("substr",Substr);
        AddMethod<String,Int,Int>("substr",Substr);
        AddMethod<String,String>("index-of",IndexOf_StrStr);


        //operators
        AddMethod<Int,Int>("<",Less_Int);
        AddMethod<String,String>("<",Less_String);
        AddMethod<Symbol,Symbol>("<",Less_Symbol);
        //Threading
        AddMemberMethod<Lock,&Lock::Get>("get");
        AddMemberMethod<Lock,&Lock::Notify>("notify");
        AddMemberMethod<Lock,&Lock::Release>("release");
        AddMemberMethod<Lock,&Lock::Wait>("wait");
        AddMethod<ThreadHandle,Int>("sleep",Sleep);
        //Symbols
        AddMethod<String>("symbol",Symbol_String);
        AddMethod<Symbol,Int>("symbol",Symbol_SymbolInt);
        //stuff
        AddMethod<Macro,Symbol>("set-name",SetName_Macro);
        AddMethod<Lambda,Symbol>("set-name",SetName_Lambda);
        AddMethod<GenericFunction,Symbol>("set-name",SetName_Generic);
        AddMethod<ClassDefinition,Symbol>("set-name",SetName_ClassDefinition);

        AddMethod<Macro>("name",Name_Macro);
        AddMethod<Lambda>("name",Name_Lambda);
        AddMethod<GenericFunction>("name",Name_Generic);
        AddMethod<ClassDefinition>("name",Name_ClassDefinition);

        AddMethod<Symbol>("is-special",IsSpecial_Symbol);
        AddMethod<Symbol>("position",Position_Symbol);
        AddMethod<MBUtility::StreamReader>("position",Stream_Position);
       
        //Filesystem stuff
        AddMethod<String>("canonical",Canonical);
        AddMethod<String>("is-file",IsFile);

        
        m_GlobalScope->SetVariable(p_GetSymbolID("*READTABLE*"),Value::MakeExternal(ReadTable()));
        //Readtables
        AddMethod<ReadTable,String>("add-reader-character",AddReaderCharacter);
        AddMethod<ReadTable,String>("remove-reader-character",RemoveReaderCharacter);
        AddMethod<ReadTable,String>("add-character-expander",AddCharacterExpander);
        AddMethod<ReadTable,String>("remove-character-expander",RemoveCharacterExpander);


        m_GlobalScope->SetVariable(p_GetSymbolID("*standard-input*"),Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBLSP::TerminalInput>())));
        m_GlobalScope->SetVariable(p_GetSymbolID("*standard-output*"),Value::MakeExternal(
                    std::unique_ptr<MBUtility::MBOctetOutputStream>( new MBUtility::TerminalOutput())));
        m_GlobalScope->SetVariable(p_GetSymbolID("is-repl"),false);
    }

    Value Evaluator::Write_OutStream BUILTIN_ARGLIST
    {
        Value ReturnValue;
        std::unique_ptr<MBUtility::MBOctetOutputStream>& OutStream = Arguments[1].GetType<std::unique_ptr<MBUtility::MBOctetOutputStream>>();
        String& StringToWrite = Arguments[0].GetType<String>();
        OutStream->Write(StringToWrite.data(),StringToWrite.size());
        return ReturnValue;
    }
    struct i_ValueStringStream : public MBUtility::MBOctetOutputStream
    {
        Value m_OutString;
    public:
        i_ValueStringStream(Value ValueToAppend) : m_OutString(std::move(ValueToAppend))
        {
        }
		virtual size_t Write(const void* DataToWrite, size_t DataToWriteSize)
        {
            m_OutString.GetType<String>().insert(m_OutString.GetType<String>().end(),(const char*) DataToWrite,((const char*) DataToWrite)+DataToWriteSize);
            return DataToWriteSize;
        }
    };
    Value Evaluator::OutStream_String BUILTIN_ARGLIST
    {
        return Value::MakeExternal(std::unique_ptr<MBUtility::MBOctetOutputStream>(new i_ValueStringStream(Arguments[0])));
    }

    Value Evaluator::InStream_String BUILTIN_ARGLIST
    {
        return Value::MakeExternal(MBUtility::StreamReader( std::make_unique<MBUtility::OwningStringStream>(Arguments[0].GetType<String>())));
    }
    Value Evaluator::Open_URI BUILTIN_ARGLIST
    {
        Value ReturnValue;
        String URIToOpen = Arguments[0].GetType<String>();
        if(Arguments.size() == 1)
        {
            return Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBUtility::InputFileStream>(URIToOpen)));
        }
        else if(Arguments.size() == 2)
        {
            String AccessString = Arguments[1].GetType<String>();
            if(AccessString.find('r') != AccessString.npos)
            {
                return Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBUtility::InputFileStream>(URIToOpen)));
            }
            else if(AccessString.find('w') != AccessString.npos)
            {
                return Value::MakeExternal(std::unique_ptr<MBUtility::MBOctetOutputStream>(new MBUtility::MBFileOutputStream(URIToOpen)));
            }
            else
            {
                throw std::runtime_error("open with access string requires that the access string contains either 'r' or 'w'");   
            }
        }
        return ReturnValue;
    }


    Value Evaluator::Thread BUILTIN_ARGLIST
    {
        ThreadHandle NewThread;
        if(Arguments.size() == 0)
        {
            throw std::runtime_error("Thread requires a callable as first argument");
        }
        ExecutionState NewExecState;
        FuncArgVector Args;
        for(int i = 1; i < Arguments.size();i++)
        {
            Args.push_back(Arguments[i]);
        }
        Context.GetEvaluator().p_Invoke(Arguments[0],Args,NewExecState);
        NewExecState.ThreadID = Context.GetEvaluator().m_ThreadingState.GetNextID();
        NewThread.ID = NewExecState.ThreadID;
        Context.GetEvaluator().m_ThreadingState.AddThread([&,ExecState=std::move(NewExecState)]() mutable
                {
                    Context.GetEvaluator().p_Eval(ExecState);
                });
        return NewThread;
    }
    Value Evaluator::This_Thread BUILTIN_ARGLIST
    {
        ThreadHandle Handle;
        Handle.ID = Context.GetEvaluator().m_ThreadingState.CurrentID();
        return Handle;
    }
    Value Evaluator::Sleep BUILTIN_ARGLIST
    {
        Context.GetEvaluator().m_ThreadingState.Sleep(Arguments[0].GetType<ThreadHandle>().ID,Arguments[1].GetType<Int>());
        return Value();
    }

    Value Evaluator::GetInternalModule BUILTIN_ARGLIST
    {
        Value ReturnValue;
        if(Arguments.size() != 1 || !Arguments[0].IsType<String>())
        {
            throw std::runtime_error("get-internal-module requires exactly 1 argument of type string");
        }
        String& AssociatedString = Arguments[0].GetType<String>();
        if( auto It = Context.GetEvaluator().m_BuiltinModules.find(AssociatedString); It != Context.GetEvaluator().m_BuiltinModules.end())
        {
            ReturnValue = Value(It->second->GetModuleScope());
        }
        else
        {
            throw std::runtime_error("no internal module with name \""+AssociatedString+"\"");   
        }
        return ReturnValue;
    }
    Value Evaluator::InternalModules BUILTIN_ARGLIST
    {
        List ReturnValue;

        for(auto const& Module : Context.GetEvaluator().m_BuiltinModules)
        {
            ReturnValue.push_back(Module.first);
        }
        return ReturnValue;
    }
    Value Evaluator::IsSpecial_Symbol BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Symbol>().ID < SymbolID(PrimitiveForms::LAST);
    }
    Value Evaluator::Position_Symbol BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Symbol>().SymbolLocation.Position;
    }
    Evaluator::Evaluator()
    {
        p_InternPrimitiveSymbols();
        p_LoadModules();
    }
    void Evaluator::p_LoadModules()
    {
        m_BuiltinModules["lsp-internal"] = std::make_unique<LSPModule>();
        m_BuiltinModules["lsp-internal"]->SetEvaluator(this);
    }
    Value Evaluator::Load BUILTIN_ARGLIST
    {
        Value ReturnValue = false;
        if(Arguments.size() !=  1)
        {
            throw std::runtime_error("Load requires exactly 1 argument, the filepath to a source file to be evaluated");   
        }
        if(!Arguments[0].IsType<String>())
        {
            throw  std::runtime_error("Load requires first argument to be a string");   
        }
        std::filesystem::path SourceFilepath = Arguments[0].GetType<String>();
        SymbolID LoadFilepathSymbol = Context.GetEvaluator().GetSymbolID("load-filepath");
        Value* CurrentLoadFilepath =  Context.GetCurrentScope()->TryGet(Context.GetEvaluator().GetSymbolID("load-filepath"));
        Value ValueToRestore = std::string("");
        if(CurrentLoadFilepath != nullptr)
        {
            ValueToRestore = *CurrentLoadFilepath;
        }
        try
        {
          Context.GetEvaluator().p_LoadFile(Context.GetCurrentScope(),SourceFilepath);
        }
        catch(LookupError const& e)
        {
            Context.GetCurrentScope()->SetVariable(LoadFilepathSymbol,std::move(ValueToRestore));
            throw std::runtime_error(e.what() + ( ": " + Context.GetEvaluator().GetSymbolString(e.GetSymbol())));
        }
        catch(...)
        {
            Context.GetCurrentScope()->SetVariable(LoadFilepathSymbol,std::move(ValueToRestore));
            throw;
        }
        return  ReturnValue;
    }
    //FSStuff
    Value Evaluator::Exists BUILTIN_ARGLIST
    {
        if(Arguments.size() != 1 ||  !Arguments[0].IsType<String>())
        {
            throw std::runtime_error("exists requires exactly 1 argument of type string");
        }
        return std::filesystem::exists(Arguments[0].GetType<String>());
    }
    Value Evaluator::Cwd BUILTIN_ARGLIST
    {
        return MBUnicode::PathToUTF8(std::filesystem::current_path())+"/";
    }
    Value Evaluator::ParentPath BUILTIN_ARGLIST
    {
        if(Arguments.size() != 1 ||  !Arguments[0].IsType<String>())
        {
            throw std::runtime_error("parent-path requires exactly 1 argument of type string");
        }
        return MBUnicode::PathToUTF8(std::filesystem::path(Arguments[0].GetType<String>()).parent_path());
    }
    Value Evaluator::Canonical BUILTIN_ARGLIST
    {
        return MBUnicode::PathToUTF8(std::filesystem::canonical(Arguments[0].GetType<String>()));
    }
    Value Evaluator::UserHomeDir BUILTIN_ARGLIST
    {
        return MBUnicode::PathToUTF8(MBSystem::GetUserHomeDirectory());
    }
    Value Evaluator::ListDir BUILTIN_ARGLIST
    {
        if(Arguments.size() != 1 ||  !Arguments[0].IsType<String>())
        {
            throw std::runtime_error("list-dir requires exactly 1 argument of type string");
        }
        List ReturnValue;
        for(auto const& Entry : std::filesystem::directory_iterator(Arguments[0].GetType<String>()))
        {
            ReturnValue.push_back( MBUnicode::PathToUTF8(Entry.path().filename()));
        }
        return ReturnValue;
    }
    Value Evaluator::IsDirectory BUILTIN_ARGLIST
    {
        if(Arguments.size() != 1 ||  !Arguments[0].IsType<String>())
        {
            throw std::runtime_error("list-dir requires exactly 1 argument of type string");
        }
        return std::filesystem::is_directory(Arguments[0].GetType<String>());
    }
    Value Evaluator::IsFile BUILTIN_ARGLIST
    {
        return std::filesystem::is_regular_file(Arguments[0].GetType<String>());
    }
    //
    void Evaluator::p_LoadFile(Ref<Scope> CurrentScope,std::filesystem::path const& LoadFilePath)
    {
        Ref<OpCodeList> OpCodes = MakeRef<OpCodeList>();
        if(!std::filesystem::exists(LoadFilePath))
        {
            throw std::runtime_error("Source file \"" +LoadFilePath.generic_string() +"\" doesn't exist");
        }
        //updates  the load-filepath
        CurrentScope->SetVariable(p_GetSymbolID("load-filepath"),LoadFilePath.generic_string());
        SymbolID URI = p_GetSymbolID(LoadFilePath.generic_string());
        std::string Content = MBUtility::ReadWholeFile(LoadFilePath.generic_string());
        Value ReaderValue = Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBUtility::IndeterminateStringStream>(Content)));
        MBUtility::StreamReader& Reader = ReaderValue.GetType<MBUtility::StreamReader>();
        assert(ReaderValue.IsType<MBUtility::StreamReader>());
        ReadTable& Table = CurrentScope->FindVariable(p_GetSymbolID("*READTABLE*")).GetType<ReadTable>();
        while(!Reader.EOFReached())
        {
            IPIndex InstructionToExecute = OpCodes->Size();
            Value NewTerm = p_Expand(CurrentScope,p_ReadTerm(CurrentScope,URI,Table,Reader,ReaderValue));
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
    void Evaluator::LoadStd()
    {
        if(std::filesystem::exists(MBSystem::GetUserHomeDirectory()/".mblisp/libs/std/index.lisp"))
        {
            try
            {
                p_LoadFile(m_GlobalScope,MBSystem::GetUserHomeDirectory()/".mblisp/libs/std/index.lisp");
            }
            catch(std::exception const& e)
            {
                throw std::runtime_error("Failed loading standard library: "+std::string(e.what()));
            }
        }
        else
        {
            throw std::runtime_error("Failed loading standard library: file not present on default location ~/.mblisp/std/index.lisp");   
        }
    }
    void Evaluator::Eval(std::filesystem::path const& SourcePath)
    {
        Ref<Scope> CurrentScope = CreateDefaultScope();
        //set load path
        p_LoadFile(CurrentScope,SourcePath);
    }
    void Evaluator::Repl()
    {
        LoadStd();
        Value StdinValue = m_GlobalScope->FindVariable(p_GetSymbolID("*standard-input*"));
        auto& Stdin  = StdinValue.GetType<MBUtility::StreamReader>();
        auto ReplScope = CreateDefaultScope();
        Value TableValue = ReplScope->FindVariable(p_GetSymbolID("*READTABLE*"));
        ReadTable const& Table = TableValue.GetType<ReadTable>();
        m_GlobalScope->SetVariable(p_GetSymbolID("is-repl"),true);
        ReplScope->SetVariable( p_GetSymbolID("load-filepath"),MBUnicode::PathToUTF8(std::filesystem::current_path()));
        SymbolID URI = p_GetSymbolID(MBUnicode::PathToUTF8(std::filesystem::current_path()));
        Ref<OpCodeList> OpCodes = MakeRef<OpCodeList>();
        while(true)
        {
            try
            {
                IPIndex InstructionToExecute = OpCodes->Size();
                Value NewTerm = p_Expand(ReplScope,p_ReadTerm(ReplScope,URI,Table,Stdin,StdinValue));
                OpCodes->Append(NewTerm);
                Print(*this,p_Eval(ReplScope,OpCodes,InstructionToExecute));
                std::cout<<std::endl;
            }
            catch(UncaughtSignal const& e)
            {
                std::cout<<"Uncaught signal: ";
                Print(*this,e.ThrownValue);
                std::cout<<std::endl;
            }
            catch(InvalidCharacter const& e)
            {
                std::cout<<e.what();
                std::cout<<std::endl;
                Stdin.ReadByte();
            }
            catch(std::exception const& e)
            {
                std::cout<<"Uncaught signal: "<<e.what()<<std::endl;
            }
        }
    }
    Ref<Scope> Evaluator::CreateDefaultScope()
    {
        Ref<Scope> ReturnValue = MakeRef<Scope>();
        ReturnValue->SetShadowingParent(m_GlobalScope);
        ReturnValue->SetVariable(p_GetSymbolID("*READTABLE*"),Value::MakeExternal(
                    ReadTable(m_GlobalScope->FindVariable(p_GetSymbolID("*READTABLE*")).GetType<ReadTable>())));
        return ReturnValue;
    }
    Value Evaluator::SetName_Macro BUILTIN_ARGLIST
    {
        Value ReturnValue;
        Arguments[0].GetType<Macro>().Name = Arguments[1].GetType<Symbol>();

        return ReturnValue;
    }
    Value Evaluator::SetName_Lambda BUILTIN_ARGLIST
    {
        Value ReturnValue;
        Arguments[0].GetType<Lambda>().Name = Arguments[1].GetType<Symbol>();
        return ReturnValue;
    }
    Value Evaluator::SetName_Generic BUILTIN_ARGLIST
    {
        Value ReturnValue;
        Arguments[0].GetType<GenericFunction>().Name = Arguments[1].GetType<Symbol>();
        return ReturnValue;
    }
    Value Evaluator::SetName_ClassDefinition BUILTIN_ARGLIST
    {
        Arguments[0].GetType<ClassDefinition>().Name = Arguments[1].GetType<Symbol>();
        return Value();
    }
    Value Evaluator::Name_Macro BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Macro>().Name;
    }
    Value Evaluator::Name_Lambda BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<Lambda>().Name;
    }
    Value Evaluator::Name_Generic BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<GenericFunction>().Name;
    }
    Value Evaluator::Name_ClassDefinition BUILTIN_ARGLIST
    {
        return Arguments[0].GetType<ClassDefinition>().Name;
    }
}
