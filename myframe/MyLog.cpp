#include "MyLog.h"
#include "MyFlags.h"

MyLog::MyLog()
{
    // output log immediately
    FLAGS_logbufsecs = 0;
    // set the log file to 100MB
    FLAGS_max_log_size = 100;
    FLAGS_stop_logging_if_full_disk = true;
    // init glog
    google::InitGoogleLogging("myframe");
    // log with level >=ERROR is output to stderr
    google::SetStderrLogging(google::GLOG_FATAL);
    // set the path for the log file
    std::string dest_dir = FLAGS_myframe_log_dir + "/info";
    google::SetLogDestination(google::GLOG_INFO, dest_dir.c_str());
}