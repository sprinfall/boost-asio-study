// Asynchronous echo client.

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"

#include "utility.h"  // for printing endpoints

using boost::asio::ip::tcp;

// Use async_resolve() or not.
#define RESOLVE_ASYNC 1

// Only resolve IPv4.
#define RESOLVE_IPV4_ONLY 1

// Use global function async_connect().
#define USE_GLOBAL_ASYNC_CONNECT 1

// -----------------------------------------------------------------------------

class Client {
 public:
  Client(boost::asio::io_context& io_context,
         const std::string& host, const std::string& port);

 private:

#if RESOLVE_ASYNC
  void ResolveHandler(boost::system::error_code ec,
                      tcp::resolver::results_type results);
#endif  // RESOLVE_ASYNC

#if USE_GLOBAL_ASYNC_CONNECT
  void ConnectHandler(boost::system::error_code ec, tcp::endpoint endpoint);
#else
  void AsyncConnect(tcp::resolver::results_type::iterator endpoint_it);
  void ConnectHandler(boost::system::error_code ec,
                      tcp::resolver::results_type::iterator endpoint_it);
#endif  // USE_GLOBAL_ASYNC_CONNECT

  void AsyncWrite();
  void WriteHandler(boost::system::error_code ec);

  void ReadHandler(boost::system::error_code ec, std::size_t length);

  tcp::socket socket_;

#if RESOLVE_ASYNC
  std::unique_ptr<tcp::resolver> resolver_;
#endif

  tcp::resolver::results_type endpoints_;

  enum { BUF_SIZE = 1024 };

  char cin_buf_[BUF_SIZE];

  // NOTE: std::vector is better than std::array in practice.
  std::array<char, BUF_SIZE> buf_;
};

// -----------------------------------------------------------------------------

Client::Client(boost::asio::io_context& io_context,
               const std::string& host, const std::string& port)
    : socket_(io_context) {

#if RESOLVE_ASYNC

  resolver_.reset(new tcp::resolver(io_context));

  resolver_->async_resolve(tcp::v4(), host, port,
                           std::bind(&Client::ResolveHandler, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));

#else

  // If you don't specify tcp::v4() as the first parameter (protocol) of
  // resolve(), the result will have two endpoints, one for v6, one for
  // v4. The first v6 endpoint will fail to connect.

  tcp::resolver resolver(io_context);

#if RESOLVE_IPV4_ONLY

  endpoints_ = resolver.resolve(tcp::v4(), host, port);
  // 127.0.0.1:2017, v4

#else

  endpoints_ = resolver.resolve(host, port);
  // [::1]:2017, v6
  // 127.0.0.1:2017, v4

#endif  // RESOLVE_IPV4_ONLY

  utility::PrintEndpoints(std::cout, endpoints_);

  // TODO: Use boost::asio::async_connect() instead.
  AsyncConnect(endpoints_.begin());

#endif  // RESOLVE_ASYNC
}

#if RESOLVE_ASYNC

void Client::ResolveHandler(boost::system::error_code ec,
                            tcp::resolver::results_type results) {
  if (ec) {
    std::cerr << "Resolve: " << ec.message() << std::endl;
  } else {
    endpoints_ = results;

#if USE_GLOBAL_ASYNC_CONNECT

    // ConnectHandler: void(boost::system::error_code, tcp::endpoint)
    boost::asio::async_connect(socket_, endpoints_,
                               std::bind(&Client::ConnectHandler, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));

#else

    AsyncConnect(endpoints_.begin());

#endif  // USE_GLOBAL_ASYNC_CONNECT
  }
}

#endif  // RESOLVE_ASYNC

#if USE_GLOBAL_ASYNC_CONNECT

void Client::ConnectHandler(boost::system::error_code ec,
                            tcp::endpoint endpoint) {
  if (ec) {
    std::cout << "Connect failed: " << ec.message() << std::endl;
    socket_.close();
  } else {
    AsyncWrite();
  }
}

#else

void Client::AsyncConnect(tcp::resolver::results_type::iterator endpoint_it) {
  if (endpoint_it != endpoints_.end()) {
    // void ConnectHandler(boost::system::error_code)
    socket_.async_connect(endpoint_it->endpoint(),
                          std::bind(&Client::ConnectHandler, this,
                                    std::placeholders::_1,
                                    endpoint_it));
  }
}

void Client::ConnectHandler(boost::system::error_code ec,
                            tcp::resolver::results_type::iterator endpoint_it) {
  if (ec) {
    // Will be here if the end point is v6.
    std::cout << "Connect failed: " << ec.message() << std::endl;

    socket_.close();

    // Try the next available endpoint.
    AsyncConnect(++endpoint_it);
  } else {
    AsyncWrite();
  }
}

#endif  // USE_GLOBAL_ASYNC_CONNECT

void Client::AsyncWrite() {
  std::size_t len = 0;
  do {
    std::cout << "Enter message: ";
    std::cin.getline(cin_buf_, BUF_SIZE);
    len = strlen(cin_buf_);
  } while (len == 0);

  // TODO: Second parameter
  // WriteHandler: void (boost::system::error_code, std::size_t)
  boost::asio::async_write(socket_,
                           boost::asio::buffer(cin_buf_, len),
                           std::bind(&Client::WriteHandler, this,
                                     std::placeholders::_1));
}

void Client::WriteHandler(boost::system::error_code ec) {
  if (!ec) {
    std::cout << "Reply is: ";

    socket_.async_read_some(boost::asio::buffer(buf_),
                            std::bind(&Client::ReadHandler, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  }
}

void Client::ReadHandler(boost::system::error_code ec, std::size_t length) {
  if (!ec) {
    std::cout.write(buf_.data(), length);
    std::cout << std::endl;
  }
}

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  std::string host = argv[1];
  std::string port = argv[2];

  boost::asio::io_context io_context;

  Client client(io_context, host, port);

  io_context.run();

  return 0;
}
