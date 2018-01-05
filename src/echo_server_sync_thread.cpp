#include <iostream>
#include <string>

// Use C++11 move semantics for the socket.
#define USE_MOVE_SEMANTICS 1

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>

#if USE_MOVE_SEMANTICS
#include <thread>
#else
#include <boost/thread.hpp>
#endif

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Synchronous echo server.
// Create a thread for each incoming connection.

#define BUF_SIZE 128

#if USE_MOVE_SEMANTICS
void Session(tcp::socket socket) {
#else
void Session(boost::shared_ptr<tcp::socket>& socket) {
#endif  // USE_MOVE_SEMANTICS
  try {
    while (true) {
      boost::array<char, BUF_SIZE> data;

      boost::system::error_code ec;

#if USE_MOVE_SEMANTICS
      size_t length = socket.read_some(boost::asio::buffer(data), ec);
#else
      size_t length = socket->read_some(boost::asio::buffer(data), ec);
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
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
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

  while (true) {
    // Handle each connection in a separate thread.

#if USE_MOVE_SEMANTICS
    std::thread(Session, acceptor.accept()).detach();

#else
    // Create a socket that will represent the connection to the client.
    boost::shared_ptr<tcp::socket> socket(new tcp::socket(ioc));

    // Wait for a connection.
    acceptor.accept(*socket);

    boost::thread t(boost::bind(Session, socket));
#endif
  }

  return 0;
}
