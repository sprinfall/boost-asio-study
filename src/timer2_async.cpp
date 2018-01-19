#include <iostream>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#if 0
#include <boost/asio.hpp>
#else
#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#endif

// Use a timer asynchronously.

// Handler to be called when the asynchronous wait finishes.
void Print(boost::system::error_code ec) {
  std::cout << "Hello, world!" << std::endl;
}

int main() {
  boost::asio::io_context ioc;
  boost::asio::deadline_timer timer(ioc, boost::posix_time::seconds(3));

  timer.async_wait(&Print);

  size_t size = ioc.run();
  std::cout << "Number of handlers executed: " << size << std::endl;  // 1

  return 0;
}
