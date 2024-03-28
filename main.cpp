#include "TaskExecutor.h"
#include <iostream>
#include <string>
#include <unistd.h> // For sleep()


#include <cereal/archives/json.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <opencv2/core/core.hpp>

#include "Infra.h"
#include "cereal_serialization.h"

// Task function that the executor will run
void SampleTask(void* arg)
{
    std::string message = *static_cast<std::string*>(arg);
    std::cout << "Executing task: " << message << std::endl;
}

int main1()
{
    // Create a task executor instance with a name
    aicv_infra::TaskExecutor executor("SampleExecutor");

    // Start the executor thread
    executor.Start();

    // Create a few sample tasks
    std::string taskMessage1 = "Task 1 running";
    std::string taskMessage2 = "Task 2 running";
    std::string taskMessage3 = "Task 3 running";

    // Schedule the first task
    executor.SetAction(SampleTask);
    executor.SetData(&taskMessage1);
    executor.NotifyNewAction();

    sleep(1); // Wait a bit to simulate time between tasks

    // Schedule the second task
    executor.SetData(&taskMessage2);
    executor.NotifyNewAction();

    sleep(1); // Wait a bit more

    // Schedule the third task
    executor.SetData(&taskMessage3);
    executor.NotifyNewAction();

    // Let the tasks complete
    sleep(2);

    // Stop the executor thread
    executor.Stop();

    // Wait for the executor to finish
    executor.Join();

    return 0;
}

//INFRA_DUMP_JSON("Cloud_" + std::to_string(1)) << processor->planesRes.mCloud;

int main()
{

	std::stringstream ss;
	ss << R"(
{
      "outDirPath": "INFRA",
      "isLogStdout": false,
      "isLogFile": true,
      "isLogCat": false
}
)";

	INFRA::ConfigTree tree;
	boost::property_tree::json_parser::read_json(ss, tree);

	// Init
	INFRA_Init(INFRA::ConfigTree());
	INFRA_UseThreadChannel("Main");

    cv::Mat_<double> m = cv::Mat_<double>::ones(5, 5);
    
    INFRA_DUMP_JSON("Cloud_" + std::to_string(1)) << m << m;


	// DUMP JSON
    /*
	INFRA_DUMP_JSON_OPEN("hftracking_frame_10");
	INFRA_DUMP_JSON_WRITE << cereal::NameValuePair<float>("age", 120);
	INFRA_DUMP_JSON_WRITE << cereal::NameValuePair<std::string>("funny", "no");	
	
	[]() { INFRA_DUMP_JSON_OPEN("hftracking_frame_11"); }();
	INFRA_DUMP_JSON_WRITE << cereal::NameValuePair<std::string>("name", "chen");
	[]() { INFRA_DUMP_JSON_WRITE << cereal::NameValuePair<std::string>("sane", "no"); }();

	// PROBE
	INFRA::Instance().InsertProbe("probe1", 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	bool threadAlive = true;
	std::thread worker([&threadAlive]() {
		int i = 0;
		while (threadAlive)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			INFRA_PROBE_RESPONSE_BEGIN("probe1")
				probe->GetStream() << "test " << i;
			INFRA_PROBE_RESPONSE_END;
			i++;
		}
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	auto probe = INFRA::Instance().InsertProbe("probe1", 100);
	if (probe == nullptr)
	{
		std::cout << "probe not successful";
	}
	else
	{
		std::cout << "probe success: " << probe->GetStream().str() << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	probe = INFRA::Instance().InsertProbe("probe1", 100);
	std::cout << "probe success: " << probe->GetStream().str() << std::endl;

	threadAlive = false;
	worker.join();
    */
}
