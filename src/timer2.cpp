#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

// Use a timer asynchronously.

// Callback function to be called when the asynchronous wait finishes.
void SayHello(const boost::system::error_code&) {
  std::cout << "Hello, world!" << std::endl;
}

int main() {
  boost::asio::io_service io_service;
  boost::asio::deadline_timer timer(io_service, boost::posix_time::seconds(3));
  timer.async_wait(&SayHello);

  // This call will not return until the timer has expired and the callback
  // has completed.
  io_service.run();

  //size_t size = io_service.run();
  //std::cout << "number of handlers executed: " << size << std::endl;  // 1

  // TODO: This won't print the "Hello, world!".
  //io_service.poll();
  //boost::this_thread::sleep(boost::posix_time::seconds(4));

  return 0;
}