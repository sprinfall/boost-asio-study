#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Connection : public boost::enable_shared_from_this<Connection> {
public:
  typedef boost::shared_ptr<Connection> Pointer;

  static Pointer Create(boost::asio::io_context& ioc) {
    return Pointer(new Connection(ioc));
  }

  tcp::socket& socket() {
    return socket_;
  }

  void Start() {
    AsyncRead();
  }

  void HandleRead(const boost::system::error_code& ec,
                  size_t bytes_transferred) {
    if (!ec) {
      std::string msg(data_.c_array(), bytes_transferred);
      AsyncWrite(bytes_transferred);
    }
  }

  void HandleWrite(const boost::system::error_code& ec) {
    if (!ec) {
      AsyncRead();
    }
  }

  void AsyncRead() {
    auto handler = boost::bind(&Connection::HandleRead, shared_from_this(), _1, _2);
    socket_.async_read_some(boost::asio::buffer(data_), handler);
  }

  void AsyncWrite(size_t bytes_transferred) {
    auto handler = boost::bind(&Connection::HandleWrite,
                               shared_from_this(),
                               boost::asio::placeholders::error);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(data_, bytes_transferred),
                             handler);
  }

private:
  Connection(boost::asio::io_context& ioc)
      : socket_(ioc) {
  }

private:
  // The socket used to communicate with the client.
  tcp::socket socket_;

  // Buffer used to store data received from the client.
  boost::array<char, 1024> data_;
};

class Server {
public:
  Server(boost::asio::io_context& ioc, short port)
      : ioc_(ioc)
      , acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
    StartAccept();
  }

private:
  void StartAccept() {
    Connection::Pointer conn(Connection::Create(ioc_));

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
  boost::asio::io_context& ioc_;
  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  unsigned short port = std::atoi(argv[1]);

  boost::asio::io_context ioc;
  Server server(ioc, port);

  ioc.run();

  return 0;
}
