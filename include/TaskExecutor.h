#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include "Thread.h"
#include <pthread.h>
#include <condition_variable>
#include <mutex>
#include <functional>

namespace aicv_infra
{
    class TaskExecutor : public Thread
    {
    public:
        using FUNCTOR = std::function<void(void*)>;
    
    private:
        std::mutex m_mutex;
        std::condition_variable m_condVar;
        FUNCTOR m_funcPtr;
        void* m_ptrData = nullptr;
        bool m_newAction = false;
        bool m_stopRequested = false;
        bool m_executing = false;

        void ExecuteAction()
        {
            if (m_funcPtr != nullptr)
            {
                m_funcPtr(m_ptrData);
            }
        }

    public:
        TaskExecutor(const std::string& name, bool mShouldBoostSched = false, int affinity = -1)
            : Thread(name, mShouldBoostSched, affinity)
        {
        }

        virtual ~TaskExecutor() 
        {
            Stop();
            Join();
        }

        void Run() override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (!m_stopRequested)
            {
                m_condVar.wait(lock, [this] { return m_newAction || m_stopRequested; });

                if (m_newAction)
                {
                    m_executing = true;
                    lock.unlock();
                    ExecuteAction();
                    lock.lock();
                    m_newAction = false;
                    m_executing = false;
                }
            }
        }

        void NotifyNewAction()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_newAction = true;
            m_condVar.notify_one();
        }

        void Stop()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopRequested = true;
            m_condVar.notify_one();
        }

        void SetAction(FUNCTOR func)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_funcPtr = func;
        }

        void SetData(void* ptrData)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_ptrData = ptrData;
        }

        bool IsExecuting() const
        {
            return m_executing;
        }
    };
} // namespace aicv_infra

#endif // TASK_EXECUTOR_H
