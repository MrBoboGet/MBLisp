
#include "Evaluator.h"
#include <MBUtility/StreamReader.h>
#include "MBLSP/LSP_Structs.h"
#include "MBUtility/MBInterfaces.h"
#include "assert.h"
#include <MBParsing/MBParsing.h>

#include  <iostream>
#include <iterator>

#include <MBUtility/MBFiles.h>
#include <MBSystem/MBSystem.h>
#include <MBUnicode/MBUnicode.h>
#include <MBUtility/MBStrings.h>

#include <atomic>


#include <MBUtility/FileStreams.h>
#include <MBUtility/SearchableStreamReader.h>

#include <iostream>
#include "Threading.h"


#include "Modules/LSP/LSPModule.h"
#include "Modules/Text/TextModule.h"
#include "Modules/IO/IO.h"
namespace MBLisp
{

    //Stacktrace
    Evaluator::StackTrace::StackTrace(ExecutionState& CurrentState,String  const& Message)
    {
        for(int i = 0; i < CurrentState.StackFrames.size();i++)
        {
            auto const& CurrentFrame = CurrentState.StackFrames[i];
            if(CurrentFrame.ExecutionPosition.OpCodeCount() == 0)
            {
                continue;   
            }
            Frame NewFrame;
            NewFrame.FrameLocation = CurrentFrame.ExecutionPosition.GetLocation(CurrentFrame.ExecutionPosition.GetIP()-1);
            NewFrame.Name = CurrentFrame.ExecutionPosition.GetName();
            Frames.push_back(NewFrame);
        }
        this->Message = Message;
    }
    //Stacktrace
   
    //ExecutionState
    
    void ExecutionState::PopFrame()
    {
        auto& StackScope = StackFrames.back().StackScope;
        StackFrames.pop_back();
    }
    //

    
    //CallContext
    Evaluator& CallContext::GetEvaluator()
    {
        assert(m_AssociatedEvaluator != nullptr);
        return *m_AssociatedEvaluator;
    }
    Value CallContext::GetVariable(std::string const& VarName)
    {
        Value ReturnValue = GetState().GetCurrentScope().FindVariable(GetEvaluator().p_GetSymbolID(VarName));
        if(ReturnValue.IsType<DynamicVariable>())
        {
            auto const& DynVal = ReturnValue.GetType<DynamicVariable>();
            auto const& Bindings = GetState().DynamicBindings[DynVal.ID];
            if(Bindings.size() == 0)
            {
                return DynVal.DefaultValue;   
            }
            else
            {
                return Bindings.back();   
            }
        }
        return ReturnValue;
    }
    ExecutionState& CallContext::GetState()
    {
        return *m_CurrentState;
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
        m_AssociatedEvaluator->m_ThreadingState.TempSuspend(m_CurrentState->AssociatedThread,true);
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
        Value LessFunc = Context.GetState().GetCurrentScope().FindVariable(Context.GetEvaluator().p_GetSymbolID("<"));
        std::sort(AssociatedList->begin(),AssociatedList->end(),[&](Value const& lhs,Value const& rhs)
                {
                    if(!(lhs.IsType<String>() && rhs.IsType<String>()))
                    {
                        return false;
                    }
                    return lhs.GetType<String>() < rhs.GetType<String>();
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
    bool Evaluator::InsertElements(List& Lhs,List& Rhs)
    {
        for(auto& Val : Rhs)
        {
            Lhs.push_back(Val);   
        }
        return true;
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
    Int Evaluator::Len_Dict(Dict& DictToInspect)
    {
        return DictToInspect.size();
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
    Int Evaluator::Plus_Int (Int Lhs,Int Rhs)
    {
        return Lhs+Rhs;
    }
    String Evaluator::Plus_String (String& Lhs,String& Rhs)
    {
        return Lhs+Rhs;
    }


    String Evaluator::GetString(StackTrace const& Trace)
    {
        String ReturnValue = "\nError at:\n";
        std::unordered_map<String,MBLSP::LineIndex> Indexes;
        for(auto const& Frame : Trace.Frames)
        {
            ReturnValue += "    ";
            if(! (Frame.Name.ID == -1))
            {
                ReturnValue += GetSymbolString(Frame.Name.ID);
            }
            else
            {
                ReturnValue += "lambda";   
            }
            if(Frame.FrameLocation.URI != -1)
            {
                //reading the  whole file...
                String URI = GetSymbolString(Frame.FrameLocation.URI);
                if(Indexes.find(URI) == Indexes.end())
                {
                    Indexes[URI] = MBLSP::LineIndex(MBUtility::ReadWholeFile(URI));   
                }
                MBLSP::Position Position = Indexes[URI].ByteOffsetToPosition(Frame.FrameLocation.Position);
                ReturnValue += " " + URI;
                ReturnValue += " line: "+std::to_string(Position.line +1);
                ReturnValue += " col: "+std::to_string(Position.character +1);
            }
            ReturnValue  += "\n";
        }
        ReturnValue += Trace.Message;
        return ReturnValue;
    }
    void Evaluator::Print(Evaluator& AssociatedEvaluator,Value const& ValueToPrint)
    {
        if(ValueToPrint.IsType<String>())
        {
            std::cout<<ValueToPrint.GetType<String>();
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
        else if(ValueToPrint.IsType<StackTrace>())
        {
            std::cout<<AssociatedEvaluator.GetString(ValueToPrint.GetType<StackTrace>());
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
            std::merge(TempSlots.begin(),TempSlots.end(),Class->SlotDefinitions.begin(),Class->SlotDefinitions.end(),std::inserter(NewSlots,NewSlots.begin()));
            //std::swap(NewSlots, TempSlots);
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
            TemporaryClass.Types.push_back(Value::GetTypeTypeID<ClassInstance>());
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
        NewClass.Envir = Context.GetState().GetScopeRef();
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
    Value Evaluator::ConstructDynamicVariable(Value DefaultValue)
    {
        DynamicVariable ReturnValue;
        ReturnValue.DefaultValue = std::move(DefaultValue);
        ReturnValue.ID = g__CurrentDynamicVarID.load();
        g__CurrentDynamicVarID.fetch_add(1);
        return ReturnValue;
    }
    Value Evaluator::Dynamic BUILTIN_ARGLIST
    {
        DynamicVariable ReturnValue;
        if(Arguments.size() != 1)
        {
            throw std::runtime_error("dynamic requires exacty 1 argument, the initial value for the dynamic variable");   
        }
        return ConstructDynamicVariable(std::move(Arguments[0]));
    }
    Value Evaluator::Expand BUILTIN_ARGLIST
    {
        return Context.GetEvaluator().p_Expand(Context.GetState(),Context.GetState().GetCurrentScope(),Arguments[0]);
    }
    Value Evaluator::Stream_ReadTerm BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            //Context.PauseThread();
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
        //copy
        Ref<ReadTable> Table = Context.GetEvaluator().p_GetDynamicValue(Context.GetState().GetCurrentScope(),Context.GetState(),"*READTABLE*").GetRef<ReadTable>();  
        Ref<DynamicVariable> DynamicLoadFilepath = Context.GetState().GetCurrentScope().FindVariable(Context.GetEvaluator().p_GetSymbolID("load-filepath")).GetRef<DynamicVariable>();
        assert(Context.GetState().DynamicBindings[DynamicLoadFilepath->ID].size() > 0);
        Value CurrentLoadFilepath = Context.GetState().DynamicBindings[DynamicLoadFilepath->ID].back();
        SymbolID URI = p_PathURI(Context.GetEvaluator(),CurrentLoadFilepath.GetType<String>());
        //TODO think through this functionality again, how needed is it for ":" expander, and could it be implemented in a more clean way
        //Ref<Scope> NewScope = Context.GetState().StackFrames.front().StackScope;
        ReturnValue = Context.GetEvaluator().p_ReadTerm(Context.GetState(),URI,Table,Reader,Arguments[0]);
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
    Value Evaluator::Stream_Symbol BUILTIN_ARGLIST
    {
        BuiltinLock Lock;
        if(Context.IsMultiThreaded())
        {
            Lock = Arguments[0].GetLock();   
            Context.PauseThread();
        }
        auto& Stream = Arguments[0].GetType<MBUtility::StreamReader>();
        PositionType Pos = Stream.Position();
        std::string SymbolString = Stream.ReadWhile([](char CharToRead)
                {
                    return (CharToRead > ' ' && CharToRead < 0x7f) && CharToRead != '"' && CharToRead != '(' && CharToRead != ')' 
                    //kinda hacky/temporary, maybe should add the ability to alter which characters can appear within a symbol, like in common lisp...
                    && CharToRead != '{' && CharToRead != '}' && CharToRead != '[' && CharToRead != ']' && CharToRead != ',';
                });
        Symbol ReturnValue = Context.GetEvaluator().p_GetSymbolID(SymbolString);
        ReturnValue.SymbolLocation.Position = Pos;
        //TODO maybe default location to current load-filepath?
        //ReturnValue.SymbolLocation.URI = 
        return ReturnValue;
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
        Int BytesToRead = Arguments[1].GetType<Int>();
        if(BytesToRead < 0)
        {
            throw std::runtime_error("Can only read a positive amount of bytes");   
        }
        String ReturnValue(BytesToRead,0);
        size_t ReadBytes = Arguments[0].GetType<MBUtility::StreamReader>().Read(ReturnValue.data(),BytesToRead);
        ReturnValue.resize(ReadBytes);
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
    bool Evaluator::Seek_Stream(MBUtility::MBSearchableInputStream& InStream,Int Offset)
    {
        InStream.SetInputPosition(Offset);
        return false;
    }
    Int Evaluator::Offset_Stream(MBUtility::MBSearchableInputStream& InStream)
    {
        return Int(InStream.GetInputPosition());
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
            throw std::runtime_error("Index out of range when indexing list: " + std::to_string(Index)); 
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
    Value Evaluator::Back_List BUILTIN_ARGLIST
    {
        List& AssociatedList = Arguments[0].GetType<List>();
        if(AssociatedList.size() == 0)
        {
            throw std::runtime_error("error accessing back element of list: list was empty");   
        }
        Value& AssociatedValue = AssociatedList[AssociatedList.size()-1];
        if(Context.IsSetting())
        {
            AssociatedValue = Context.GetSetValue();
        }
        return AssociatedValue;
    }
    Value Evaluator::Append_List BUILTIN_ARGLIST
    {
        assert(!Arguments[0].IsType<List>() || Arguments[0].GetRef<List>() != nullptr);
        List& AssociatdList = Arguments[0].GetType<List>();
        for(int i = 1; i < Arguments.size();i++)
        {
            AssociatdList.push_back(Arguments[i]);
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
    Value Evaluator::Pop_List(List& ListToModify)
    {
        if(ListToModify.size() == 0)
        {
            throw std::runtime_error("Cannot pop from empty list");
        }
        Value ReturnValue = ListToModify.back();
        ListToModify.pop_back();
        return ReturnValue;
    }
    bool Evaluator::Reverse_List(List& ListToModify)
    {
        std::reverse(ListToModify.begin(),ListToModify.end());
        return false;
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
    bool Evaluator::Is_Type(ClassDefinition& Lhs,ClassDefinition& Rhs)
    {
        bool ReturnValue = false;
        if(Rhs.ID == 0)
        {
            return true;   
        }
        auto It = std::lower_bound(Lhs.Types.begin(),Lhs.Types.end(),Rhs.Types.back());
        if(It != Lhs.Types.end() && *It == Rhs.Types.back())
        {
            ReturnValue = true;   
        }
        return ReturnValue;
    }
    Value Evaluator::Eq_Any BUILTIN_ARGLIST
    {
        return false;
    }
    bool Evaluator::Eq_ThreadHandle(ThreadHandle  lhs,ThreadHandle rhs)
    {
        return lhs.ID == rhs.ID;
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
    Value Evaluator::Append_String BUILTIN_ARGLIST
    {
        Arguments[0].GetType<String>().append(Arguments[1].GetType<String>());
        return Arguments[0];
    }
    Value Evaluator::In_String BUILTIN_ARGLIST
    {
        return Arguments[1].GetType<String>().find(Arguments[0].GetType<String>()) != std::string::npos;
    }
    Value Evaluator::In_Environment BUILTIN_ARGLIST
    {
        Scope& Envir = Arguments[1].GetType<Scope>();
        Symbol& SymbolToCheck = Arguments[0].GetType<Symbol>();
        return Envir.TryGetLocalByID(SymbolToCheck.ID) != nullptr || Envir.TryGet(SymbolToCheck.ID) != nullptr;
    }
    Value Evaluator::Str_Symbol BUILTIN_ARGLIST
    {
        return Context.GetEvaluator().GetSymbolString(Arguments[0].GetType<Symbol>().ID);
    }
    Value Evaluator::Str_Int BUILTIN_ARGLIST
    {
        return std::to_string(Arguments[0].GetType<Int>());
    }
    String Evaluator::Str_String(String& Input)
    {
        return Input;
    }
    String Evaluator::Str_ThreadHandle(ThreadHandle Input)
    {
        return std::to_string(Input.ID);
    }
    Int Evaluator::Int_ThreadHandle(ThreadHandle Input)
    {
        return Input.ID;
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
    Value Evaluator::Str_StackTrace BUILTIN_ARGLIST
    {
        return Context.GetEvaluator().GetString(Arguments[0].GetType<StackTrace>());
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
        if(Arguments.size() > 2)
        {
            ReturnValue.SymbolLocation.URI = Arguments[2].GetType<Symbol>().ID;   
        }
        return ReturnValue;
    }
    Value Evaluator::GenSym BUILTIN_ARGLIST
    {
        return Symbol(Context.GetEvaluator().GenerateSymbol());
    }
    Value Evaluator::Environment BUILTIN_ARGLIST
    {
        Value ReturnValue = Context.GetState().GetScopeRef();
        assert(ReturnValue.IsType<Scope>());
        Context.GetState().StackFrames.back().ScopeRetrieved = true;
        return ReturnValue;
    }
    Value Evaluator::NewEnvironment BUILTIN_ARGLIST
    {
        Ref<Scope> NewScope = Context.GetEvaluator().CreateDefaultScope();
        return NewScope;
    }
    Value Evaluator::Index_Environment BUILTIN_ARGLIST
    {
        Scope& AssociatedScope = Arguments[0].GetType<Scope>();
        SymbolID SymbolIndex = Arguments[1].GetType<Symbol>().ID;
        Value* ReturnValue = nullptr;
        if(auto LocalVarPointer = AssociatedScope.TryGetLocalByID(SymbolIndex); LocalVarPointer != nullptr)
        {
            ReturnValue = LocalVarPointer;
        }
        else
        {
            ReturnValue = AssociatedScope.TryGet(SymbolIndex);
        }
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
    List Evaluator::Vars (Scope& Environemnt)
    {
        List ReturnValue;
        for(auto ID : Environemnt.Vars())
        {
            ReturnValue.push_back(Symbol(ID));
        }
        return ReturnValue;
    }
    List Evaluator::AllVars (Scope& Environemnt)
    {
        List ReturnValue;

        return ReturnValue;
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
            throw std::runtime_error("Couldn't find symbol '" + Context.GetEvaluator().GetSymbolString(IndexSymbol.ID) + "' when indexing class instance");
        }
        if(Context.IsSetting())
        {
            SymbolIt->second = Context.GetSetValue();
        }
        return SymbolIt->second;
    }
    List Evaluator::Slots_ClassInstance(ClassInstance& InstanceToInspect)
    {
        List ReturnValue;
        for(auto const& Slot : InstanceToInspect.Slots)
        {
            ReturnValue.push_back(Symbol(Slot.first));   
        }
        return ReturnValue;
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
    Value Evaluator::Eval(ExecutionState& CurrentState,Value Callable,FuncArgVector Arguments)
    {
        p_Invoke(Callable,Arguments,CurrentState);
        return p_Eval(CurrentState,CurrentState.StackFrames.size()-1);
    }
    Value Evaluator::Eval(Value Callable,FuncArgVector Arguments)
    {
        ExecutionState State;
        State.StackFrames.push_back(StackFrame());
        State.StackFrames.back().StackScope = m_GlobalScope;
        return Eval(State,std::move(Callable),std::move(Arguments));
    }
    Value Evaluator::Eval(Ref<Scope> AssociatedScope,Value Callable,FuncArgVector Arguments)
    {
        ExecutionState NewState;   
        NewState.StackFrames.push_back(StackFrame());
        NewState.StackFrames.back().StackScope = std::move(AssociatedScope);
        NewState.CurrentCallContext.m_AssociatedEvaluator = this;
        NewState.CurrentCallContext.m_CurrentState = &NewState;
        p_Invoke(Callable,Arguments,NewState);
        return p_Eval(NewState,NewState.StackFrames.size()-1);
    }
    void Evaluator::Unwind(ExecutionState& CurrentState,int TargetIndex)
    {
        if(TargetIndex == CurrentState.StackFrames.size())
        {
            return;   
        }
        CurrentState.UnwindingStack = true;
        CurrentState.FrameTarget = TargetIndex-1;
        try
        {
            p_Eval(CurrentState,TargetIndex);
        }
        catch(ContinueUnwind const&)
        {
        }
        CurrentState.FrameTarget = -1;
        CurrentState.UnwindingStack = false;
    }
    void Evaluator::AddInternalModule(std::string const& Name,Ref<Scope> ModuleScope)
    {
        if(m_BuiltinModules.find(Name) != m_BuiltinModules.end())
        {
            throw std::runtime_error("Mopdule with name \""+Name+"\" already defined");
        }
        m_BuiltinModules[Name] = std::make_unique<ScopeModule>(ModuleScope);
    }
    void Evaluator::p_EmitSignal(ExecutionState& CurrentState,Value SignalValue,bool ForceUnwind)
    {
        //the signal form returns a value in the current frame, which
        int CurrentFrameIndex = CurrentState.StackFrames.size()-1;
        bool SignalFound = false;
        for(int i = CurrentFrameIndex; i >= 0; i--)
        {
            auto& CurrentFrame = CurrentState.StackFrames.back();
            auto& StackFrames = CurrentState.StackFrames;
            //signals cannot go past UnwindProtect handlers
            //even non forced, as those could invoke unwind above
            if(CurrentFrame.ProtectDepth > 0)
            {
                assert(CurrentFrame.ActiveUnwindProtectors.size() > 0);
                auto const& CurrentProtector = CurrentFrame.ActiveUnwindProtectors.back();
                CurrentFrame.ArgumentStack.resize(CurrentProtector.EndStackCount+1);
                CurrentFrame.ExecutionPosition.SetIP(CurrentProtector.End);
                return;
            }

            //if(CurrentFrame.SignalFrameIndex != -1 && i == CurrentFrameIndex)
            //{
            //    continue;   
            //}
            //Ensure that exceptions in signal handlers can handle their own exceptions, or signal
            //above the original signals handler position
            if(CurrentFrame.SignalFrameIndex != -1 && (i < CurrentFrameIndex && i > CurrentFrame.SignalFrameIndex))
            {
                i = CurrentFrame.SignalFrameIndex;
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
        //TODO fix case where stack gets unwound
        if(!SignalFound)
        {
            auto& CurrentFrame = CurrentState.StackFrames.back();
            CurrentFrame.ArgumentStack.push_back(false);
            if(ForceUnwind)
            {
                UncaughtSignal Exception;
                Exception.ThrownValue = SignalValue;
                Exception.AssociatedScope = CurrentFrame.StackScope;
                throw Exception;
            }
        }
    }
    void Evaluator::p_InvokeTrapHandler(ExecutionState& State)
    {
        Value Handler = m_DebugState.GetTrapHandler();
        FuncArgVector Args;
        State.StackFrames.back().PopExtra = 1;
        p_Invoke(Handler,Args,State,false,true);
        State.TraphandlerIndex = State.StackFrames.size();
    }
    void Evaluator::p_Invoke(Value& ObjectToCall,FuncArgVector& Arguments,ExecutionState& CurrentState,bool Setting,bool IsTrapHandler)
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
                Value Result = AssociatedFunc(CurrentState.CurrentCallContext,Arguments);
                CurrentCallStack.back().ArgumentStack.push_back(std::move(Result));
            }
            catch(ContinueUnwind const& e)
            {
                   
            }
            catch(std::exception const& e)
            {
                p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState, String(e.what())),true);
            }
            if(CurrentState.CurrentCallContext.m_ThreadPaused)
            {
                m_ThreadingState.TempSuspend(CurrentState.AssociatedThread,false);
                CurrentState.CurrentCallContext.m_ThreadPaused = false;
            }
            assert(CurrentCallStack.back().ArgumentStack.size() == 0 || !CurrentCallStack.back().ArgumentStack.back().IsType<List>() || CurrentCallStack.back().ArgumentStack.back().GetRef<List>() != nullptr);
        }
        else if(ObjectToCall.IsType<FunctionObject>())
        {
            FunctionObject& ObjectToInvoke = ObjectToCall.GetType<FunctionObject>();
            for(auto& Arg : Arguments)
            {
                assert(!Arg.IsType<List>() || Arg.GetRef<List>() != nullptr);
            }
            try
            {
                CurrentState.CurrentCallContext.m_IsSetting = Setting;
                Value Result = ObjectToInvoke(CurrentState.CurrentCallContext,Arguments);
                CurrentCallStack.back().ArgumentStack.push_back(std::move(Result));
            }
            catch(ContinueUnwind const& e)
            {
                   
            }
            catch(std::exception const& e)
            {
                p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState, String(e.what())),true);
            }
            if(CurrentState.CurrentCallContext.m_ThreadPaused)
            {
                m_ThreadingState.TempSuspend(CurrentState.AssociatedThread,false);
                CurrentState.CurrentCallContext.m_ThreadPaused = false;
            }
            assert(CurrentCallStack.back().ArgumentStack.size() == 0 || !CurrentCallStack.back().ArgumentStack.back().IsType<List>() || CurrentCallStack.back().ArgumentStack.back().GetRef<List>() != nullptr);

        }
        else if(ObjectToCall.IsType<Lambda>())
        {
            Lambda& AssociatedLambda = ObjectToCall.GetType<Lambda>();
            if(Arguments.size() < AssociatedLambda.Definition->Arguments.size())
            {
                //throw std::runtime_error("To few arguments for function call with function \""+AssociatedLambda.Name+"\"");
                p_EmitSignal(CurrentState, Value::EmplaceExternal<StackTrace>( CurrentState,"To few arguments for function call with function \""+GetSymbolString(AssociatedLambda.Name.ID)+"\""),true);
                return;
            }
            if(AssociatedLambda.Definition->RestParameter == 0)
            {
                if(Arguments.size() > AssociatedLambda.Definition->Arguments.size())
                {
                    //throw std::runtime_error("To many arguments for function call \""+AssociatedLambda.Name+"\"");
                    p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"To many arguments for function call \""+GetSymbolString(AssociatedLambda.Name.ID)+"\""),true);
                    return;
                }
            }
            StackFrame NewStackFrame(OpCodeExtractor(AssociatedLambda.Definition->Instructions));
            NewStackFrame.StackScope = MakeRef<Scope>(*AssociatedLambda.Definition);
            NewStackFrame.StackScope->GetStackInfo().IsStackScope = true;
            NewStackFrame.StackScope->SetParentScope(AssociatedLambda.AssociatedScope);
            int CurrentLocalIndex = 0;
            for(int i = 0; i < AssociatedLambda.Definition->Arguments.size();i++)
            {
                //NewStackFrame.StackScope->OverrideVariable(AssociatedLambda.Definition->Arguments[i].ID,Arguments[i]);   
                NewStackFrame.StackScope->SetLocalDirect(CurrentLocalIndex,Arguments[i]);   
                CurrentLocalIndex++;
            }
            if(AssociatedLambda.Definition->RestParameter != 0)
            {
                List RestList;
                for(int i = AssociatedLambda.Definition->Arguments.size();i < Arguments.size();i++)
                {
                    RestList.push_back(Arguments[i]);
                }
                //NewStackFrame.StackScope->OverrideVariable(AssociatedLambda.Definition->RestParameter,std::move(RestList));
                NewStackFrame.StackScope->SetLocalDirect(CurrentLocalIndex,std::move(RestList));
                CurrentLocalIndex++;
            }
            if(AssociatedLambda.Definition->EnvirParameter != 0)
            {
                //NewStackFrame.StackScope->OverrideVariable(AssociatedLambda.Definition->EnvirParameter,CurrentCallStack.back().StackScope);
                NewStackFrame.StackScope->SetLocalDirect(CurrentLocalIndex,CurrentCallStack.back().StackScope);
                CurrentLocalIndex++;
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
            NewStackFrame.StackScope->SetParentScope(NewInstance->AssociatedClass->Envir);
            Value NewValue = NewInstance;
            NewStackFrame.StackScope->OverrideVariable(p_GetSymbolID("INIT"),NewValue);
            if(NewInstance->AssociatedClass->Constructor != nullptr)
            {
                FuncArgVector Args = {NewValue};
                for(auto& Arg : Arguments)
                {
                    Args.push_back(Arg);
                }
                p_Invoke(*NewInstance->AssociatedClass->Constructor,Args,CurrentState,Setting,IsTrapHandler);
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
                p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"No method associated with the argument list for generic \""+GetSymbolString(GenericToInvoke.Name.ID)+"\""),true);
                return;
            }
            p_Invoke(*Callable,Arguments,CurrentState,Setting,IsTrapHandler);
            //should only invoke trap handler at the lowest point of the recursive call to p_Invoke
            return;
        }
        else
        {
            //throw std::runtime_error("Cannot invoke object");   
            p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"Cannot invoke object"),true);
        }
    }
    
    Value Evaluator::p_Eval(ExecutionState& CurrentState,int ReturnIndex)
    {
        Value ReturnValue;
        CurrentState.CurrentCallContext.m_AssociatedEvaluator = this;
        CurrentState.CurrentCallContext.m_CurrentState = &CurrentState;
        auto& StackFrames = CurrentState.StackFrames;
        while(StackFrames.size() != 0)
        {
            assert(StackFrames.size() >=  ReturnIndex);
            if(m_ThreadingState.MultipleThreadsActive())
            {
                m_ThreadingState.WaitForTurn(CurrentState.AssociatedThread,&CurrentState);
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
                    if (CurrentState.StackFrames.back().ActiveUnwindProtectors.size() == 0)
                    {
                        CurrentState.PopFrame();
                        if(StackFrames.size() == ReturnIndex)
                        {
                            throw ContinueUnwind();
                        }
                        continue;
                    }
                    else if (CurrentState.StackFrames.back().Unwinding == false)
                    {
                        CurrentState.StackFrames.back().ExecutionPosition.SetIP(CurrentState.StackFrames.back().ActiveUnwindProtectors.back().Begin);
                        CurrentState.StackFrames.back().Unwinding = true;
                    }
                }
            }


            StackFrame& CurrentFrame = CurrentState.StackFrames.back();
            if(CurrentFrame.PopExtra != 0)
            {
                assert(CurrentFrame.ArgumentStack.size() >= CurrentFrame.PopExtra);
                for(int i = 0; i < CurrentFrame.PopExtra;i++)
                {
                    CurrentFrame.ArgumentStack.pop_back();
                }
                CurrentFrame.PopExtra = 0;
            }
            if(CurrentFrame.ExecutionPosition.Finished())
            {
                assert(CurrentFrame.ArgumentStack.size() == 1);
                assert(CurrentFrame.ActiveSignalHandlers.size() == 0);
                assert(CurrentFrame.ActiveUnwindProtectors.size() == 0);
                assert(CurrentFrame.SignalHandlerBunchSize.size() == 0);
                ReturnValue = std::move(CurrentFrame.ArgumentStack.back());
                CurrentState.PopFrame();
                if(StackFrames.size() != ReturnIndex)
                {
                    StackFrames.back().ArgumentStack.push_back(ReturnValue);   
                }
                else
                {
                    return ReturnValue;   
                }
                continue;
            }
            if(CurrentState.StackFrames.size() > 10000)
            {
                throw std::runtime_error("Error in evaluation: amount of stack frames exceeded arbitrary limit of 10 000");
            }

            OpCode& CurrentCode = CurrentFrame.ExecutionPosition.GetCurrentCode();
    
            //Next opcode to execute determine, also determine wheter or not we want to invoke the debugger
            if(m_DebugState.DebuggingActive())
            {
                //trap handler cannot invoke another trap handler
                //both convenience for implementation, and for removing problematic
                //infinite loops
                if(!(CurrentState.TraphandlerIndex == CurrentState.StackFrames.size()))
                {
                    if(!(CurrentState.TraphandlerIndex == CurrentState.StackFrames.size() +1  ))
                    {
                        if(m_DebugState.IsTrapped(CurrentState))
                        {
                            p_InvokeTrapHandler(CurrentState);
                            continue;
                        }
                    }
                    else
                    {
                        //entering hear means that  this opcode previously triggered a trap handle for the current call stack, now we simply 
                        //allow it to continue executing
                        CurrentState.TraphandlerIndex = -1;
                    }
                }
            }
            
            CurrentFrame.ExecutionPosition.Pop();
            if(CurrentCode.IsType<OpCode_Pop>())
            {
                assert(CurrentFrame.ArgumentStack.size() != 0);
                CurrentFrame.ArgumentStack.pop_back();
            }
            else if(CurrentCode.IsType<OpCode_PushVar>())
            {
                OpCode_PushVar& PushCode = CurrentCode.GetType<OpCode_PushVar>();
                Value* VarPointer = nullptr;
                if(!PushCode.Local)
                {
                    VarPointer = CurrentFrame.StackScope->TryGet(PushCode.ID);
                }
                else
                {
                    VarPointer = &CurrentFrame.StackScope->GetLocal(PushCode.ID);
                }

                if(VarPointer == nullptr)
                {
                    p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"Error finding variable with name \""+GetSymbolString(PushCode.ID)+"\""),true);
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
                    Ref<Lambda> NewLambda = MakeRef<Lambda>(LiteralToPush.GetType<Lambda>());
                    NewLambda->AssociatedScope = CurrentFrame.StackScope;
                    LiteralToPush = Value(NewLambda);
                    CurrentFrame.ScopeRetrieved = true;
                }
                else if(LiteralToPush.IsType<String>())
                {
                    LiteralToPush = Value(LiteralToPush.GetType<String>());
                }
                 CurrentFrame.ArgumentStack.push_back(LiteralToPush);
            }
            else if(CurrentCode.IsType<OpCode_Goto>())
            {
                OpCode_Goto& GotoCode = CurrentCode.GetType<OpCode_Goto>();
                assert(GotoCode.NewUnwindSize -1 || GotoCode.NewUnwindSize <= CurrentFrame.ActiveUnwindProtectors.size());
                if(GotoCode.NewUnwindSize == -1 || GotoCode.NewUnwindSize == CurrentFrame.ActiveUnwindProtectors.size())
                {
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
                else
                {
                    //return to this position, and execute the jump after the appropriate 
                    //unwind handlers have been resolved
                    NonLocalGotoInfo& GotoInfo = CurrentFrame.StoredGotos.emplace_back();
                    GotoInfo.TargetUnwindDepth = GotoCode.NewUnwindSize;
                    GotoInfo.ReturnAdress = CurrentFrame.ExecutionPosition.GetIP()-1;
                    GotoInfo.StackSize = CurrentFrame.ArgumentStack.size();
                    CurrentFrame.ExecutionPosition.SetIP(CurrentFrame.ActiveUnwindProtectors.back().Begin);
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
                OpCode_CallFunc&  CallFuncCode = CurrentCode.GetType<OpCode_CallFunc>();
                Value FunctionToCall = std::move(CurrentFrame.ArgumentStack[CurrentFrame.ArgumentStack.size()-(CallFuncCode.ArgumentCount+1)]);
                FuncArgVector Arguments;
                for(int i = 0; i < CallFuncCode.ArgumentCount;i++)
                {
                    Arguments.push_back(std::move(CurrentFrame.ArgumentStack[CurrentFrame.ArgumentStack.size()-CallFuncCode.ArgumentCount+i]));
                }
                CurrentFrame.ArgumentStack.resize(CurrentFrame.ArgumentStack.size()-(CallFuncCode.ArgumentCount+1));
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
                auto const& SetCode = CurrentCode.GetType<OpCode_Set>();
                //first value of stack is symbol, second is value
                assert(CurrentFrame.ArgumentStack.size() >= 2);
                Value SymbolToAssign = std::move(*(CurrentFrame.ArgumentStack.end()-1));
                Value AssignedValue = std::move(*(CurrentFrame.ArgumentStack.end()-2));
                CurrentFrame.ArgumentStack.pop_back();
                CurrentFrame.ArgumentStack.pop_back();
                assert(!SymbolToAssign.IsType<List>() || SymbolToAssign.GetRef<List>() != nullptr);
                if(SymbolToAssign.IsType<Symbol>())
                {
                    Value* Variable = nullptr;
                    if(SetCode.LocalSetIndex == -1)
                    {
                        //CurrentFrame.StackScope->SetVariable(SymbolToAssign.GetType<Symbol>().ID,AssignedValue);
                        Variable = CurrentFrame.StackScope->GetOrCreate(SymbolToAssign.GetType<Symbol>().ID);
                    }
                    else
                    {
                        //CurrentFrame.StackScope->SetLocalVariable(SetCode.LocalSetIndex,AssignedValue);
                        Variable = &CurrentFrame.StackScope->GetLocal(SetCode.LocalSetIndex);
                    }
                    assert(Variable != nullptr);
                    if(!Variable->IsType<DynamicVariable>())
                    {
                        *Variable = AssignedValue;
                    }
                    else
                    {
                        DynamicVariable& AssociatedVariable = Variable->GetType<DynamicVariable>();
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
                        p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"signal handler value specifier evaluated to non class type"),true);
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
                auto const& UnwindAddCode = CurrentCode.GetType<OpCode_UnwindProtect_Add>();
                UnwindProtectInfo& NewInfo = CurrentFrame.ActiveUnwindProtectors.emplace_back();
                NewInfo.Begin = UnwindAddCode.UnwindBegin;
                NewInfo.End = UnwindAddCode.UnwindEnd;
                NewInfo.EndStackCount = UnwindAddCode.EndStackCount;
            }
            else if(CurrentCode.IsType<OpCode_UnwindProtect_Begin>())
            {
                CurrentFrame.ProtectDepth += 1;
            }
            else if(CurrentCode.IsType<OpCode_UnwindProtect_Pop>())
            {
                assert(CurrentFrame.ActiveUnwindProtectors.size() > 0);
                CurrentFrame.ActiveUnwindProtectors.pop_back();
                CurrentFrame.ProtectDepth -= 1;
                if(CurrentState.UnwindingStack)
                {
                    if(CurrentFrame.ActiveUnwindProtectors.size() > 0)
                    {
                        CurrentFrame.ExecutionPosition.SetIP(CurrentFrame.ActiveUnwindProtectors.back().Begin);
                    }
                }
                //TODO are these mutually exclusive...
                else if(CurrentFrame.StoredGotos.size() > 0)
                {
                    assert(CurrentFrame.StoredGotos.back().TargetUnwindDepth <= CurrentFrame.ActiveUnwindProtectors.size());
                    if(CurrentFrame.ActiveUnwindProtectors.size() == CurrentFrame.StoredGotos.back().TargetUnwindDepth)
                    {
                        CurrentFrame.ExecutionPosition.SetIP(CurrentFrame.StoredGotos.back().ReturnAdress);
                        assert(CurrentFrame.ArgumentStack.size() >= CurrentFrame.StoredGotos.back().StackSize);
                        CurrentFrame.ArgumentStack.resize(CurrentFrame.StoredGotos.back().StackSize);
                        CurrentFrame.StoredGotos.pop_back();
                    }
                    else
                    {
                        //goto next unwind protext
                        CurrentFrame.ExecutionPosition.SetIP(CurrentFrame.ActiveUnwindProtectors.back().Begin);
                    }
                }
                CurrentFrame.Unwinding = false;
            }
            else if(CurrentCode.IsType<OpCode_Unwind>())
            {
                assert(CurrentFrame.SignalFrameIndex != -1 && CurrentFrame.SignalFrameIndex < StackFrames.size());
                //like goto, execute current unwinds depths before
                auto const& UnwindCode = CurrentCode.GetType<OpCode_Unwind>();

                auto& TargetFrame = StackFrames[CurrentFrame.SignalFrameIndex];
                assert(UnwindCode.TargetUnwindDepth -1 || UnwindCode.TargetUnwindDepth <= TargetFrame.ActiveUnwindProtectors.size());
                CurrentState.UnwindForced = false;
                CurrentState.UnwindingStack = true;
                CurrentState.FrameTarget = CurrentFrame.SignalFrameIndex;
                if(UnwindCode.TargetUnwindDepth -1 || UnwindCode.TargetUnwindDepth == TargetFrame.ActiveUnwindProtectors.size())
                {
                    TargetFrame.ExecutionPosition.SetIP(CurrentCode.GetType<OpCode_Unwind>().HandlersEnd);
                    TargetFrame.ArgumentStack.resize(CurrentCode.GetType<OpCode_Unwind>().NewStackSize+1);
                }
                else
                {
                    //return to this position, and execute the jump after the appropriate 
                    //unwind handlers have been resolved
                    NonLocalGotoInfo& GotoInfo = TargetFrame.StoredGotos.emplace_back();
                    GotoInfo.TargetUnwindDepth = UnwindCode.TargetUnwindDepth;
                    GotoInfo.ReturnAdress = UnwindCode.HandlersEnd;
                    GotoInfo.StackSize = UnwindCode.NewStackSize+1;
                    TargetFrame.ExecutionPosition.SetIP(TargetFrame.ActiveUnwindProtectors.back().Begin);
                }
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
                    if(!Arguments[i*3].IsType<Scope>())
                    {
                        //throw std::runtime_error("first part of binding triplet has to be a scope");
                        p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"first part of binding triplet has to be a scope"),true);
                        continue;
                    }   
                    Scope& ScopeToModify = Arguments[i*3].GetType<Scope>();
                    if(!Arguments[i*3+1].IsType<Symbol>())
                    {
                        //throw std::runtime_error("second part of binding triplet has to be a symbol");
                        p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"second part of binding triplet has to be a symbol"),true);
                        continue;
                    }   
                    SymbolID IDToModify = Arguments[i*3+1].GetType<Symbol>().ID;
                    Value* VariableToInspect = ScopeToModify.TryGet(IDToModify);
                    if(VariableToInspect == nullptr)
                    {
                        //throw std::runtime_error("couldn't find dynamic variable  in scope");
                        p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"couldn't find dynamic variable in scope"),true);
                        continue;
                    }
                    if(!VariableToInspect->IsType<DynamicVariable>())
                    {
                        //throw std::runtime_error("variable was not a dynamic variable");
                        p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"variable was not a dynamic variable"),true);
                        continue;
                    }
                    ModifiedBindings.push_back(VariableToInspect->GetType<DynamicVariable>().ID);
                    NewValues.push_back(std::move(Arguments[i*3+2]));
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
                        p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"argument to eval not of type environment"),true);
                        continue;
                    }
                    ScopeToUse = ScopeValue.GetRef<Scope>();
                }
                Value ValueToEvaluate = CurrentFrame.ArgumentStack.back();
                CurrentFrame.ArgumentStack.pop_back();
                try
                {
                    ValueToEvaluate = p_GetExecutableTerm(CurrentState,*ScopeToUse,std::move(ValueToEvaluate));
                }
                catch (ContinueUnwind const& Unwind)
                {
                    //continue loop, and unwind. UncaughtException should be propagated
                    continue;
                }
                Ref<OpCodeList> NewOpcodes;
                try
                {
                    NewOpcodes = MakeRef<OpCodeList>(*ScopeToUse,ValueToEvaluate); 
                }
                catch (std::exception const& e)
                {
                    p_EmitSignal(CurrentState,Value::EmplaceExternal<StackTrace>( CurrentState,"Error evaluating form: "+std::string(e.what())),true);
                }
                if(NewOpcodes != nullptr)
                {
                    StackFrame NewFrame = StackFrame(OpCodeExtractor(NewOpcodes));
                    NewFrame.StackScope = ScopeToUse;
                    CurrentState.StackFrames.push_back(std::move(NewFrame));
                }
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
    Value Evaluator::p_GetDynamicValue(Scope& AssociatedScope,ExecutionState& CurrentState,std::string const& VariableName)
    {
        Value ReturnValue;
        Ref<DynamicVariable> DynamicVar = AssociatedScope.FindVariable(p_GetSymbolID(VariableName)).GetRef<DynamicVariable>();
        auto& Bindings = CurrentState.DynamicBindings[DynamicVar->ID];
        if(Bindings.size() == 0)
        {
            return DynamicVar->DefaultValue;   
        }
        else
        {
            return Bindings.back();
        }
        return ReturnValue;
    }
    Value Evaluator::p_Eval(ExecutionState& CurrentState,Ref<OpCodeList> OpCodes,IPIndex Offset)
    {
        StackFrame NewFrame = StackFrame(OpCodeExtractor(OpCodes));
        NewFrame.ExecutionPosition.SetIP(Offset);
        NewFrame.StackScope = CurrentState.StackFrames.back().StackScope;
        CurrentState.StackFrames.push_back(std::move(NewFrame));
        //always a dummy stack at the top
        return p_Eval(CurrentState,CurrentState.StackFrames.size()-1);
    }
    Value Evaluator::p_Expand(ExecutionState&  CurrentState,Scope& Namespace,Value ValueToExpand)
    {
        if(ValueToExpand.IsType<List>())
        {
            return  p_Expand(CurrentState,Namespace,ValueToExpand.GetType<List>());
        }
        return ValueToExpand;
    }
    Value Evaluator::p_Expand(ExecutionState&  CurrentState,Scope& Namespace,List const& ListToExpand)
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
                if(auto  VarIt = Namespace.TryGet(HeadSymbol.ID); VarIt != nullptr && VarIt->IsType<Macro>())
                {
                    Macro& AssociatedMacro = VarIt->GetType<Macro>();
                    FuncArgVector Arguments;
                    for(int i = 1; i < ListToExpand.size();i++)
                    {
                        Arguments.push_back(ListToExpand[i]);
                    }
                    ReturnValue =  p_Expand(CurrentState,Namespace,Eval(CurrentState,*AssociatedMacro.Callable,std::move(Arguments)));
                    if(ReturnValue.IsType<List>())
                    {
                        ReturnValue.GetType<List>().SetLocation(ListToExpand.GetLocation());   
                        ReturnValue.GetType<List>().SetDepth(ListToExpand.GetDepth());   
                    }
                    return ReturnValue;
                }
            }
        }
        else if(ListToExpand[0].IsType<Macro>())
        {
            Macro const& AssociatedMacro = ListToExpand[0].GetType<Macro>();
            FuncArgVector Arguments;
            for(int i = 1; i < ListToExpand.size();i++)
            {
                Arguments.push_back(ListToExpand[i]);
            }
            ReturnValue =  p_Expand(CurrentState,Namespace,Eval(CurrentState,*AssociatedMacro.Callable,std::move(Arguments)));
            if(ReturnValue.IsType<List>())
            {
                ReturnValue.GetType<List>().SetLocation(ListToExpand.GetLocation());   
                ReturnValue.GetType<List>().SetDepth(ListToExpand.GetDepth());   
            }
            return ReturnValue;
        }
        //head wasn't macro, try to expand children
        if(ListToExpand[0].IsType<Symbol>() && ListToExpand[0].GetType<Symbol>().ID
                == p_GetSymbolID("quote"))
        {
            return ListToExpand;   
        }
        
        List NewList;
        NewList.SetLocation(ListToExpand.GetLocation());
        NewList.SetDepth(ListToExpand.GetDepth());
        //NewList.push_back(ListToExpand[0]);
        for(int i = 0; i < ListToExpand.size();i++)
        {
            if(ListToExpand[i].IsType<List>())
            {
                NewList.push_back(p_Expand(CurrentState,Namespace,ListToExpand[i].GetType<List>()));
            }
            else
            {
                NewList.push_back(ListToExpand[i]);
            }
        }
        return NewList;
    }
    //infinite for recursive lists...
    inline void h_AssignDepths(Value& CurrentValue,int CurrentDepth)
    {
        if(CurrentValue.IsType<List>())
        {
            CurrentValue.GetType<List>().SetDepth(CurrentDepth);
            for(auto& Elem : CurrentValue.GetType<List>())
            {
                if(Elem.IsType<List>())
                {
                    Elem.GetType<List>().SetDepth(CurrentDepth+1);
                    h_AssignDepths(Elem,CurrentDepth+1);
                }   
            }
        }
    }
    Value Evaluator::p_GetExecutableTerm(ExecutionState&  CurrentState,Scope& Namespace,Value ValueToExpand)
    {
        h_AssignDepths(ValueToExpand,0);
        return p_Expand(CurrentState,Namespace,std::move(ValueToExpand));
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
    String h_UnescapeString(String const& StringToEscape)
    {
        String ReturnValue;
        size_t ParseOffset = 0;
        while(ParseOffset < StringToEscape.size())
        {
            size_t NextSlash = StringToEscape.find('\\',ParseOffset);
            if(NextSlash == StringToEscape.npos)
            {
                break;   
            }
            //assert(NextSlash != StringToEscape.size()-1);
            if(NextSlash == StringToEscape.size() -1)
            {
                break;   
            }
            ReturnValue.insert(ReturnValue.end(),StringToEscape.begin()+ParseOffset,StringToEscape.begin()+NextSlash);
            if(StringToEscape[NextSlash+1] == 'n')
            {
                ReturnValue += '\n';   
            }
            else if(StringToEscape[NextSlash+1] == 'r')
            {
                ReturnValue += '\r';   
            }
            else if(StringToEscape[NextSlash+1] == 't')
            {
                ReturnValue += '\t';   
            }
            else
            {
                ReturnValue += StringToEscape[NextSlash+1];   
            }
            ParseOffset = NextSlash+2;
        }
        if(ParseOffset < StringToEscape.size())
        {
            ReturnValue.insert(ReturnValue.end(),StringToEscape.begin()+ParseOffset,StringToEscape.end());   
        }
        return  ReturnValue;
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
        return h_UnescapeString(ReturnValue);
    }
    Value Evaluator::p_ReadSymbol(ExecutionState&  CurrentState,SymbolID URI,Ref<ReadTable>& Table,MBUtility::StreamReader& Content)
    {
        Value ReturnValue;
        size_t Position = Content.Position();
        //TODO improve...
        std::string SymbolString = Content.ReadWhile([](char CharToRead)
                {
                    return (CharToRead > ' ' && CharToRead < 0x7f) && CharToRead != '"' && CharToRead != '(' && CharToRead != ')' 
                    //kinda hacky/temporary, maybe should add the ability to alter which characters can appear within a symbol, like in common lisp...
                    && CharToRead != '{' && CharToRead != '}' && CharToRead != '[' && CharToRead != ']' && CharToRead != ',';
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
            for(auto const& ExpandPairs : Table->ExpandMappings)
            {
                if(SymbolString.find(ExpandPairs.first) != std::string::npos)
                {
                    ReturnValue = Eval(CurrentState,ExpandPairs.second,{ReturnValue});
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
    List Evaluator::p_ReadList(ExecutionState&  CurrentState,SymbolID URI,Ref<ReadTable>& Table,MBUtility::StreamReader& Content,Value& StreamValue)
    {
        List ReturnValue;
        Location CurrentLocation;
        CurrentLocation.URI = URI;
        CurrentLocation.Position = Content.Position();
        ReturnValue.SetLocation(CurrentLocation);
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
            ReturnValue.push_back(p_ReadTerm(CurrentState,URI,Table,Content,StreamValue));
            p_SkipWhiteSpace(Content);
        }
        return ReturnValue;
    }
    Value Evaluator::p_ReadTerm(ExecutionState&  CurrentState,SymbolID URI,Ref<ReadTable>& Table,MBUtility::StreamReader& Content,Value& StreamValue)
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
            ReturnValue = p_ReadList(CurrentState,URI,Table,Content,StreamValue);
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
        else if(auto ReaderIt = Table->Mappings.find(NextChar); ReaderIt != Table->Mappings.end())
        {
            Content.ReadByte();
            ReturnValue = Eval(CurrentState,ReaderIt->second,{StreamValue});
        }
        else
        {
            ReturnValue = p_ReadSymbol(CurrentState,URI,Table,Content);
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
        if(!(Arguments[0].IsType<ClassDefinition>() || Arguments[0].IsType<ClassInstance>()))
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
            //return Arguments[0];   
            return Context.GetEvaluator().m_BuiltinTypeDefinitions[Value::GetTypeTypeID<ClassDefinition>()];
        }
        else if(Arguments[0].IsType<ClassInstance>())
        {
            return Arguments[0].GetType<ClassInstance>().AssociatedClass;
        }
        return ReturnValue;
    }

    static void InsertAt(List& Target,Int Index,Value Val)
    {
        if(Index < 0 || Index > Target.size())
        {
            throw std::runtime_error("Cannot insert element at index "+std::to_string(Index)+" in list of size "+std::to_string(Target.size()));
        }
        Target.insert(Target.begin()+Index,Val);
    }
    static Int Times_Int(Int lhs,Int rhs)
    {
        return lhs * rhs;
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
                                  "setl",
                                  })
        {
            p_GetSymbolID(String);
        }
        for(auto const& Pair : std::vector<std::pair<std::string,BuiltinFuncType>>{
                    {"print",Print},
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
        p_RegisterBuiltinClass<ClassInstance>("object_t");
        p_RegisterBuiltinClass<Any>("any_t");
        p_RegisterBuiltinClass<ThreadHandle>("thread_t");
        p_RegisterBuiltinClass<Scope>("envir_t");
        p_RegisterBuiltinClass<LispStackFrame>("stackframe_t");
        p_RegisterBuiltinClass<Function,FunctionObject>();
        p_RegisterBuiltinClass<MBUtility::StreamReader>("in-stream_t");
        
        //list
        AddMethod<List>("append",Append_List);
        AddGeneric<InsertAt>("insert-at");
        AddMethod<List>("back",Back_List);
        AddMethod<List,Int>("index",Index_List);
        AddMethod<List>("len",Len_List);
        AddMethod<List>("sort",Sort);
        AddGeneric<Pop_List>("pop");
        AddGeneric<Reverse_List>("reverse");
        //class
        AddMethod<ClassInstance,Symbol>("index",Index_ClassInstance);
        AddGeneric<Slots_ClassInstance>("slots");

        //scope
        AddMethod<Scope,Symbol>("index",Index_Environment);
        AddMethod<Scope,Symbol,Any>("set-var",SetVar_Environment);
        AddMethod<Scope,Scope>("set-parent",SetParent_Environment);
        AddMethod<Scope,Scope>("add-parent",AddParent_Environment);
        AddMethod<Scope,Symbol>("shadow",Shadow_Environment);
        AddGeneric<Vars>("vars");
        //comparisons
        AddMethod<String,String>("eq",Eq_String);
        AddMethod<Symbol,Symbol>("eq",Eq_Symbol);
        AddMethod<bool,bool>("eq",Eq_Bool);
        AddMethod<Int,Int>("eq",Eq_Int);
        AddMethod<ClassDefinition,ClassDefinition>("eq",Eq_Type);
        AddGeneric<Is_Type>("is");
        AddMethod<Any,Any>("eq",Eq_Any);
        AddMethod<Null,Null>("eq",Eq_Null);
        AddMethod<ThreadHandle,ThreadHandle>("eq",Eq_Null);
        AddGeneric<Eq_ThreadHandle>("eq");

        //streams
        AddMethod<MBUtility::StreamReader>("eof",Stream_EOF);
        AddMethod<MBUtility::StreamReader>("peek-byte",Stream_PeakByte);
        AddMethod<MBUtility::StreamReader>("read-byte",Stream_ReadByte);
        AddMethod<MBUtility::StreamReader>("skip-whitespace",Stream_SkipWhitespace);
        AddMethod<MBUtility::StreamReader,Int>("read-bytes",Stream_ReadBytes);
        AddMethod<MBUtility::StreamReader>("read-string",Stream_ReadString);
        AddMethod<MBUtility::StreamReader>("read-symbol",Stream_Symbol);
        AddMethod<MBUtility::StreamReader>("read-number",Stream_ReadNumber);
        AddMethod<MBUtility::StreamReader>("read-line",Stream_ReadLine);
        AddMethod<MBUtility::StreamReader,String>("read-until",Stream_ReadUntil);
        AddMethod<MBUtility::MBOctetOutputStream,String>("write",Write_OutStream);
        AddGeneric<Seek_Stream>("seek");
        AddGeneric<Offset_Stream>("offset");
        AddGeneric<Flush>("flush");

        AddMethod<String>("out-stream",OutStream_String);
        AddMethod<String>("in-stream",InStream_String);
        AddMethod<String>("open",Open_URI);
        AddMethod<String,String>("open",Open_URI);

        //Dict
        AddMethod<Dict,Any>("index",Index_Dict);
        AddMethod<Any,Dict>("in",In_Dict);
        AddMethod<Dict>("keys",Keys_Dict);
        AddGeneric<Len_Dict>("len");
        
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
        AddGeneric<Str_String>("str");
        AddGeneric<Str_ThreadHandle>("str");
        AddGeneric<Int_ThreadHandle>("int");
        AddMethod<StackTrace>("str",Str_StackTrace);
        AddMethod<String>("int",Int_Str);
        AddMethod<String,Int>("substr",Substr);
        AddMethod<String,Int,Int>("substr",Substr);
        AddMethod<String,String>("index-of",IndexOf_StrStr);
        AddMethod<String,String>("append",Append_String);


        //operators
        AddMethod<Int,Int>("<",Less_Int);
        AddMethod<String,String>("<",Less_String);
        AddMethod<Symbol,Symbol>("<",Less_Symbol);

        AddGeneric<&Plus_Int>("plus");
        AddMethod<Int,Int>("minus",Minus_Int);
        AddGeneric<Times_Int>("times");
        AddGeneric<&Plus_String>("plus");
        AddGeneric<&InsertElements>("insert-elements");
        

        //Threading
        AddMemberMethod<Lock,&Lock::Get>("get");
        AddMemberMethod<Lock,&Lock::Notify>("notify");
        AddMemberMethod<Lock,&Lock::Release>("release");
        AddMemberMethod<Lock,&Lock::Wait>("wait");
        AddMethod<ThreadHandle,Int>("sleep",Sleep);
        AddMethod<ThreadHandle>("pause",Pause);
        AddMethod<ThreadHandle>("remove",Remove);
        AddMethod<ThreadHandle>("resume",Resume);
        AddMethod<ThreadHandle>("get-stack-frames",GetStackFrames);
        AddMethod<ThreadHandle>("stack-count",StackCount);
        AddGeneric<GetScope>("get-frame-envir");
        AddGeneric<GetName_Stackframe>("get-frame-name");
        AddGeneric<GetLocation_StackFrame>("get-frame-location");
        AddGeneric<GetDepth_StackFrame>("get-depth");
        AddGeneric<Thread_Handle>("thread-handle");

        m_GlobalScope->SetVariable(p_GetSymbolID("active-threads"),ActiveThreads);
        //Symbols
        AddMethod<Symbol,Int,Symbol>("symbol",Symbol_SymbolInt);
        AddMethod<Symbol,Int>("symbol",Symbol_SymbolInt);
        AddMethod<String>("symbol",Symbol_String);
        //stuff
        AddMethod<Macro,Symbol>("set-name",SetName_Macro);
        AddMethod<Lambda,Symbol>("set-name",SetName_Lambda);
        AddMethod<GenericFunction,Symbol>("set-name",SetName_Generic);
        AddMethod<ClassDefinition,Symbol>("set-name",SetName_ClassDefinition);

        AddMethod<Macro>("name",Name_Macro);
        AddMethod<Lambda>("name",Name_Lambda);
        AddMethod<GenericFunction>("name",Name_Generic);
        AddMethod<ClassDefinition>("name",Name_ClassDefinition);
        AddMethod<GenericFunction>("applicable",Applicable);

        AddMethod<Symbol>("is-special",IsSpecial_Symbol);
        AddMethod<Symbol>("position",Position_Symbol);
        AddMethod<Symbol>("uri",URI_Symbol);
        AddMethod<MBUtility::StreamReader>("position",Stream_Position);
       
        //Filesystem stuff
        AddMethod<String>("canonical",Canonical);
        AddMethod<String>("is-file",IsFile);
        AddMethod<String>("path-id",PathID);

        
        //Readtables
        AddMethod<ReadTable,String>("add-reader-character",AddReaderCharacter);
        AddMethod<ReadTable,String>("remove-reader-character",RemoveReaderCharacter);
        AddMethod<ReadTable,String>("add-character-expander",AddCharacterExpander);
        AddMethod<ReadTable,String>("remove-character-expander",RemoveCharacterExpander);


        m_GlobalScope->SetVariable(p_GetSymbolID("is-repl"),false);
        m_GlobalScope->SetVariable(p_GetSymbolID("*standard-input*"),ConstructDynamicVariable(Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBLSP::TerminalInput>()))));
        m_GlobalScope->SetVariable(p_GetSymbolID("*standard-output*"),ConstructDynamicVariable(Value::EmplacePolymorphic<MBUtility::TerminalOutput,MBUtility::MBOctetOutputStream>()));
        m_GlobalScope->SetVariable(p_GetSymbolID("load-filepath"), ConstructDynamicVariable(std::string("")));
        m_GlobalScope->SetVariable(p_GetSymbolID("load-envir"), ConstructDynamicVariable(Scope()));
        m_GlobalScope->SetVariable(p_GetSymbolID("*READTABLE*"),ConstructDynamicVariable(Value::MakeExternal(ReadTable())));
    }

    Value Evaluator::TestTest BUILTIN_ARGLIST
    {
        return Value::EmplaceExternal<TestClass>();
    }
    Value Evaluator::Write_OutStream BUILTIN_ARGLIST
    {
        Value ReturnValue;
        MBUtility::MBOctetOutputStream& OutStream = Arguments[0].GetType<MBUtility::MBOctetOutputStream>();
        String& StringToWrite = Arguments[1].GetType<String>();
        OutStream.Write(StringToWrite.data(),StringToWrite.size());
        return ReturnValue;
    }
    bool Evaluator::Flush(MBUtility::MBOctetOutputStream& OutStream)
    {
        OutStream.Flush();
        return false;
    }
    bool Evaluator::Close(MBUtility::MBFileOutputStream& OutStream)
    {
        //TODO implement
        return false;
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
        return Value::EmplacePolymorphic<i_ValueStringStream,MBUtility::MBOctetOutputStream>(Arguments[0]);
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
            return Value::EmplacePolymorphic<MBUtility::SearchableStreamReader,MBUtility::StreamReader,MBUtility::MBOctetInputStream,MBUtility::MBSearchableInputStream>( 
                        std::make_unique<MBUtility::InputFileStream>( URIToOpen));
        }
        else if(Arguments.size() == 2)
        {
            String AccessString = Arguments[1].GetType<String>();
            if(AccessString.find('r') != AccessString.npos)
            {
                return Value::EmplacePolymorphic<MBUtility::SearchableStreamReader,MBUtility::StreamReader,MBUtility::MBOctetInputStream,MBUtility::MBSearchableInputStream>( 
                        std::make_unique<MBUtility::InputFileStream>( URIToOpen));
                //return Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBUtility::InputFileStream>(URIToOpen)));
            }
            else if(AccessString.find('w') != AccessString.npos)
            {
                return Value::EmplacePolymorphic<MBUtility::MBFileOutputStream,MBUtility::MBOctetOutputStream,MBUtility::MBSearchableOutputStream>(
                        URIToOpen);
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
        NewExecState.StackFrames.push_back(StackFrame());
        NewExecState.StackFrames.back().StackScope = Context.GetState().StackFrames.back().StackScope;
        FuncArgVector Args;
        for(int i = 1; i < Arguments.size();i++)
        {
            Args.push_back(Arguments[i]);
        }
        //janky...
        //
        NewExecState.StackFrames.push_back(StackFrame());
        NewExecState.StackFrames.back().StackScope = Context.GetState().GetScopeRef();
        NewExecState.CurrentCallContext.m_CurrentState = &NewExecState;
        NewExecState.CurrentCallContext.m_AssociatedEvaluator = &Context.GetEvaluator();
        Context.GetEvaluator().p_Invoke(Arguments[0],Args,NewExecState);
        NewExecState.AssociatedThread = Context.GetEvaluator().m_ThreadingState.GetNextID();
        NewThread.ID = NewExecState.AssociatedThread;
        Context.GetEvaluator().m_ThreadingState.AddThread([&,ExecState=std::move(NewExecState)]() mutable
                {
                    Context.GetEvaluator().p_Eval(ExecState,1);
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
        Context.GetEvaluator().m_ThreadingState.Sleep(Arguments[0].GetType<ThreadHandle>().ID,float(Arguments[1].GetType<Int>())/1000 );
        return Value();
    }
    Value Evaluator::Pause BUILTIN_ARGLIST
    {
        Context.GetEvaluator().m_ThreadingState.Pause(Arguments[0].GetType<ThreadHandle>().ID);
        return Value();
    }
    Value Evaluator::Remove BUILTIN_ARGLIST
    {
        Value ReturnValue = false;
        Context.GetEvaluator().m_ThreadingState.Remove(Arguments[0].GetType<ThreadHandle>().ID);
        return ReturnValue;
    }
    Value Evaluator::Resume BUILTIN_ARGLIST
    {
        Value ReturnValue = false;
        Context.GetEvaluator().m_ThreadingState.Resume(Arguments[0].GetType<ThreadHandle>().ID);
        return ReturnValue;
    }
    Value Evaluator::ActiveThreads BUILTIN_ARGLIST
    {
        List ReturnValue;
        for(auto const& ID : Context.GetEvaluator().m_ThreadingState.ActiveThreads())
        {
            ReturnValue.push_back(Int(ID));
        }
        return ReturnValue;
    }
    Value Evaluator::StackCount BUILTIN_ARGLIST
    {
        ExecutionState* StateToInspect = nullptr;
        if(Context.GetEvaluator().m_ThreadingState.CurrentID() == Arguments[0].GetType<ThreadHandle>().ID)
        {
            StateToInspect = &Context.GetState();
        }
        else
        {
            StateToInspect = Context.GetEvaluator().m_ThreadingState.GetState(Arguments[0].GetType<ThreadHandle>().ID);
        }
        return StateToInspect->StackFrames.size();
    }
    Value Evaluator::GetStackFrames BUILTIN_ARGLIST
    {
        List ReturnValue;
        ExecutionState* StateToInspect = nullptr;
        if(Context.GetEvaluator().m_ThreadingState.CurrentID() == Arguments[0].GetType<ThreadHandle>().ID)
        {
            StateToInspect = &Context.GetState();
        }
        else
        {
            StateToInspect = Context.GetEvaluator().m_ThreadingState.GetState(Arguments[0].GetType<ThreadHandle>().ID);
        }
        for(int i = 0;  i < StateToInspect->StackFrames.size();i++)
        {
            auto const& StackFrame = StateToInspect->StackFrames[i];
            if(StackFrame.ExecutionPosition.OpCodeCount() == 0)
            {
                continue;   
            }
            LispStackFrame NewFrame;
            NewFrame.StackScope = StackFrame.StackScope;
            NewFrame.Name = StackFrame.ExecutionPosition.GetName();
            if(StackFrame.ExecutionPosition.Finished())
            {
                NewFrame.Position = StackFrame.ExecutionPosition.GetLocation(StackFrame.ExecutionPosition.OpCodeCount()-1);
                NewFrame.Depth = StackFrame.ExecutionPosition.GetDepth(StackFrame.ExecutionPosition.OpCodeCount()-1);
            }
            else
            {
                NewFrame.Position = StackFrame.ExecutionPosition.GetLocation(StackFrame.ExecutionPosition.GetIP());
                NewFrame.Depth = StackFrame.ExecutionPosition.GetDepth(StackFrame.ExecutionPosition.GetIP());
            }
            ReturnValue.push_back(Value::EmplaceExternal<LispStackFrame>(std::move(NewFrame)));
        }
        return ReturnValue;
    }
    Ref<Scope> Evaluator::GetScope(LispStackFrame& StackeFrame)
    {
        return StackeFrame.StackScope;
    }
    ThreadHandle Evaluator::Thread_Handle(Int ID)
    {
        ThreadHandle ReturnValue;
        ReturnValue.ID = ID;
        return ReturnValue;
    }
    Symbol Evaluator::GetName_Stackframe(LispStackFrame& StackeFrame)
    {
        return  StackeFrame.Name;
    }
    Symbol Evaluator::GetLocation_StackFrame(LispStackFrame& StackeFrame)
    {
        Symbol ReturnValue;
        ReturnValue.SymbolLocation = StackeFrame.Position;
        return ReturnValue;
    }
    Int Evaluator::GetDepth_StackFrame(LispStackFrame& StackeFrame)
    {
        return StackeFrame.Depth;
    }
    Value Evaluator::GetInternalModule BUILTIN_ARGLIST
    {
        Value ReturnValue;
        if(Arguments.size() != 1 || !Arguments[0].IsType<String>())
        {
            throw std::runtime_error("get-internal-module requires exactly 1 argument of type string");
        }
        String& AssociatedString = Arguments[0].GetType<String>();
        if(auto It = Context.GetEvaluator().m_BuiltinModules.find(AssociatedString); It != Context.GetEvaluator().m_BuiltinModules.end())
        {
            if(auto ScopeIt = Context.GetEvaluator().m_LoadedModules.find(AssociatedString); ScopeIt == Context.GetEvaluator().m_LoadedModules.end())
            {
                Context.GetEvaluator().m_LoadedModules[AssociatedString] = It->second->GetModuleScope(Context.GetEvaluator());
            }
            ReturnValue = Value(Context.GetEvaluator().m_LoadedModules[AssociatedString]);
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
    Value Evaluator::URI_Symbol BUILTIN_ARGLIST
    {
        return  Context.GetEvaluator().GetSymbolString(Arguments[0].GetType<Symbol>().SymbolLocation.URI);
    }
    Evaluator::Evaluator()
    {
        p_InternPrimitiveSymbols();
        p_LoadModules();
    }
    void Evaluator::p_LoadModules()
    {
        m_BuiltinModules["lsp-internal"] = std::make_unique<LSPModule>();
        m_BuiltinModules["debug"] = std::make_unique<DebugModule>();
        m_BuiltinModules["text"] = std::make_unique<TextModule>();
        m_BuiltinModules["io"] = std::make_unique<IOModule>();
    }
    Value Evaluator::Load BUILTIN_ARGLIST
    {
        Value ReturnValue = false;
        bool KeepBindings = true;
        if(Arguments.size() ==  0 || Arguments.size() > 2)
        {
            throw std::runtime_error("Load requires 1 argument, the filepath to a source file to be evaluated");   
        }
        if(!Arguments[0].IsType<String>())
        {
            throw  std::runtime_error("Load requires first argument to be a string");   
        }
        if(Arguments.size() > 1)
        {
            if(!Arguments[1].IsType<bool>())
            {
                throw  std::runtime_error("Second argumen to load must be a bool");   
            }
            KeepBindings = Arguments[1].GetType<bool>();
        }
        std::filesystem::path SourceFilepath = Arguments[0].GetType<String>();
        Context.GetEvaluator().p_LoadFile(Context.GetState(),SourceFilepath,KeepBindings);
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
    Value Evaluator::PathID BUILTIN_ARGLIST
    {
        return Symbol(p_PathURI(Context.GetEvaluator(),Arguments[0].GetType<String>()));
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
    Value Evaluator::p_LoadFile(ExecutionState&  CurrentState,std::filesystem::path const& LoadFilePath,bool KeepReadtable)
    {
        Ref<OpCodeList> OpCodes = MakeRef<OpCodeList>();
        if(!std::filesystem::exists(LoadFilePath))
        {
            throw std::runtime_error("Source file \"" +LoadFilePath.generic_string() +"\" doesn't exist");
        }
        //updates  the load-filepath
        
        Ref<DynamicVariable> DynamicLoadFilepath = CurrentState.GetCurrentScope().FindVariable(p_GetSymbolID("load-filepath")).GetRef<DynamicVariable>();
        int OriginalVarDepth = CurrentState.DynamicBindings[DynamicLoadFilepath->ID].size();
        CurrentState.DynamicBindings[DynamicLoadFilepath->ID].emplace_back(Value(std::filesystem::weakly_canonical(LoadFilePath).generic_string()));

        //always load file with new *READTABLE* binding
        Ref<DynamicVariable> ReadTableDyn = CurrentState.GetCurrentScope().FindVariable(p_GetSymbolID("*READTABLE*")).GetRef<DynamicVariable>();
        if(!KeepReadtable)
        {
            CurrentState.DynamicBindings[ReadTableDyn->ID].emplace_back(Value::EmplaceExternal<ReadTable>( ReadTableDyn->DefaultValue.GetType<ReadTable>()));
        }
        else
        {
            auto& Bindings = CurrentState.DynamicBindings[ReadTableDyn->ID];
            if(Bindings.size() == 0)
            {
                Bindings.push_back(ReadTableDyn->DefaultValue);
            }
            else
            {
                Bindings.push_back(Bindings.back());
            }
        }
        Ref<ReadTable> TableRef = CurrentState.DynamicBindings[ReadTableDyn->ID].back().GetRef<ReadTable>();
        
        Value ReturnValue = TableRef;
        Ref<DynamicVariable> DynamicLoadEnvir = CurrentState.GetCurrentScope().FindVariable(p_GetSymbolID("load-envir")).GetRef<DynamicVariable>();
        CurrentState.DynamicBindings[DynamicLoadEnvir->ID].emplace_back(CurrentState.GetScopeRef());
        try
        {
            //CurrentState.GetCurrentScope().SetVariable(p_GetSymbolID("load-filepath"),LoadFilePath.generic_string());
            SymbolID URI = p_PathURI(*this,LoadFilePath);
            std::string Content = MBUtility::ReadWholeFile(LoadFilePath.generic_string());
            Value ReaderValue = Value::MakeExternal(MBUtility::StreamReader(std::make_unique<MBUtility::IndeterminateStringStream>(Content)));
            MBUtility::StreamReader& Reader = ReaderValue.GetType<MBUtility::StreamReader>();
            assert(ReaderValue.IsType<MBUtility::StreamReader>());

            size_t BeginStackSize = CurrentState.StackSize();
            while(!Reader.EOFReached())
            {
                IPIndex InstructionToExecute = OpCodes->Size();
                Value NewTerm = p_GetExecutableTerm(CurrentState,CurrentState.GetCurrentScope() ,p_ReadTerm(CurrentState,URI,TableRef,Reader,ReaderValue));
                p_SkipWhiteSpace(Reader);
                OpCodes->Append(NewTerm);
                //if(NewTerm.IsType<List>())
                //{
                //    OpCodes->Append(NewTerm.GetType<List>());
                //}
                //else
                //{
                //    List ListToEncode = {std::move(NewTerm)};
                //    OpCodes->Append(ListToEncode);
                //}
                p_Eval(CurrentState,OpCodes,InstructionToExecute);
                assert(BeginStackSize == CurrentState.StackSize());
            }
            assert(BeginStackSize == CurrentState.StackSize());
            CurrentState.DynamicBindings[DynamicLoadFilepath->ID].pop_back();
            CurrentState.DynamicBindings[ReadTableDyn->ID].pop_back();
            CurrentState.DynamicBindings[DynamicLoadEnvir->ID].pop_back();
            assert(CurrentState.DynamicBindings[DynamicLoadFilepath->ID].size() == OriginalVarDepth);
        }
        catch(...)
        {
            CurrentState.DynamicBindings[DynamicLoadFilepath->ID].pop_back();
            CurrentState.DynamicBindings[ReadTableDyn->ID].pop_back();
            CurrentState.DynamicBindings[DynamicLoadEnvir->ID].pop_back();
            assert(CurrentState.DynamicBindings[DynamicLoadFilepath->ID].size() == OriginalVarDepth);
            throw;
        }
        return ReturnValue;
    }
    void Evaluator::LoadStd()
    {
        std::filesystem::path StdFile = MBSystem::GetUserHomeDirectory()/".mblisp/libs/std/index.lisp";
        if(std::filesystem::exists(StdFile))
        {
            try
            {
                ExecutionState CurrentState;
                CurrentState.StackFrames.push_back(StackFrame());
                CurrentState.StackFrames.back().StackScope = m_GlobalScope;
                Value ReadTable = p_LoadFile(CurrentState,StdFile);
                m_GlobalScope->FindVariable(p_GetSymbolID("*READTABLE*")).GetType<DynamicVariable>().DefaultValue = ReadTable;
            }
            catch (MBLisp::UncaughtSignal& e)
            {
                std::cout<<"Uncaught signal:";
                Eval(e.AssociatedScope, e.AssociatedScope->FindVariable(GetSymbolID("print")), {e.ThrownValue});
            }
            catch(std::exception const& e)
            {
                throw std::runtime_error("Failed loading standard library: "+std::string(e.what()));
            }
        }
        else
        {
            throw std::runtime_error("Failed loading standard library: file not present on default location "+MBUnicode::PathToUTF8(StdFile));
        }
    }
    void Evaluator::Eval(std::filesystem::path const& SourcePath)
    {
        Ref<Scope> CurrentScope = CreateDefaultScope();
        ExecutionState CurrentState;
        CurrentState.StackFrames.push_back(StackFrame());
        CurrentState.StackFrames.back().StackScope = CurrentScope;
        //set load path
        p_LoadFile(CurrentState,SourcePath);
    }
    void Evaluator::Repl()
    {
        LoadStd();
        auto ReplScope = CreateDefaultScope();
        m_GlobalScope->SetVariable(p_GetSymbolID("is-repl"),true);
        
        SymbolID URI = p_PathURI(*this,std::filesystem::current_path());
        Ref<OpCodeList> OpCodes = MakeRef<OpCodeList>();
        ExecutionState CurrentState;
        Ref<DynamicVariable> LoadFilepath = m_GlobalScope->FindVariable(p_GetSymbolID("load-filepath")).GetRef<DynamicVariable>();
        CurrentState.DynamicBindings[LoadFilepath->ID].push_back(MBUnicode::PathToUTF8(std::filesystem::weakly_canonical( std::filesystem::current_path())));

        Ref<DynamicVariable> StdinValueRef = m_GlobalScope->FindVariable(p_GetSymbolID("*standard-input*")).GetRef<DynamicVariable>();
        Value& StdinValue = StdinValueRef->DefaultValue;
        auto& Stdin  = StdinValue.GetType<MBUtility::StreamReader>();

        CurrentState.StackFrames.push_back(StackFrame());
        CurrentState.StackFrames.back().StackScope = ReplScope;
        Ref<ReadTable> TableRef  =  p_GetDynamicValue(CurrentState.GetCurrentScope(),CurrentState,"*READTABLE*").GetRef<ReadTable>();
        while(true)
        {
            size_t InitialStackSize = CurrentState.StackSize();
            try
            {
                IPIndex InstructionToExecute = OpCodes->Size();
                Value NewTerm = p_GetExecutableTerm(CurrentState,CurrentState.GetCurrentScope() ,p_ReadTerm(CurrentState,URI,TableRef,Stdin,StdinValue));
                OpCodes->Append(NewTerm);
                Print(*this,p_Eval(CurrentState,OpCodes,InstructionToExecute));
                std::cout<<std::endl;
            }
            catch(UncaughtSignal const& e)
            {
                std::cout<<"Uncaught signal: ";
                Print(*this,e.ThrownValue);
                std::cout<<std::endl;
                Unwind(CurrentState,InitialStackSize);
            }
            catch(InvalidCharacter const& e)
            {
                std::cout<<e.what();
                std::cout<<std::endl;
                Stdin.ReadByte();
                Unwind(CurrentState,InitialStackSize);
            }
            catch(std::exception const& e)
            {
                std::cout<<"Uncaught signal: "<<e.what()<<std::endl;
                Unwind(CurrentState,InitialStackSize);
            }
        }
    }
    SymbolID Evaluator::p_PathURI(Evaluator& AssociatedEvaluator,std::string const& Path)
    {
        return p_PathURI(AssociatedEvaluator,std::filesystem::path(Path));
    }
    SymbolID Evaluator::p_PathURI(Evaluator& AssociatedEvaluator,std::filesystem::path const& Path)
    {
        return AssociatedEvaluator.p_GetSymbolID(MBUnicode::PathToUTF8(std::filesystem::canonical(Path)));
    }
    Ref<Scope> Evaluator::CreateDefaultScope()
    {
        Ref<Scope> ReturnValue = MakeRef<Scope>();
        ReturnValue->SetShadowingParent(m_GlobalScope);
        //ReturnValue->SetVariable(p_GetSymbolID("*READTABLE*"),Value::MakeExternal(ReadTable(m_GlobalScope->FindVariable(p_GetSymbolID("*READTABLE*")).GetType<ReadTable>())));
        return ReturnValue;
    }
    DebugState&  Evaluator::GetDebugState()
    {
        return m_DebugState;   
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
        Arguments[0].GetType<Lambda>().Definition->Instructions->SetName(Arguments[1].GetType<Symbol>());
        return Arguments[0];
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
    Value Evaluator::Applicable BUILTIN_ARGLIST
    {
        Value ReturnValue = false;
        if(Arguments.size() == 0 || !Arguments[0].IsType<GenericFunction>())
        {
            throw std::runtime_error("first argument for applicable needs to a generic function");
        }
        GenericFunction& FuncToCheck = Arguments[0].GetType<GenericFunction>();
        Value* Method = nullptr;
        if(Arguments.size() > 1)
        {
            Method = FuncToCheck.GetMethod(&Arguments[1],(&Arguments.back())+1);
        }
        else
        {
            //zero size argument list
            Method = FuncToCheck.GetMethod(&Arguments[1],&Arguments[1]);
        }
        if(Method != nullptr)
        {
            ReturnValue = true;   
        }
        return ReturnValue;
    }
}
