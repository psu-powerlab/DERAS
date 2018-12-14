#include "include/logger.h"
#include <iostream>
#include <fstream>
#include <ctime>

// the constructor starts the message with DateTime
Logger::Logger (std::string context, std::string path) 
	: context_(context), path_(path) {
	msg_ =  Logger::GetDateTime () + '\t';
}  // end constructor

// becuase the logger object is constructor inline, it is destroyed at the end
// of the line which then writes the message to the log file.
Logger::~Logger () {
	Logger::ToFile ();
}  // end destructor

// To File
// - constructs the filename based on the date and context of the logger
void Logger::ToFile  () {
	std::string file_name = path_ + context_ + "_" + Logger::GetDate () 
		+ ".log";
	std::ofstream output_file(file_name, std::ios_base::app);
	if (output_file.is_open()) {
		output_file << msg_ << '\n';
	}
	output_file.close();
}  // end To File

// Get Date
// - gets the local time date for the file name
std::string Logger::GetDate () {
    time_t now = time(0);
    struct tm ts = *localtime(&now);
    char buf[100];
    strftime(buf, sizeof(buf), "%F", &ts);
    return std::string(buf);
} // end Get Date

// Get Date Time
// - gets the local date and time for indexing of the data
std::string Logger::GetDateTime () {
    time_t now = time(0);
    struct tm ts = *localtime(&now);
    char buf[100];
    strftime(buf, sizeof(buf), "%F %T", &ts);
    return std::string(buf);
} // end Get Date Time
