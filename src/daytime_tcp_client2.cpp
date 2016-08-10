#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class tcp_client {
public:
  tcp_client(boost::asio::io_service& io_service, const std::string& host)
    : socket_(io_service) {
    start_connect(host);
  }

private:
  void start_connect(const std::string& host) {
    // TODO
    tcp::resolver resolver(socket_.get_io_service());
    tcp::resolver::query query(host, "daytime");

    resolver.async_resolve(query, boost::bind(&tcp_client::handle_resolver, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::iterator));
  }

  void handle_resolver(const boost::system::error_code& error, tcp::resolver::iterator it) {
    if (!error) {
      boost::asio::async_connect(socket_, it,
                                 boost::bind(&tcp_client::handle_connect, this,
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::iterator));
    }
  }

  void handle_connect(const boost::system::error_code& error, tcp::resolver::iterator it) {
    if (error) {
      return;
    }

    while (true) {
      boost::array<char, 128> buf;
      boost::system::error_code error;
      size_t len = socket_.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        break;  // Connection closed cleanly by peer.
      } else if (error) {
        break;  // Some other error.
      }

      std::cout.write(buf.data(), len);
    }
  }

private:
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {
  boost::asio::io_service io_service;
  tcp_client client(io_service, "localhost");
  io_service.run();

  return 0;
}