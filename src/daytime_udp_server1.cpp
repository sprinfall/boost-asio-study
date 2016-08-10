#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

// A synchronous UDP daytime server.

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

int main() {
  boost::asio::io_service io_service;

  // Create a udp::socket to receive requests on UDP port 13.
  udp::socket socket(io_service, udp::endpoint(udp::v4(), 13));

  // Wait for a client to initiate contact with us.
  while (true) {
    boost::array<char, 1> recv_buf;
    // The remote_endpoint will be populated by receive_from().
    udp::endpoint remote_endpoint;
    boost::system::error_code error;
    socket.receive_from(boost::asio::buffer(recv_buf),
                        remote_endpoint,
                        0,
                        error);

    if (error && error != boost::asio::error::message_size) {
      break;
    }

    std::string msg = make_daytime_string();

    // Send the message to the remote endpoint.
    boost::system::error_code ignored_error;
    socket.send_to(boost::asio::buffer(msg), remote_endpoint, 0, ignored_error);
  }

  return 0;
}