#ifndef __CONCURRENTQUEUE_H__
#define __CONCURRENTQUEUE_H__

#include <queue>
#include <set>
#include "CriticalSection.h"

namespace aicv_infra
{

	// Defines the expected behavior of a queue, with a "hidden" method of acquiring the actual front of the queue (Which is a dangerous operation and available only for subclasses)
	template <typename T>
	class AbstractQueue
	{
	public:
		virtual ~AbstractQueue() = default;

		virtual int Count() = 0;
		virtual bool IsEmpty() = 0;

		// Inserts a copy of given item at the tail of the queue.
		virtual bool Enqueue(const T &item) = 0;
		// Removes the head of the queue and returns a copy of it, since the removal is also destructive.
		virtual bool TryDequeue(T &result) = 0;

	protected:
		// Returns the head of queue, without removing it. Note that this operation is risky, as another context may remove such element, causing it to become invalid.
		virtual const T &Front(bool calledInternally = false) = 0;

		// Attempts to remove an element. Defaults to no op in case the underlying model doesn't support such operation.
		virtual bool TryErase(const T &toErase)
		{
			return false;
		}
	};

	// thread-safe queue
	template <typename T>
	class ConcurrentQueue : public AbstractQueue<T>
	{
	public:
		virtual ~ConcurrentQueue() = default;

		int Count()
		{
			return (int)m_queue.size();
		}

		bool IsEmpty()
		{
			return m_queue.empty();
		}

		bool Enqueue(const T &item)
		{
			CriticalSectionLocker lock(m_critialSection);
			m_queue.emplace(item);
			return true;
		}

		bool TryDequeue(T &result)
		{
			CriticalSectionLocker lock(m_critialSection);
			if (m_queue.empty())
			{
				return false;
			}

			// result points to some chunk of memory. Assuming that memory is valid prior to entering this function,
			// any operation on result, is actually done on that chunk of memory.
			// Therefore, m_queue.front() followed by m_queue.pop() is safe!

#ifdef I_KNOW_CPLUSPLUS_11_AND_UNDERSTAND_MOVE_SEMANTICS
			result = std::move(Front(true)); // Since pop is going to be called, let the compiler know it is safe to transfer data from temporary m_queue.front() to memory referenced by result.
#else
			result = Front(true);
#endif
			m_queue.pop();
			return true;
		}

	protected:
		const T &Front(bool calledInternally = false)
		{
			if (!calledInternally)
			{
				CriticalSectionLocker lock(m_critialSection);
				return m_queue.front();
			}
			return m_queue.front();
		}

	private:
		CriticalSection m_critialSection;
		std::queue<T> m_queue;
	};

} // namespace aicv_infra

#endif