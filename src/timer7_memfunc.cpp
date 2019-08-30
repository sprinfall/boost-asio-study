// Wait a timer asynchronously.
// Use a member function as handler.

#include <chrono>
#include <functional>
#include <iostream>

#include "boost/asio/steady_timer.hpp"
#include "boost/asio/io_context.hpp"

class Printer {
 public:
  explicit Printer(boost::asio::io_context& io_context)
      : timer_(io_context, std::chrono::seconds(1)), count_(0) {
    // For member function handlers, always bind |this| as the first argument.
    // Unlike global functions, '&' is mandatory for referring to member
    // function pointers.
    timer_.async_wait(std::bind(&Printer::Print, this, std::placeholders::_1));
  }

  ~Printer() {
    std::cout << "Final count is " << count_ << std::endl;
  }

 private:
  void Print(const boost::system::error_code& ec) {
    if (count_ < 3) {
      std::cout << count_ << std::endl;
      ++count_;

      timer_.expires_after(std::chrono::seconds(1));

      timer_.async_wait(std::bind(&Printer::Print, this,
                                  std::placeholders::_1));
    }
  }

  boost::asio::steady_timer timer_;
  int count_;
};

int main() {
  boost::asio::io_context io_context;

  Printer printer{ io_context };

  io_context.run();

  return 0;
}
