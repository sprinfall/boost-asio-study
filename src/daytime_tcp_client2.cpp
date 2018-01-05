#include <iostream>

#include <boost/array.hpp>
#include <boost/bind.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class TcpClient {
public:
  TcpClient(boost::asio::io_context& ioc, const std::string& host)
      : socket_(ioc) {
    StartConnect(host);
  }

private:
  void StartConnect(const std::string& host) {
    tcp::resolver resolver(socket_.get_executor().context());

    boost::system::error_code ec;
    tcp::resolver::results_type endpoints = resolver.resolve(host, "daytime", ec);

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

  boost::asio::io_context ioc;
  TcpClient tcp_client(ioc, host);

  ioc.run();

  return 0;
}