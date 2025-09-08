//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TEST_CLIENTS_HPP
#define TEST_CLIENTS_HPP

#include <rest_in_beast/detail/logger.hpp>
#include <rest_in_beast/util/shared_proxy.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include <future>
#include <memory>

namespace test {
using string_request =
    boost::beast::http::request<boost::beast::http::string_body>;
using string_response =
    boost::beast::http::response<boost::beast::http::string_body>;

/**
 * @brief The PlainClient class - test client that sends requests asynchronously
 */
class PlainClient : public std::enable_shared_from_this<PlainClient> {
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ip::tcp::socket socket_;
  std::shared_ptr<rest_in_beast::Logger> logger_;

  boost::beast::flat_buffer buff_;
  string_response tmp_response_;

  std::vector<string_request> requests_;
  std::vector<string_response> responses_;
  std::promise<std::vector<string_response>> promise_;

  PlainClient(boost::asio::io_context& ctx,
              std::shared_ptr<rest_in_beast::Logger> logger,
              std::vector<string_request> requests)
      : resolver_{ctx}, socket_{ctx}, logger_{std::move(logger)},
        requests_{std::move(requests)}, responses_(std::size(requests_)) {}

  friend rest_in_beast::util::SharedProxy<PlainClient>;
  static std::shared_ptr<PlainClient> make_shared(
      boost::asio::io_context& ctx,
      std::shared_ptr<rest_in_beast::Logger> logger,
      std::vector<boost::beast::http::request<boost::beast::http::string_body>>
          requests) {
    return std::make_shared<rest_in_beast::util::SharedProxy<PlainClient>>(
        ctx, std::move(logger), std::move(requests));
  }

public:
  static std::future<std::vector<string_response>>
  send(boost::asio::io_context& ctx,
       std::shared_ptr<rest_in_beast::Logger> logger,
       const boost::asio::ip::tcp::endpoint& endpoint,
       std::vector<boost::beast::http::request<boost::beast::http::string_body>>
           requests) {

    return make_shared(ctx, std::move(logger), std::move(requests))
        ->do_resolve(endpoint);
  }

private:
  void
  on_resolve(const boost::system::error_code& ec,
             const boost::asio::ip::tcp::resolver::results_type& endpoints) {
    if (ec) {
      logger_->log("Client", "on_resolve", ec);
      promise_.set_value({});
      return;
    }

    do_connect(endpoints);
  }

  std::future<std::vector<string_response>>
  do_resolve(const boost::asio::ip::tcp::endpoint& endpoint) {
    resolver_.async_resolve(
        endpoint, boost::beast::bind_front_handler(&PlainClient::on_resolve,
                                                   this->shared_from_this()));

    return promise_.get_future();
  }

  void
  on_connect(const boost::system::error_code& ec,
             const boost::asio::ip::tcp::resolver::endpoint_type& endpoint) {
    if (ec) {
      logger_->log("Client", "on_connect", ec);
      promise_.set_value({});
      return;
    }

    do_send();
  }

  void
  do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
    return boost::asio::async_connect(
        socket_, endpoints,
        boost::beast::bind_front_handler(&PlainClient::on_connect,
                                         this->shared_from_this()));
  }

  void on_send(const boost::system::error_code& ec, std::size_t bytes) {
    if (ec) {
      logger_->log("Client", "on_send", ec);
      promise_.set_value({});
      return;
    }
    requests_.pop_back();

    return do_receive();
  }

  void do_send() {
    if (std::empty(requests_)) {
      promise_.set_value(std::move(responses_));
      return do_close();
    }

    boost::beast::http::async_write(
        socket_, requests_.back(),
        boost::beast::bind_front_handler(&PlainClient::on_send,
                                         this->shared_from_this()));
  }

  void on_receive(const boost::system::error_code& ec, std::size_t bytes) {
    if (ec) {
      logger_->log("Client", "on_receive", ec);
      promise_.set_value({});
      return;
    }

    // На момент получения запрос уже удалён из очереди
    responses_[std::size(requests_)] = std::move(tmp_response_);

    return do_send();
  }

  void do_receive() {
    boost::beast::http::async_read(
        socket_, buff_, tmp_response_,
        boost::beast::bind_front_handler(&PlainClient::on_receive,
                                         this->shared_from_this()));
  }

  void do_close() {
    boost::system::error_code ec{};
    socket_.close(ec);
    if (ec) {
      return logger_->log("Client", "do_close", ec);
    }
  }
};

/**
 * @brief The SecureClient class - test client that sends requests
 * asynchronously
 */
class SecureClient : public std::enable_shared_from_this<SecureClient> {
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  std::shared_ptr<rest_in_beast::Logger> logger_;

  boost::beast::flat_buffer buff_;
  string_response tmp_response_;

  std::vector<string_request> requests_;
  std::vector<string_response> responses_;
  std::promise<std::vector<string_response>> promise_;

  SecureClient(
      boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx,
      std::shared_ptr<rest_in_beast::Logger> logger,
      std::vector<boost::beast::http::request<boost::beast::http::string_body>>
          requests)
      : resolver_{io_ctx}, socket_{io_ctx, ssl_ctx}, logger_{std::move(logger)},
        requests_{std::move(requests)}, responses_(std::size(requests_)) {}

  friend rest_in_beast::util::SharedProxy<SecureClient>;
  static std::shared_ptr<SecureClient> make_shared(
      boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx,
      std::shared_ptr<rest_in_beast::Logger> logger,
      std::vector<boost::beast::http::request<boost::beast::http::string_body>>
          requests) {
    return std::make_shared<rest_in_beast::util::SharedProxy<SecureClient>>(
        io_ctx, ssl_ctx, std::move(logger), std::move(requests));
  }

public:
  static std::future<std::vector<string_response>>
  send(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx,
       std::shared_ptr<rest_in_beast::Logger> logger,
       const boost::asio::ip::tcp::endpoint& endpoint,
       std::vector<boost::beast::http::request<boost::beast::http::string_body>>
           requests) {
    return make_shared(io_ctx, ssl_ctx, std::move(logger), std::move(requests))
        ->do_resolve(endpoint);
  }

private:
  void
  on_resolve(const boost::beast::error_code& ec,
             const boost::asio::ip::tcp::resolver::results_type& endpoints) {
    if (ec) {
      logger_->log("SecureClient", "on_resolve", ec);
      promise_.set_value({});
      return;
    }

    return do_connect(endpoints);
  }

  std::future<std::vector<string_response>>
  do_resolve(const boost::asio::ip::tcp::endpoint& endpoint) {
    resolver_.async_resolve(
        endpoint, boost::beast::bind_front_handler(&SecureClient::on_resolve,
                                                   this->shared_from_this()));

    return promise_.get_future();
  }

  void
  on_connect(const boost::beast::error_code& ec,
             const boost::asio::ip::tcp::resolver::endpoint_type& endpoint) {
    if (ec) {
      logger_->log("SecureClient", "on_connect", ec);
      promise_.set_value({});
      return;
    }

    return do_handshake();
  }

  void
  do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) {
    return boost::asio::async_connect(
        socket_.lowest_layer(), endpoints,
        boost::beast::bind_front_handler(&SecureClient::on_connect,
                                         shared_from_this()));
  }

  void on_handshake(const boost::beast::error_code& ec) {
    if (ec) {
      logger_->log("SecureClient", "on_handshake", ec);
      promise_.set_value({});
      return;
    }

    do_send();
  }

  void do_handshake() {
    return socket_.async_handshake(
        boost::asio::ssl::stream_base::client,
        boost::beast::bind_front_handler(&SecureClient::on_handshake,
                                         shared_from_this()));
  }

  void on_send(const boost::beast::error_code& ec, std::size_t bytes) {
    if (ec) {
      logger_->log("SecureClient", "on_send", ec);
      promise_.set_value({});
      return;
    }

    requests_.pop_back();

    return do_receive();
  }

  void do_send() {
    if (std::empty(requests_)) {
      promise_.set_value(std::move(responses_));
      return do_shutdown();
    }

    boost::beast::http::async_write(
        socket_, requests_.back(),
        boost::beast::bind_front_handler(&SecureClient::on_send,
                                         this->shared_from_this()));
  }

  void on_receive(const boost::beast::error_code& ec, std::size_t bytes) {
    if (ec) {
      logger_->log("SecureClient", "on_receive", ec);
      promise_.set_value({});
      return;
    }

    responses_[std::size(requests_)] = std::move(tmp_response_);

    return do_send();
  }

  void do_receive() {
    boost::beast::http::async_read(
        socket_, buff_, tmp_response_,
        boost::beast::bind_front_handler(&SecureClient::on_receive,
                                         this->shared_from_this()));
  }

  void do_close() {
    boost::beast::error_code ec{};
    socket_.lowest_layer().close(ec);

    if (ec)
      return logger_->log("SecureClient", "do_close", ec);
  }

  void on_shutdown(const boost::beast::error_code& ec) {
    if (ec)
      return logger_->log("SecureClient", "on_shutdown", ec);

    do_close();
  }

  void do_shutdown() {
    socket_.async_shutdown(boost::beast::bind_front_handler(
        &SecureClient::on_shutdown, this->shared_from_this()));
  }
};
} // namespace test
#endif // TEST_CLIENTS_HPP
