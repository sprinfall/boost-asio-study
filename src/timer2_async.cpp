// Wait a timer asynchronously.

#include <iostream>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

// Handler to be called when the asynchronous wait finishes.
// NOTE:
// The |error_code| parameter for all handlers in Asio is passed by value
// now instead of const reference. In some old examples, you may still notice
// the usage of const reference.
void Print(boost::system::error_code ec) {
  std::cout << "Hello, World!" << std::endl;
}

int main() {
  boost::asio::io_context io_context;

  boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(3));

  timer.async_wait(&Print);

  // NOTE:
  // Removing '&' from "&Print" also works (but not for member functions): 
  //   timer.async_wait(Print);

  std::size_t size = io_context.run();
  std::cout << "Number of handlers executed: " << size << std::endl;  // 1

  return 0;
}
