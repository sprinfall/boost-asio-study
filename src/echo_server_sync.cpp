#include <iostream>
#include <string>

#include <boost/array.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#define SOCKET_REF 1

enum {
  BUF_SIZE = 1024
};

#if SOCKET_REF
void Session(tcp::socket& socket) {
#else
void Session(tcp::socket socket) {  // Better
#endif
  try {
    while (true) {
      boost::array<char, BUF_SIZE> data;

      boost::system::error_code ec;
      size_t length = socket.read_some(boost::asio::buffer(data), ec);

      if (ec == boost::asio::error::eof) {
        std::cout << "Connection closed cleanly by peer\n";
        break;
      } else if (ec) {
        // Some other error
        throw boost::system::system_error(ec);
      }

      boost::asio::write(socket, boost::asio::buffer(data, length));
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " <<  e.what() << std::endl;
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
    while (true) {
#if SOCKET_REF
#if 0
      tcp::socket socket(ioc);
      acceptor.accept(socket);
      Session(socket);
#else
      tcp::socket socket = acceptor.accept();
      Session(socket);
#endif
#else
      // Better
      Session(acceptor.accept());
#endif
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
