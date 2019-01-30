// Wait a timer asynchronously.
// Use a member function as handler.

#include <functional>
#include <iostream>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

class Printer {
 public:
  Printer(boost::asio::io_context& io_context)
      : timer_(io_context, boost::posix_time::seconds(1)), count_(0) {
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void Start() {
    // For member function handlers, always bind |this| as the first argument.
    // Unlike global functions, '&' is mandatory for referring to member
    // function pointers.
    timer_.async_wait(std::bind(&Printer::Print, this, std::placeholders::_1));
  }

 private:
  void Print(boost::system::error_code ec) {
    if (count_ < 3) {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(1));
      timer_.async_wait(std::bind(&Printer::Print, this,
                                  std::placeholders::_1));
    }
  }

  boost::asio::deadline_timer timer_;
  int count_;
};

int main() {
  boost::asio::io_context io_context;

  Printer printer(io_context);
  printer.Start();

  io_context.run();

  return 0;
}
