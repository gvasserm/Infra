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

	//INFRA::ConfigTree tree;
	//boost::property_tree::json_parser::read_json(ss, tree);

	// Init
	auto DEVLConfig = INFRA::ConfigTree();
	DEVLConfig.put("outDirPath", "dump");
	INFRA_Init(DEVLConfig);
	INFRA_UseThreadChannel("Main");

	std::vector<double> arr = {1.1, 2.2, 3.3, 4.4, 5.5};
	cameraIntrinsic c;

    cv::Mat_<double> m = (cv::Mat_<double>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
	cv::Mat m_ = (cv::Mat_<double>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    INFRA_DUMP_JSON("Cloud_" + std::to_string(1)) << arr << m << m_ << c;
	INFRA_DUMP_JSON("Cloud_" + std::to_string(1)) << arr;

}
