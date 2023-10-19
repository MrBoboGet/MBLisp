#include "Threading.h"


namespace MBLisp
{
       
    void ThreadingState::p_ScheduleNext()
    {
        clock_t LastClock = m_LastClock.load();
        clock_t CurrentClock = clock();
        double Diff = (CurrentClock-LastClock)/double(CLOCKS_PER_SEC);
        m_ElapsedTime.store(m_ElapsedTime.load() + Diff);
        if(m_ElapsedTime.load() > m_SwapTime)
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
                p_ScheduleNext();
                return;
            }
            if(NextID != -1)
            {
                auto ThreadIt = m_ActiveThreads.find(NextID);
                assert(ThreadIt != m_ActiveThreads.end());
                std::shared_ptr<ThreadSchedulingInfo>& ThreadToWake = ThreadIt->second;
                std::lock_guard<std::mutex> WakeMutex(ThreadToWake->WaitMutex);
                ThreadToWake->WakedUp = true;
                ThreadToWake->WaitConditional.notify_one();
                m_CurrentThread = NextID;
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
    void ThreadingState::WaitForTurn(ThreadID ID)
    {
        std::shared_ptr<ThreadSchedulingInfo> ThreadInfo;
        m_ThreadInfoMutex.lock();
        bool Unlocked = false;
        auto ThreadIt = m_ActiveThreads.find(ID);
        if(ThreadIt == m_ActiveThreads.end())
        {
            m_ThreadInfoMutex.unlock();
            throw std::runtime_error("Invalid thread ID when calling WaitForTurn");   
        }
        ThreadInfo = ThreadIt->second;
        p_ScheduleNext();
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
            assert(ThreadInfo->SleepDuration <= 0);
            m_ThreadInfoMutex.unlock();
            while(!ThreadInfo->WakedUp)
            {
                ThreadInfo->WaitConditional.wait(Lock);
            }
            ThreadInfo->WakedUp = false;
        }
        if(!Unlocked)
        {
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
        }
        m_ThreadCount.fetch_sub(1);
        if(InfoToRemove->Paused)
        {
            InfoToRemove->WaitConditional.notify_one();
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
    void ThreadingState::Sleep(ThreadID ID,float Duration)
    {
        std::lock_guard<std::mutex> InternalLock(m_ThreadInfoMutex);
        auto ThreadIt = m_ActiveThreads.find(ID);
        if(ThreadIt == m_ActiveThreads.end())
        {
            throw std::runtime_error("Invalid thread ID when calling sleep");
        }
        std::lock_guard<std::mutex> ThreadLock(ThreadIt->second->WaitMutex);
        ThreadIt->second->SleepDuration = Duration;
    }
    void ThreadingState::Join(ThreadID ID)
    {
           
    }
};
