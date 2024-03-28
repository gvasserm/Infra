#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <map>
#include <string>
#include <mutex>
#include <iomanip>
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <boost/property_tree/ptree_fwd.hpp> // For definition: #include "boost/property_tree/json_parser.hpp"
//#include <flatbuffers/flatbuffers.h>

#include "WorkerThread.h"
#include "ConcurrentQueue.h"
#include <cereal/archives/json.hpp>

#include "Probe.h"

namespace cv
{	
	class Mat;
}

namespace cereal
{
	class JSONOutputArchive;
}

// namespace flatbuffers
// {
// 	class FlatBufferBuilder;
// }

// See: https://stackoverflow.com/questions/1597007/creating-c-macro-with-and-line-token-concatenation-with-positioning-macr
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

//#define DISABLE_INFRA

// In GCC4.9 Streams are not moveable, hence the extra layer of complication!
// https://stackoverflow.com/questions/27878495/ofstream-not-working-on-linux
using MoveableOfstream = std::unique_ptr<std::ofstream>;

class INFRA;
extern INFRA* sINFRA_Instance;

// class VisBuffer
// {
// public:
// 	virtual flatbuffers::FlatBufferBuilder* Finish(std::string name) = 0;
// protected:
// 	flatbuffers::FlatBufferBuilder* mBuilder;
// };

class INFRA
{
public:
	using ChannelId = std::string;
	using ConfigTree = boost::property_tree::basic_ptree<std::string, std::string>;
	using Probe = ::Probe;
	using ProbeGuard = ::ProbeGuard;

protected:
	/**
	Contains thread local data for INFRA
	*/
	struct Channel
	{
		std::string name;
		std::ofstream logFile;
		std::ofstream timestampLogFile;
		std::unique_ptr<cereal::JSONOutputArchive> currentArchive;
		MoveableOfstream currentArchiveFile;

		Channel(std::string name, INFRA& INFRA_);

		~Channel();
			
		void Log(const std::string& msg);

		std::string CreateDumpPath(const std::string& name, std::string extension = "dump");
		MoveableOfstream Dump(const std::string& name);

		void ScopePush(std::string name) { mScopeStack.push_back(name); }
		void ScopePop() { mScopeStack.pop_back(); }
		void ResetArchive(std::string name);

	protected:
		INFRA& mINFRA;		
		std::vector<std::string> mScopeStack;
		std::map<std::string, unsigned int> mDumpNameOccurances;
	};

	using ChannelPtr = std::unique_ptr<Channel>;

public:

	static INFRA& Instance()
	{
		if (sINFRA_Instance == nullptr)
		{
			sINFRA_Instance = new INFRA();
		}
		return *sINFRA_Instance;
	}

	void Init(const ConfigTree&);
	void Init();

	void UseThreadChannel(ChannelId);

	std::string GetChannelName() { return GetThreadChannel().name; }

	Channel& GetThreadChannel() { return *mThreadToChannel[std::this_thread::get_id()]; }		

	void Log(const std::string& msg);

	void Show(std::string title, const cv::Mat& img);

	//void Show3d(std::string title, VisBuffer&);
	
	//aicv_infra::ConcurrentQueue<std::shared_ptr<flatbuffers::FlatBufferBuilder>>& RegisterObjects3dStream();

	//void UnregisterObjects3dStream(aicv_infra::ConcurrentQueue<std::shared_ptr<flatbuffers::FlatBufferBuilder>>&);

	
	// std::ofstream Dump(const std::string& name) 
	// { 
	// 	std::ofstream&& fd = GetThreadChannel().Dump(name);
	// 	return std::move(fd);
	// }
	

	Probe* TryGrabProbe(ProbeId probeId)
	{
		auto probe = mProbes[probeId];
		if (probe == nullptr)
			return nullptr;
		
		return probe->Grab() ? probe : nullptr;
	}

	void SetProbe(ProbeId id, Probe* probe)
	{
		// Warning: this is not thread safe
		mProbes[id] = probe;
	}

	Probe* GetProbe(ProbeId probeId)
	{
		return mProbes[probeId];
	}

	cv::Mat GetShownImage(std::string title);

	std::vector<std::string> GetShownImageTitles();

	//std::shared_ptr<flatbuffers::FlatBufferBuilder> GetShown3d(std::string title);
	//std::vector<std::string> GetShown3dTitles();

	~INFRA();

protected:

	void SyncStdout(std::string msg)
	{
		std::lock_guard<std::mutex> lock(mStdoutMutex);
		std::cout << msg;
	}

	INFRA() : 
		mIsLogStdout(false), 
		mIsLogStorage(false), 
		mGuiThread("INFRA_GUI", false), 
		mIsDumpScopeTrail(false), 
		mIsSaveLastShownImage(false), 
		mIsDumpShownImage(false)
	{
		std::cout << "Created INFRA! " << std::endl;
	}

	void GuiThreadMain();

	std::string GetLogsPath() { return mLogsPath; }
	std::string GetDumpsPath() { return mDumpsPath; }

	std::map<std::thread::id, Channel*> mThreadToChannel;
	std::map<ChannelId, ChannelPtr> mChannels;
	std::string mOutDirPath;
	bool mIsLogStdout;
	bool mIsLogStorage;
	::aicv_infra::WorkerThread mGuiThread;
	bool mIsDumpScopeTrail;
	bool mIsSaveLastShownImage;
	bool mIsDumpShownImage;
	bool mIsLogTrace;

	std::mutex mStdoutMutex;
	std::string mLogsPath;
	std::string mDumpsPath;
	
	std::queue<std::pair<std::string, cv::Mat*>> mImageQueue;
	std::unique_ptr<Channel> mUnifiedChannel;
	std::map<ProbeId, Probe*> mProbes;
	//std::map<std::string, std::shared_ptr<flatbuffers::FlatBufferBuilder> > mObjects3d;
	std::mutex mObjects3dMutex;
	std::map<std::string, cv::Mat*> mImageShowMap;
	std::mutex mImageShowMutex;
	//std::deque<aicv_infra::ConcurrentQueue<std::shared_ptr<flatbuffers::FlatBufferBuilder>>> mObjects3dQueues;
};

// Possible Shared Library API
extern "C" void INFRA_Log(const std::string& msg);
extern "C" void INFRA_Show(std::string title, const cv::Mat& img);
//extern "C" void INFRA_Show_3d(std::string title, VisBuffer&);
extern "C" void INFRA_Init(const INFRA::ConfigTree& config);
extern "C" void INFRA_Init_IAR();
extern "C" void INFRA_UseThreadChannel(INFRA::ChannelId channelId);
extern "C" Probe* INFRA_GetProbe(ProbeId);
extern "C" void INFRA_Destroy();

#ifdef DISABLE_INFRA
#define INFRA_INFO(x) ;
#define INFRA_LOG(x) 
#define INFRA_SHOW(name, img) 
#define INFRA_SHOW_3D(name, obj)
#define INFRA_SCOPE(name) 
#define INFRA_PROBE_RESPONSE_BEGIN(probeId) if(0) {
#define INFRA_PROBE_RESPONSE_END }
#define INFRA_DUMP(name)
#define INFRA_DUMP_JSON(name)
#define INFRA_DUMP_JSON_OPEN(name)
#define INFRA_DUMP_OPEN_NODE_2(nodeName) 
#define INFRA_DUMP_OPEN_NODE(active, nodeName) 
#define INFRA_DUMP_CLOSE_NODE(active) 

template <typename T> // Variadic arg
void INFRA_DUMP_JSON_WRITE(bool active, const std::string& name, T&& arg)
{
}

template <typename T, typename... Ts> // Variadic arg
void INFRA_DUMP_JSON_WRITE(bool active, const std::string& name, T&& arg, const std::string& nextName, Ts&&... args)
{
}


#else
#define INFRA_INFO(x) std::stringstream TOKENPASTE2(ss, __LINE__); TOKENPASTE2(ss, __LINE__) << x; INFRA::Instance().Log(TOKENPASTE2(ss, __LINE__).str());
#define INFRA_LOG(x) INFRA_Log(x)
#define INFRA_SHOW(name, img) INFRA_Show(name, img)
#define INFRA_SHOW_3D(name, obj) INFRA_Show_3d(name, obj)
#define INFRA_SCOPE(name) Scope __scope__(name)
#define INFRA_PROBE_RESPONSE_BEGIN(probeId) { INFRA::Probe* probe = INFRA::Instance().TryGrabProbe(probeId); if(probe) { INFRA::ProbeGuard probeGuard(probe); 
#define INFRA_PROBE_RESPONSE_END } }

// if we use INFRA_DUMP_JSON in two lines in the same function, it could have create the same variable twice, which is forbidden.
// So, we use TOKENPASTE2: when we create variable using INFRA_DUMP_JSON, we want it to get unique name, so we include line number in it. 
// "fd" & "ar" are just the arbitrary names for these variables. 
// for line = 40, we get:
// fd40 = INFRA::Instance().GetThreadChannel().Dump(name); *fd40 
// cereal::JSONOutputArchive ar50(fd50); ar50
// the "ar50" in the end of the previous line is for: ar50 << <some variable>.
// EXAMPLE: INFRA_DUMP_JSON("example_dump") << ST_input;
//          translates to:
//          fd40 = INFRA::Instance().GetThreadChannel().Dump(name);  
//			cereal::JSONOutputArchive ar50(fd50); ar50 << ST_input;

#define INFRA_DUMP(name) auto TOKENPASTE2(fd, __LINE__) = INFRA::Instance().GetThreadChannel().Dump(name); *TOKENPASTE2(fd, __LINE__)
#define INFRA_DUMP_JSON(name) INFRA_DUMP(name); cereal::JSONOutputArchive TOKENPASTE2(ar, __LINE__)(*TOKENPASTE2(fd, __LINE__)); TOKENPASTE2(ar, __LINE__)
#define INFRA_DUMP_JSON_OPEN(name)  INFRA::Instance().GetThreadChannel().ResetArchive(name);

//#define INFRA_DUMP_JSON_WRITE(active, name, val) if (active) {*INFRA::Instance().GetThreadChannel().currentArchive.get() << cereal::make_nvp(name, val) ;};
#define INFRA_DUMP_OPEN_NODE_2(nodeName) cereal::JSONOutputArchive& ar = *INFRA::Instance().GetThreadChannel().currentArchive.get(); ar.setNextName(nodeName); ar.startNode();
#define INFRA_DUMP_OPEN_NODE(active, nodeName) if (active) { INFRA_DUMP_OPEN_NODE_2(nodeName);};

#define INFRA_DUMP_CLOSE_NODE(active) if (active) { INFRA::Instance().GetThreadChannel().currentArchive.get()->finishNode();};

template <typename T> // Variadic arg
void INFRA_DUMP_JSON_WRITE_Internal(const std::string& name, T&& arg)
{
	*INFRA::Instance().GetThreadChannel().currentArchive.get() << cereal::make_nvp(name, std::forward<T>(arg));
}

template <typename T, typename... Ts> // Variadic arg
void INFRA_DUMP_JSON_WRITE_Internal(const std::string& name, T&& arg, const std::string& nextName, Ts&&... args)
{
	*INFRA::Instance().GetThreadChannel().currentArchive.get() << cereal::make_nvp(name, std::forward<T>(arg));
	INFRA_DUMP_JSON_WRITE_Internal(nextName, std::forward<Ts>(args)...);
}

template <typename T> // Variadic arg
void INFRA_DUMP_JSON_WRITE(bool active, const std::string& name, T&& arg)
{
	if (active)
	{
		*INFRA::Instance().GetThreadChannel().currentArchive.get() << cereal::make_nvp(name, std::forward<T>(arg));
	}
}

template <typename T, typename... Ts> // Variadic arg
void INFRA_DUMP_JSON_WRITE(bool active, const std::string& name, T&& arg, const std::string& nextName, Ts&&... args)
{
	if (active)
	{
		*INFRA::Instance().GetThreadChannel().currentArchive.get() << cereal::make_nvp(name, std::forward<T>(arg));
		INFRA_DUMP_JSON_WRITE_Internal(nextName, std::forward<Ts>(args)...);
	}
}

#endif

class Scope
{
public:
	Scope(std::string name)
	{
		INFRA::Instance().GetThreadChannel().ScopePush(name);
		INFRA_INFO("<" + name);
	}

	~Scope()
	{
		INFRA_INFO(">");
		INFRA::Instance().GetThreadChannel().ScopePop();
	}
};
