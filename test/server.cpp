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

#define BOOST_TEST_MODULE ServerTests
#include <boost/test/unit_test.hpp>

#include "support/test_asio_thread.hpp"
#include "support/test_clients.hpp"
#include "support/test_logger.hpp"
#include "support/test_requests.hpp"
#include "support/test_respondent.hpp"
#include "support/test_signal_handler.hpp"
#include "support/test_ssl_util.hpp"

#include <rest_in_beast/detail/logger.hpp>
#include <rest_in_beast/detail/respondent.hpp>
#include <rest_in_beast/server.hpp>

#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <future>
#include <memory>
#include <thread>

namespace rib = rest_in_beast;
namespace net = boost::asio;
namespace beast = boost::beast;

struct ServerFixture {
  const net::ip::tcp::endpoint endpoint{net::ip::make_address("127.0.0.1"),
                                        5000};

  std::shared_ptr<test::Respondent> respondent{
      test::Respondent::make_shared(test::responses_map())};
};

BOOST_FIXTURE_TEST_SUITE(server_tests, ServerFixture);

BOOST_AUTO_TEST_CASE(plain_to_plain) {
  auto server_logger = test::Logger::make_shared();
  auto client_logger = test::MemoLogger::make_shared();

  boost::asio::io_context io_ctx;

  net::signal_set signals(io_ctx, SIGINT);
  signals.async_wait(test::SignalsHandler{io_ctx, server_logger});

  test::ASIOThread server_worker{io_ctx};
  std::thread server_thread{server_worker.thread_body()};

  rib::PlainServer::start(io_ctx, endpoint, server_logger,
                          {.respondent = respondent, .logger = server_logger});

  const auto [requests, responses] = test::requests_test_data();

  auto future{
      test::PlainClient::send(io_ctx, client_logger, endpoint, requests)};

  BOOST_REQUIRE(future.valid());
  BOOST_REQUIRE(std::future_status::ready ==
                future.wait_for(std::chrono::seconds{5}));

  io_ctx.stop();
  server_thread.join();

  BOOST_REQUIRE(not server_worker.thread_exception);
  if (server_worker.thread_exception) {
    std::rethrow_exception(server_worker.thread_exception);
  }

  const auto responses_ret = future.get();

  BOOST_REQUIRE(not client_logger->last_ec().failed());
  BOOST_REQUIRE(not std::empty(responses_ret));
  BOOST_REQUIRE(std::size(responses_ret) == std::size(responses));

  for (std::size_t idx{}; idx < std::size(responses); ++idx) {
    BOOST_REQUIRE(responses[idx].result() == responses_ret[idx].result());
    BOOST_REQUIRE(responses[idx].body() == responses_ret[idx].body());
  }
}

BOOST_AUTO_TEST_CASE(secure_to_secur) {
  auto server_logger = test::Logger::make_shared();
  auto client_logger = test::MemoLogger::make_shared();

  boost::asio::io_context io_ctx;

  boost::asio::ssl::context server_ssl_ctx{test::make_server_ssl_ctx()};
  boost::asio::ssl::context client_ssl_ctx{test::make_client_ssl_ctx()};

  net::signal_set signals(io_ctx, SIGINT);
  signals.async_wait(test::SignalsHandler{io_ctx, server_logger});

  test::ASIOThread server_worker{io_ctx};
  std::thread server_thread{server_worker.thread_body()};

  rib::SecureServer::start(io_ctx, endpoint, server_logger,
                           {.ssl_ctx = server_ssl_ctx,
                            .respondent = respondent,
                            .logger = server_logger});

  const auto [requests, responses] = test::requests_test_data();

  auto future{test::SecureClient::send(io_ctx, client_ssl_ctx, client_logger,
                                       endpoint, requests)};

  BOOST_REQUIRE(future.valid());
  BOOST_REQUIRE(std::future_status::ready ==
                future.wait_for(std::chrono::seconds{5}));

  io_ctx.stop();
  server_thread.join();

  BOOST_REQUIRE(not server_worker.thread_exception);
  if (server_worker.thread_exception) {
    std::rethrow_exception(server_worker.thread_exception);
  }

  const auto responses_ret = future.get();

  BOOST_REQUIRE(not client_logger->last_ec().failed());
  BOOST_REQUIRE(not std::empty(responses_ret));
  BOOST_REQUIRE(std::size(responses_ret) == std::size(responses));

  for (std::size_t idx{}; idx < std::size(responses); ++idx) {
    BOOST_REQUIRE(responses[idx].result() == responses_ret[idx].result());
    BOOST_REQUIRE(responses[idx].body() == responses_ret[idx].body());
  }
}

BOOST_AUTO_TEST_CASE(secure_to_secure_wrong_ca) {
  auto server_logger = test::Logger::make_shared();
  auto client_logger = test::MemoLogger::make_shared();

  boost::asio::io_context io_ctx;

  boost::asio::ssl::context server_ssl_ctx{test::make_server_ssl_ctx()};
  boost::asio::ssl::context client_ssl_ctx{test::make_client_fake_ca_ssl_ctx()};

  net::signal_set signals(io_ctx, SIGINT);
  signals.async_wait(test::SignalsHandler{io_ctx, server_logger});

  test::ASIOThread server_worker{io_ctx};
  std::thread server_thread{server_worker.thread_body()};

  rib::SecureServer::start(io_ctx, endpoint, server_logger,
                           {.ssl_ctx = server_ssl_ctx,
                            .respondent = respondent,
                            .logger = server_logger});

  const auto [requests, responses] = test::requests_test_data();

  auto future{test::SecureClient::send(io_ctx, client_ssl_ctx, client_logger,
                                       endpoint, requests)};

  BOOST_REQUIRE(future.valid());
  BOOST_REQUIRE(std::future_status::ready ==
                future.wait_for(std::chrono::seconds{5}));

  io_ctx.stop();
  server_thread.join();

  BOOST_REQUIRE(not server_worker.thread_exception);
  if (server_worker.thread_exception) {
    std::rethrow_exception(server_worker.thread_exception);
  }

  const auto responses_ret = future.get();

  constexpr auto cert_verify_ec{167772294};
  constexpr auto unknown_ca_ec{167773208};

  BOOST_REQUIRE(client_logger->last_ec().failed());

  BOOST_REQUIRE(client_logger->last_ec().value() == cert_verify_ec);
  BOOST_REQUIRE(server_logger->last_ec().value() == unknown_ca_ec);

  BOOST_REQUIRE(std::empty(responses_ret));
}

BOOST_AUTO_TEST_CASE(plain_to_flex) {
  auto server_logger = test::Logger::make_shared();
  auto client_logger = test::MemoLogger::make_shared();

  boost::asio::io_context io_ctx;

  boost::asio::ssl::context server_ssl_ctx{test::make_server_ssl_ctx()};

  net::signal_set signals(io_ctx, SIGINT);
  signals.async_wait(test::SignalsHandler{io_ctx, server_logger});

  test::ASIOThread server_worker{io_ctx};
  std::thread server_thread{server_worker.thread_body()};

  rib::FlexServer::start(io_ctx, endpoint, server_logger,
                         {.ssl_ctx = server_ssl_ctx,
                          .respondent = respondent,
                          .logger = server_logger});

  const auto [requests, responses] = test::requests_test_data();

  auto future{
      test::PlainClient::send(io_ctx, client_logger, endpoint, requests)};

  BOOST_REQUIRE(future.valid());
  BOOST_REQUIRE(std::future_status::ready ==
                future.wait_for(std::chrono::seconds{5}));

  io_ctx.stop();
  server_thread.join();

  BOOST_REQUIRE(not server_worker.thread_exception);
  if (server_worker.thread_exception) {
    std::rethrow_exception(server_worker.thread_exception);
  }

  const auto responses_ret = future.get();

  BOOST_REQUIRE(not client_logger->last_ec().failed());
  BOOST_REQUIRE(not std::empty(responses_ret));
  BOOST_REQUIRE(std::size(responses_ret) == std::size(responses));

  for (std::size_t idx{}; idx < std::size(responses); ++idx) {
    BOOST_REQUIRE(responses[idx].result() == responses_ret[idx].result());
    BOOST_REQUIRE(responses[idx].body() == responses_ret[idx].body());
  }
}

BOOST_AUTO_TEST_CASE(secure_to_flex) {
  auto server_logger = test::Logger::make_shared();
  auto client_logger = test::MemoLogger::make_shared();

  boost::asio::io_context io_ctx;

  boost::asio::ssl::context server_ssl_ctx{test::make_server_ssl_ctx()};
  boost::asio::ssl::context client_ssl_ctx{test::make_client_ssl_ctx()};

  net::signal_set signals(io_ctx, SIGINT);
  signals.async_wait(test::SignalsHandler{io_ctx, server_logger});

  test::ASIOThread server_worker{io_ctx};
  std::thread server_thread{server_worker.thread_body()};

  rib::FlexServer::start(io_ctx, endpoint, server_logger,
                         {.ssl_ctx = server_ssl_ctx,
                          .respondent = respondent,
                          .logger = server_logger});

  const auto [requests, responses] = test::requests_test_data();

  auto future{test::SecureClient::send(io_ctx, client_ssl_ctx, client_logger,
                                       endpoint, requests)};

  BOOST_REQUIRE(future.valid());
  BOOST_REQUIRE(std::future_status::ready ==
                future.wait_for(std::chrono::seconds{5}));

  io_ctx.stop();
  server_thread.join();

  BOOST_REQUIRE(not server_worker.thread_exception);
  if (server_worker.thread_exception) {
    std::rethrow_exception(server_worker.thread_exception);
  }

  const auto responses_ret = future.get();

  BOOST_REQUIRE(not client_logger->last_ec().failed());
  BOOST_REQUIRE(not std::empty(responses_ret));
  BOOST_REQUIRE(std::size(responses_ret) == std::size(responses));

  for (std::size_t idx{}; idx < std::size(responses); ++idx) {
    BOOST_REQUIRE(responses[idx].result() == responses_ret[idx].result());
    BOOST_REQUIRE(responses[idx].body() == responses_ret[idx].body());
  }
}

BOOST_AUTO_TEST_CASE(secure_to_flex_wrong_ca) {
  auto server_logger = test::Logger::make_shared();
  auto client_logger = test::MemoLogger::make_shared();

  boost::asio::io_context io_ctx;

  boost::asio::ssl::context server_ssl_ctx{test::make_server_ssl_ctx()};
  boost::asio::ssl::context client_ssl_ctx{test::make_client_fake_ca_ssl_ctx()};

  net::signal_set signals(io_ctx, SIGINT);
  signals.async_wait(test::SignalsHandler{io_ctx, server_logger});

  test::ASIOThread server_worker{io_ctx};
  std::thread server_thread{server_worker.thread_body()};

  rib::FlexServer::start(io_ctx, endpoint, server_logger,
                         {.ssl_ctx = server_ssl_ctx,
                          .respondent = respondent,
                          .logger = server_logger});

  const auto [requests, responses] = test::requests_test_data();

  auto future{test::SecureClient::send(io_ctx, client_ssl_ctx, client_logger,
                                       endpoint, requests)};

  BOOST_REQUIRE(future.valid());
  BOOST_REQUIRE(std::future_status::ready ==
                future.wait_for(std::chrono::seconds{5}));

  io_ctx.stop();
  server_thread.join();

  BOOST_REQUIRE(not server_worker.thread_exception);
  if (server_worker.thread_exception) {
    std::rethrow_exception(server_worker.thread_exception);
  }

  const auto responses_ret = future.get();

  // Client reports certificate verification failure
  constexpr auto FAILED_CERT_VERIFY{167772294};
  // Server reports client's unknown CA
  constexpr auto UNKNOWN_CA{167773208};

  BOOST_REQUIRE(client_logger->last_ec().failed());

  BOOST_REQUIRE(client_logger->last_ec().value() == FAILED_CERT_VERIFY);
  BOOST_REQUIRE(server_logger->last_ec().value() == UNKNOWN_CA);

  BOOST_REQUIRE(std::empty(responses_ret));
}

BOOST_AUTO_TEST_SUITE_END();
