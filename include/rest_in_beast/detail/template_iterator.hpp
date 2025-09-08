//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RESIN_IN_BEAST_TEMPLATE_ITERATOR_HPP
#define RESIN_IN_BEAST_TEMPLATE_ITERATOR_HPP

#include <iterator>
#include <string_view>
#include <utility>

namespace rest_in_beast {
/**
 * @brief The TemplateIterator class is inspired by
 * std::filesystem::directory_iterator to provide tokenization of simple strings
 * templates
 */
class TemplateIterator {
  std::string_view tail{};
  std::string_view current{};
  size_t open_pos{};
  size_t close_pos{};

public:
  /**
   * @brief The Token class is an proxy type for iterator
   */
  struct Token {
    bool is_name{};
    std::string_view current;
  };

  using value_type = Token;
  using difference_type = ptrdiff_t;
  using pointer = const std::pair<bool, std::string_view>*;
  using reference = const std::pair<bool, std::string_view>&;
  using iterator_category = std::input_iterator_tag;

  constexpr TemplateIterator() = default;

  constexpr TemplateIterator(std::string_view src) : tail{src} {
    open_pos = tail.find("{{");
    current = tail.substr(0, open_pos);
  }

  constexpr TemplateIterator operator++(int) {
    const auto it = *this;
    ++(*this);
    return it;
  }

  constexpr TemplateIterator operator++() const {
    auto it = *this;
    return ++it;
  }

  constexpr TemplateIterator& operator++() {
    if (std::empty(tail))
      return *this;

    // head -> guardian
    if (open_pos == std::string_view::npos)
      return (*this = {});

    // HEAD: open_pos != 0; close_pos == 0
    // head -> name
    if (close_pos == 0) {
      close_pos = tail.find("}}");

      // ошибка!!! для открывающей скобки не нашлось закрывающей
      if (close_pos == std::string_view::npos)
        return (*this = {});

      current = tail.substr(open_pos + 2, (close_pos - open_pos - 2));
      return *this;
    }

    // NAME: open_pos != 0; close_pos != 0
    // name -> head
    tail = tail.substr(close_pos + 2);
    close_pos = 0;
    open_pos = tail.find("{{");
    current = tail.substr(0, open_pos);
    return *this;
  }

  constexpr value_type operator*() const noexcept {
    return {close_pos != 0, current};
  }

  constexpr TemplateIterator* operator->() noexcept { return this; }
  constexpr const TemplateIterator* operator->() const noexcept { return this; }

  constexpr bool operator==(const TemplateIterator& other) const noexcept {
    return tail == other.tail;
  }

  constexpr bool operator!=(const TemplateIterator& other) const noexcept {
    return !(*this == other);
  }
};
// free functions like in std::filesystem::directory_iteartor
constexpr inline TemplateIterator begin(TemplateIterator it) noexcept {
  return it;
}
constexpr inline TemplateIterator end(TemplateIterator) noexcept { return {}; }
constexpr inline const TemplateIterator
cbegin(const TemplateIterator& it) noexcept {
  return it;
}
constexpr inline const TemplateIterator cend(const TemplateIterator&) noexcept {
  return {};
}

/**
 * @brief recurl_variable extends string_view of var_name to surrounding curled
 * braces, removed on parsing
 * @param var_name
 * @return
 */
inline constexpr std::string_view
recurl_variable(std::string_view var_name) noexcept {
  return {std::data(var_name) - 2, std::size(var_name) + 4};
}

/**
 * @brief recurl_variable extends string_view of var_name to surrounding curled
 * braces, removed on parsing
 * @param var_name
 * @return
 */
inline constexpr std::string_view
recurl_variable(const TemplateIterator::Token& tok) {
  if (tok.is_name)
    return recurl_variable(tok.current);

  return {};
}

} // namespace rest_in_beast

#endif // RESIN_IN_BEAST_TEMPLATE_ITERATOR_HPP
