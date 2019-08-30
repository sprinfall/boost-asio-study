// Wait a timer asynchronously.
// Use a lambda as the wait handler.

#include <chrono>
#include <iostream>

#include "boost/asio/io_context.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/core/ignore_unused.hpp"

// Call run_one() instead of run() of io_context.
#define CALL_RUN_ONE 0

int main() {
  boost::asio::io_context io_context;

  boost::asio::steady_timer timer{ io_context, std::chrono::seconds(3) };

#if CALL_RUN_ONE

  boost::system::error_code ec = boost::asio::error::would_block;

  timer.async_wait([&ec](boost::system::error_code inner_ec) {
    ec = inner_ec;
    std::cout << "Hello, World!" << std::endl;
  });

  // Block until the asynchronous operation has completed.
  // The do...while loop is optional for this case because we have triggered
  // only one async operation.
  // TODO: Trigger another asynchronous operation from within the handler.
  do {
    io_context.run_one();
  } while (ec == boost::asio::error::would_block);

#else

  timer.async_wait([](boost::system::error_code ec) {
    boost::ignore_unused(ec);
    std::cout << "Hello, World!" << std::endl;
  });

  io_context.run();

#endif  // CALL_RUN_ONE

  return 0;
}
