#ifndef __PROBE_H__
#define __PROBE_H__

#include "EventWaitHandle.h"

using ProbeId = std::string;

class Probe;

extern "C" Probe *INFRA_GetProbe(ProbeId);

class Probe
{
public:
	Probe(std::string name) : mEvent(true, aicv_infra::EventResetMode::ManualReset) {}

	// Insert the probe for a given time
	bool Insert(int timeout)
	{
		// Clear state
		mStringStream.str("");

		mEvent.Reset();
		if (mEvent.WaitOne(timeout) == true)
		{
			// Was grabbed during the wait
			return true;
		}
		else
		{
			// Wasn't grabbed
			mEvent.Set();
			return false;
		}
	}

	bool Grab()
	{
		return mEvent.WaitOne(0) == false;
	}

	void Release()
	{
		mEvent.Set();
	}

	std::stringstream &GetStream() { return mStringStream; }
	std::mutex &GetMutex() { return mMutex; }

protected:
	aicv_infra::EventWaitHandle mEvent;
	std::stringstream mStringStream;
	std::mutex mMutex;
};

class ProbeGuard
{
public:
	ProbeGuard(Probe *probe)
	{
		mProbe = probe;
	}

	~ProbeGuard()
	{
		if (mProbe)
			mProbe->Release();
	}

public:
	Probe *mProbe;
};

template <typename T = Probe>
class ProbeInsertGuard
{
public:
	ProbeInsertGuard(ProbeId probeId) : mWasGrabbed(false)
	{
		mProbe = INFRA_GetProbe(probeId);
		if (mProbe == nullptr)
			return;

		mProbe->GetMutex().lock();
	}

	~ProbeInsertGuard()
	{
		if (mProbe)
			mProbe->GetMutex().unlock();
	}

	void Insert(int timeout)
	{
		mWasGrabbed = mProbe->Insert(timeout);
	}

	T *Get() const { return (T *)mProbe; }

	bool WasGrabbed() const { return mWasGrabbed; }

protected:
	bool mWasGrabbed;
	Probe *mProbe;
};

#endif