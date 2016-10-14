#include "server.hpp"
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

Server::Server(const std::string& address,
               const std::string& port,
               const std::string& doc_root,
               std::size_t thread_pool_size)
    : thread_pool_size_(thread_pool_size)
    , signals_(io_service_)
    , acceptor_(io_service_)
    , new_connection_()
    , request_handler_(doc_root) {
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through asio.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif  // defined(SIGQUIT)

  signals_.async_wait(boost::bind(&Server::HandleStop, this));

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::resolver::query query(address, port);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  StartAccept();
}

void Server::Run() {
  // Create a pool of threads to run all of the io_services.
  std::vector<boost::shared_ptr<boost::thread> > threads;
  for (std::size_t i = 0; i < thread_pool_size_; ++i) {
    boost::shared_ptr<boost::thread> thread(new boost::thread(
      boost::bind(&boost::asio::io_service::run, &io_service_)));
    threads.push_back(thread);
  }

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads.size(); ++i) {
    threads[i]->join();
  }
}

void Server::StartAccept() {
  new_connection_.reset(new Connection(io_service_, request_handler_));
  acceptor_.async_accept(new_connection_->socket(),
                         boost::bind(&Server::HandleAccept, this,
                         boost::asio::placeholders::error));
}

void Server::HandleAccept(const boost::system::error_code& ec) {
  if (!ec) {
    new_connection_->Start();
  }

  StartAccept();
}

void Server::HandleStop() {
  io_service_.stop();
}
