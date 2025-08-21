#ifndef REST_IN_BEAST_SESSION_HPP
#define REST_IN_BEAST_SESSION_HPP

#include "../util/shared_proxy.hpp"
#include "logger.hpp"
#include "respondent.hpp"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <boost/beast/http/error.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/write.hpp>

#include <chrono>
#include <memory>

namespace rest_in_beast {
namespace detail {

/**
 * @brief The PlainSession class is an INSECURE TCP session
 */
class PlainSession : public std::enable_shared_from_this<PlainSession> {
  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;

  std::shared_ptr<Respondent> respondent_;
  std::shared_ptr<Logger> logger_;
  boost::beast::http::request<boost::beast::http::string_body> request_{};

  PlainSession(boost::asio::ip::tcp::socket&& peer,
               boost::beast::flat_buffer buffer,
               std::shared_ptr<Respondent> respondent,
               std::shared_ptr<Logger> logger)
      : stream_{std::move(peer)}, buffer_{std::move(buffer)},
        respondent_{std::move(respondent)}, logger_{std::move(logger)} {}

  /**
   * @brief start_reading - strand dispatch
   */
  void start_reading() {
    // ATTENTION! Execude code io operations in stream's strand
    boost::asio::dispatch(
        stream_.get_executor(),
        boost::beast::bind_front_handler(&PlainSession::do_read,
                                         this->shared_from_this()));
  }

  friend util::SharedProxy<PlainSession>;
  static std::shared_ptr<PlainSession> make_shared(
      boost::asio::ip::tcp::socket&& peer, boost::beast::flat_buffer buffer,
      std::shared_ptr<Respondent> respondent, std::shared_ptr<Logger> logger) {
    return std::make_shared<util::SharedProxy<PlainSession>>(
        std::move(peer), std::move(buffer), std::move(respondent),
        std::move(logger));
  }

public:
  PlainSession(const PlainSession&) = delete;
  PlainSession& operator=(const PlainSession&) = delete;

  PlainSession(PlainSession&&) = delete;
  PlainSession& operator=(PlainSession&&) = delete;

  ~PlainSession() = default;

  /**
   * @brief start - main interface of session
   * @param peer - incoming connection
   * @param respondent - shared object that generates responses
   * @param logger - shared object that handles boost::asio errors
   */
  static void start(boost::asio::ip::tcp::socket&& peer,
                    boost::beast::flat_buffer buffer,
                    std::shared_ptr<Respondent> respondent,
                    std::shared_ptr<Logger> logger) {
    return make_shared(std::move(peer), std::move(buffer),
                       std::move(respondent), std::move(logger))
        ->start_reading();
  }

private:
  void on_read(boost::beast::error_code ec, std::size_t _) {
    // It's not an error
    if (ec == boost::beast::http::error::end_of_stream)
      return do_eof();

    if (ec) {
      return logger_->log("PlainSession", "on_read", ec);
    }

    do_write(respondent_->make_response(std::move(request_)));
  }

  void do_read() {
    // TODO: make_customizable
    boost::beast::get_lowest_layer(stream_).expires_after(
        std::chrono::seconds{30});

    boost::beast::http::async_read(
        stream_, buffer_, request_,
        boost::beast::bind_front_handler(&PlainSession::on_read,
                                         this->shared_from_this()));
  }

  void on_write(bool keep_alive, boost::beast::error_code ec, std::size_t _) {
    if (ec) {
      return logger_->log("PlainSession", "on_write", ec);
    }

    if (!keep_alive) {
      return do_eof();
    }

    do_read();
  }

  void do_write(boost::beast::http::message_generator&& response) {
    const bool keep_alive = response.keep_alive();
    boost::beast::async_write(
        stream_, std::move(response),
        boost::beast::bind_front_handler(&PlainSession::on_write,
                                         this->shared_from_this(), keep_alive));
  }

  /**
   * @brief on_eof closes stream
   */
  void do_eof() {
    boost::beast::error_code ec;
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

    if (ec) {
      logger_->log("PlainSession", "do_eof", ec);
    }
  }
};

/**
 * @brief The SecureSession class is an SECURE TCP session
 */
class SecureSession : public std::enable_shared_from_this<SecureSession> {
  boost::asio::ssl::stream<boost::beast::tcp_stream> stream_;
  boost::beast::flat_buffer buffer_;

  std::shared_ptr<Respondent> respondent_;
  std::shared_ptr<Logger> logger_;
  boost::beast::http::request<boost::beast::http::string_body> request_{};

  // Я вам запрещаю конструировать
  SecureSession(boost::asio::ip::tcp::socket&& peer,
                boost::asio::ssl::context& ssl_ctx,
                boost::beast::flat_buffer buffer,
                std::shared_ptr<Respondent> respondent,
                std::shared_ptr<Logger> logger)
      : stream_{std::move(peer), ssl_ctx}, buffer_{std::move(buffer)},
        respondent_{std::move(respondent)}, logger_{std::move(logger)} {}

  friend util::SharedProxy<SecureSession>;
  static std::shared_ptr<SecureSession> make_shared(
      boost::asio::ip::tcp::socket&& peer, boost::asio::ssl::context& ssl_ctx,
      boost::beast::flat_buffer buffer, std::shared_ptr<Respondent> respondent,
      std::shared_ptr<Logger> logger) {
    return std::make_shared<util::SharedProxy<SecureSession>>(
        std::move(peer), ssl_ctx, std::move(buffer), std::move(respondent),
        std::move(logger));
  }

  /**
   * @brief start_handshake - strand dispatch
   */
  void start_handshake() {
    // ATTENTION! Execude code io operations in stream's strand
    boost::asio::dispatch(
        stream_.get_executor(),
        boost::beast::bind_front_handler(&SecureSession::do_handshake,
                                         this->shared_from_this()));
  }

public:
  SecureSession(const SecureSession&) = delete;
  SecureSession& operator=(const SecureSession&) = delete;

  SecureSession(SecureSession&&) = delete;
  SecureSession& operator=(SecureSession&&) = delete;

  ~SecureSession() = default;

  /**
   * @brief start - main interface of session
   * @param peer - incoming connection
   * @param ssl_ctx - ssl context
   * @param respondent - shared object that generates responses
   * @param logger - shared object that handles boost::asio errors
   */
  static void start(boost::asio::ip::tcp::socket&& peer,
                    boost::asio::ssl::context& ssl_ctx,
                    boost::beast::flat_buffer buffer,
                    std::shared_ptr<Respondent> respondent,
                    std::shared_ptr<Logger> logger) {
    return make_shared(std::move(peer), ssl_ctx, std::move(buffer),
                       std::move(respondent), std::move(logger))
        ->start_handshake();
  }

private:
  void on_handshake(boost::beast::error_code ec,
                    std::size_t bytes_transferred) {
    if (ec) {
      return logger_->log("SecureSession", "on_handshake", ec);
    }

    // Nuance of SSL
    buffer_.consume(bytes_transferred);

    do_read();
  }

  void do_handshake() {
    // TODO: make customisable
    boost::beast::get_lowest_layer(stream_).expires_after(
        std::chrono::seconds{30});

    stream_.async_handshake(
        boost::asio::ssl::stream_base::server, buffer_.data(),
        boost::beast::bind_front_handler(&SecureSession::on_handshake,
                                         this->shared_from_this()));
  }

  void on_read(boost::beast::error_code ec, std::size_t _) {
    // It's not an error
    if (ec == boost::beast::http::error::end_of_stream)
      return do_eof();

    if (ec) {
      return logger_->log("SecureSession", "on_read", ec);
    }

    do_write(respondent_->make_response(std::move(request_)));
  }

  void do_read() {
    // TODO: make_customizable
    boost::beast::get_lowest_layer(stream_).expires_after(
        std::chrono::seconds{30});

    boost::beast::http::async_read(
        stream_, buffer_, request_,
        boost::beast::bind_front_handler(&SecureSession::on_read,
                                         this->shared_from_this()));
  }

  void on_write(bool keep_alive, boost::beast::error_code ec, std::size_t _) {
    if (ec) {
      return logger_->log("SecureSession", "on_write", ec);
    }

    if (!keep_alive) {
      return do_eof();
    }

    do_read();
  }

  void do_write(boost::beast::http::message_generator&& response) {
    // save respons'es keep_alive state
    const bool keep_alive = response.keep_alive();
    boost::beast::async_write(
        stream_, std::move(response),
        boost::beast::bind_front_handler(&SecureSession::on_write,
                                         this->shared_from_this(), keep_alive));
  }

  void on_eof(boost::beast::error_code ec) {
    if (ec) {
      return logger_->log("SecureSession", "on_eof", ec);
    }
  }

  /**
   * @brief on_eof closes stream
   */
  void do_eof() {
    stream_.async_shutdown(boost::beast::bind_front_handler(
        &SecureSession::on_eof, this->shared_from_this()));
  }
};

/**
 * @brief The SSLDetector class is a wrapper class which starts SecureSession if
 * TLS session detecter or PlainSession otherwise
 */
class DetectSSLSession : public std::enable_shared_from_this<DetectSSLSession> {
  boost::beast::tcp_stream stream_;
  boost::asio::ssl::context& ssl_ctx_;
  boost::beast::flat_buffer buffer_;

  std::shared_ptr<Respondent> respondent_;
  std::shared_ptr<Logger> logger_;

  DetectSSLSession(boost::asio::ip::tcp::socket&& peer,
                   boost::asio::ssl::context& ssl_ctx,
                   std::shared_ptr<Respondent> respondent,
                   std::shared_ptr<Logger> logger)
      : stream_{std::move(peer)}, ssl_ctx_{ssl_ctx},
        respondent_{std::move(respondent)}, logger_{std::move(logger)} {}

  friend util::SharedProxy<DetectSSLSession>;
  static std::shared_ptr<DetectSSLSession> make_shared(
      boost::asio::ip::tcp::socket&& peer, boost::asio::ssl::context& ssl_ctx,
      std::shared_ptr<Respondent> respondent, std::shared_ptr<Logger> logger) {
    return std::make_shared<util::SharedProxy<DetectSSLSession>>(
        std::move(peer), ssl_ctx, std::move(respondent), std::move(logger));
  }

  /**
   * @brief start_detection - strand dispatch
   */
  void start_detection() {
    // ATTENTION! Execude code io operations in stream's strand
    boost::asio::dispatch(
        stream_.get_executor(),
        boost::beast::bind_front_handler(&DetectSSLSession::do_detect,
                                         this->shared_from_this()));
  }

public:
  /**
   * @brief start - main interface of session
   * @param peer - incoming connection
   * @param ssl_ctx - ssl context
   * @param respondent - shared object that generates responses
   * @param logger - shared object that handles boost::asio errors
   */
  static void start(boost::asio::ip::tcp::socket&& peer,
                    boost::asio::ssl::context& ssl_ctx,
                    std::shared_ptr<Respondent> respondent,
                    std::shared_ptr<Logger> logger) {
    return make_shared(std::move(peer), ssl_ctx, std::move(respondent),
                       std::move(logger))
        ->start_detection();
  };

private:
  void on_detect(boost::beast::error_code ec, bool result) {
    if (ec) {
      return logger_->log("DetectSSLSession", "on_detect", ec);
    }

    if (result) {
      return detail::SecureSession::start(stream_.release_socket(), ssl_ctx_,
                                          std::move(buffer_), respondent_,
                                          logger_);
    }

    return detail::PlainSession::start(
        stream_.release_socket(), std::move(buffer_), respondent_, logger_);
  }

  void do_detect() {
    boost::beast::get_lowest_layer(stream_).expires_after(
        std::chrono::seconds{30});

    boost::beast::async_detect_ssl(
        stream_, buffer_,
        boost::beast::bind_front_handler(&DetectSSLSession::on_detect,
                                         this->shared_from_this()));
  }
};

/**
 * @brief The PlainSessionFactory class
 */
struct PlainSessionFactory {
  std::shared_ptr<Respondent> respondent;
  std::shared_ptr<Logger> logger;

  void start_session(boost::asio::ip::tcp::socket&& peer) {
    return PlainSession::start(std::move(peer), boost::beast::flat_buffer{},
                               respondent, logger);
  }
};

/**
 * @brief The SecureSessionFactory class
 */
struct SecureSessionFactory {
  boost::asio::ssl::context& ssl_ctx;
  std::shared_ptr<Respondent> respondent;
  std::shared_ptr<Logger> logger;

  void start_session(boost::asio::ip::tcp::socket&& peer) {
    return SecureSession::start(std::move(peer), ssl_ctx, {}, respondent,
                                logger);
  }
};

/**
 * @brief The DetectSSLSessionFactory class
 */
struct DetectSSLSessionFactory {
  boost::asio::ssl::context& ssl_ctx;
  std::shared_ptr<Respondent> respondent;
  std::shared_ptr<Logger> logger;

  void start_session(boost::asio::ip::tcp::socket&& peer) {
    return DetectSSLSession::start(std::move(peer), ssl_ctx, respondent,
                                   logger);
  }
};

} // namespace detail
} // namespace rest_in_beast

#endif // REST_IN_BEAST_SESSION_HPP
