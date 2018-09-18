#include <iostream>
#include <thread>

#define BOOST_ASIO_NO_DEPRECATED

#include "boost/asio/bind_executor.hpp"
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/strand.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

using boost::asio::bind_executor;
using boost::posix_time::seconds;

// Synchronise callback handlers in a multithreaded program.
// Class strand provides the ability to post and dispatch handlers with the
// guarantee that none of those handlers will execute concurrently.

class Printer {
public:
  Printer(boost::asio::io_context& io_context)
      : strand_(io_context),
        timer1_(io_context, seconds(1)),
        timer2_(io_context, seconds(1)),
        count_(0) {
    timer1_.async_wait(bind_executor(strand_, std::bind(&Printer::Print1, this)));
    timer2_.async_wait(bind_executor(strand_, std::bind(&Printer::Print2, this)));
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void Print1() {
    if (count_ < 6) {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;

      timer1_.expires_at(timer1_.expires_at() + seconds(1));
      timer1_.async_wait(bind_executor(strand_, std::bind(&Printer::Print1, this)));
    }
  } 

  void Print2() {
    if (count_ < 6) {
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      timer2_.expires_at(timer2_.expires_at() + seconds(1));
      timer2_.async_wait(bind_executor(strand_, std::bind(&Printer::Print2, this)));
    }
  }

private:
  boost::asio::io_context::strand strand_;
  boost::asio::deadline_timer timer1_;
  boost::asio::deadline_timer timer2_;
  int count_;
};

int main() {
  boost::asio::io_context io_context;

  Printer printer(io_context);

  // The new thread runs a loop.
  std::thread thread(std::bind(&boost::asio::io_context::run, &io_context));

  // The main thread runs another loop.
  io_context.run();

  // Wait for the thread to finish.
  thread.join();

  return 0;
}
