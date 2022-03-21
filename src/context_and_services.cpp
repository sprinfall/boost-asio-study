// context_and_services.cpp
// A study on the execution context and the services.

#include <iostream>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/detail/deadline_timer_service.hpp"
#include "boost/asio/detail/resolver_service.hpp"
#include "boost/asio/detail/strand_service.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/strand.hpp"

namespace asio = boost::asio;

int main() {
  asio::io_context io_context;

  bool has_service = false;
  std::cout << std::boolalpha;

  // NOTE: asio::deadline_timer::traits_type ->
  //       asio::time_traits<boost::posix_time::ptime>
  using DeadlineTimerService =
      asio::detail::deadline_timer_service<asio::deadline_timer::traits_type>;

  std::cout << asio::has_service<DeadlineTimerService>(io_context) << std::endl;
  // false

  // Create a deadline timer then the service will be available.
  asio::deadline_timer timer{ io_context };
  std::cout << asio::has_service<DeadlineTimerService>(io_context) << std::endl;
  // true

  // Calling `asio::use_service` instead also makes the service available:
  //   asio::use_service<DeadlineTimerService>(io_context) 

  using ResolverService = asio::detail::resolver_service<asio::ip::tcp>;
  asio::ip::tcp::resolver resolver{ io_context };
  std::cout << asio::has_service<ResolverService>(io_context) << std::endl;
  // true

  std::cout << asio::has_service<asio::detail::strand_service>(io_context)
            << std::endl;
  // false

  asio::io_context::strand strand{ io_context };
  std::cout << asio::has_service<asio::detail::strand_service>(io_context)
            << std::endl;
  // true

  // ...

  return 0;
}
