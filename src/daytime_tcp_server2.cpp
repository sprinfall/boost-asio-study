#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::tcp;

// An asynchronous TCP daytime server.

std::string Now() {
  std::time_t now = std::time(NULL);
  return std::ctime(&now);
}

class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {
public:
  typedef boost::shared_ptr<TcpConnection> Pointer;

  static Pointer Create(boost::asio::io_service& io_service) {
    return Pointer(new TcpConnection(io_service));
  }

  tcp::socket& socket() {
    return socket_;
  }

  // Serve the data to the client.
  void Start() {
    msg_ = Now();

    auto handler = boost::bind(&TcpConnection::WriteHandler,
                               shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred);
    boost::asio::async_write(socket_, boost::asio::buffer(msg_), handler);
  }

private:
  explicit TcpConnection(boost::asio::io_service& io_service)
      : socket_(io_service) {
  }

  // If parameters are not needed, it is possible to remove them from the function.
  void WriteHandler(const boost::system::error_code& ec, size_t bytes_transferred) {
  }

private:
  tcp::socket socket_;

  // Store the data to send as member variable as we need to keep it valid until
  // the async operation (async_write) is complete.
  std::string msg_;
};

class TcpServer {
public:
  TcpServer(boost::asio::io_service& io_service)
      // An acceptor listening on TCP port 13.
      : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13)) {
    StartAccept();
  }

private:
  // Create a socket and initiates an asynchronous accept operation to wait
  // for a new connection.
  void StartAccept() {
    std::cout << "Start accept" << std::endl;

    TcpConnection::Pointer connection = TcpConnection::Create(acceptor_.get_io_service());

    auto handler = boost::bind(&TcpServer::HandleAccept,
                               this,
                               connection,
                               boost::asio::placeholders::error);
    acceptor_.async_accept(connection->socket(), handler);
  }

  void HandleAccept(TcpConnection::Pointer connection,
                    const boost::system::error_code& ec) {
    if (!ec) {
      connection->Start();
    }

    StartAccept();
  }

private:
  // TCP socket acceptor is used to accept new socket connections.
  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
  boost::asio::io_service io_service;
  TcpServer server(io_service);
  io_service.run();

  return 0;
}