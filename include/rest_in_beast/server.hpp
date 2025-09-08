//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Based on Boost.Beast's HTTP server examples:
//
// https://github.com/boostorg/beast/blob/develop/example/advanced/server
// https://github.com/boostorg/beast/blob/develop/example/advanced/server-flex
// https://github.com/boostorg/beast/tree/develop/example/common
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RESIN_IN_BEAST_SERVER_HPP
#define RESIN_IN_BEAST_SERVER_HPP

#include "rest_in_beast/detail/session.hpp"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/strand.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/detect_ssl.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>

#include <memory>
#include <utility>

namespace rest_in_beast {

namespace detail {
/**
 * @brief The Server class is a template of server supports PlainSession,
 * SecureSession or DetectSSLSession sessions
 */
template <typename SessionFactory>
class Server : public std::enable_shared_from_this<Server<SessionFactory>> {
  boost::asio::ip::tcp::acceptor acceptor_;
  std::shared_ptr<Logger> logger_;
  SessionFactory session_factory_;

  /**
   * @brief Server constructor is private because Server class uses CRTP.
   * Use Server::start_server instead of creating any instance of Server
   * @param io_ctx
   * @param endpoint
   * @param logger
   * @param SessionFactory may be constructed with universal initialization
   * with requests_handler adjusted bu customer
   */
  Server(boost::asio::io_context& io_ctx,
         const boost::asio::ip::tcp::endpoint& endpoint,
         std::shared_ptr<Logger> logger, SessionFactory session_factory)
      : acceptor_{boost::asio::make_strand(io_ctx), endpoint},
        logger_{std::move(logger)},
        session_factory_{std::move(session_factory)} {}

  friend struct util::SharedProxy<Server>;
  static std::shared_ptr<Server>
  make_shared(boost::asio::io_context& io_ctx,
              const boost::asio::ip::tcp::endpoint& endpoint,
              std::shared_ptr<Logger> logger, SessionFactory session_factory) {
    return std::make_shared<util::SharedProxy<Server>>(
        io_ctx, endpoint, std::move(logger), std::move(session_factory));
  }

public:
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  Server(Server&&) = delete;
  Server& operator=(Server&&) = delete;

  ~Server() = default;

  /**
   * @brief start - starts server with specified session type
   * @param io_ctx
   * @param endpoint
   * @param logger
   * @param session_factory - custom object
   */
  static void start(boost::asio::io_context& io_ctx,
                    const boost::asio::ip::tcp::endpoint& endpoint,
                    std::shared_ptr<Logger> logger,
                    SessionFactory session_factory) {
    make_shared(io_ctx, endpoint, std::move(logger), std::move(session_factory))
        ->do_accept();
  }

private:
  /**
   * @brief on_accept starts new session
   * @param ec
   * @param peer - incoming connection (tcp socket)
   */
  void on_accept(boost::beast::error_code ec,
                 boost::asio::ip::tcp::socket peer) {
    if (ec) {
      return logger_->log("Server", "on_accept", ec);
    } else {
      session_factory_.start_session(std::move(peer));
    }

    do_accept();
  }

  /**
   * @brief do_accept attaches callback Server::on_accept to acceptor_ on new
   * incoming connection in new strand
   */
  void do_accept() {
    acceptor_.async_accept(
        boost::asio::make_strand(
            acceptor_.get_executor()), // Create separate strand for incoming
                                       // connection. New session MUST switch to
                                       // it new strand
        boost::beast::bind_front_handler(&Server<SessionFactory>::on_accept,
                                         this->shared_from_this()));
  }
};

} // namespace detail

using PlainServer = detail::Server<detail::PlainSessionFactory>;
using SecureServer = detail::Server<detail::SecureSessionFactory>;
using FlexServer = detail::Server<detail::DetectSSLSessionFactory>;

} // namespace rest_in_beast

#endif // RESIN_IN_BEAST_SERVER_HPP
