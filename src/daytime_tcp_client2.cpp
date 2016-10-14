#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class TcpClient {
public:
  TcpClient(boost::asio::io_service& io_service, const std::string& host)
      : socket_(io_service) {
    StartConnect(host);
  }

private:
  void StartConnect(const std::string& host) {
    tcp::resolver resolver(socket_.get_io_service());
    tcp::resolver::query query(host, "daytime");

    boost::system::error_code ec;
    tcp::resolver::iterator it = resolver.resolve(query, ec);

    if (!ec) {
      auto handler = boost::bind(&TcpClient::HandleConnect,
                                 this,
                                 boost::asio::placeholders::error);
      socket_.async_connect(*it, handler);
    }
  }

  //void HandleResolve(const boost::system::error_code& ec, tcp::resolver::iterator it) {
  //  if (!ec) {
  //    boost::asio::async_connect(socket_,
  //                               it,
  //                               boost::bind(&TcpClient::ConnectHandler, this,
  //                               boost::asio::placeholders::error));
  //  }
  //}

  void HandleConnect(const boost::system::error_code& ec) {
    if (ec) {
      return;
    }

    boost::array<char, 128> buf;

    while (true) {
      boost::system::error_code ec;
      size_t len = socket_.read_some(boost::asio::buffer(buf), ec);

      if (ec == boost::asio::error::eof) {
        break;  // Connection closed cleanly by peer.
      } else if (ec) {
        break;  // Some other error.
      }

      std::cout.write(buf.data(), len);
    }
  }

private:
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {
  const char* host = NULL;
  if (argc == 2) {
    host = argv[1];
  } else {
    host = "time-a.nist.gov";  // or IP 129.6.15.28
    std::cout << "Host not provided, use '" << host << "' by default." << std::endl;
  }

  boost::asio::io_service io_service;
  TcpClient tcp_client(io_service, host);

  io_service.run();

  return 0;
}