#include <iostream>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#if 0
#include <boost/asio.hpp>
#else
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/placeholders.hpp>
#endif

// Use a member function as handler.

class Printer {
public:
  Printer(boost::asio::io_context& ioc)
      : timer_(ioc, boost::posix_time::seconds(1)), count_(0) {
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void Start() {
    // ISSUE:
    // Cannot compile if add parameter "error_code" to Print().
    // Bug?
    timer_.async_wait(boost::bind(&Printer::Print, this/*, boost::asio::placeholders::error*/));
  }

private:
  void Print(/*const boost::system::error_code& ec*/) {
    if (count_ < 3) {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(1));
      timer_.async_wait(boost::bind(&Printer::Print, this));
    }
  }

private:
  boost::asio::deadline_timer timer_;
  int count_;
};

int main() {
  boost::asio::io_context ioc;

  Printer printer(ioc);
  printer.Start();

  ioc.run();

  return 0;
}