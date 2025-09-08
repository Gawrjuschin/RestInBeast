//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RESIN_IN_BEAST_HASHER_HPP
#define RESIN_IN_BEAST_HASHER_HPP

#include <boost/utility/string_view.hpp>
#include <string>
#include <string_view>

namespace rest_in_beast {
namespace util {
/**
 * @brief The string_view_hash class is an advanced hasher supports string_view
 */
struct string_view_hash {
  using is_transparent = void;

  [[nodiscard]] std::size_t operator()(boost::string_view text) const noexcept {
    return std::hash<std::string_view>{}(
        std::string_view{std::data(text), std::size(text)});
  }

  [[nodiscard]] std::size_t operator()(const char* text) const noexcept {
    return std::hash<std::string_view>{}(text);
  }

  [[nodiscard]] std::size_t operator()(std::string_view text) const noexcept {
    return std::hash<std::string_view>{}(text);
  }

  [[nodiscard]] std::size_t operator()(const std::string& text) const noexcept {
    return std::hash<std::string>{}(text);
  }
};
} // namespace util
} // namespace rest_in_beast

#endif // RESIN_IN_BEAST_HASHER_HPP
