#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// Client to test the allocation server.

using boost::asio::ip::tcp;

class Client {
public:
  Client(boost::asio::io_service& io_service)
      : socket_(io_service) {
  }

  void Connect(const std::string& host, const std::string& port) {
    tcp::resolver resolver(socket_.get_io_service());

    boost::system::error_code ec;
    // NOTE: Without protocol "tcp::v4()", "localhost" won't be resolved.
    tcp::resolver::query query(tcp::v4(), host, port);
    tcp::resolver::iterator it = resolver.resolve(query, ec);

    if (!ec) {
      auto handler = boost::bind(&Client::HandleConnect,
                                 this,
                                 boost::asio::placeholders::error);
      socket_.async_connect(*it, handler);
    }
  }

private:
  void HandleConnect(const boost::system::error_code& ec) {
    if (ec) {
      std::cerr << ec << std::endl;
      return;
    }

    std::string msg = "hello";
    boost::array<char, 128> buf;

    {
      boost::system::error_code ec;
      socket_.write_some(boost::asio::buffer(msg), ec);

      if (ec) {
        return;
      }

      size_t len = socket_.read_some(boost::asio::buffer(buf), ec);

      if (ec == boost::asio::error::eof) {
        return;  // Connection closed cleanly by peer.
      } else if (ec) {
        return;  // Some other error.
      }

      std::cout.write(buf.data(), len);
      std::cout << std::endl;
    }
  }

private:
  tcp::socket socket_;
};

int main() {
  boost::asio::io_service io_service;

  Client client(io_service);

  client.Connect("127.0.0.1", "5999");
  //client.Connect("localhost", "5999");

  io_service.run();

  return 0;
}