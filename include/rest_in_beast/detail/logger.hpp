#ifndef RESIN_IN_BEAST_LOGGER_HPP
#define RESIN_IN_BEAST_LOGGER_HPP

#include <boost/system/error_code.hpp>

namespace rest_in_beast {
/**
 * @brief The Logger class provides interface for any project logger
 */
class Logger {
public:
  Logger() = default;

  virtual ~Logger() = default;

  virtual void log(std::string_view class_name, std::string_view function_name,
                   boost::system::error_code ec) = 0;
};

} // namespace rest_in_beast

#endif // RESIN_IN_BEAST_LOGGER_HPP
