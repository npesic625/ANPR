/*
 * Logger.cpp
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */


#include "Logger.h"

#include <assert.h>
#include <core/mat.hpp>
#include <core/mat.inl.hpp>
#include <imgcodecs.hpp>
#include <sys/stat.h>
#include <ctime>
#include <sstream>
#include <utility>

// Integration
// #include "TestBed/TestBedSource/TestBedMain.h"

Logger* Logger::loggerInstance = NULL;
const int tabSpaces = 8;
const char* CATEGORY_HEADER_Color = "orange";

Logger* Logger::lInstance()
{
	//if instance is not yet defined
	if (!loggerInstance)
	{
		loggerInstance = new Logger();
	}

	return loggerInstance;
}

Logger::~Logger() {
	// TODO Auto-generated destructor stub

	//close log file if open
	if (!lastOpenTag.empty())
	{
		closeFile();
	}
}

void Logger::initialize(std::string testCaseName)
{
	if (this->isInitialized())
	{
		return;
	}
	//Initialize Logger
	this->setLogLevel(logLevel::OFF);

	//Initial file properties
	this->htmlFileName = testCaseName + "_" + getTimeStamp();
	this->fQualDirName = "Logs/" + htmlFileName;
	if (mkdir(fQualDirName.c_str(), 0775) == -1)
	{
		this->fQualDirName = "Logs";
	}
	this->fQualifiedFileName = fQualDirName + "/" + htmlFileName + ".html";

	//Open file;
	this->logFile.open(fQualifiedFileName.c_str());
	//Setup header
	this->logFile << "<!DOCTYPE html>\n<html>";
	lastOpenTag.push_back("html");
	this->logFile << "<head><title>" << htmlFileName <<"</title></head>\n";
	this->logFile << "<h1>" << htmlFileName <<"</h1>\n";
	this->logFile << "</br></br>\n";

	this->setInitialized(true);
}

std::string Logger::getfQTestCaseName()
{
	return this->fQualifiedFileName;
}

std::string Logger::htmlFName()
{
	return this->htmlFileName;
}

void Logger::setInitialized(bool initVal)
{
	this->initialized = initVal;
}

void Logger::setLogLevel(logLevel lev)
{
	this->level = lev;
}

Logger::logLevel Logger::getLogLevel()
{
	return this->level;
}

bool Logger::isInitialized()
{
	return this->initialized;
}

void Logger::log(std::string message, logLevel methodLogLevel)
{
	if (methodLogLevel <= this->getLogLevel() && isInitialized())
	{
		if (methodLogLevel == CRITICAL_LOGGER_ERROR)
		{
			this->logFile << "<li>[" << getTimeStamp() << "]<font color=\"red\">" << message <<"</font></li>\n";
		}
		else
		{
			this->logFile << "<li>[" << getTimeStamp() << "] " << message <<"</li>\n";
		}
	}
}

/*
 * Log image.
 *
 * @param - Type of block. Should be of type IMAGE = 7
 * @param - Image as Mat
 * @param - Caption that should be displayed as image tag
 * @param - Method level requirement. If requirement is less than or equal logger level then image
 * 			will be logged
 */
void Logger::log(blockType bType, cv::Mat imgData, std::string imgAlias, logLevel methodLogLevel)
{
	if (methodLogLevel <= this->getLogLevel() && isInitialized())
	{
		if (imgData.empty())
		{
			log("Image " + imgAlias + " is empty.",CRITICAL_LOGGER_ERROR);
			return;
		}

		//set image name. Time stamp will help avoid overwriting images.
		std::string imgName = imgAlias + "_" + getTimeStamp() + ".png";
		std::string imgFQName = fQualDirName + "/" + imgName;
		cv::imwrite(imgFQName, imgData);

		//set img title/caption
		log("Image: " + imgAlias,methodLogLevel);
		//set img html code
		this->logFile << "<img src= \"" + imgName + "\" alt=\" [IMAGE]" + imgFQName + "\" title=\"" + imgAlias +"\" />";
	}
}

void Logger::logHeader(blockType bType, std::string data, int headerAlignmentTabMultiplier)
{
	if (this->getLogLevel() != Logger::logLevel::OFF && isInitialized())
	{
	//ensure the input is of header type
	//header type ranges from HEADER_H1 to METHOD_HEADER
	assert((bType >= Logger::blockType::HEADER_H1) && (bType <= Logger::blockType::METHOD_HEADER));

	//add appropriate header info
	switch(bType)
	{
	case Logger::blockType::HEADER_H1:
		this->logFile << "<h1 style = \"padding-left:" << headerAlignmentTabMultiplier * tabSpaces<<"px;\">" << data <<"</h1>\n";
		break;
	case Logger::blockType::HEADER_H2:
		this->logFile << "<h2 style = \"padding-left:" << headerAlignmentTabMultiplier * tabSpaces<<"px;\">" << data <<"</h2>\n";
		break;
	case Logger::blockType::HEADER_H3:
		this->logFile << "<h3 style = \"padding-left:" << headerAlignmentTabMultiplier * tabSpaces<<"px;\">" << data <<"</h3>\n";
		break;
	case Logger::blockType::CATEGORY_HEADER:
		this->logFile << "<h2 style = \"padding-left:" << headerAlignmentTabMultiplier * tabSpaces<<"px; color: " << CATEGORY_HEADER_Color << "\">" << data <<"</h2>\n";
		break;
	case Logger::blockType::METHOD_HEADER:
			break;
	default:
		std::stringstream sStream;
		sStream << "Attempting to log "<< (int) bType << "as a header";
		log(sStream.str(), Logger::logLevel::CRITICAL_LOGGER_ERROR);
		break;
	}

	//init paragraph
	this->logFile << "<p>\n";
	lastOpenTag.push_back("p");

	//add unorganized list tag
	//<ul style="padding-left:680px;">
	this->logFile << "<ul style = \"padding-left:" << (headerAlignmentTabMultiplier + 2) * tabSpaces<<"px;\">" ;
	lastOpenTag.push_back("ul");
	}
}

void Logger::logResultsTable(
		std::map<std::string, std::string> expectedAnswerMap,
		std::map<std::string, std::string> actualAnswerMap,
		std::map<std::string, std::string> tCaseLogMap,
		std::map<std::string, bool> tResultMap)
{
	std::string htmlFName;
	int trueCounter = 0, falseCounter = 0;

	this->logFile<<"<table border=\"1\">";

	//set table header
	this->logFile<<"<tr>";
	this->logFile<<"<th>Testcase</th>";
	this->logFile<<"<th>Result</th>";
	this->logFile<<"<th>Expected Answer</th>";
	this->logFile<<"<th>Actual Answer</th>";
	this->logFile<<"</tr>";

	for(auto& it : tCaseLogMap)
	{
// Integration
//		htmlFName = TestBedMain::split(it.second,'/').back();
		htmlFName = it.second;

		this->logFile<<"<tr>";
		//set testcase name via hyperlink
		this->logFile<<"<td><a href=\"../../" + it.second + "\"> " + htmlFName +"</a>"+"</td>";

		//set test result
		if (tResultMap.at(it.first) == true)
		{
			this->logFile<<"<td><span style=\"color:green;\"> PASS </span></td>";
			trueCounter++;
		}
		else
		{
			this->logFile<<"<td><span style=\"color:red;\"> FAIL </span></td>";
			falseCounter++;
		}

		//set expected answer and actual answer
		this->logFile<<"<td> " + expectedAnswerMap.at(it.first) +"</td>";
		this->logFile<<"<td> " + actualAnswerMap.at(it.first) +"</td>";
		this->logFile<<"</tr>";
	}

	this->logFile<<"</table>";

	//Summary
	this->logFile<<"</br></br><table border=\"1\" width=\"400\">";
	this->logFile<<"<tr><td><span style=\"font-weight:bold\">Pass</span></td><td>" + std::to_string(trueCounter) + "/" + std::to_string(trueCounter + falseCounter) +"</td></tr>";
	this->logFile<<"<tr><td><span style=\"font-weight:bold\">Fail</span></td><td>" + std::to_string(falseCounter) + "/" + std::to_string(trueCounter + falseCounter) +"</td></tr>";
	this->logFile<<"</table>";

}

std::string Logger::getTimeStamp()
{
	time_t now = time(0);
	std::stringstream sStream;

	tm *localTimeNow = localtime(&now);

	sStream << localTimeNow->tm_hour << ":" << localTimeNow->tm_min << ":" << localTimeNow->tm_sec;
	return sStream.str();
}

//close tags until the last header tag </p> tags generated by Logger::logHeader
void Logger::closeLastHeaderBlock()
{
	std::string lastTag;

	while(lastTag != "p")
	{
		lastTag = lastOpenTag.back();
		lastOpenTag.pop_back();
		this->logFile << "</" << lastTag << ">\n";

		if (lastOpenTag.empty())
		{
			break;
		}
	}
}

void Logger::closeLastTag()
{
	std::string lastTag = lastOpenTag.back();
	lastOpenTag.pop_back();
	this->logFile << "</" << lastTag << ">\n";
}

void Logger::closeFile()
{
	if (this->isInitialized())
	{
		while(!lastOpenTag.empty())
		{
			closeLastHeaderBlock();
		}
		this->logFile.close();
	}

	//new file must be reinitialized
	this->initialized = false;
}




