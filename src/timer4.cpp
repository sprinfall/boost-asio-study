#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Use a member function as callback.
// Exactly the same behavior as timer3.

class printer {
public:
  printer(boost::asio::io_service& io_service)
    : timer_(io_service, boost::posix_time::seconds(1)),
      count_(0) {
    // boost::asio::placeholders::error is not specified.
    timer_.async_wait(boost::bind(&printer::print, this));
  }

  ~printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void print() {
    if (count_ < 3) {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(1));
      timer_.async_wait(boost::bind(&printer::print, this));
    }
  }

private:
  boost::asio::deadline_timer timer_;
  int count_;
};

int main() {
  boost::asio::io_service io_service;
  printer p(io_service);
  io_service.run();

  return 0;
}