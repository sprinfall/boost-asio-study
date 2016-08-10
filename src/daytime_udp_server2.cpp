#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::udp;

// An asynchronous UDP daytime server.

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

class udp_server {
public:
  udp_server(boost::asio::io_service& io_service)
    : socket_(io_service, udp::endpoint(udp::v4(), 13)) {
    start_receive();
  }

private:
  void start_receive() {
    auto handler = boost::bind(&udp_server::handle_receive,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred);

    socket_.async_receive_from(boost::asio::buffer(recv_buf_),
                               remote_endpoint_,
                               handler);
                               
  }

  // Service the client request.
  void handle_receive(const boost::system::error_code& error,
                      size_t bytes_transferred) {
    // error::message_size means the client sent anything larger than the
    // 1-byte recv_buf_, ignore such an error.
    if (!error || error == boost::asio::error::message_size) {
      boost::shared_ptr<std::string> msg(new std::string(make_daytime_string()));

      auto handler = boost::bind(&udp_server::handle_send, this, msg,
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred);

      socket_.async_send_to(boost::asio::buffer(*msg),
                            remote_endpoint_,
                            handler);

      // Start listening for the next client request.
      start_receive();
    }
  }

  // The first argument makes sure the message won't be destroyed until the
  // send is done.
  void handle_send(boost::shared_ptr<std::string> msg,
                   const boost::system::error_code& error,
                   size_t bytes_transferred) {
  }

private:
  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  boost::array<char, 1> recv_buf_;
};

int main() {
  boost::asio::io_service io_service;
  udp_server server(io_service);
  io_service.run();

  return 0;
}