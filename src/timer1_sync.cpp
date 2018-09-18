// Wait a timer synchronously.

#include <iostream>

// Don't use any deprecated definitions (e.g., io_service).
#define BOOST_ASIO_NO_DEPRECATED

// Including "boost/asio.hpp" directly might simplify a lot but also introduce
// many unnecessary header files. You should never include "boost/asio.hpp" in
// your own header files. Try to include as less as possible.
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

int main() {
  boost::asio::io_context io_context;

  boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(3));

  // All asynchronous APIs in Asio start with a prefix "async_".
  // Here we just call a synchronous API which will block until the timer
  // expires.
  timer.wait();

  // 3 seconds later, print hello world.
  std::cout << "Hello, World!" << std::endl;

  // No need to call io_context.run() since we don't have any asynchronous
  // operations to execute.

  return 0;
}
