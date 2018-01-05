#include <iostream>
#include <array>
#include <string>

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

// This example sends a HTTP request to a web server to download the homepage.

// Adapted from:
//   http://theboostcpplibraries.com/boost.asio-network-programming
// Example 32.5. A web client with boost::asio::ip::tcp::socket

// 2018/1/5:
// Since theboostcpplibraries.com has transferred to HTTPS, this example
// cannot download the real page now.

using boost::asio::ip::tcp;

#define ASYNC_RESOLVE 1

#define HOST "theboostcpplibraries.com"
#define PORT "80"

class WebClient {
public:
  WebClient(boost::asio::io_context& ioc)
      : socket_(ioc) {
#if ASYNC_RESOLVE
    // It's important to make sure the resolver doesn't go out of scope.
    // Keeping it as member variable ensures this.
    resolver_.reset(new tcp::resolver(ioc));
#endif

    request_ = "GET / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";

    Start();
  }

  ~WebClient() {
  }

private:
  void Start() {
#if ASYNC_RESOLVE
    auto handler = boost::bind(&WebClient::HandleResolve,
                               this,
                               boost::placeholders::_1,
                               boost::placeholders::_2);
    // tcp::v4() is optional, but without it, the first resolved endpoint
    // will be V6.
    resolver_->async_resolve(tcp::v4(), HOST, PORT, handler);
#else
    tcp::resolver resolver(socket_.get_executor().context());

    tcp::resolver::results_type endpoints = resolver.resolve(tcp::v4(), HOST, PORT);

    endpoints_ = endpoints;
    AsyncConnect(endpoints_.begin());
#endif  // ASYNC_RESOLVE
  }

#if ASYNC_RESOLVE
  void HandleResolve(const boost::system::error_code& ec,
                     tcp::resolver::results_type results) {
    if (ec) {
      std::cerr << "Resolve: " << ec.message() << std::endl;
    } else {
      endpoints_ = results;
      AsyncConnect(endpoints_.begin());
    }
  }
#endif  // ASYNC_RESOLVE

  void AsyncConnect(tcp::resolver::results_type::iterator endpoint_it) {
    if (endpoint_it != endpoints_.end()) {
      auto handler = boost::bind(&WebClient::HandleConnect,
                                 this,
                                 boost::placeholders::_1,
                                 endpoint_it);
      socket_.async_connect(endpoint_it->endpoint(), handler);
    }
  }

  void HandleConnect(const boost::system::error_code& ec,
                     tcp::resolver::results_type::iterator endpoint_it) {
    if (ec) {
      // Will be here if the end point is v6.
      std::cout << "Connect error: " << ec.message() << std::endl;

      socket_.close();

      // Try the next available endpoint.
      AsyncConnect(++endpoint_it);
    } else {
      AsyncWrite();
    }
  }

  void AsyncWrite() {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(request_),
                             boost::bind(&WebClient::HandleWrite,
                                         this,
                                         boost::placeholders::_1));
  }

  void HandleWrite(const boost::system::error_code& ec) {
    if (!ec) {
      AsyncRead();
    }
  }

  void AsyncRead() {
    auto handler = boost::bind(&WebClient::HandleRead,
                               this,
                               boost::placeholders::_1,
                               boost::placeholders::_2);
    socket_.async_read_some(boost::asio::buffer(bytes_), handler);
  }

  void HandleRead(const boost::system::error_code& ec,
                  std::size_t bytes_transferred) {
    if (!ec) {
      std::cout.write(bytes_.data(), bytes_transferred);

      auto handler = boost::bind(&WebClient::HandleRead,
                                 this,
                                 boost::placeholders::_1,
                                 boost::placeholders::_2);
      socket_.async_read_some(boost::asio::buffer(bytes_), handler);
    }
  }

private:
#if ASYNC_RESOLVE
  boost::scoped_ptr<tcp::resolver> resolver_;
#endif  // ASYNC_RESOLVE

  tcp::resolver::results_type endpoints_;

  tcp::socket socket_;

  std::string request_;

  std::array<char, 4096> bytes_;
};

int main() {
  boost::asio::io_context ioc;

  WebClient web_client(ioc);
  ioc.run();

  return 0;
}
