#define BOOST_TEST_MODULE TemplateTests
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>
#include <rest_in_beast/detail/template_iterator.hpp>
#include <rest_in_beast/template.hpp>
#include <string_view>

namespace rib = rest_in_beast;

using namespace std::string_view_literals;

namespace test {
struct PageData {
  std::string title;
  std::string alt_title;
};

} // namespace test

struct TemplatesRenderFixture {
  static void PageDataWriter(std::back_insert_iterator<std::string> out_it,
                             std::string_view var_name,
                             const test::PageData& data) {
    if (var_name == "title") {
      std::copy(std::cbegin(data.title), std::cend(data.title), out_it);
      return;
    }

    if (var_name == "alt_title") {
      std::copy(std::cbegin(data.alt_title), std::cend(data.alt_title), out_it);
      return;
    }

    std::string_view skipped_var{rib::recurl_variable(var_name)};
    std::copy(std::cbegin(skipped_var), std::cend(skipped_var), out_it);
  }
};

BOOST_FIXTURE_TEST_SUITE(templates_render, TemplatesRenderFixture)

BOOST_AUTO_TEST_CASE(no_args) {
  constexpr std::string_view result{R"(<div>example</div>)"};
  std::string buffer;
  buffer.reserve(32);

  rib::TemplateView<test::PageData> tmpl{};
  tmpl.load("<div>example</div>");

  tmpl.render(std::back_inserter(buffer), PageDataWriter,
              test::PageData{.title = "example"});

  BOOST_REQUIRE(buffer == result);
}

BOOST_AUTO_TEST_CASE(skip_arg) {
  constexpr std::string_view result{R"(<div>{{toitle}}</div>)"};
  std::string buffer;
  buffer.reserve(32);

  rib::TemplateView<test::PageData> tmpl{};
  tmpl.load("<div>{{toitle}}</div>");

  tmpl.render(std::back_inserter(buffer), PageDataWriter,
              test::PageData{.title = "example"});

  BOOST_REQUIRE(buffer == result);
}

BOOST_AUTO_TEST_CASE(one_arg) {
  constexpr std::string_view result{R"(<div>example</div>)"};
  std::string buffer;
  buffer.reserve(32);

  rib::TemplateView<test::PageData> tmpl{};
  tmpl.load("<div>{{title}}</div>");

  tmpl.render(std::back_inserter(buffer), PageDataWriter,
              test::PageData{.title = "example"});

  BOOST_REQUIRE(buffer == result);
}

BOOST_AUTO_TEST_CASE(repeate_arg) {
  constexpr std::string_view result{R"(<div>example::example</div>)"};
  std::string buffer;
  buffer.reserve(32);

  rib::TemplateView<test::PageData> tmpl{};
  tmpl.load("<div>{{title}}::{{title}}</div>");

  tmpl.render(std::back_inserter(buffer), PageDataWriter,
              test::PageData{.title = "example"});

  BOOST_REQUIRE(buffer == result);
}

BOOST_AUTO_TEST_CASE(two_args) {
  constexpr std::string_view result{R"(<div>example::alternative</div>)"};
  std::string buffer;
  buffer.reserve(32);

  rib::TemplateView<test::PageData> tmpl{};
  tmpl.load("<div>{{title}}::{{alt_title}}</div>");

  tmpl.render(std::back_inserter(buffer), PageDataWriter,
              test::PageData{.title = "example", .alt_title = "alternative"});

  BOOST_REQUIRE(buffer == result);
}

BOOST_AUTO_TEST_SUITE_END();
