//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TEST_ASIO_THREAD_HPP
#define TEST_ASIO_THREAD_HPP

#include <boost/asio.hpp>
#include <exception>

namespace test {

/**
 * @brief The ASIOThread class - body of asio thread
 */
struct ASIOThread {
  boost::asio::io_context& ctx;
  std::exception_ptr thread_exception;

  auto thread_body() {
    return [this]() {
      try {
        ctx.run();
      } catch (const std::exception& e) {
        thread_exception = std::make_exception_ptr(e);
      }
    };
  }
};

} // namespace test

#endif // TEST_ASIO_THREAD_HPP
