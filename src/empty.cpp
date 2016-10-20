#include <iostream>
#include <boost/asio.hpp>

// Empty loop.

int main() {
  boost::asio::io_service io_service;
  io_service.run();

  std::cout << "Hello, world!" << std::endl;

  return 0;
}