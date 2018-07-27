#include <iostream>
#include <string>
#include <memory>

#include "boost/array.hpp"
#include "boost/bind.hpp"

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"

using boost::asio::ip::tcp;

// -----------------------------------------------------------------------------

#define USE_BIND 1  // Use std::bind or lambda
#define USE_MOVE_ACCEPT 1

enum { BUF_SIZE = 1024 };

// -----------------------------------------------------------------------------

class Session : public std::enable_shared_from_this<Session> {
public:
#if USE_MOVE_ACCEPT
  Session(tcp::socket socket) : socket_(std::move(socket)) {
  }
#else
  Session()
#endif  // USE_MOVE_ACCEPT

  void Start() {
    DoRead();
  }

  void DoRead() {
#if USE_BIND
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            std::bind(&Session::HandleRead,
                                      shared_from_this(),
                                      std::placeholders::_1,
                                      std::placeholders::_2));
#else
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            DoWrite(length);
          }
        });
#endif  // USE_BIND
  }

  void DoWrite(std::size_t length) {
#if USE_BIND
    boost::asio::async_write(socket_,
                             boost::asio::buffer(buffer_, length),
                             std::bind(&Session::HandleWrite,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
#else
    auto self(shared_from_this());

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(buffer_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            DoRead();
          }
        });
#endif  // USE_BIND
  }

#if USE_BIND
  void HandleRead(boost::system::error_code ec, std::size_t length) {
    if (!ec) {
      DoWrite(length);
    }
  }

  void HandleWrite(boost::system::error_code ec, std::size_t /*length*/) {
    if (!ec) {
      DoRead();
    }
  }
#endif  // USE_BIND

private:
  tcp::socket socket_;
  std::array<char, BUF_SIZE> buffer_;
};

// -----------------------------------------------------------------------------

class Server {
public:
  Server(boost::asio::io_context& io_context, unsigned short port)
      : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    DoAccept();
  }

private:
  void DoAccept() {
    // A note about bind:

    // std::bind works in VS2015.
    //acceptor_.async_accept(std::bind(&Server::HandleAccept,
    //                                 this,
    //                                 std::placeholders::_1,
    //                                 std::placeholders::_2));

    // boost::bind doesn't work in VS2015.

    // Both std::bind and boost::bind don't work in VS2013.
    // The reason is mainly about the move semantics.

    // But of course, async_accept has other signatures which could be
    // used with bind.

#if USE_MOVE_ACCEPT
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec) {
            std::make_shared<Session>(std::move(socket))->Start();
          }
          DoAccept();
        });
#else
    // TODO
    tcp::socket socket(acceptor_.get_executor().context());

    acceptor_.async_accept(
        socket,
        [this](boost::system::error_code ec) {
          if (!ec) {
            std::make_shared<Session>(socket)->Start();
          }
          DoAccept();
        });
#endif  // USE_MOVE_ACCEPT
  }

private:
  tcp::acceptor acceptor_;
};

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  unsigned short port = std::atoi(argv[1]);

  boost::asio::io_context io_context;

  Server server(io_context, port);

  io_context.run();

  return 0;
}
