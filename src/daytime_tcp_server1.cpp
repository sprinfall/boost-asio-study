#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// A synchronous TCP daytime server.

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

int main() {
  boost::asio::io_service io_service;

  // Create an acceptor to listen for new connections.
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 13));

  // Handle one connection at a time.
  while (true) {
    // Create a socket that will represent the connection to the client.
    tcp::socket socket(io_service);
    // Wait for a connection.
    acceptor.accept(socket);

    // A client is connected.
    std::string msg = make_daytime_string();
    std::cout << "Client connected." << std::endl;

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(msg), ignored_error);
  }

  return 0;
}