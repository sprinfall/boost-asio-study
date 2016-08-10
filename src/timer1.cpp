#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Use a timer synchronously.

int main() {
  boost::asio::io_service io_service;

  boost::asio::deadline_timer timer(io_service, boost::posix_time::seconds(3));
  timer.wait();  // Wait for the timer to expire.

  std::cout << "Hello, world!" << std::endl;

  return 0;
}