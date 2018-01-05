#include <iostream>

#include <boost/array.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#if 0
#include <boost/asio.hpp>
#else
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#endif

// Synchronous echo client.

using boost::asio::ip::tcp;

#define USE_GLOBAL_READ 0

#define BUF_SIZE 128

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  const char* host = argv[1];
  const char* port = argv[2];

  boost::asio::io_context ioc;

  try {
    tcp::resolver resolver(ioc);

    // You can simply use "auto" instead of "tcp::resolver::results_type".
    tcp::resolver::results_type endpoints = resolver.resolve(tcp::v4(), host, port);

    // Don't use socket.connect() directly.
    // Function connect() calls socket.connect() internally.
    tcp::socket socket(ioc);
    boost::asio::connect(socket, endpoints);

    std::cout << "Enter message: ";

    // Get user input.
    char request[BUF_SIZE];
    std::cin.getline(request, BUF_SIZE);

    // Write to the socket.
    size_t request_length = strlen(request);
    boost::asio::write(socket, boost::asio::buffer(request, request_length));

    // Read the response.
    // Use global read() or not (please note the difference).

    std::cout << "Reply is: ";

#if USE_GLOBAL_READ
    // Receive response with global read().

    char reply[BUF_SIZE];

    // Global read() returns once the specified size of buffer has been
    // fully filled.
    size_t reply_length = boost::asio::read(
        socket,
        boost::asio::buffer(reply, request_length));

    std::cout.write(reply, reply_length);
#else
    // Receive response with socket.read_some().

    size_t total_response_length = 0;

    while (true) {
      boost::array<char, BUF_SIZE> reply;
      boost::system::error_code ec;
 
      size_t response_length = socket.read_some(boost::asio::buffer(reply), ec);

      if (ec == boost::asio::error::eof) {
        break;  // Connection closed cleanly by peer.
      } else if (ec) {
        throw boost::system::system_error(ec);  // Some other error.
      }

      std::cout.write(reply.data(), response_length);

      // Complete condition!
      total_response_length += response_length;
      if (total_response_length >= request_length) {
        break;
      }
    }
#endif  // USE_GLOBAL_READ

    std::cout << std::endl;

  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
