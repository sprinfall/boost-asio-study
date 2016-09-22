#include <iostream>
#include <boost/asio.hpp>

// libuv equivalent examples.

// http://nikhilm.github.io/uvbook/basics.html#event-loops
// helloworld/main.c

int main() {
  boost::asio::io_service io_service;

  std::cout << "Now quitting." << std::endl;

  io_service.run();

  return 0;
}