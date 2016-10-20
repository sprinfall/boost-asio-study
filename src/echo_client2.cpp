#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

// Asynchronous echo client.

using boost::asio::ip::tcp;

static const size_t MAX_SIZE = 1024;

class Client {
public:
  Client(boost::asio::io_service& io_service,
         const std::string& host,
         const std::string& port)
      : socket_(io_service) {
    resolver_.reset(new tcp::resolver(socket_.get_io_service()));
    query_.reset(new tcp::resolver::query(tcp::v4(), host, port));

    auto handler = boost::bind(&Client::HandleResolve, this, _1, _2);
    resolver_->async_resolve(*query_, handler);
  }

private:
  void HandleResolve(const boost::system::error_code& ec,
                     tcp::resolver::iterator it) {
    if (!ec) {
      auto handler = boost::bind(&Client::HandleConnect, this, _1);
      socket_.async_connect(*it, handler);
    }
  }

  void HandleConnect(const boost::system::error_code& ec) {
    if (!ec) {
      DoWrite();
    }
  }

  void HandleWrite(const boost::system::error_code& ec) {
    if (!ec) {
      socket_.async_read_some(boost::asio::buffer(buf_),
                              boost::bind(&Client::HandleRead, this, _1, _2));
    }
  }

  void HandleRead(const boost::system::error_code& ec,
                  size_t bytes_transferred) {
    if (!ec) {
      std::cout.write(buf_.data(), bytes_transferred);
      std::cout << std::endl;

      DoWrite();
    }
  }

  void DoWrite() {
    size_t len = 0;
    do {
      std::cin.getline(cin_buf_, MAX_SIZE);
      len = strlen(cin_buf_);
    } while (len == 0);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(cin_buf_, len),
                             boost::bind(&Client::HandleWrite, this, _1));
  }

private:
  tcp::socket socket_;

  boost::scoped_ptr<tcp::resolver> resolver_;
  boost::scoped_ptr<tcp::resolver::query> query_;

  char cin_buf_[MAX_SIZE];
  boost::array<char, MAX_SIZE> buf_;
};

int main() {
  boost::asio::io_service io_service;
  Client client(io_service, "localhost", "2016");
  io_service.run();
  return 0;
}