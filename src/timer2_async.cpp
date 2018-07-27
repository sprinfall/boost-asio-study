#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp"

#define BOOST_ASIO_NO_DEPRECATED
#if 0
#include "boost/asio.hpp"
#else
#include "boost/asio/io_context.hpp"
#include "boost/asio/deadline_timer.hpp"
#endif

// Use a timer asynchronously.

// Handler to be called when the asynchronous wait finishes.
void Print(boost::system::error_code ec) {
  std::cout << "Hello, world!" << std::endl;
}

int main() {
  boost::asio::io_context io_context;
  boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(3));

  timer.async_wait(&Print);

  std::size_t size = io_context.run();
  std::cout << "Number of handlers executed: " << size << std::endl;  // 1

  return 0;
}
