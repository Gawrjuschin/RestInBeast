//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TEST_REQUESTS_HPP
#define TEST_REQUESTS_HPP

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <vector>

namespace test {
inline const auto& responses_map() {
  namespace http = boost::beast::http;
  auto MakeResponses = []() {
    std::unordered_map<std::string_view, http::response<http::string_body>>
        responses{};

    // not_found
    {
      http::response<http::empty_body> response{http::status::not_found, 11};
      response.set(boost::beast::http::field::server,
                   BOOST_BEAST_VERSION_STRING);

      responses.emplace("not_found", std::move(response));
    }
    // not_implemented
    {
      http::response<http::empty_body> response{http::status::not_implemented,
                                                11};
      response.set(boost::beast::http::field::server,
                   BOOST_BEAST_VERSION_STRING);

      responses.emplace("not_implemented", std::move(response));
    }

    // index
    {
      http::response<http::empty_body> response{http::status::ok, 11};
      response.set(boost::beast::http::field::server,
                   BOOST_BEAST_VERSION_STRING);

      responses.emplace("/", std::move(response));
    }

    // TODO: add more requests with body

    return responses;
  };

  static auto responses{MakeResponses()};
  return responses;
}

inline auto requests_test_data() {
  namespace http = boost::beast::http;

  std::pair<std::vector<http::request<http::string_body>>,
            std::vector<http::response<http::string_body>>>
      requests_data;

  // not exist request
  {
    requests_data.first.emplace_back(http::verb::get, "/not_exist", 11);
    requests_data.first.back().set(http::field::host, "127.0.0.1");
    requests_data.first.back().set(boost::beast::http::field::user_agent,
                                   BOOST_BEAST_VERSION_STRING);

    requests_data.second.emplace_back(responses_map().at("not_found"));
  }

  // index post request
  {
    requests_data.first.emplace_back(http::verb::post, "/", 11);
    requests_data.first.back().set(http::field::host, "127.0.0.1");
    requests_data.first.back().set(boost::beast::http::field::user_agent,
                                   BOOST_BEAST_VERSION_STRING);

    requests_data.second.emplace_back(responses_map().at("not_implemented"));
  }

  // index get request
  {
    requests_data.first.emplace_back(boost::beast::http::verb::get, "/", 11);
    requests_data.first.back().set(boost::beast::http::field::host,
                                   "127.0.0.1");
    requests_data.first.back().set(boost::beast::http::field::user_agent,
                                   BOOST_BEAST_VERSION_STRING);

    requests_data.second.emplace_back(responses_map().at("/"));
  }

  return requests_data;
}

} // namespace test

#endif // TEST_REQUESTS_HPP
