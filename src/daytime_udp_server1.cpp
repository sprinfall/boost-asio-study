#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

// A synchronous UDP daytime server.

std::string Now() {
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
    boost::system::error_code ec;
    socket.receive_from(boost::asio::buffer(recv_buf),
                        remote_endpoint,
                        0,
                        ec);

    if (ec && ec != boost::asio::error::message_size) {
      break;
    }

    std::string msg = Now();

    // Send the message to the remote endpoint.
    boost::system::error_code ignored_ec;
    socket.send_to(boost::asio::buffer(msg), remote_endpoint, 0, ignored_ec);
  }

  return 0;
}