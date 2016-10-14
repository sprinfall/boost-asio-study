#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// A synchronous TCP daytime server.

std::string Now() {
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
    std::cout << "Client connected." << std::endl;

    std::string msg = Now();

    boost::system::error_code ec;
    boost::asio::write(socket, boost::asio::buffer(msg), ec);
  }

  return 0;
}