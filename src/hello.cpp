#include <iostream>

#include "boost/asio/io_context.hpp"

// Empty loop.

int main() {
  boost::asio::io_context io_context;
  io_context.run();

  std::cout << "Hello, world!" << std::endl;

  return 0;
}