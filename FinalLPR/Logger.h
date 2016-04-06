/*
 * Logger.h
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <cv.h>

#ifndef LOGGER_H_
#define LOGGER_H_

#define logDir logs



class Logger {

public:
/*
 * These levels indicate the threshold at which the corresponding message will be logged.
 *
 * ie. if Message log level is greater than the threshold level then it will be logged
 */
	enum logLevel: int
	{
		OFF = 0,
		ON = 1, //Used to record minimal data. ie test case name, id, and application version
		CRITICAL_LOGGER_ERROR = 2,
		DEBUG_MINIMUM = 3, //Barebones debugging. Log input image and result
		DEBUG_LEVEL_4 = 4,
		DEBUG_LEVEL_5 = 5,
		DEBUG_LEVEL_6 = 6,
		DEBUG_LEVEL_7 = 7,
		DEBUG_LEVEL_8 = 8,
		DEBUG_LEVEL_9 = 9,
		DEBUG_MAXIMUM = 10 //Used to add all debug messages to the log.

	};

	enum blockType: int
	{
		HEADER_H1 = 1,
		HEADER_H2 = 2,
		HEADER_H3 = 3,
		CATEGORY_HEADER = 4, //Used to organize overall report category structure. i.e. "Test Results" "Sys Info"
		METHOD_HEADER = 5,
		MESSAGE = 6,
		IMAGE = 7
	};

	static Logger* lInstance();

	void initialize(std::string testCaseName);
	void log(std::string message, logLevel methodLogLevel);
	void log(blockType bType, cv::Mat imgData, std::string imgAlias, logLevel methodLogLevel);
	void logHeader(blockType bType, std::string data, int headerAlignmentTabMultiplier);
	void logResultsTable(
			std::map<std::string, std::string> expectedAnswerMap,
			std::map<std::string, std::string> actualAnswerMap,
			std::map<std::string, std::string> tCaseLogMap,
			std::map<std::string, bool> tResultMap);
	Logger::logLevel getLogLevel();
	void closeLastHeaderBlock();
	void closeLastTag();
	void closeFile();
	void setLogLevel(logLevel lev);
	bool isInitialized();
	std::string getfQTestCaseName();
	std::string htmlFName();

private:
	Logger(){};
	Logger(Logger const&){}; //copy constructor
	Logger& operator = (Logger const&){}; //assignment constructor
	virtual ~Logger();

	static Logger* loggerInstance;
	std::string htmlFileName;
	std::string fQualifiedFileName;
	std::string fQualDirName;
	std::ofstream logFile;
	std::vector<std::string> lastOpenTag;
	logLevel level = OFF;
	bool initialized = false;

	std::string getTimeStamp();
	void setInitialized(bool initVal);


};

#endif /* LOGGER_H_ */

