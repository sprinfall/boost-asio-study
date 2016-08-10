#include <iostream>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// An asynchronous TCP daytime server.

static std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

// NOTE: enable_shared_from_this has been a part of C++11.
class tcp_connection :
  public boost::enable_shared_from_this<tcp_connection> {
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_service& io_service) {
    return pointer(new tcp_connection(io_service));
  }

  tcp::socket& socket() {
    return socket_;
  }

  // Serve the data to the client.
  void start() {
    msg_ = make_daytime_string();

    boost::asio::async_write(socket_, boost::asio::buffer(msg_),
                             boost::bind(&tcp_connection::handle_write,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

    //boost::asio::async_write(socket_, boost::asio::buffer(msg_),
    //                         boost::bind(&tcp_connection::handle_write, shared_from_this()));
  }

private:
  tcp_connection(boost::asio::io_service& io_service)
    : socket_(io_service) {
  }

  // If parameters are not needed, it is possible to remove them from the function.
  void handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
  }

private:
  tcp::socket socket_;

  // Store the data to send as member variable as we need to keep it valid until
  // the async operation (async_write) is complete.
  std::string msg_;
};

class tcp_server {
public:
  tcp_server(boost::asio::io_service& io_service)
    // An acceptor listening on TCP port 13.
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 13)) {
    start_accept();
  }

private:
  // Create a socket and initiates an asynchronous accept operation to wait
  // for a new connection.
  void start_accept() {
    std::cout << "tcp_server::start_accept" << std::endl;

    tcp_connection::pointer new_connection =
      tcp_connection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
                           boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error) {
    std::cout << "tcp_server::handle_accept" << std::endl;

    if (!error) {
      new_connection->start();
    }

    start_accept();
  }

private:
  // TCP socket acceptor is used to accept new socket connections.
  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
  boost::asio::io_service io_service;
  tcp_server server(io_service);
  io_service.run();

  return 0;
}