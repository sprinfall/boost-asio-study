#include <iostream>

#include <boost/asio/io_context.hpp>

// Empty loop.

int main() {
  boost::asio::io_context ioc;
  ioc.run();

  std::cout << "Hello, world!" << std::endl;

  return 0;
}