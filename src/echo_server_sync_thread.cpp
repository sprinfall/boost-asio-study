// Synchronous echo server.
// Create a thread for each incoming connection.

#include <array>
#include <iostream>
#include <memory>
#include <string>

// Use C++11 move semantics for the socket.
// Need 2015 or above for Visual Studio.
#define USE_MOVE_SEMANTICS 0

#if USE_MOVE_SEMANTICS
#include <thread>
#else
#include "boost/thread.hpp"
#endif

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"

using boost::asio::ip::tcp;

enum { BUF_SIZE = 128 };

#if USE_MOVE_SEMANTICS
void Session(tcp::socket socket) {
#else
void Session(std::shared_ptr<tcp::socket>& socket) {
#endif  // USE_MOVE_SEMANTICS
  try {
    while (true) {
      std::array<char, BUF_SIZE> data;

      boost::system::error_code ec;

#if USE_MOVE_SEMANTICS
      std::size_t length = socket.read_some(boost::asio::buffer(data), ec);
#else
      std::size_t length = socket->read_some(boost::asio::buffer(data), ec);
#endif

      if (ec == boost::asio::error::eof) {
        break;  // Connection closed cleanly by peer.
      } else if (ec) {
        throw boost::system::system_error(ec);  // Some other error.
      }

#if USE_MOVE_SEMANTICS
      boost::asio::write(socket, boost::asio::buffer(data, length));
#else
      boost::asio::write(*socket, boost::asio::buffer(data, length));
#endif
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  unsigned short port = std::atoi(argv[1]);

  boost::asio::io_context io_context;

  // Create an acceptor to listen for new connections.
  tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

  while (true) {
    // Handle each connection in a separate thread.

#if USE_MOVE_SEMANTICS

    // The socket object returned from accept will be moved to Session's
    // parameter without any copy cost.
    std::thread(&Session, acceptor.accept()).detach();

#else

    // Create a socket that will represent the connection to the client.
    std::shared_ptr<tcp::socket> socket(new tcp::socket(io_context));

    // Wait for a connection.
    acceptor.accept(*socket);

    boost::thread t(std::bind(&Session, socket));

#endif  // USE_MOVE_SEMANTICS
  }

  return 0;
}
