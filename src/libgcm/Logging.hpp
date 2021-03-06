#ifndef GCM_LOGGING_H_
#define GCM_LOGGING_H_

#include "libgcm/config.hpp"

#if CONFIG_ENABLE_LOGGING
    #include <log4cxx/logger.h>
    #include <log4cxx/mdc.h>
    #define USE_LOGGER log4cxx::LoggerPtr logger;
    #define INIT_LOGGER(name) do { logger = log4cxx::Logger::getLogger(name); } while (0)
    #define USE_AND_INIT_LOGGER(name) log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(name);

    #define LOG_INFO(message)  do { LOG4CXX_INFO (logger, message) } while (0)
    #define LOG_WARN(message)  do { LOG4CXX_WARN (logger, message) } while (0)
    #define LOG_FATAL(message) do { LOG4CXX_FATAL(logger, message) } while (0)
    #define LOG_ERROR(message) do { LOG4CXX_ERROR(logger, message) } while (0)
    #define LOG_TRACE(message) do { LOG4CXX_TRACE(logger, message) } while (0)
    #define LOG_DEBUG(message) do { LOG4CXX_DEBUG(logger, message) } while (0)
#else
    #define USE_LOGGER ;
    #define USE_AND_INIT_LOGGER(name);
    #define INIT_LOGGER(name) do {} while (0)

    #define LOG_INFO(message)  do {} while (0)
    #define LOG_WARN(message)  do {} while (0)
    #define LOG_FATAL(message) do {} while (0)
    #define LOG_ERROR(message) do {} while (0)
    #define LOG_TRACE(message) do {} while (0)
    #define LOG_DEBUG(message) do {} while (0)
#endif

#endif
