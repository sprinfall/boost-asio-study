#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
  boost::asio::io_service io_service;

  // Create an acceptor to listen for new connections.
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 2016));

  boost::array<char, 1024> data;

  // Handle one connection at a time.
  while (true) {
    // Create a socket that will represent the connection to the client.
    tcp::socket socket(io_service);
    // Wait for a connection.
    acceptor.accept(socket);

    boost::system::error_code ec;
    size_t bytes = socket.read_some(boost::asio::buffer(data), ec);
    if (!ec && bytes > 0) {
      boost::asio::write(socket, boost::asio::buffer(data, bytes), ec);
    }
  }

  return 0;
}
