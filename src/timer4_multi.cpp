// Wait multiple timers asynchronously.

#include <chrono>
#include <iostream>

#include "boost/asio/io_context.hpp"
#include "boost/asio/steady_timer.hpp"

void Print(boost::system::error_code ec) {
  std::cout << "Hello, World!" << std::endl;
}

int main() {
  boost::asio::io_context io_context;

  boost::asio::steady_timer timer1{ io_context, std::chrono::seconds(3) };
  boost::asio::steady_timer timer2{ io_context, std::chrono::seconds(3) };

  timer1.async_wait(&Print);
  timer2.async_wait(&Print);

  std::size_t size = io_context.run();
  std::cout << "Number of handlers executed: " << size << std::endl;  // 1

  return 0;
}
