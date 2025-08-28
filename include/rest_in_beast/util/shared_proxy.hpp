#ifndef REST_IN_BEAST_SHARED_PROXY_HPP
#define REST_IN_BEAST_SHARED_PROXY_HPP

#include <utility>

namespace rest_in_beast {
namespace util {

/**
 * @brief The SharedProxy class is the proxy class for classes with hidden
 * constructor and provides static make_shared or make_unique methods to create
 * it's instance
 */
template <typename Base> struct SharedProxy : public Base {
  template <typename... Ts>
  SharedProxy(Ts&&... args) : Base{std::forward<Ts>(args)...} {}
};
} // namespace util
} // namespace rest_in_beast

#endif // REST_IN_BEAST_SHARED_PROXY_HPP
