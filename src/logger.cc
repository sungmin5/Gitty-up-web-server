// boost logger

#include "logger.h"

BOOST_LOG_GLOBAL_LOGGER_INIT(logger, boost::log::sources::severity_logger_mt)
{
  boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger;
  static const std::string COMMON_FMT("[%TimeStamp%][%ThreadID%][%Severity%]:  %Message%");
  boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
  boost::log::add_console_log
  (
      std::cout,
      boost::log::keywords::format = COMMON_FMT,
      boost::log::keywords::auto_flush = true
  );


  boost::log::add_file_log (
      boost::log::keywords::file_name = "server_log_%m%d%Y_%H%M%S_%5N.log",
      //rotate files every 10 Mb
      boost::log::keywords::rotation_size = 10000000,
      //rotate every midnight
      boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
      boost::log::keywords::format = COMMON_FMT,
      boost::log::keywords::auto_flush = true
  );

  boost::log::add_common_attributes();
  return logger;
}
