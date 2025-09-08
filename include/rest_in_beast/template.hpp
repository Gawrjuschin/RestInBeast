//
// Author: Dmitriy Gavryushin (https://github.com/Gawrjuschin)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RESIN_IN_BEAST_TEMPLATE_NEW_HPP
#define RESIN_IN_BEAST_TEMPLATE_NEW_HPP

#include "detail/template_iterator.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace rest_in_beast {
/**
 * @brief The Template class for simple page templates than replaces template
 * variables with their renders
 *
 * DOES NOT own template's body
 */
template <typename TemplateData> class TemplateView {
  std::vector<TemplateIterator::Token> template_tokens_;

public:
  TemplateView() = default;

  TemplateView(std::string_view tmpl)
      : template_tokens_{TemplateIterator{tmpl}, TemplateIterator{}} {}

  TemplateView(const TemplateView&) = default;
  TemplateView& operator=(const TemplateView&) = default;

  TemplateView(TemplateView&&) = default;
  TemplateView& operator=(TemplateView&&) = default;

  ~TemplateView() = default;

  /**
   * @brief loads the template body to the template class object
   * @param tmpl is the template body
   */
  void load(std::string_view tmpl) {
    template_tokens_.assign(TemplateIterator{tmpl}, TemplateIterator{});
  }

  /**
   * @brief is_template_variables_renderer is a check for template variables
   * writer callable object
   */
  template <typename Fn>
  static constexpr bool is_template_variables_renderer =
      std::is_invocable_v<Fn, std::back_insert_iterator<std::string>,
                          std::string_view, const TemplateData&>;

  /**
   * @brief render method renders template, where template variables will be
   * replaced with results of template_vars_writer calls with specified data
   * @param out_it is an output iterator for optimized by memory allocations
   * output
   * @param template_vars_writer is a callable object that implements each
   * template's variable rendering
   * @param data is an template data, passed to the template_vars_writer
   */
  template <typename Fn>
  void render(std::back_insert_iterator<std::string> out_it,
              Fn&& template_vars_writer, const TemplateData& data) const {
    static_assert(is_template_variables_renderer<Fn>,
                  "an callable object with back_inserter_iterator<std::string> "
                  "and TemplateData arguments expected");

    for (const auto& [is_variable, value] : template_tokens_) {
      if (is_variable) {
        static_assert(
            std::is_invocable_v<Fn, std::back_insert_iterator<std::string>,
                                std::string_view, const TemplateData&>);
        std::invoke(template_vars_writer, out_it, value, data);
      } else {
        // std::format_to(out_it, "{}", value);
        std::copy(std::cbegin(value), std::cend(value), out_it);
      }
    }
  }
};

/**
 * @brief The Template class for simple page templates. Replaces template
 * variables with their renders
 *
 *  DOES own it's template body
 */
template <typename TemplateData>
class Template : private TemplateView<TemplateData> {
  std::string template_body_;

public:
  Template() = default;

  Template(const std::string& template_body)
      : template_body_{template_body},
        TemplateView<TemplateData>{template_body_} {}

  Template(std::string&& template_body)
      : template_body_{std::move(template_body)},
        TemplateView<TemplateData>{template_body_} {}

  Template(const Template&) = default;
  Template& operator=(const Template&) = default;

  Template(Template&&) = default;
  Template& operator=(Template&&) = default;

  ~Template() = default;

  /**
   * @brief load - loads the template body to the template class object
   * @param template_body
   */
  void load(const std::string& template_body) {
    template_body_.assign(template_body);
    TemplateView<TemplateData>::load(template_body_);
  }

  /**
   * @brief load - loads the template body to the template class object
   * @param template_body
   */
  void load(std::string&& template_body) {
    template_body_.assign(std::move(template_body));
    TemplateView<TemplateData>::load(template_body_);
  }

  /**
   * @brief render method renders template, where template variables will be
   * replaced with results of template_vars_writer calls with specified data
   * @param out_it is an output iterator for optimized by memory allocations
   * output
   * @param template_vars_writer is a callable object that implements each
   * template's variable rendering
   * @param data is an template data, passed to the template_vars_writer
   */
  template <typename Fn>
  void render(std::back_insert_iterator<std::string> out_it,
              Fn&& template_vars_writer, const TemplateData& data) const {
    return TemplateView<TemplateData>::render(
        out_it, std::forward<Fn>(template_vars_writer), data);
  }
};

} // namespace rest_in_beast

#endif // RESIN_IN_BEAST_TEMPLATE_NEW_HPP
