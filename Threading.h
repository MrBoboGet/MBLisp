#pragma once
#include "Value.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>

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
            bool Paused = false;
            float SleepDuration = 0;
            //not existing means that the scheduler represents the main thread
            std::unique_ptr<std::thread> SystemThread;
            bool WakedUp = false;
            std::mutex WaitMutex;
            std::condition_variable WaitConditional;
            ExecutionState* ExecutionState = nullptr;
        };
        static constexpr float m_SwapTime = 0.005f;

        ThreadID m_CurrentThread = 0;
        std::atomic<bool> m_Exiting{false};
        std::atomic<int> m_ThreadCount{0};

        std::atomic<float> m_ElapsedTime{0};
        std::atomic<clock_t> m_LastClock;

        std::mutex m_ThreadInfoMutex;
        ThreadID m_CurrentID = 1;
        std::unordered_map<ThreadID,std::shared_ptr<ThreadSchedulingInfo>> m_ActiveThreads;

        void p_ScheduleNext();
        void p_StartNext();
    public:
        ThreadingState();
        ~ThreadingState();
        ThreadingState(ThreadingState const& ) = delete;
        ThreadingState(ThreadingState&& ) = delete;
        ThreadingState& operator=(ThreadingState const&) = delete;

        bool MultipleThreadsActive();
        void WaitForTurn(ThreadID ID,ExecutionState* State);

        void Remove(ThreadID ID);
        ThreadID AddThread(std::function<void()> Func);
        ThreadID GetNextID();
        ThreadID CurrentID();

        std::vector<ThreadID> ActiveThreads();
        ExecutionState* GetState(ThreadID ID);
        
        void Pause(ThreadID ID);
        void Resume(ThreadID ID);
        void Sleep(ThreadID ID,float Duration);
        void Join(ThreadID ID);
    };
};
