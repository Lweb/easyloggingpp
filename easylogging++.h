/***********************************************************************\
* easylogging++.h - Core of EasyLogging++                              *
*   EasyLogging++ v2.846                                                 *
*   Cross platform logging made easy for C++ applications              *
*   Author Majid Khan <mkhan3189@gmail.com>                            *
*   http://www.icplusplus.com                                          *
*   https://github.com/mkhan3189/EasyLoggingPP                         *
*                                                                      *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3 of the License.              *
\***********************************************************************/
#ifndef EASYLOGGINGPP_H
#define EASYLOGGINGPP_H

#define _LOGGING_ENABLED 1

#define _ENABLE_DEBUG_LOGS 1
#define _DEBUG_LOGS_TO_STANDARD_OUTPUT 0
#define _DEBUG_LOGS_TO_FILE 1

#define _ENABLE_INFO_LOGS 1
#define _INFO_LOGS_TO_STANDARD_OUTPUT 1
#define _INFO_LOGS_TO_FILE 1

#define _ENABLE_WARNING_LOGS 1
#define _WARNING_LOGS_TO_STANDARD_OUTPUT 1
#define _WARNING_LOGS_TO_FILE 1

#define _ENABLE_ERROR_LOGS 1
#define _ERROR_LOGS_TO_STANDARD_OUTPUT 1
#define _ERROR_LOGS_TO_FILE 1

#define _ENABLE_FATAL_LOGS 1
#define _FATAL_LOGS_TO_STANDARD_OUTPUT 1
#define _FATAL_LOGS_TO_FILE 1

#define _ENABLE_PERFORMANCE_LOGS 1
#define _PERFORMANCE_LOGS_TO_STANDARD_OUTPUT 0
#define _PERFORMANCE_LOGS_TO_FILE 1

#define _ENABLE_HINTS 1
#define _HINTS_TO_STANDARD_OUTPUT 1
#define _HINTS_TO_FILE 0

#define _ENABLE_STATUS 1
#define _STATUS_TO_STANDARD_OUTPUT 1
#define _STATUS_TO_FILE 0

#if _LOGGING_ENABLED
#ifdef _WIN32
 #define _WINDOWS 1
 #define _WINDOWS_32 1
#endif //_WIN32
#ifdef _WIN64
 #define _WINDOWS 1
 #define _WINDOWS_64 1
#endif //_WIN64
#ifdef __linux
 #define _LINUX 1
#endif //__linux
#ifdef __APPLE__
 #if TARGET_OS_MAC
  #define _MAC 1
 #endif //TARGET_OS_MAC
#endif //__APPLE__
#include <ctime>
#include <cstring>
#include <cstdlib>
#if _WINDOWS
 #include <direct.h> //digital mars compiler
 #include <windows.h>
#endif //_WINDOWS
#if _LINUX || _MAC
 #include <sys/stat.h>
 #include <sys/time.h>
#endif //_LINUX || _MAC
#include <iostream>
#include <sstream>
#include <fstream>
namespace easyloggingpp {
//////////////////////////////////////////////
//   Configuration for logging             ///
//////////////////////////////////////////////

/**
 * This is the format for your log. Please see github readme for details
 */
const std::string LOG_FORMAT = "[%type] [%datetime] [%user@%host] [%func] [%loc] %log%n";

/**
 * Flag for showing log in standard output using std::cout
 */
const bool SHOW_STD_OUTPUT = true;

/**
 * Flag to set whether to save log to file
 */
const bool SAVE_TO_FILE = true;

/*
 * If saving to file, this defines the filename
 */
const std::string LOG_FILENAME = "myeasylog.log";

/**
 * Flag to set whether to save log file in custom location
 */
const bool USE_CUSTOM_LOCATION = true;

/**
 * If using custom location, this is where custom location is picked up from.
 * Note: This should end with last slash and should be a fully qualified path. 
 * Relative paths are not allowed
 */
const std::string CUSTOM_LOG_FILE_LOCATION = "logs/";

/**
 * Determines whether to show log when starting any time tracked function
 */
const bool SHOW_START_FUNCTION_LOG = false;

////////////////////////////////////////////////////////////////////
///         END OF CONFIGURATION FOR LOGGING                     ///
////////////////////////////////////////////////////////////////////
static const std::string kFinalFilename = (::easyloggingpp::USE_CUSTOM_LOCATION ? ::easyloggingpp::CUSTOM_LOG_FILE_LOCATION : "") + ::easyloggingpp::LOG_FILENAME;
static const bool kShowDate = ::easyloggingpp::LOG_FORMAT.find("%date") != std::string::npos;
static const bool kShowDateTime = ::easyloggingpp::LOG_FORMAT.find("%datetime") != std::string::npos;
static const bool kShowLocation = ::easyloggingpp::LOG_FORMAT.find("%loc") != std::string::npos;
static std::string user;
static std::string host;
static std::stringstream *logStream;
static std::stringstream tempStream;
static std::stringstream tempStream2;
static bool toStandardOutput;
static bool toFile;
static const short kDateBufferSize = 25;
static char dateBuffer[kDateBufferSize];
static char dateFormat[kDateBufferSize];
static bool loggerInitialized = false;
static bool fileNotOpenedErrorDisplayed = false;
static std::string tempLogFormat = "";

static inline void internalMessage(const std::string& message) {
    std::cout << std::endl << "[EasyLogging++] " << message << std::endl << std::endl;
}

static bool logPathExist(void) {
#if _WINDOWS
  DWORD fileType = GetFileAttributesA(::easyloggingpp::CUSTOM_LOG_FILE_LOCATION.c_str());
  if (fileType == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (fileType & FILE_ATTRIBUTE_DIRECTORY);
#elif _LINUX || _MAC
  struct stat st;
  return (stat(::easyloggingpp::CUSTOM_LOG_FILE_LOCATION.c_str(), &st) == 0);
#endif //_WINDOWS
}

static void createLogPath(void) {
  if ((::easyloggingpp::USE_CUSTOM_LOCATION) && (::easyloggingpp::CUSTOM_LOG_FILE_LOCATION.size() > 0) && (!::easyloggingpp::logPathExist())) {
    int status = -1;
#if _WINDOWS
    std::string pathDelimiter = "\\";
#elif _LINUX
    std::string pathDelimiter = "/";
#endif //_WINDOWS
    std::string tempPath = CUSTOM_LOG_FILE_LOCATION;
    std::string dir = "";
    short foundAt = -1;
    std::string madeSoFar = tempPath.substr(0, 1) == pathDelimiter ? pathDelimiter : "";
    while ((foundAt = tempPath.find(pathDelimiter)) != std::string::npos) {
      dir = tempPath.substr(0, foundAt);
      if (dir != "") {
        madeSoFar += dir + pathDelimiter;
#if _WINDOWS
        status = _mkdir(madeSoFar.c_str());
#elif _LINUX || _MAC
        status = mkdir(madeSoFar.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IWGRP | S_IRGRP | S_IXGRP | S_IWOTH | S_IXOTH); /* rwx,rwx,wx */
#endif //_WINDOWS
      }
      tempPath = tempPath.erase(0, foundAt + 1);
    }
    if (status == -1) {
      ::easyloggingpp::internalMessage("Unable to create log path [" + ::easyloggingpp::CUSTOM_LOG_FILE_LOCATION + "]");
    }
  }
}

static inline std::string getDateTime(void) {
  if (!(::easyloggingpp::kShowDateTime || ::easyloggingpp::kShowDate)) return "";
#if _WINDOWS
    time_t currTime;
#elif _LINUX || _MAC
    timeval currTime;
    gettimeofday(&currTime, NULL);
    int milliSeconds = 0;
    if (::easyloggingpp::kShowDateTime) {
      milliSeconds = currTime.tv_usec / 1000;
    }
#endif //_WINDOWS
    struct tm * timeInfo;
#if _WINDOWS
    timeInfo = localtime(&currTime);
#elif _LINUX || _MAC
    timeInfo = localtime(&currTime.tv_sec);
#endif //_WINDOWS
    strftime(::easyloggingpp::dateBuffer, ::easyloggingpp::kDateBufferSize, dateFormat, timeInfo);
#if _LINUX || _MAC
    if (::easyloggingpp::kShowDateTime) {
      sprintf(::easyloggingpp::dateBuffer, "%s.%d", ::easyloggingpp::dateBuffer, milliSeconds);
    }
#endif //_LINUX || _MAC
  return std::string(::easyloggingpp::dateBuffer);
}

#ifndef __FILE__
 #define __FILE__ ""
#endif //__FILE
#ifndef __LINE__
 #define __LINE__ ""
#endif //_LINE__
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
 #define __func__ __FUNCTION__
#elif defined(__GNUC__) && (__GNUC__ >= 2)
 #define __func__ __PRETTY_FUNCTION__
#else
 #define __func__ ""
#endif //defined(_MSC_VER) && (_MSC_VER >= 1020)

static inline std::string getUsername(void) {
#if _WINDOWS
  char* username = getenv("USERNAME");
#elif _LINUX || _MAC
  char* username = getenv("USER");
#endif //_WINDOWS
  if ((username == NULL) || ((strcmp(username, "") == 0))) {
    return std::string("");
  } else {
    return std::string(username);
  }
}

static inline std::string getHostname(void) {
#if _WINDOWS
  char* hostname = getenv("COMPUTERNAME");
#elif _LINUX || _MAC
  char* hostname = getenv("HOSTNAME");
#endif //_WINDOWS
  if ((hostname == NULL) || ((strcmp(hostname, "") == 0))) {
    return std::string("unknown-host");
  } else {
    return std::string(hostname);
  }
}

static inline void cleanStream(void) {
  ::easyloggingpp::logStream->str("");
}

static inline std::string colourful(const std::string& text, const std::string& colour, bool bold = false) {
#if _LINUX
  short code = 30;
  if (colour == "black") code = 30;
  else if (colour == "red") code = 31;
  else if (colour == "green") code = 32;
  else if (colour == "yellow") code = 33;
  else if (colour == "blue") code = 34;
  else if (colour == "white") code = 37;
  std::stringstream finalText;
  finalText << "\033[" << (bold ? "1" : "0") << ";" << code << "m" << text << "\033[0m";
  return finalText.str();
#else
  return text;
#endif //_LINUX
}

static void init(void) {
  // Logger
  ::easyloggingpp::logStream = new std::stringstream();\
  // Path
  ::easyloggingpp::createLogPath();
  // Log file
  std::ofstream logFile(::easyloggingpp::kFinalFilename.c_str(), std::ofstream::out | std::ofstream::app);
  if (logFile.is_open()) {
    logFile.close();
  } else if (!fileNotOpenedErrorDisplayed) {
    ::easyloggingpp::internalMessage("Unable to open log file [" + ::easyloggingpp::kFinalFilename + "]");
    fileNotOpenedErrorDisplayed = true;
  }
  // Date format
  const char* dateFormatLocal = "%d/%m/%Y";
  const char* timeFormatLocal = "%H:%M:%S";
  if (::easyloggingpp::kShowDate) strcpy(::easyloggingpp::dateFormat, dateFormatLocal);
  if (::easyloggingpp::kShowDateTime) {
      if (::easyloggingpp::kShowDate) {
          strcat(::easyloggingpp::dateFormat, " ");
          strcat(::easyloggingpp::dateFormat, timeFormatLocal);
      } else {
          strcpy(::easyloggingpp::dateFormat, timeFormatLocal);
      }
  }
  // Username and host
  ::easyloggingpp::user = ::easyloggingpp::getUsername();
  ::easyloggingpp::host = ::easyloggingpp::getHostname();
  loggerInitialized = true; 
}

static std::string readLog(void) {
  std::stringstream ss;
  if (::easyloggingpp::SAVE_TO_FILE) {
    std::ifstream logFile(::easyloggingpp::kFinalFilename.c_str(), std::ifstream::in);
    if (logFile.is_open()) {
      std::string line;
      while (logFile.good()) {
        std::getline(logFile, line);
        ss << line << std::endl;
      }
      logFile.close();
    } else {
      ss << "Error opening log file [" << ::easyloggingpp::kFinalFilename << "]";
    }
  } else {
    ss << "Logs are not being saved to file!";
  }
  return ss.str();
}

static void writeLog(void) {
  if ((::easyloggingpp::SHOW_STD_OUTPUT) && (::easyloggingpp::toStandardOutput)) {
    std::cout << ::easyloggingpp::logStream->str();
  }
  if ((!fileNotOpenedErrorDisplayed) && (::easyloggingpp::SAVE_TO_FILE) && (::easyloggingpp::toFile)) {
    std::ofstream logFile(::easyloggingpp::kFinalFilename.c_str(), std::ofstream::out | std::ofstream::app);
    logFile << ::easyloggingpp::logStream->str();
    logFile.close();
  }
  ::easyloggingpp::cleanStream();
}

static void replaceFirst(const std::string& replaceWhat, const std::string& replaceWith, std::string& str) {
  size_t foundAt = -1;
  while ((foundAt = str.find(replaceWhat)) != std::string::npos) {
    str = str.replace(foundAt, replaceWhat.size(), replaceWith);
    break;
  }
}
static void buildFormat(const char* func, const char* file, const double line, const std::string& type) {
  if (!::easyloggingpp::loggerInitialized) {
    ::easyloggingpp::init();
    ::easyloggingpp::tempStream.str("");  
    ::easyloggingpp::tempStream2.str("");  
  }
  tempLogFormat = ::easyloggingpp::LOG_FORMAT;
 
  replaceFirst("%type", type, ::easyloggingpp::tempLogFormat);
  if (::easyloggingpp::kShowDateTime) {
    replaceFirst("%datetime", ::easyloggingpp::getDateTime(), ::easyloggingpp::tempLogFormat);
  } else if (::easyloggingpp::kShowDate) {
    replaceFirst("%date", ::easyloggingpp::getDateTime(), ::easyloggingpp::tempLogFormat);
  }
  replaceFirst("%func", std::string(func), ::easyloggingpp::tempLogFormat);
  if (::easyloggingpp::kShowLocation) {
    ::easyloggingpp::tempStream.str("");
    ::easyloggingpp::tempStream << file << ":" << line; 
    replaceFirst("%loc", ::easyloggingpp::tempStream.str(), ::easyloggingpp::tempLogFormat);
    ::easyloggingpp::tempStream.str("");
  }
  replaceFirst("%user", ::easyloggingpp::user, ::easyloggingpp::tempLogFormat);
  replaceFirst("%host", ::easyloggingpp::host, ::easyloggingpp::tempLogFormat);
  replaceFirst("%log", ::easyloggingpp::tempStream2.str(), ::easyloggingpp::tempLogFormat);
  replaceFirst("%n", "\n", ::easyloggingpp::tempLogFormat);
  if      (type == "DEBUG") { ::easyloggingpp::toStandardOutput = _DEBUG_LOGS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _DEBUG_LOGS_TO_FILE; }
  else if (type == "INFO") { ::easyloggingpp::toStandardOutput = _INFO_LOGS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _INFO_LOGS_TO_FILE; }
  else if (type == "WARNING") { ::easyloggingpp::toStandardOutput = _WARNING_LOGS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _WARNING_LOGS_TO_FILE; }
  else if (type == "ERROR") { ::easyloggingpp::toStandardOutput = _ERROR_LOGS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _ERROR_LOGS_TO_FILE; }
  else if (type == "FATAL") { ::easyloggingpp::toStandardOutput = _FATAL_LOGS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _FATAL_LOGS_TO_FILE; }
  else if (type == "PERFORMANCE") { ::easyloggingpp::toStandardOutput = _PERFORMANCE_LOGS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _PERFORMANCE_LOGS_TO_FILE; }
  else if (type == "HINT") { ::easyloggingpp::toStandardOutput = _HINTS_TO_STANDARD_OUTPUT; easyloggingpp::toFile = _HINTS_TO_FILE; }
  else if (type == "STATUS") { ::easyloggingpp::toStandardOutput = _STATUS_TO_STANDARD_OUTPUT; ::easyloggingpp::toFile = _STATUS_TO_FILE; }
  (*::easyloggingpp::logStream) << tempLogFormat;
  ::easyloggingpp::tempStream2.str("");  
}
#define WRITE_LOG(type, log)\
  ::easyloggingpp::tempStream2 << log;\
  ::easyloggingpp::buildFormat(__func__, __FILE__, __LINE__, std::string(type));\
  ::easyloggingpp::writeLog();
 
  #if _ENABLE_DEBUG_LOGS
    #define DEBUG(logStr) WRITE_LOG(std::string("DEBUG"),logStr)
  #else
    #define DEBUG(x)
  #endif //_ENABLE_DEBUG_LOGS
  #if _ENABLE_INFO_LOGS
    #define INFO(logStr) WRITE_LOG(std::string("INFO"),logStr)
  #else
    #define INFO(x)
  #endif //_ENABLE_INFO_LOGS
  #if _ENABLE_WARNING_LOGS
    #define WARNING(logStr) WRITE_LOG(std::string("WARNING"),logStr)
  #else
    #define WARNING(x)
  #endif //_ENABLE_WARNING_LOGS
  #if _ENABLE_ERROR_LOGS
    #define ERROR(logStr) WRITE_LOG(std::string("ERROR"),logStr)
  #else
    #define ERROR(x)
  #endif //_ENABLE_ERROR_LOGS
  #if _ENABLE_FATAL_LOGS
    #define FATAL(logStr) WRITE_LOG(std::string("FATAL"),logStr)
  #else
    #define FATAL(x)
  #endif //_ENABLE_FATAL_LOGS
  #if _ENABLE_PERFORMANCE_LOGS
    inline std::string formatSeconds(double secs) {
      double result = secs;
      std::string unit = "seconds";
      std::stringstream ss;
      if (result > 60.0f) {result /= 60; unit = "minutes";
        if (result > 60.0f) {result /= 60; unit = "hours";
          if (result > 24.0f) {result /= 24; unit = "days";}
        }
      }
      ss << result << " " << unit;
      return ss.str();
    }
    #define PERFORMANCE(logStr) WRITE_LOG(std::string("PERFORMANCE"),logStr)
    #define START_FUNCTION_LOG "Executing [" << __func__ << "]"
    #define TIME_OUTPUT "Executed [" << __func__ << "] in [~ " << ::easyloggingpp::formatSeconds(difftime(functionEndTime,functionStartTime)) << "]"
    #define FUNC_SUB_COMMON_START { if (::easyloggingpp::SHOW_START_FUNCTION_LOG) { PERFORMANCE(START_FUNCTION_LOG) } time_t functionStartTime, functionEndTime; time(&functionStartTime);
    #define FUNC_SUB_COMMON_END time(&functionEndTime); PERFORMANCE(TIME_OUTPUT);
    #define SUB(FUNCTION_NAME,PARAMS) void FUNCTION_NAME PARAMS FUNC_SUB_COMMON_START 
    #define END_SUB FUNC_SUB_COMMON_END }
    #define FUNC(RETURNING_TYPE,FUNCTION_NAME,PARAMS) RETURNING_TYPE FUNCTION_NAME PARAMS FUNC_SUB_COMMON_START 
    #define END_FUNC FUNC_SUB_COMMON_END }
    #define RETURN(expr) FUNC_SUB_COMMON_END return expr;
  #else
    #define PERFORMANCE(x)
    #define SUB(FUNCTION_NAME,PARAMS) void FUNCTION_NAME PARAMS {
    #define END_SUB }
    #define FUNC(RETURNING_TYPE,FUNCTION_NAME,PARAMS) RETURNING_TYPE FUNCTION_NAME PARAMS {
    #define END_FUNC }
    #define RETURN(expr) return expr;
  #endif //_ENABLE_PERFORMANCE_LOGS
  #if _ENABLE_HINTS
    #define HINT(logStr) WRITE_LOG(std::string("HINT"),logStr)
  #else
    #define HINT(x)
  #endif //_ENABLE_HINTS
  #if _ENABLE_STATUS
    #define STATUS(logStr) WRITE_LOG(std::string("STATUS"),logStr)
  #else
    #define STATUS(x)
  #endif //_ENABLE_STATUS
} //namespace easyloggingpp
#else
  #define DEBUG(x)
  #define INFO(x)
  #define WARNING(x)
  #define ERROR(x)
  #define FATAL(x)
  #define PERFORMANCE(x)
  #define HINT(x)
  #define STATUS(x)
  #define SUB(FUNCTION_NAME,PARAMS) void FUNCTION_NAME PARAMS {
  #define END_SUB }
  #define FUNC(RETURNING_TYPE,FUNCTION_NAME,PARAMS) RETURNING_TYPE FUNCTION_NAME PARAMS {
  #define END_FUNC }
  #define RETURN(expr) return expr;
#endif //_LOGGING_ENABLED
#endif //EASYLOGGINGPP_H
