#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Use a timer asynchronously.
// Bind arguments to a callback function.

void Print(const boost::system::error_code& ec,
           boost::asio::deadline_timer* timer,
           int* count) {
  if (*count < 3) {
    std::cout << *count << std::endl;
    ++(*count);

    // Change the timer's expiry time in the callback function.
    timer->expires_at(timer->expires_at() + boost::posix_time::seconds(1));

    // Start a new asynchronous wait.
    timer->async_wait(boost::bind(Print, boost::asio::placeholders::error, timer, count));
  }
}

int main() {
  boost::asio::io_service io_service;

  boost::asio::deadline_timer timer(io_service, boost::posix_time::seconds(1));
  int count = 0;

  // async_wait() expects a handler function (or function object) with the signature
  // void(const boost::system::error_code&).
  // Binding the additional parameters converts your Print function into a function object
  // that matches the signature correctly.
  timer.async_wait(boost::bind(Print, boost::asio::placeholders::error, &timer, &count));

  io_service.run();

  std::cout << "Final count is " << count << std::endl;

  return 0;
}