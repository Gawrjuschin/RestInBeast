//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>
#include <iostream>
#include <rest_in_beast/detail/logger.hpp>

namespace test {

struct SignalsHandler {
  boost::asio::io_context& ctx;
  std::shared_ptr<rest_in_beast::Logger> logger;

  void operator()(const boost::beast::error_code& ec, int signal) {
    if (ec) {
      logger->log("signals_handler", "operator()", ec);
    } else {
      std::cerr << "[ERROR] " << "signals_handler" << "operator()" << signal
                << std::endl;
    }
    ctx.stop();
  }
};

} // namespace test
