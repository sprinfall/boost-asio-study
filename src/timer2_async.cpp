// Wait a timer asynchronously.

#include <chrono>
#include <iostream>

#include "boost/asio/io_context.hpp"
#include "boost/asio/steady_timer.hpp"

// Handler to be called when the asynchronous wait finishes.
// The |error_code| parameter for all handlers in Asio is passed by value
// now instead of const reference. In some old examples, you may still notice
// the usage of const reference.
void Print(boost::system::error_code ec) {
  std::cout << "Hello, World!" << std::endl;
}

int main() {
  boost::asio::io_context io_context;

  boost::asio::steady_timer timer{ io_context, std::chrono::seconds(3) };

  timer.async_wait(&Print);

  // NOTE:
  // Removing '&' from "&Print" also works (but not for member functions): 
  //   timer.async_wait(Print);

  std::size_t size = io_context.run();
  std::cout << "Number of handlers executed: " << size << std::endl;  // 1

  return 0;
}
