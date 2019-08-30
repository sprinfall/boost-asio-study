// Asynchronous echo client.

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "boost/asio.hpp"
#include "boost/core/ignore_unused.hpp"

#include "utility.h"  // for printing endpoints

using boost::asio::ip::tcp;

// Use async_resolve() or not.
#define RESOLVE_ASYNC 1

// Only resolve IPv4.
#define RESOLVE_IPV4_ONLY 1

// -----------------------------------------------------------------------------

class Client {
 public:
  Client(boost::asio::io_context& io_context,
         const std::string& host, const std::string& port);

 private:

#if RESOLVE_ASYNC
  void OnResolve(boost::system::error_code ec,
                 tcp::resolver::results_type endpoints);
#endif  // RESOLVE_ASYNC

  void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint);

  void DoWrite();
  void OnWrite(boost::system::error_code ec, std::size_t length);

  void OnRead(boost::system::error_code ec, std::size_t length);

  tcp::socket socket_;

#if RESOLVE_ASYNC
  tcp::resolver resolver_;
#endif

  enum { BUF_SIZE = 1024 };

  char cin_buf_[BUF_SIZE];

  // NOTE: std::vector is better than std::array in practice.
  std::array<char, BUF_SIZE> buf_;
};

// -----------------------------------------------------------------------------

Client::Client(boost::asio::io_context& io_context,
               const std::string& host, const std::string& port)
#if RESOLVE_ASYNC
    : socket_(io_context), resolver_(io_context) {
#else
    : socket_(io_context) {
#endif

#if RESOLVE_ASYNC

  resolver_.async_resolve(tcp::v4(), host, port,
                          std::bind(&Client::OnResolve, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));

#else

  // If you don't specify tcp::v4() as the first parameter (protocol) of
  // resolve(), the result will have two endpoints, one for v6, one for
  // v4. The first v6 endpoint will fail to connect.

  tcp::resolver resolver(io_context);

#if RESOLVE_IPV4_ONLY

  auto endpoints = resolver.resolve(tcp::v4(), host, port);
  // 127.0.0.1:2017, v4

#else

  auto endpoints = resolver.resolve(host, port);
  // [::1]:2017, v6
  // 127.0.0.1:2017, v4

#endif  // RESOLVE_IPV4_ONLY

  utility::PrintEndpoints(std::cout, endpoints);

  // ConnectHandler: void(boost::system::error_code, tcp::endpoint)
  boost::asio::async_connect(socket_, endpoints,
                             std::bind(&Client::OnConnect, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));

#endif  // RESOLVE_ASYNC
}

#if RESOLVE_ASYNC

void Client::OnResolve(boost::system::error_code ec,
                       tcp::resolver::results_type endpoints) {
  if (ec) {
    std::cerr << "Resolve: " << ec.message() << std::endl;
  } else {
    // ConnectHandler: void(boost::system::error_code, tcp::endpoint)
    boost::asio::async_connect(socket_, endpoints,
                               std::bind(&Client::OnConnect, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
  }
}

#endif  // RESOLVE_ASYNC

void Client::OnConnect(boost::system::error_code ec, tcp::endpoint endpoint) {
  if (ec) {
    std::cout << "Connect failed: " << ec.message() << std::endl;
    socket_.close();
  } else {
    DoWrite();
  }
}

void Client::DoWrite() {
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
                           std::bind(&Client::OnWrite, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void Client::OnWrite(boost::system::error_code ec, std::size_t length) {
  boost::ignore_unused(length);

  if (!ec) {
    std::cout << "Reply is: ";

    socket_.async_read_some(boost::asio::buffer(buf_),
                            std::bind(&Client::OnRead, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  }
}

void Client::OnRead(boost::system::error_code ec, std::size_t length) {
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

  Client client{ io_context, host, port };

  io_context.run();

  return 0;
}
