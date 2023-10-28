#include "Threading.h"
#include <iostream>

namespace MBLisp
{
       
    void ThreadingState::p_ScheduleNext(ThreadSchedulingInfo& CurrentThreadInfo)
    {
        clock_t LastClock = m_LastClock.load();
        clock_t CurrentClock = clock();
        double Diff = (CurrentClock-LastClock)/double(CLOCKS_PER_SEC);
        m_ElapsedTime.store(m_ElapsedTime.load() + Diff);
        if(m_ElapsedTime.load() > m_SwapTime ||   CurrentThreadInfo.Paused  == true || CurrentThreadInfo.SleepDuration  >= 0)
        {
            ThreadID NextID = -1;
            float MinSleepTime = 1000;
            for(auto& ThreadIt : m_ActiveThreads)
            {
                if(ThreadIt.second->SleepDuration > 0)
                {
                    ThreadIt.second->SleepDuration -= Diff;   
                    if(ThreadIt.second->SleepDuration < 0)
                    {
                        ThreadIt.second->SleepDuration = 0;
                    }
                }
                if(!ThreadIt.second->Paused && ThreadIt.second->SleepDuration <= 0 && ThreadIt.first != m_CurrentThread)
                {
                    NextID = ThreadIt.first;
                }
                if(!ThreadIt.second->Paused)
                {
                    MinSleepTime = std::min(MinSleepTime,ThreadIt.second->SleepDuration);
                }
            }
            if(NextID == -1 && m_ActiveThreads[m_CurrentThread]->SleepDuration > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(int(MinSleepTime*1000)));
                p_ScheduleNext(CurrentThreadInfo);
                return;
            }
            if(NextID != -1)
            {
                auto ThreadIt = m_ActiveThreads.find(NextID);
                assert(ThreadIt != m_ActiveThreads.end());
                std::shared_ptr<ThreadSchedulingInfo>& ThreadToWake = ThreadIt->second;
                std::lock_guard<std::mutex> WakeMutex(ThreadToWake->WaitMutex);
                assert(NextID != m_CurrentThread);
                m_CurrentThread = NextID;
                ThreadToWake->WakedUp = true;

                ThreadToWake->WaitConditional.notify_one();
            }
            //new thread to schedule
        }
        m_LastClock.store(clock());
    }
    void ThreadingState::p_StartNext()
    {
        clock_t LastClock = m_LastClock.load();
        clock_t CurrentClock = clock();
        double Diff = (CurrentClock-LastClock)/double(CLOCKS_PER_SEC);
        m_ElapsedTime.store(m_ElapsedTime.load() + Diff);
        if(m_ElapsedTime.load() > m_SwapTime)
        {
            ThreadID NextID = -1;
            float LowestSleepTime = std::numeric_limits<float>::max();
            float MinSleepTime = 1000;
            for(auto& ThreadIt : m_ActiveThreads)
            {
                if(ThreadIt.second->SleepDuration > 0)
                {
                    ThreadIt.second->SleepDuration -= Diff;   
                    if(ThreadIt.second->SleepDuration < 0)
                    {
                        ThreadIt.second->SleepDuration = 0;
                    }
                }
                if(ThreadIt.second->Paused || ThreadIt.first == m_CurrentThread)
                {
                    continue;
                }
                if(!ThreadIt.second->Paused && ThreadIt.second->SleepDuration <= 0 && ThreadIt.first != m_CurrentThread)
                {
                    if(ThreadIt.second->SleepDuration < LowestSleepTime)
                    {
                        NextID = ThreadIt.first;
                        LowestSleepTime = ThreadIt.second->SleepDuration;
                    }
                }
            }
            if(NextID != -1)
            {
                std::shared_ptr<ThreadSchedulingInfo> ThreadToWake = m_ActiveThreads[NextID];
                std::lock_guard<std::mutex> WakeMutex(ThreadToWake->WaitMutex);
                ThreadToWake->WakedUp = true;
                ThreadToWake->WaitConditional.notify_one();
                ThreadToWake->SleepDuration = 0;
                m_CurrentThread = NextID;
            }
            //new thread to schedule
        }
        m_LastClock.store(clock());
           
    }
    ThreadingState::ThreadingState()
    {
        auto& NewThread = m_ActiveThreads[0];
        NewThread = std::make_unique<ThreadSchedulingInfo>();
    }
    ThreadingState::~ThreadingState()
    {
        m_Exiting.store(true);
        for(auto& ThreadIt : m_ActiveThreads)
        {
            {
                std::lock_guard<std::mutex> WakeMutex(ThreadIt.second->WaitMutex);
                ThreadIt.second->WaitConditional.notify_one();
                ThreadIt.second->WakedUp = true;
            }
            if(ThreadIt.second->SystemThread != nullptr)
            {
                ThreadIt.second->SystemThread->join();
            }
        }
    }

    bool ThreadingState::MultipleThreadsActive()
    {
        return m_ThreadCount.load() > 0; 
    }
    //TODO  refactor, mega hacky
    void ThreadingState::WaitForTurn(ThreadID ID,ExecutionState* State)
    {
        std::shared_ptr<ThreadSchedulingInfo> ThreadInfo;
        m_ThreadInfoMutex.lock();
        for(auto& RemovedThread : m_RemovedThreads)
        {
            if(RemovedThread.first == ID)
            {
                //Current thread was removed, skip everyting
                throw std::runtime_error("Thread removed");   
            }
            m_ThreadCount.fetch_sub(1);
            if(RemovedThread.second->Paused)
            {
                RemovedThread.second->WaitConditional.notify_one();
            }
            if(RemovedThread.second->SystemThread != nullptr)
            {
                RemovedThread.second->SystemThread->join();
            }
        }
        if(m_RemovedThreads.size() > 0)
        {
            m_ThreadCount.fetch_sub(m_RemovedThreads.size());
            m_RemovedThreads.clear();
        }
        bool Unlocked = false;
        auto ThreadIt = m_ActiveThreads.find(ID);
        if(ThreadIt == m_ActiveThreads.end())
        {
            m_ThreadInfoMutex.unlock();
            throw std::runtime_error("Invalid thread ID when calling WaitForTurn");   
        }
        ThreadInfo = ThreadIt->second;
        if(m_ActiveThreads.size() == 1)
        {
            m_CurrentThread = ID;
            ThreadInfo->WakedUp = false;
            assert(ThreadInfo->SleepDuration <= 0.001);
            assert(m_CurrentThread == ID);
            m_ThreadInfoMutex.unlock();
            return;
        }
        p_ScheduleNext(*ThreadIt->second);
        if(m_CurrentThread != ID)
        {
            std::unique_lock<std::mutex> Lock(ThreadInfo->WaitMutex);
            Unlocked = true;

#if !defined(NDEBUG)
            bool AllArePaused = true;
            for(auto const& Info : m_ActiveThreads)
            {
                if(Info.first != ID && Info.second->Paused == false)
                {
                    AllArePaused = false;
                    break;
                }
            }
            assert(!AllArePaused);
#endif
            //Kinda obscure edge case,  but when adding a new thread that thread might not  have begun exeucting to the
            //point where  it enters this function, and schedukling that thread would mean that this value was set to true 
            //here
            ThreadInfo->WakedUp = false;
            m_ThreadInfoMutex.unlock();
            ThreadInfo->ExecutionState = State;
            while(!ThreadInfo->WakedUp)
            {
                ThreadInfo->WaitConditional.wait(Lock);
            }
            ThreadInfo->WakedUp = false;
            assert(ThreadInfo->SleepDuration <= 0.001);
            assert(m_CurrentThread == ID || (std::cout<<m_CurrentThread<<" "<<ID && false));
        }
        if(!Unlocked)
        {
            ThreadInfo->WakedUp = false;
            assert(ThreadInfo->SleepDuration <= 0.001);
            assert(m_CurrentThread == ID || (std::cout<<m_CurrentThread<<" "<<ID && false));
            m_ThreadInfoMutex.unlock();   
        }
        if(m_Exiting.load())
        {
            throw std::runtime_error("Should exit");   
        }
    }

    void ThreadingState::Pause(ThreadID ID)
    {
        std::lock_guard<std::mutex> Lock(m_ThreadInfoMutex);
        auto It = m_ActiveThreads.find(ID);
        if(It == m_ActiveThreads.end())
        {
            throw std::runtime_error("Invalid thread id: no active thread with id "+std::to_string(ID));
        }
        if(It->second != nullptr)
        {
            std::lock_guard<std::mutex> ThreadLock(It->second->WaitMutex);
            It->second->Paused = true;
        }
        //in order for lock's in builtin functions to work so must this function also take care of the case where
        //ID == m_CurrentThread and schedule a new one.
        if(ID == m_CurrentThread)
        {
            p_StartNext();
        }
    }
    void ThreadingState::Resume(ThreadID ID)
    {
        std::lock_guard<std::mutex> Lock(m_ThreadInfoMutex);
        auto It = m_ActiveThreads.find(ID);
        if(It == m_ActiveThreads.end())
        {
            throw std::runtime_error("Invalid thread id: no active thread with id "+std::to_string(ID));
        }
        if(It->second != nullptr)
        {
            std::lock_guard<std::mutex> ThreadLock(It->second->WaitMutex);
            It->second->Paused = false;
        }
    }
    void ThreadingState::Remove(ThreadID ID)
    {
        if(m_Exiting) 
        {
            return;
        }
        std::shared_ptr<ThreadSchedulingInfo> InfoToRemove;
        {
            std::lock_guard<std::mutex> InternalsLock(m_ThreadInfoMutex);
            auto ThreadIt = m_ActiveThreads.find(ID);
            if(ThreadIt == m_ActiveThreads.end())
            {
                throw std::runtime_error("Invalid thread ID when calling remove");   
            }
            InfoToRemove = std::move(ThreadIt->second);
            m_ActiveThreads.erase(ThreadIt);
            m_RemovedThreads[ID] = std::move(InfoToRemove);
            p_StartNext();
        }
    }
    ThreadID ThreadingState::AddThread(std::function<void()> Func)
    {
        std::lock_guard<std::mutex> InternalsLock(m_ThreadInfoMutex);
        m_ThreadCount.fetch_add(1);
        ThreadID ReturnValue = m_CurrentID;
        m_CurrentID++;
        auto& NewThread = m_ActiveThreads[ReturnValue];
        NewThread = std::make_unique<ThreadSchedulingInfo>();
        NewThread->SystemThread = std::make_unique<std::thread>(
                    [&,Func=std::move(Func)](ThreadID ID)
                    {
                        try
                        {
                            Func();
                        }
                        catch(...)
                        {
                            
                        }
                        try
                        {
                            this->Remove(ID);
                        }
                        catch(...)
                        {
                            
                        }
                    } ,ReturnValue
                );
        return ReturnValue;
    }
    ThreadID ThreadingState::GetNextID()
    {
        std::lock_guard<std::mutex> InternalsLock(m_ThreadInfoMutex);
        return m_CurrentID;
    }
    ThreadID ThreadingState::CurrentID()
    {
        std::lock_guard<std::mutex> InternalsLock(m_ThreadInfoMutex);
        return m_CurrentThread;
    }
    std::vector<ThreadID> ThreadingState::ActiveThreads()
    {
        std::vector<ThreadID> ReturnValue;
        for(auto const& Thread : m_ActiveThreads)
        {
            ReturnValue.push_back(Thread.first);   
        }
        return ReturnValue;
    }
    ExecutionState* ThreadingState::GetState(ThreadID ID)
    {
        std::lock_guard<std::mutex> InternalLock(m_ThreadInfoMutex);
        auto ThreadIt = m_ActiveThreads.find(ID);
        if(ThreadIt == m_ActiveThreads.end())
        {
            throw std::runtime_error("Invalid thread ID when calling GetState");
        }
        assert(ThreadIt->second->ExecutionState  != nullptr);
        return ThreadIt->second->ExecutionState;
    }
    void ThreadingState::Sleep(ThreadID ID,float Duration)
    {
        std::lock_guard<std::mutex> InternalLock(m_ThreadInfoMutex);
        auto ThreadIt = m_ActiveThreads.find(ID);
        if(ThreadIt == m_ActiveThreads.end())
        {
            throw std::runtime_error("Invalid thread ID when calling sleep");
        }
        std::lock_guard<std::mutex> ThreadLock(ThreadIt->second->WaitMutex);
        if(m_ActiveThreads.size() == 1 && ID == m_CurrentThread)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(int(Duration*1000)));
        }
        else
        {
            ThreadIt->second->SleepDuration = Duration;
        }
    }
    void ThreadingState::Join(ThreadID ID)
    {
           
    }
};
