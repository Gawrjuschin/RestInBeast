#ifndef RESIN_IN_BEAST_RESPONDENT_H
#define RESIN_IN_BEAST_RESPONDENT_H

#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/message_generator_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>

namespace rest_in_beast {
/**
 * @brief The Respondent is an interface for user-customizable requests
 * handler classes used in session to make response. It is the main point for
 * behaviour customization of server.
 */
class Respondent {
public:
  Respondent() = default;

  virtual ~Respondent() = default;

  /**
   * @brief make_response is the only abstract class of request_handler
   * @param request - string_body http request is used in session
   * @return type-erased http response
   */
  virtual boost::beast::http::message_generator
  make_response(boost::beast::http::request<boost::beast::http::string_body>&&
                    request) = 0;
};

} // namespace rest_in_beast

#endif // RESIN_IN_BEAST_RESPONDENT_H
