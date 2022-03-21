#include <chrono>
#include <iostream>
#include <thread>

#include "boost/asio/bind_executor.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/asio/strand.hpp"

// Synchronise callback handlers in a multi-threaded program.
// Class strand provides the ability to post and dispatch handlers with the
// guarantee that none of those handlers will execute concurrently.

class Printer {
public:
  explicit Printer(boost::asio::io_context& io_context)
      : strand_(boost::asio::make_strand(io_context)),
        timer1_(strand_, std::chrono::seconds(1)),
        timer2_(strand_, std::chrono::seconds(1)) {
    timer1_.async_wait(std::bind(&Printer::Print1, this));
    timer2_.async_wait(std::bind(&Printer::Print2, this));
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void Print1() {
    if (count_ < 6) {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;

      timer1_.expires_after(std::chrono::seconds(1));
      timer1_.async_wait(std::bind(&Printer::Print1, this));
    }
  } 

  void Print2() {
    if (count_ < 6) {
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      timer2_.expires_after(std::chrono::seconds(1));
      timer2_.async_wait(std::bind(&Printer::Print2, this));
    }
  }

private:
  // NOTE: `asio::strand` is a different class from `io_context::strand`.
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::asio::steady_timer timer1_;
  boost::asio::steady_timer timer2_;
  int count_ = 0;
};

int main() {
  boost::asio::io_context io_context;

  Printer printer{ io_context };

  // The new thread runs a loop.
  std::thread t{ std::bind(&boost::asio::io_context::run, &io_context) };

  // The main thread runs another loop.
  io_context.run();

  // Wait for the thread to finish.
  t.join();

  return 0;
}
