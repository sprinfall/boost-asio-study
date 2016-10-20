#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::tcp;

class Connection : public boost::enable_shared_from_this<Connection> {
public:
  typedef boost::shared_ptr<Connection> Pointer;

  static Pointer Create(boost::asio::io_service& io_service) {
    return Pointer(new Connection(io_service));
  }

  tcp::socket& socket() {
    return socket_;
  }

  void Start() {
    DoRead();
  }

  void HandleRead(const boost::system::error_code& ec,
                  size_t bytes_transferred) {
    if (!ec) {
      DoWrite(bytes_transferred);
    }
  }

  void HandleWrite(const boost::system::error_code& ec) {
    if (!ec) {
      DoRead();
    }
  }

  void DoRead() {
    auto handler = boost::bind(&Connection::HandleRead, shared_from_this(), _1, _2);

    socket_.async_read_some(boost::asio::buffer(data_), handler);
  }

  void DoWrite(size_t bytes_transferred) {
    auto handler = boost::bind(&Connection::HandleWrite,
                               shared_from_this(),
                               boost::asio::placeholders::error);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(data_, bytes_transferred),
                             handler);
  }

private:
  Connection(boost::asio::io_service& io_service)
    : socket_(io_service) {
  }

private:
  // The socket used to communicate with the client.
  tcp::socket socket_;

  // Buffer used to store data received from the client.
  boost::array<char, 1024> data_;
};

class Server {
public:
  Server(boost::asio::io_service& io_service, short port)
      : io_service_(io_service)
      , acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
    StartAccept();
  }

private:
  void StartAccept() {
    Connection::Pointer conn(Connection::Create(io_service_));

    auto handler = boost::bind(&Server::HandleAccept, this, conn, _1);
    acceptor_.async_accept(conn->socket(), handler);
  }

  void HandleAccept(boost::shared_ptr<Connection> conn,
                    const boost::system::error_code& ec) {
    if (!ec) {
      conn->Start();
    }
    StartAccept();
  }

private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};

int main() {
  boost::asio::io_service io_service;
  Server server(io_service, 2016);
  io_service.run();
  return 0;
}
