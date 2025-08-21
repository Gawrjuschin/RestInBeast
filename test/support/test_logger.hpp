#ifndef TEST_LOGGER_HPP
#define TEST_LOGGER_HPP

// #include <iostream>
#include <memory>
// #include <syncstream>

#include <rest_in_beast/detail/logger.hpp>
#include <rest_in_beast/util/shared_proxy.hpp>

namespace test {
/**
 * @brief The Logger class prints error
 */
class Logger : public rest_in_beast::Logger {
  boost::system::error_code last_ec_;

  Logger() = default;

  friend rest_in_beast::util::SharedProxy<Logger>;

public:
  ~Logger() = default;

  static std::shared_ptr<Logger> make_shared() {
    return std::make_shared<rest_in_beast::util::SharedProxy<Logger>>();
  }

  void log(std::string_view class_name, std::string_view function_name,
           boost::system::error_code ec) override {
    std::swap(last_ec_, ec);
  }

  const boost::system::error_code& last_ec() const noexcept { return last_ec_; }
};

/**
 * @brief The MemoLogger class saves last error
 */
class MemoLogger : public rest_in_beast::Logger {
  boost::system::error_code last_ec_;

  MemoLogger() = default;

  friend rest_in_beast::util::SharedProxy<MemoLogger>;

public:
  ~MemoLogger() = default;

  static std::shared_ptr<MemoLogger> make_shared() {
    return std::make_shared<rest_in_beast::util::SharedProxy<MemoLogger>>();
  }

  void log(std::string_view class_name, std::string_view function_name,
           boost::system::error_code ec) override {
    std::swap(last_ec_, ec);
  }

  const boost::system::error_code& last_ec() const noexcept { return last_ec_; }
};

} // namespace test

#endif // TEST_LOGGER_HPP
