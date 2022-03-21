// Wait a timer asynchronously.
// Bind extra arguments to a function so that it matches the signature of
// the expected handler.

#include <chrono>
#include <functional>
#include <iostream>

#include "boost/asio/steady_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

void Print(boost::system::error_code ec,
           boost::asio::steady_timer& timer,
           int& count) {
  if (count < 3) {
    std::cout << count << std::endl;
    ++count;

    // Change the timer's expiry time.
    timer.expires_after(std::chrono::seconds(1));

    // Start a new asynchronous wait.
    timer.async_wait(std::bind(&Print, std::placeholders::_1, std::ref(timer),
                               std::ref(count)));
  }
}

int main() {
  boost::asio::io_context io_context;

  boost::asio::steady_timer timer{ io_context, std::chrono::seconds(1) };

  int count = 0;

  // async_wait() expects a handler function (or function object) with the
  // signature |void(boost::system::error_code)|.
  // Binding the additional parameters converts your Print function into a
  // function object that matches the signature correctly.
  timer.async_wait(std::bind(&Print, std::placeholders::_1, std::ref(timer),
                             std::ref(count)));

  io_context.run();

  std::cout << "Final count is " << count << std::endl;

  return 0;
}
