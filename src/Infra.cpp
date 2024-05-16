#include "Infra.h"
//#include "Path.h"
#include <boost/property_tree/json_parser.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <cereal/archives/json.hpp>
//#include <flatbuffers/flatbuffers.h>
//#include <direct.h>

#include <sys/stat.h>
#include <sys/types.h>

// Function to create a directory
void createDirectory(const char* path) {
    // The mode argument specifies the permissions to use.
    // It is modified by the process's umask in the usual way: the permissions of the created directory are (mode & ~umask & 0777).
    // Common modes include 0755 for executable directories or 0700 for private directories.
    if (mkdir(path, 0755) == -1) {
        perror("mkdir");
        // Handle error
    }
}

std::string CombinePaths(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;

    // Check if the first path ends with a directory separator
    char lastChar = path1.back();
    bool hasSeparator = (lastChar == '/' || lastChar == '\\');
    
    // Add a directory separator if needed
    std::string separator = hasSeparator ? "" : "/";

    return path1 + separator + path2;
}

#undef HAVE_OPENCV_VIDEOIO

//#include "opencv2/highgui/highgui.hpp"
void INFRA::Init()
{
	auto outDirPath = std::string("INFRA");// config.get_child("outDirPath", ConfigTree("INFRA")).get_value<std::string>();
	//TRACE_INFO("INFRA initialized with: %s", outDirPath.c_str());

	mIsLogStdout = "false";//config.get_child("isLogStdout", ConfigTree("false")).get_value<bool>();
	mIsLogStorage = "true"; //config.get_child("isLogFile", ConfigTree("false")).get_value<bool>();
	mIsDumpScopeTrail = "false";// config.get("isDumpScopeTrail", mIsDumpScopeTrail);

	mOutDirPath = outDirPath;

	mLogsPath = CombinePaths(mOutDirPath, "logs");

	mDumpsPath = CombinePaths(mOutDirPath, ""/*"dumps_unislam"*/);
	std::string logPath = GetLogsPath();

	//TRACE_INFO("INFRA log path: %s", logPath.c_str());

	createDirectory(mOutDirPath.c_str());
	createDirectory(logPath.c_str());
	createDirectory(mDumpsPath.c_str());

	if (0/*config.get<bool>("isLogUnified", false)*/)
	{
		mUnifiedChannel.reset(new Channel("unified", *this));
	}

	mGuiThread.Start();
	mGuiThread.SetData(this);
	mGuiThread.SetAction([](void* data) {((INFRA*)data)->GuiThreadMain(); });
	mGuiThread.NotifyNewAction();
}


void INFRA::Init(const ConfigTree& config)
{
	auto outDirPath = config.get_child("outDirPath", ConfigTree("INFRA")).get_value<std::string>();
	//TRACE_INFO("INFRA initialized with: %s", outDirPath.c_str());

	mIsLogStdout = config.get_child("isLogStdout", ConfigTree("false")).get_value<bool>();
	mIsLogStorage = config.get_child("isLogFile", ConfigTree("false")).get_value<bool>();
	mIsLogTrace = config.get_child("isLogTrace", ConfigTree("false")).get_value<bool>();
	mIsDumpScopeTrail = config.get("isDumpScopeTrail", mIsDumpScopeTrail);	
	mIsSaveLastShownImage = config.get("isSaveLastShownImage", mIsSaveLastShownImage);
	mIsDumpShownImage = config.get("isDumpShownImage", mIsDumpShownImage);
	mOutDirPath = outDirPath;

	mLogsPath = CombinePaths(mOutDirPath, "logs");
	mDumpsPath = CombinePaths(mOutDirPath, "");
	
	std::string logPath = GetLogsPath();

	//TRACE_INFO("INFRA log path: %s", logPath.c_str());

	createDirectory(mOutDirPath.c_str());
	createDirectory(logPath.c_str());
	createDirectory(mDumpsPath.c_str());

	if (config.get<bool>("isLogUnified", false))
	{
		mUnifiedChannel.reset(new Channel("unified", *this));
	}

	mGuiThread.Start();
	mGuiThread.SetData(this);
	mGuiThread.SetAction([](void* data) {((INFRA*)data)->GuiThreadMain(); });	
	mGuiThread.NotifyNewAction();
}

void INFRA::UseThreadChannel(ChannelId ChannelId)
{
	if (!mChannels.count(ChannelId))
	{
		// Create new Channel
		mChannels[ChannelId] = ChannelPtr(new INFRA::Channel(ChannelId, *this));
	}
	mThreadToChannel[std::this_thread::get_id()] = mChannels[ChannelId].get();
}

void INFRA::Log(const std::string& msg)
{
	if (mIsLogStdout)
	{
		std::stringstream ss;
		ss << msg << std::endl;
		SyncStdout(ss.str());
	}
	if (mIsLogStorage)
	{		
		GetThreadChannel().Log(msg);
	}
	if (mUnifiedChannel)
	{
		mUnifiedChannel->Log(msg);
	}
	if (mIsLogTrace)
	{
		//TRACE_INFO("%s", msg.c_str());
	}
}

void INFRA::Show(std::string title, const cv::Mat& img)
{
	mImageQueue.push({ title, new cv::Mat(img) });

	if (mIsSaveLastShownImage)
	{
		std::lock_guard<std::mutex> lock(mImageShowMutex);
		if (mImageShowMap.count(title))
		{
			delete mImageShowMap[title];
		}
		//TODO: Already did copy above...
		mImageShowMap[title] = new cv::Mat(img);
	}	
	if (mIsDumpShownImage)
	{
		auto dumpPath = GetThreadChannel().CreateDumpPath(title, "png");
		cv::imwrite(dumpPath, img);
	}

}

cv::Mat INFRA::GetShownImage(std::string title)
{
	std::lock_guard<std::mutex> lock(mImageShowMutex);
	if (mImageShowMap.count(title))
	{
		return *mImageShowMap[title];
	}

	return cv::Mat3b(24,24,cv::Vec3b(0,0,255));
}

std::vector<std::string> INFRA::GetShownImageTitles()
{
	std::vector<std::string> titles;
	std::lock_guard<std::mutex> lock(mImageShowMutex);
	for (auto& item : mImageShowMap)
	{
		titles.push_back(item.first);
	}
	return titles;
}

// void INFRA::Show3d(std::string title, VisBuffer& visBuffer)
// {	
// 	auto builder = visBuffer.Finish(title);
// 	const uint8_t* buffer = builder->GetBufferPointer();

// 	if (false)
// 	{
// 		// Save immediatly
// 		std::ofstream out;
// 		out.open(R"(C:\GIT\ARIK_UniSLAM\Source\INFRA\test.fb)", std::ios::binary);
// 		out.write((const char*)buffer, builder->GetSize());
// 	}

// 	{
// 		// Store 
// 		std::lock_guard<std::mutex> lock(mObjects3dMutex);
// 		mObjects3d[title].reset(builder);

// 		// Publish
// 		for (auto& queue : mObjects3dQueues)
// 		{
// 			queue.Enqueue(mObjects3d[title]);
// 		}
// 	}
// }

// aicv_infra::ConcurrentQueue<std::shared_ptr<flatbuffers::FlatBufferBuilder>>& INFRA::RegisterObjects3dStream()
// {
// 	std::lock_guard<std::mutex> lock(mObjects3dMutex);
// 	//TRACE_INFO("Registering Objects 3d Stream now");
// 	mObjects3dQueues.emplace_back();
// 	return mObjects3dQueues.back();
// }

// void INFRA::UnregisterObjects3dStream(aicv_infra::ConcurrentQueue<std::shared_ptr<flatbuffers::FlatBufferBuilder>>&)
// {
// 	//TODO
// }

// std::shared_ptr<flatbuffers::FlatBufferBuilder> INFRA::GetShown3d(std::string title)
// {
// 	std::lock_guard<std::mutex> lock(mObjects3dMutex);
// 	if (mObjects3d.count(title))
// 	{
// 		return mObjects3d[title];
// 	}
// 	return nullptr;
// }

// std::vector<std::string> INFRA::GetShown3dTitles()
// {
// 	std::vector<std::string> titles;
// 	std::lock_guard<std::mutex> lock(mObjects3dMutex);
// 	for (auto& item : mObjects3d)
// 	{
// 		titles.push_back(item.first);
// 	}
// 	return titles;
// }

INFRA::~INFRA()
{
	mGuiThread.Stop();
	mGuiThread.Join();
}

INFRA::Channel::~Channel()
{
	currentArchive.reset();
}

INFRA::Channel::Channel(std::string name, INFRA& INFRA_) : mINFRA(INFRA_)
{
	std::string logPath = CombinePaths(mINFRA.GetLogsPath(), name + ".txt");
	std::string timestampLogPath = CombinePaths(mINFRA.GetLogsPath(), name + ".ts.txt");
	logFile.open(logPath, std::ios_base::out);
	timestampLogFile.open(timestampLogPath, std::ios_base::out);
}

void INFRA::Channel::Log(const std::string& msg)
{
	logFile << msg << std::endl;
	auto ts = std::chrono::system_clock::now();
	timestampLogFile << std::chrono::duration_cast<std::chrono::microseconds>(ts.time_since_epoch()).count() << std::endl;
}

std::string INFRA::Channel::CreateDumpPath(const std::string& name, std::string extension)
{
	std::stringstream ss;
	if (mINFRA.mIsDumpScopeTrail)
	{
		for (auto scope : mScopeStack)
		{
			ss << scope << "_";
		}
	}
	ss << name;

	auto uniqueStr = ss.str() + extension;
	if (mDumpNameOccurances.count(uniqueStr))
	{
		auto occurance = mDumpNameOccurances.at(uniqueStr);
		if (occurance > 0)
		{			
			ss << "_" << (occurance + 1);
		}
		mDumpNameOccurances.at(uniqueStr) = occurance + 1;
	}
	else
	{
		mDumpNameOccurances[uniqueStr] = 0;
	}	

	ss << "." << extension;

	return CombinePaths(mINFRA.GetDumpsPath(), ss.str());
}

MoveableOfstream INFRA::Channel::Dump(const std::string& name)
{
	auto outPath = CreateDumpPath(name);
	MoveableOfstream out(new std::ofstream(outPath));
	//INFRA_Log("Dump: " + outPath);
	return out;
}

void INFRA::Channel::ResetArchive(std::string name)
{
	currentArchive.reset();
	currentArchiveFile = Dump(name);
	currentArchive.reset(new cereal::JSONOutputArchive(*currentArchiveFile.get()));
}

void INFRA::GuiThreadMain()
{
	while (mGuiThread.isRunning())
	{
		if (!mImageQueue.empty())
		{
			auto& pair = mImageQueue.front();			
			cv::imshow(pair.first, *pair.second);
			mImageQueue.pop();
			delete(pair.second);			
		}
		cv::waitKey(1);
	}	
}

INFRA& INFRA_Instance()
{
	return INFRA::Instance();
}

void INFRA_Init(const INFRA::ConfigTree& config)
{
	INFRA::Instance().Init(config);
}

void INFRA_Init_AICV()
{
	INFRA::Instance().Init();
}

void INFRA_Log(const std::string& msg)
{
	INFRA::Instance().Log(msg);
}

void INFRA_Show(std::string title, const cv::Mat& img)
{
	INFRA::Instance().Show(title, img);
}

// void INFRA_Show_3d(std::string title, VisBuffer& obj)
// {
// 	INFRA::Instance().Show3d(title, obj);
// }

void INFRA_UseThreadChannel(INFRA::ChannelId channelId)
{
	INFRA::Instance().UseThreadChannel(channelId);
}

Probe* INFRA_GetProbe(ProbeId probeId)
{
	return INFRA::Instance().GetProbe(probeId);
}

void INFRA_Destroy()
{
	if(sINFRA_Instance != nullptr)
		delete sINFRA_Instance;
	sINFRA_Instance = nullptr;
}

INFRA* sINFRA_Instance = nullptr;