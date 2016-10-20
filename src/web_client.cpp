#include <iostream>
#include <array>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// This example sends a HTTP request to a web server to download the homepage.

// Adapted from:
// http://theboostcpplibraries.com/boost.asio-network-programming
// Example 32.5. A web client with boost::asio::ip::tcp::socket

using boost::asio::ip::tcp;

#define ASYNC_RESOLVE 1

class WebClient {
public:
  WebClient(boost::asio::io_service& io_service)
      : socket_(io_service) {
    StartConnect();
  }

  ~WebClient() {
#if ASYNC_RESOLVE
    delete resolver_;
    delete query_;
#endif  // ASYNC_RESOLVE
  }

private:
  void StartConnect() {
#if ASYNC_RESOLVE
    resolver_ = new tcp::resolver(socket_.get_io_service());
    query_ = new tcp::resolver::query("theboostcpplibraries.om", "80");

    // If resolver or query is auto variable, the error code in ResolverHandler() will be:
    // The I/O operation has been aborted because of either a thread exit or an application request
    // That means the resolver object goes out of scope.
    // See http://stackoverflow.com/questions/4636608/boostasioasync-resolve-problem

    auto handler = boost::bind(&WebClient::HandleResolve,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::iterator);
    resolver_->async_resolve(*query_, handler);
#else
    tcp::resolver resolver(socket_.get_io_service());
    tcp::resolver::query query("theboostcpplibraries.com", "80");

    boost::system::error_code ec;
    tcp::resolver::iterator it = resolver.resolve(query, ec);

    if (!ec) {
      auto handler = boost::bind(&WebClient::HandleConnect,
                                 this,
                                 boost::asio::placeholders::error);
      socket_.async_connect(*it, handler);
    }
#endif  // ASYNC_RESOLVE
  }

#if ASYNC_RESOLVE
  void HandleResolve(const boost::system::error_code& ec, tcp::resolver::iterator it) {
    if (!ec) {
      auto handler = boost::bind(&WebClient::HandleConnect,
                                 this,
                                 boost::asio::placeholders::error);
      socket_.async_connect(*it, handler);
    } else {
      std::cerr << ec.message() << std::endl;
    }
  }
#endif  // ASYNC_RESOLVE

  void HandleConnect(const boost::system::error_code& ec) {
    if (!ec) {
      std::string r = "GET / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";
      write(socket_, boost::asio::buffer(r));

      auto handler = boost::bind(&WebClient::HandleRead,
                                 this,
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred);
      socket_.async_read_some(boost::asio::buffer(bytes_), handler);
    }
  }

  void HandleRead(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
      std::cout.write(bytes_.data(), bytes_transferred);

      auto handler = boost::bind(&WebClient::HandleRead,
                                 this,
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred);
      socket_.async_read_some(boost::asio::buffer(bytes_), handler);
    }
  }

private:
#if ASYNC_RESOLVE
  tcp::resolver* resolver_;
  tcp::resolver::query* query_;
#endif  // ASYNC_RESOLVE

  tcp::socket socket_;
  std::array<char, 4096> bytes_;
};

int main() {
  boost::asio::io_service io_service;
  WebClient web_client(io_service);
  io_service.run();

  return 0;
}
