#ifndef __IACV_THREAD_H__
#define __IACV_THREAD_H__

#include <pthread.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <cassert>
#include <atomic>

namespace aicv_infra
{
    class Thread
    {
    public:
        Thread(const std::string &name, bool shouldBoostSched = false, int affinity = -1)
            : m_name(name), mThread(0), mShouldBoostSched(shouldBoostSched), mAffinity(affinity), mRunning(false)
        {
        }

        virtual ~Thread()
        {
            if (mThread != 0)
            {
                pthread_join(mThread, nullptr);
            }
        }

        virtual void Run() = 0;

        bool Start()
        {
            if (mRunning.load())
            {
                std::cerr << "Thread already running" << std::endl;
                return false;
            }

            pthread_attr_t attr;
            pthread_attr_init(&attr);

            if (mAffinity != -1)
            {
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(mAffinity, &cpuset);
                pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
            }

            int ret = pthread_create(&mThread, &attr, ThreadProc, this);
            pthread_attr_destroy(&attr);

            if (ret != 0)
            {
                std::cerr << "Failed to create thread: " << std::strerror(errno) << std::endl;
                return false;
            }

            mRunning.store(true);
            return true;
        }

        void Join()
        {
            if (mThread != 0)
            {
                pthread_join(mThread, nullptr);
                mThread = 0;
                mRunning.store(false);
            }
        }

        bool isRunning() const
        {
            return mRunning.load();
        }

        static void Sleep(int milliseconds)
        {
            usleep(milliseconds * 1000);
        }

        const std::string &GetName() const
        {
            return m_name;
        }

    protected:
        static void *ThreadProc(void *arg)
        {
            Thread *thread = static_cast<Thread *>(arg);
            thread->Run();
            thread->mRunning.store(false);
            return nullptr;
        }

    private:
        std::string m_name;
        pthread_t mThread;
        bool mShouldBoostSched;
        int mAffinity;
        std::atomic<bool> mRunning;
    };
} // namespace aicv_infra

#endif // THREAD_H