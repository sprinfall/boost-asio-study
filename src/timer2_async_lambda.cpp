// Wait a timer asynchronously.
// Use a lambda as the wait handler.

#include <iostream>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

// Call io_context.run_one() instead.
#define CALL_RUN_ONE 1

int main() {
  boost::asio::io_context io_context;

  boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(3));

#if CALL_RUN_ONE

  boost::system::error_code ec = boost::asio::error::would_block;

  timer.async_wait([&ec](boost::system::error_code inner_ec) {
    ec = inner_ec;
    std::cout << "Hello, world!" << std::endl;
  });

  // Block until the asynchronous operation has completed.
  //do {
    io_context.run_one();
  //} while (ec == boost::asio::error::would_block);

#else

  timer.async_wait([](boost::system::error_code /*ec*/) {
    std::cout << "Hello, world!" << std::endl;
  });

  io_context.run();

#endif  // CALL_RUN_ONE

  return 0;
}
