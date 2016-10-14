#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

// The stand class provides the ability to post and dispatch handlers with the
// guarantee that none of those handlers will execute concurrently.

class Printer {
public:
  Printer(boost::asio::io_service& io_service)
    : strand_(io_service),
      timer1_(io_service, boost::posix_time::seconds(1)),
      timer2_(io_service, boost::posix_time::seconds(1)),
      count_(0) {
    timer1_.async_wait(strand_.wrap(boost::bind(&Printer::Print1, this)));
    timer2_.async_wait(strand_.wrap(boost::bind(&Printer::Print2, this)));
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void Print1() {
    if (count_ < 6) {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;

      timer1_.expires_at(timer1_.expires_at() + boost::posix_time::seconds(1));
      timer1_.async_wait(strand_.wrap(boost::bind(&Printer::Print1, this)));
    }
  } 

  void Print2() {
    if (count_ < 6) {
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      timer2_.expires_at(timer2_.expires_at() + boost::posix_time::seconds(1));
      timer2_.async_wait(strand_.wrap(boost::bind(&Printer::Print2, this)));
    }
  }

private:
  boost::asio::io_service::strand strand_;
  boost::asio::deadline_timer timer1_;
  boost::asio::deadline_timer timer2_;
  int count_;
};

int main() {
  boost::asio::io_service io_service;
  Printer printer(io_service);

  // io_service::run() is called from two thread.
  boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
  io_service.run();

  thread.join();

  return 0;
}
