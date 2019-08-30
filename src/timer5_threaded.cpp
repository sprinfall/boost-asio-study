// Wait multiple timers asynchronously.
// At the same time, run the loop in two threads.

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include "boost/asio/io_context.hpp"
#include "boost/asio/steady_timer.hpp"

std::mutex g_io_mutex;

void Print(boost::system::error_code ec) {
  std::lock_guard<std::mutex> lock(g_io_mutex);

  std::cout << "Hello, World!";
  std::cout << " (" << std::this_thread::get_id() << ")" << std::endl;
}

int main() {
  boost::asio::io_context io_context;

  boost::asio::steady_timer timer1{ io_context, std::chrono::seconds(3) };
  boost::asio::steady_timer timer2{ io_context, std::chrono::seconds(3) };

  timer1.async_wait(&Print);
  timer2.async_wait(&Print);

  // Run the loop in 2 threads.
  std::thread t1{ &boost::asio::io_context::run, &io_context };
  std::thread t2{ &boost::asio::io_context::run, &io_context };

  // Wait for the 2 loops to end.
  t1.join();
  t2.join();

  return 0;
}
