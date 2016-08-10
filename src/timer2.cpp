#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Use a timer asynchronously.

// Callback function to be called when the asynchronous wait finishes.
void print(const boost::system::error_code&) {
  std::cout << "Hello, world!" << std::endl;
}

int main() {
  boost::asio::io_service io_service;
  boost::asio::deadline_timer timer(io_service, boost::posix_time::seconds(3));
  timer.async_wait(&print);

  // This call will not return until the timer has expired and the callback
  // has completed.
  io_service.run();

  return 0;
}