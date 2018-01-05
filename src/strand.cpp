#include <iostream>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#if 0
#include <boost/asio.hpp>
#else
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/strand.hpp>
#endif

using boost::asio::bind_executor;

// Synchronise callback handlers in a multithreaded program.
// Class strand provides the ability to post and dispatch handlers with the
// guarantee that none of those handlers will execute concurrently.

class Printer {
public:
  Printer(boost::asio::io_context& ioc)
      : strand_(ioc)
      , timer1_(ioc, boost::posix_time::seconds(1))
      , timer2_(ioc, boost::posix_time::seconds(1))
      , count_(0) {

    timer1_.async_wait(bind_executor(strand_, boost::bind(&Printer::Print1, this)));
    timer2_.async_wait(bind_executor(strand_, boost::bind(&Printer::Print2, this)));
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void Print1() {
    if (count_ < 6) {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;

      timer1_.expires_at(timer1_.expires_at() + boost::posix_time::seconds(1));
      timer1_.async_wait(bind_executor(strand_, boost::bind(&Printer::Print1, this)));
    }
  } 

  void Print2() {
    if (count_ < 6) {
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      timer2_.expires_at(timer2_.expires_at() + boost::posix_time::seconds(1));
      timer2_.async_wait(bind_executor(strand_, boost::bind(&Printer::Print2, this)));
    }
  }

private:
  boost::asio::io_context::strand strand_;
  boost::asio::deadline_timer timer1_;
  boost::asio::deadline_timer timer2_;
  int count_;
};

int main() {
  boost::asio::io_context ioc;

  Printer printer(ioc);

  // The new thread runs a loop.
  boost::thread thread(boost::bind(&boost::asio::io_context::run, &ioc));

  // The main thread runs another loop.
  ioc.run();

  // Wait for the thread to finish.
  thread.join();

  return 0;
}
