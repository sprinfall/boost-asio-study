#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Use a timer asynchronously with binding extra parameters.

void SayHello(const boost::system::error_code&,
              boost::asio::deadline_timer* timer,
              int* count) {
  std::cout << "Hello, world!" << std::endl;

  if (--(*count) > 0) {
    timer->expires_at(timer->expires_at() + boost::posix_time::seconds(1));
    timer->async_wait(boost::bind(SayHello, _1, timer, count));
  }
}

int main() {
  boost::asio::io_service io_service;
  boost::asio::deadline_timer timer(io_service, boost::posix_time::seconds(1));

  int count = 3;
  timer.async_wait(boost::bind(SayHello, _1, &timer, &count));

  io_service.run();

  return 0;
}