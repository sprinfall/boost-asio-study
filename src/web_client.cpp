#include <iostream>
#include <array>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// This example sends a HTTP request to a web server to download the homepage.

// Adapted from:
// http://theboostcpplibraries.com/boost.asio-network-programming
// Example 32.5. A web client with boost::asio::ip::tcp::socket

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

#if 0

boost::asio::io_service io_service;
asio::ip::tcp::socket tcp_socket(io_service);
std::array<char, 4096> bytes;

void read_handler(const error_code& ec, std::size_t bytes_transferred) {
  if (!ec) {
    std::cout.write(bytes.data(), bytes_transferred);
    tcp_socket.async_read_some(asio::buffer(bytes), read_handler);
  }
}

void connect_handler(const error_code& ec) {
  if (!ec) {
    std::string r = "GET / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";
    write(tcp_socket, asio::buffer(r));
    tcp_socket.async_read_some(asio::buffer(bytes), read_handler);
  }
}

void resolve_handler(const error_code& ec, tcp::resolver::iterator it) {
  if (!ec)
    tcp_socket.async_connect(*it, connect_handler);
}

int main() {
  tcp::resolver resolver(io_service);
  tcp::resolver::query q("theboostcpplibraries.com", "80");
  resolver.async_resolve(q, resolve_handler);

  io_service.run();
}

#else

#define ASYNC_RESOLVE 1

class WebClient {
public:
  WebClient(asio::io_service& io_service)
      : socket_(io_service) {
    StartConnect();
  }

  ~WebClient() {
    delete resolver_;
    delete query_;
  }

private:
  void StartConnect() {
#if ASYNC_RESOLVE
    resolver_ = new tcp::resolver(socket_.get_io_service());
    query_ = new tcp::resolver::query("theboostcpplibraries.com", "80");

    // If resolver or query is auto variable, the error code in ResolverHandler() will be:
    // The I/O operation has been aborted because of either a thread exit or an application request
    // That means the resolver object goes out of scope.
    // See http://stackoverflow.com/questions/4636608/boostasioasync-resolve-problem

    resolver_->async_resolve(
      *query_,
      boost::bind(&WebClient::ResolverHandler,
      this,
      asio::placeholders::error,
      asio::placeholders::iterator));

    delete resolver_;
#else
    tcp::resolver resolver(socket_.get_io_service());
    tcp::resolver::query query("theboostcpplibraries.com", "80");

    error_code ec;
    tcp::resolver::iterator it = resolver.resolve(query, ec);

    if (!ec) {
      socket_.async_connect(*it,
                            boost::bind(&WebClient::ConnectHandler, this, asio::placeholders::error));
    }
#endif  // ASYNC_RESOLVE
  }

#if ASYNC_RESOLVE
  void ResolverHandler(const error_code& ec, tcp::resolver::iterator it) {
    if (!ec) {
      socket_.async_connect(*it,
                            boost::bind(&WebClient::ConnectHandler, this, asio::placeholders::error));
    } else {
      std::cerr << ec.message() << std::endl;
    }
  }
#endif  // ASYNC_RESOLVE

  void ConnectHandler(const error_code& ec) {
    if (!ec) {
      std::string r = "GET / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";
      write(socket_, asio::buffer(r));
      socket_.async_read_some(asio::buffer(bytes_),
                              boost::bind(&WebClient::ReadHandler, this,
                              asio::placeholders::error,
                              asio::placeholders::bytes_transferred));
    }
  }

  void ReadHandler(const error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
      std::cout.write(bytes_.data(), bytes_transferred);

      socket_.async_read_some(asio::buffer(bytes_),
                              boost::bind(&WebClient::ReadHandler, this,
                              asio::placeholders::error,
                              asio::placeholders::bytes_transferred));
    }
  }

private:
  tcp::resolver* resolver_;
  tcp::resolver::query* query_;
  tcp::socket socket_;
  std::array<char, 4096> bytes_;
};

int main() {
  boost::asio::io_service io_service;
  WebClient web_client(io_service);
  io_service.run();

  return 0;
}

#endif
