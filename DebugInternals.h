#pragma once
#include "OpCodes.h"
#include "Module.h"
#include <unordered_set>

namespace MBLisp
{
    //policy kinda hardcoded...
    enum class DepthChangeType
    {
        Next,
        Step,
        None,
    };
    enum class StackChangedType
    {
        Any,
        New,
        Popped,
        None,
    };

    struct ExecutionState;
    class DebugState
    {
        int m_StateID = 0;
        Value m_Handler;
        std::unordered_set<Location,LocationHasher> m_Traps;
        std::unordered_set<ThreadID> m_ExcludedThreads;
        bool m_TrapSignals = false;


        StackChangedType m_StackChangeType =  StackChangedType::None;
        int m_StackSizeTarget = -1;
        DepthChangeType m_DepthChangeType = DepthChangeType::None;
        int m_TargetDepth = -1;

        void p_UpdateTraps(OpCodeExtractor&  ExecutionPosition);
    public:
        void SetTrapHandler(Value Handler)
        {
            m_Handler = Handler;
        }
        Value GetTrapHandler()
        {
            return m_Handler;   
        }
        //State modifiers
        void SetTrap(Location TrapLocation);
        void RemoveTrap(Location TrapLocation);

        void SetStackSizeTarget(int StackSize);
        void SetStackChanged(StackChangedType Type);

        void SetTrapSignals(bool TrapSignals);

        void SetTargetDepth(int Depth);
        void SetTrapDepthChanged(DepthChangeType Type);

        void SetExcludeThread(ThreadID ThreadToExclude,bool Exclude);
        void ClearTraps();


        //Observers
        bool DebuggingActive();
        bool IsTrapped(ExecutionState&  CurrentState);

    };

    class DebugModule : public Module
    {
        static Value SetTrap BUILTIN_ARGLIST;
        static Value RemoveTrap BUILTIN_ARGLIST;
        static Value SetTrapHandler BUILTIN_ARGLIST;
        static Value SetTrapSignals BUILTIN_ARGLIST;
        static Value SetStackChanged BUILTIN_ARGLIST;
        static Value SetStackSizeTarget BUILTIN_ARGLIST;
        static Value SetTrapDepthChanged BUILTIN_ARGLIST;
        static Value SetTargetDepth BUILTIN_ARGLIST;
        static Value SetExcludeThread BUILTIN_ARGLIST;
    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator) override;
    };
}
