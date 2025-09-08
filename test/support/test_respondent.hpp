//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TEST_RESPONDENT_H
#define TEST_RESPONDENT_H

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <memory>
#include <rest_in_beast/detail/respondent.hpp>
#include <rest_in_beast/util/shared_proxy.hpp>

namespace test {
using string_request =
    boost::beast::http::request<boost::beast::http::string_body>;

using string_response =
    boost::beast::http::response<boost::beast::http::string_body>;

class Respondent : public rest_in_beast::Respondent {
  std::unordered_map<std::string_view, string_response> responses_;

  Respondent(std::unordered_map<std::string_view, string_response>&& responses)
      : responses_{std::move(responses)} {
    if (!responses_.contains("not_found")) {
      throw std::runtime_error{"responses does not contain 'not_found'"};
    }
    if (!responses_.contains("not_implemented")) {
      throw std::runtime_error{"responses does not contain 'not_found'"};
    }
    // TODO: other responses
  };

  friend rest_in_beast::util::SharedProxy<Respondent>;

public:
  ~Respondent() = default;

  static std::shared_ptr<Respondent>
  make_shared(std::unordered_map<std::string_view, string_response> responses) {
    return std::make_shared<rest_in_beast::util::SharedProxy<Respondent>>(
        std::move(responses));
  }

  boost::beast::http::message_generator
  make_response(string_request&& request) override {
    switch (request.method()) {
    case boost::beast::http::verb::get: {
      auto response_it = responses_.find(request.target());

      if (response_it == std::cend(responses_)) {
        response_it = responses_.find("not_found");
        assert(response_it != std::end(responses_));
      }

      auto response = response_it->second;
      // IMPORTANT!
      response.prepare_payload();

      return response;
    }
    default: {
      const auto response_it = responses_.find("not_implemented");
      assert(response_it != std::cend(responses_));

      auto response = response_it->second;
      // IMPORTANT!
      response.prepare_payload();

      return response;
    }
    }
  }
};

} // namespace test

#endif // TEST_RESPONDENT_H
