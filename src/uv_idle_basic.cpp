#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// http://nikhilm.github.io/uvbook/basics.html#event-loops
// idle-basic/main.c

int64_t counter = 0;

void wait_for_a_while(/*boost::asio::io_service& io_service*/) {
  counter++;

  if (counter < 100) {
    //io_service.post(boost::bind(wait_for_a_while, io_service));
  }

  std::cout << "wait for a while." << std::endl;
}

int main() {
  boost::asio::io_service io_service;

  //io_service.post(boost::bind(wait_for_a_while, io_service));
  io_service.post(wait_for_a_while);
  io_service.post(wait_for_a_while);

  io_service.run();

  return 0;
}