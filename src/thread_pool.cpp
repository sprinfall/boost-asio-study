#include <iostream>

// For VS2013 with Boost 1.61
// Boost 1.58 doesn't need this for VS2013.
#define BOOST_NO_CXX11_TEMPLATE_ALIASES

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

// Boost doc of io_service::run says:
// Multiple threads may call the run() function to set up a pool of threads
// from which the io_service may execute handlers. All threads that are waiting
// in the pool are equivalent and the io_service may choose any one of them to
// invoke a handler.

// See also:
// http://progsch.net/wordpress/?p=71
// http://stackoverflow.com/questions/17156541/why-do-we-need-to-use-boostasioio-servicework

class ThreadPool {
public:
  explicit ThreadPool(size_t size) : work_(io_service_) {
    for (size_t i = 0; i < size; ++i) {
      workers_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
    }
  }

  ~ThreadPool() {
    io_service_.stop();
    workers_.join_all();
  }

  // Add new work item to the pool.
  template<class F>
  void Enqueue(F f) {
    io_service_.post(f);
  }

private:
  boost::thread_group workers_;
  boost::asio::io_service io_service_;

  // Why need io_service::work? See:
  //   http://stackoverflow.com/questions/13219296/why-should-i-use-io-servicework
  boost::asio::io_service::work work_;
};

int main() {
  // Create a thread pool of 4 worker threads.
  ThreadPool pool(4);

  // Queue a bunch of work items.
  for (int i = 0; i < 8; ++i) {
    pool.Enqueue([i] {
      std::cout << "hello " << i << std::endl;
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      std::cout << "world " << i << std::endl;
    });
  }

  return 0;
}
