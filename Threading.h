#pragma once
#include "Value.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>
#include <map>

#include <MBUtility/MOFunction.h>

namespace MBLisp
{
    class Lock
    {
        std::recursive_mutex m_LockMutex;
        std::condition_variable_any m_LockConditional;
        std::atomic<bool> m_WasNotified = false;
    public:
        Value Wait()
        {
            while(true)
            {
                std::unique_lock<std::recursive_mutex> CurrentLock(m_LockMutex);
                m_LockConditional.wait(CurrentLock);
                if(m_WasNotified)
                {
                    m_WasNotified = false;   
                    break;
                }
            }
            return Value();
        }
        Value Get()
        {
            m_LockMutex.lock();
            return Value();
        }
        Value Release()
        {
            m_LockMutex.unlock();
            return Value();
        }
        Value Notify()
        {
            m_WasNotified.store(true);
            m_LockConditional.notify_one();
            return Value();
        }
    };

    struct ExecutionState;
    class ThreadingState
    {
        struct ThreadSchedulingInfo
        {
            bool HardPaused = false;
            bool TemporaryPaused = false;
            float SleepDuration = 0;
            //not existing means that the scheduler represents the main thread
            std::unique_ptr<std::thread> SystemThread;
            bool WakedUp = false;
            std::condition_variable WaitConditional;
            std::mutex WaitMutex;
            ExecutionState* State = nullptr;
        };

        struct ThreadIterator : MBUtility::Iterator_Base<ThreadIterator,ValueType>
        {
            ThreadingState& State;

            ThreadID m_InitialID = 0;
            typedef std::map<ThreadID,std::shared_ptr<ThreadSchedulingInfo>>::iterator ItType;
            typedef std::map<ThreadID,std::shared_ptr<ThreadSchedulingInfo>>::iterator::value_type ValueType;
            ItType m_CurrentIt;
            ItType m_EndIt;
            bool m_Ended = false;
            bool m_First = true;

            bool IsEqual(ThreadIterator const& rhs) const
            {
                if(rhs.m_Ended && m_Ended)
                {
                    return true;   
                }
                if(rhs.m_Ended != m_Ended)
                {
                    return false;   
                }
                return std::tie(m_InitialID,m_CurrentIt,m_EndIt) == std::tie(rhs.m_InitialID,rhs.m_CurrentIt,rhs.m_EndIt);   
            }
            void Increment()
            {
                if(m_CurrentIt->first == m_InitialID && !m_First)
                {
                    m_Ended = true;
                }
                ++m_CurrentIt;
                m_First = false;
                if(m_CurrentIt == m_EndIt && !m_Ended)
                {
                    m_CurrentIt = State.m_ActiveThreads.begin();
                }
            }
            ValueType& GetRef()
            {
                return *m_CurrentIt;
            }


            ThreadIterator(ThreadingState& NewState,ThreadID InitialId)
                : State(NewState)
            {
                auto It = State.m_ActiveThreads.find(InitialId);
                if(It == State.m_ActiveThreads.end())
                {
                    m_Ended = true;   
                    return;
                }
                m_CurrentIt = It;
                m_InitialID = InitialId;
                m_EndIt = State.m_ActiveThreads.end();
                Increment();
            }
        };

        struct ThreadIteratable
        {
            ThreadingState& State;
            ThreadID m_ID = 0;

            auto begin()
            {
                return ThreadIterator(State,m_ID);
            }
            auto end()
            {
                return ThreadIterator(State,-1);
            }

            ThreadIteratable(ThreadingState& NewState,ThreadID ID)
                : State(NewState)
            {
                m_ID = ID;
            }
        };

        ThreadIteratable Iterate(ThreadID ID)
        {
            return ThreadIteratable(*this,ID);
        }

        static constexpr float m_SwapTime = 0.005f;

        ThreadID m_CurrentThread = 0;
        std::atomic<bool> m_Exiting{false};
        std::atomic<int> m_ThreadCount{0};

        std::atomic<float> m_ElapsedTime{0};
        std::atomic<clock_t> m_LastClock;

        std::mutex m_ThreadInfoMutex;
        ThreadID m_CurrentID = 1;
        std::map<ThreadID,std::shared_ptr<ThreadSchedulingInfo>> m_ActiveThreads;
        std::map<ThreadID,std::shared_ptr<ThreadSchedulingInfo>> m_RemovedThreads;
        void p_ScheduleNext(ThreadSchedulingInfo& CurrentThreadInfo,ThreadID ID,bool Force = false);
        //void p_StartNext();
    public:
        ThreadingState();
        ~ThreadingState();
        ThreadingState(ThreadingState const& ) = delete;
        ThreadingState(ThreadingState&& ) = delete;
        ThreadingState& operator=(ThreadingState const&) = delete;

        bool MultipleThreadsActive();
        void WaitForTurn(ThreadID ID,ExecutionState* State);

        void Remove(ThreadID ID);
        ThreadID AddThread(MBUtility::MOFunction<void()> Func);
        ThreadID GetNextID();
        ThreadID CurrentID();

        std::vector<ThreadID> ActiveThreads();
        ExecutionState* GetState(ThreadID ID);
        void Pause(ThreadID ID);
        void TempSuspend(ThreadID ID,bool TempSuspended);
        void Resume(ThreadID ID);
        void Sleep(ThreadID ID,float Duration);
        void Join(ThreadID ID);
    };
};
