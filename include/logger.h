// boost logger
#ifndef LOGGER_H
#define LOGGER_H

// source: http://gernotklingler.com/blog/simple-customized-logger-based-boost-log-v2/ 
// #define BOOST_LOG_DYN_LINK 1 // necessary when linking the boost_log library dynamically

#include <string>
#include <boost/log/trivial.hpp>
#include "boost/log/utility/setup.hpp"
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/manipulators/dump.hpp>

// register a global logger
BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)

// just a helper macro used by the macros below - don't use it in your code
#define LOG(severity) BOOST_LOG_SEV(logger::get(),boost::log::trivial::severity)

// ===== log macros =====
#define LOG_TRACE   LOG(trace)
#define LOG_DEBUG   LOG(debug)
#define LOG_INFO    LOG(info)
#define LOG_WARNING LOG(warning)
#define LOG_ERROR   LOG(error)
#define LOG_FATAL   LOG(fatal)

#endif

