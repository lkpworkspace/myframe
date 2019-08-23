#include "MyLog.h"

#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

MyLog::MyLog()
{
    namespace logging = boost::log;
    namespace keywords = boost::log::keywords;
    namespace expr = boost::log::expressions;
    namespace attrs = boost::log::attributes;
    namespace src = boost::log::sources;
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::trace
    );
}