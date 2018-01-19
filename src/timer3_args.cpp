#include <iostream>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#if 0
#include <boost/asio.hpp>
#else
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#endif

// Use a timer asynchronously.
// Bind arguments to a callback function.

void Print(boost::system::error_code ec,
           boost::asio::deadline_timer* timer,
           int* count) {
  if (*count < 3) {
    std::cout << *count << std::endl;
    ++(*count);

    // Change the timer's expiry time in the callback function.
    timer->expires_at(timer->expires_at() + boost::posix_time::seconds(1));

    // Start a new asynchronous wait.
    timer->async_wait(boost::bind(Print, boost::placeholders::_1, timer, count));

    // The following 3 other ways are also OK:
    // timer->async_wait(boost::bind(Print, boost::asio::placeholders::error, timer, count));
    // timer->async_wait(boost::bind(Print, _1, timer, count));
    // timer->async_wait(std::bind(Print, std::placeholders::_1, timer, count));

    // The 2nd way works because of "using namespace boost::placeholders;" in
    // boost/bind.hpp.
  }
}

int main() {
  boost::asio::io_context ioc;

  boost::asio::deadline_timer timer(ioc, boost::posix_time::seconds(1));
  int count = 0;

  // async_wait() expects a handler function (or function object) with the
  // signature void(boost::system::error_code).
  // Binding the additional parameters converts your Print function into a
  // function object that matches the signature correctly.
  timer.async_wait(boost::bind(Print, boost::placeholders::_1, &timer, &count));

  ioc.run();

  std::cout << "Final count is " << count << std::endl;

  return 0;
}