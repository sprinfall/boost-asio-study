#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp"

// Don't use any deprecated definitions (e.g., io_service).
#define BOOST_ASIO_NO_DEPRECATED

#if 0
// boost/asio.hpp includes a lot of commonly used Asio header files.
#include "boost/asio.hpp"
#else
// You may prefer to include as less as possible:
#include "boost/asio/io_context.hpp"
#include "boost/asio/deadline_timer.hpp"
#endif

// Use a timer synchronously.

int main() {
  boost::asio::io_context io_context;

  boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(3));
  timer.wait();  // Wait for the timer to expire.

  std::cout << "Hello, world!" << std::endl;

  return 0;
}