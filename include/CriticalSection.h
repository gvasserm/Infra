#ifndef __CRITICALSELECTION_H__
#define __CRITICALSELECTION_H__

#include <pthread.h>

namespace aicv_infra
{
    class CriticalSection
    {
    public:
        CriticalSection()
        {
            pthread_mutex_init(&mutex, NULL);
        }

        ~CriticalSection()
        {
            pthread_mutex_destroy(&mutex);
        }

        void Lock()
        {
            pthread_mutex_lock(&mutex);
        }

        void Unlock()
        {
            pthread_mutex_unlock(&mutex);
        }

    private:
        pthread_mutex_t mutex;
    };

    class CriticalSectionLocker
    {
    public:
        explicit CriticalSectionLocker(CriticalSection &cs) : criticalSection(cs), isLocked(true)
        {
            criticalSection.Lock();
        }

        ~CriticalSectionLocker()
        {
            if (isLocked)
            {
                criticalSection.Unlock();
            }
        }

        void Unlock()
        {
            if (isLocked)
            {
                criticalSection.Unlock();
                isLocked = false;
            }
        }

        void Relock()
        {
            if (!isLocked)
            {
                criticalSection.Lock();
                isLocked = true;
            }
        }

    private:
        CriticalSection &criticalSection;
        bool isLocked;
    };
} // namespace aicv_infra

#endif