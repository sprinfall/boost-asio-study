#include <iostream>

#include "boost/asio/io_context.hpp"

// Empty loop.
// Print after the loop ends.

int main() {
  boost::asio::io_context io_context;
  io_context.run();

  std::cout << "Hello, World!" << std::endl;

  return 0;
}