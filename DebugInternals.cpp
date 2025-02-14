#include "DebugInternals.h"
#include "Evaluator.h"
namespace MBLisp
{
    void DebugState::SetTrap(Location TrapLocation)
    {
        m_StateID += 1;
        m_Traps.insert(TrapLocation);
    }
    void DebugState::RemoveTrap(Location TrapLocation)
    {
        m_StateID += 1;
        m_Traps.erase(m_Traps.find(TrapLocation));
    }
    void DebugState::SetTrapSignals(bool TrapSignals)
    {
        m_TrapSignals = TrapSignals;
    }
    void DebugState::SetExcludeThread(ThreadID ThreadToExclude,bool Exclude)
    {
        if(Exclude)
        {
            m_ExcludedThreads.insert(ThreadToExclude);
        }
        else
        {
            auto It = m_ExcludedThreads.find(ThreadToExclude);
            if(It != m_ExcludedThreads.end())
            {
                m_ExcludedThreads.erase(It);
            }
        }
    }
    void DebugState::SetStackSizeTarget(int StackSize)
    {
        m_StackSizeTarget = StackSize;
    }
    void DebugState::SetStackChanged(StackChangedType Type)
    {
        m_StackChangeType = Type;
    }
    void DebugState::SetTrapDepthChanged(DepthChangeType Type)
    {
        m_DepthChangeType = Type;
    }
    void DebugState::SetTargetDepth(int Depth)
    {
        m_TargetDepth = Depth;
    }
    void DebugState::p_UpdateTraps(OpCodeExtractor&  ExecutionPosition)
    {
        if(ExecutionPosition.Modified() || ExecutionPosition.GetDebugID() != m_StateID)
        {
            ExecutionPosition.ClearTraps();
            for(auto const& Trap : m_Traps)
            {
                ExecutionPosition.SetTrap(Trap);   
            }
            ExecutionPosition.SetDebugID(m_StateID);
        }
    }
    bool DebugState::IsTrapped(ExecutionState&  State)
    {
        if(State.InTrapHandler())
        {
            return false;   
        }
        if(m_ExcludedThreads.find(State.AssociatedThread) != m_ExcludedThreads.end())
        {
            return false;   
        }
        auto& ExecutionPosition = State.StackFrames.back().ExecutionPosition;
        bool ReturnValue = false;
        p_UpdateTraps(ExecutionPosition);
        if(ExecutionPosition.IsTrapped(ExecutionPosition.GetIP()))
        {
            return ReturnValue = true;
        }
        if(m_DepthChangeType != DepthChangeType::None)
        {
            int CurrentDepth = ExecutionPosition.GetDepth(ExecutionPosition.GetIP());
            if(m_TargetDepth == -1 && CurrentDepth != -1)
            {
                ReturnValue = true;
            }
            if(m_TargetDepth != -1 && CurrentDepth != -1)
            {
                if(m_DepthChangeType ==  DepthChangeType::Next
                        && CurrentDepth == m_TargetDepth || CurrentDepth < m_TargetDepth)
                {
                    ReturnValue = true;
                }
                else if (m_DepthChangeType == DepthChangeType::Step 
                        && CurrentDepth !=  m_TargetDepth)
                {
                    ReturnValue = true;
                }
            }
            if(m_StackSizeTarget != -1)
            {
                ReturnValue = ReturnValue && m_StackSizeTarget == State.StackSize();   
            }
        }
        if(m_StackChangeType !=  StackChangedType::None && m_StackSizeTarget != -1)
        {
            size_t CurrentStackSize  = State.StackSize();
            if(m_StackChangeType == StackChangedType::New || m_StackChangeType == StackChangedType::Any)
            {
                if(CurrentStackSize  >  m_StackSizeTarget)
                {
                    ReturnValue = true;
                }
            }
            if(m_StackChangeType  ==StackChangedType::Popped || m_StackChangeType ==   StackChangedType::Any)
            {
                if(CurrentStackSize <  m_StackSizeTarget)
                {
                    ReturnValue = true;   
                }
            }
        }
        return ReturnValue;
    }
    bool DebugState::SignalTrapped(ExecutionState&  CurrentState,bool Forced)
    {
        if(CurrentState.InTrapHandler())
        {
            return false;   
        }
        if(m_ExcludedThreads.find(CurrentState.AssociatedThread) != m_ExcludedThreads.end())
        {
            return false;   
        }
        return m_TrapSignals && Forced;
    }
    void DebugState::ClearTraps()
    {
        m_Traps.clear();
        m_StateID += 1;
    }
    bool DebugState::DebuggingActive()
    {
        return m_Traps.size() > 0;
    }
    Ref<Scope> DebugModule::GetModuleScope(Evaluator& AssociatedEvaluator) 
    {
        Ref<Scope> ReturnValue = AssociatedEvaluator.CreateDefaultScope();
        AssociatedEvaluator.AddMethod<Symbol>(ReturnValue,"set-trap",SetTrap);
        AssociatedEvaluator.AddMethod<Symbol>(ReturnValue, "remove-trap",RemoveTrap);
        AssociatedEvaluator.AddMethod<Any>(ReturnValue, "set-trap-handler",SetTrapHandler);
        AssociatedEvaluator.AddMethod<bool>(ReturnValue, "set-trap-signals",SetTrapSignals);
        AssociatedEvaluator.AddMethod<ThreadHandle,bool>(ReturnValue, "set-exclude-thread",SetExcludeThread);
        AssociatedEvaluator.AddMethod<String>(ReturnValue, "set-trap-depth-changed",SetTrapDepthChanged);
        AssociatedEvaluator.AddMethod<Int>(ReturnValue, "set-target-depth",SetTargetDepth);
        AssociatedEvaluator.AddMethod<String>(ReturnValue, "set-trap-stack-changed",SetStackChanged);
        AssociatedEvaluator.AddMethod<Int>(ReturnValue, "set-stack-size-target",SetStackSizeTarget);

        return ReturnValue;
    }
    Value DebugModule::SetTrap BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().SetTrap(Arguments[0].GetType<Symbol>().SymbolLocation);
        return Value();
    }
    Value DebugModule::SetExcludeThread BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().SetExcludeThread(Arguments[0].GetType<ThreadHandle>().ID,Arguments[1].GetType<bool>());
        return Value();
    }
    Value DebugModule::RemoveTrap BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().RemoveTrap(Arguments[0].GetType<Symbol>().SymbolLocation);
        return Value();
    }
    Value DebugModule::SetTrapHandler BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().SetTrapHandler(Arguments[0]);
        return Value();
    }
    Value DebugModule::SetTrapSignals BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().SetTrapSignals(Arguments[0].GetType<bool>());
        return Value();
    }
    Value DebugModule::SetTrapDepthChanged BUILTIN_ARGLIST
    {
        DepthChangeType Type = DepthChangeType::None;
        String& Arg = Arguments[0].GetType<String>();
        if(Arg == "none")
        {
            Type =  DepthChangeType::None;   
        }
        else if(Arg == "step")
        {
            Type = DepthChangeType::Step;   
        }
        else if(Arg == "next")
        {
            Type = DepthChangeType::Next;   
        }
        Context.GetEvaluator().GetDebugState().SetTrapDepthChanged(Type);
        return Value();
    }
    Value DebugModule::SetTargetDepth BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().SetTargetDepth(Arguments[0].GetType<Int>());
        return Value();
    }
    Value DebugModule::SetStackChanged BUILTIN_ARGLIST
    {
        StackChangedType Type = StackChangedType::None;
        String& Arg = Arguments[0].GetType<String>();
        if(Arg == "new")
        {
            Type =  StackChangedType::New;
        }
        else if(Arg == "pop")
        {
            Type = StackChangedType::Popped;
        }
        else if(Arg == "any")
        {
            Type = StackChangedType::Any;
        }
        Context.GetEvaluator().GetDebugState().SetStackChanged(Type);
        return Value();
    }
    Value DebugModule::SetStackSizeTarget BUILTIN_ARGLIST
    {
        Context.GetEvaluator().GetDebugState().SetStackSizeTarget(Arguments[0].GetType<Int>());
        return Value();
    }
}
