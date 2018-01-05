#include <iostream>
#include <string>

#include <boost/array.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#define BUF_SIZE 128

void Session(tcp::socket& socket) {
  while (true) {
    boost::array<char, BUF_SIZE> data;

    boost::system::error_code ec;
    size_t length = socket.read_some(boost::asio::buffer(data), ec);

    if (ec == boost::asio::error::eof) {
      break;  // Connection closed cleanly by peer.
    } else if (ec) {
      throw boost::system::system_error(ec);  // Some other error.
    }

    boost::asio::write(socket, boost::asio::buffer(data, length));
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  unsigned short port = std::atoi(argv[1]);

  boost::asio::io_context ioc;

  // Create an acceptor to listen for new connections.
  tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port));

  try {
    // Handle one connection at a time.
    // TODO: Create a thread for each incoming connection.
    while (true) {
      // Create a socket that will represent the connection to the client.
      tcp::socket socket(ioc);

      // Wait for a connection.
      acceptor.accept(socket);

      Session(socket);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
