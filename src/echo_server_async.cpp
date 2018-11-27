// Asynchronous echo server.

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#define BOOST_ASIO_NO_DEPRECATED

#include "boost/asio.hpp"
#include "boost/core/ignore_unused.hpp"

using boost::asio::ip::tcp;

// -----------------------------------------------------------------------------

#define USE_BIND 1  // Use std::bind or lambda

enum { BUF_SIZE = 1024 };

// -----------------------------------------------------------------------------

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket socket) : socket_(std::move(socket)) {
  }

  void Start() {
    DoRead();
  }

 private:
  void DoRead() {
#if USE_BIND
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            std::bind(&Session::OnRead, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
#else
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            AsyncWrite(length);
          }
        });
#endif  // USE_BIND
  }

  void DoWrite(std::size_t length) {
#if USE_BIND
    boost::asio::async_write(socket_,
                             boost::asio::buffer(buffer_, length),
                             std::bind(&Session::OnWrite, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));
#else
    auto self(shared_from_this());

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(buffer_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            AsyncRead();
          }
        });
#endif  // USE_BIND
  }

#if USE_BIND
  void OnRead(boost::system::error_code ec, std::size_t length) {
    if (!ec) {
      DoWrite(length);
    }
  }

  void OnWrite(boost::system::error_code ec, std::size_t length) {
    boost::ignore_unused(length);

    if (!ec) {
      DoRead();
    }
  }
#endif  // USE_BIND

  tcp::socket socket_;
  std::array<char, BUF_SIZE> buffer_;
};

// -----------------------------------------------------------------------------

class Server {
 public:
  Server(boost::asio::io_context& io_context, std::uint16_t port)
      : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    DoAccept();
  }

 private:
  void DoAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec) {
            std::make_shared<Session>(std::move(socket))->Start();
          }
          DoAccept();
        });
  }

  tcp::acceptor acceptor_;
};

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  std::uint16_t port = std::atoi(argv[1]);

  boost::asio::io_context io_context;

  Server server(io_context, port);

  io_context.run();

  return 0;
}
