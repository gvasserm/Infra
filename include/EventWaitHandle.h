#ifndef __EVENTWAITHANDLE_H__
#define __EVENTWAITHANDLE_H__

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace aicv_infra
{

	enum class EventResetMode
	{
		AutoReset,
		ManualReset
	};

	class EventWaitHandle
	{
	public:
		EventWaitHandle(bool initialState, EventResetMode mode) : signaled(initialState),
																  mode(mode)
		{
		}

		~EventWaitHandle() = default;

		void Reset()
		{
			std::lock_guard<std::mutex> lock(mutex);
			signaled = false;
		}

		void Set()
		{
			std::lock_guard<std::mutex> lock(mutex);
			signaled = true;
			if (mode == EventResetMode::ManualReset)
			{
				condition.notify_all();
			}
			else
			{
				condition.notify_one();
			}
		}

		bool WaitOne(int millisecondsTimeout = std::numeric_limits<int>::max()) {
        std::unique_lock<std::mutex> lock(mutex);
        if (millisecondsTimeout == std::numeric_limits<int>::max()) {
            condition.wait(lock, [this] { return signaled.load(); });
            if (mode == EventResetMode::AutoReset) {
                signaled = false;
            }
            return true;
        } else {
            auto now = std::chrono::steady_clock::now();
            if (condition.wait_until(lock, now + std::chrono::milliseconds(millisecondsTimeout), [this] { return signaled.load(); })) {
                if (mode == EventResetMode::AutoReset) {
                    signaled = false;
                }
                return true;
            }
            return false;
        }
    }

	private:
		std::mutex mutex;
		std::condition_variable condition;
		std::atomic<bool> signaled;
		EventResetMode mode;
	};

} // namespace aicv_infra

#endif