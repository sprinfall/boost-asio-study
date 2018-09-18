// Synchronous echo client.

#include <array>
#include <iostream>

#define BOOST_ASIO_NO_DEPRECATED

#include "boost/asio/connect.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

using boost::asio::ip::tcp;

#define USE_GLOBAL_READ 0

enum { BUF_SIZE = 1024 };

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  const char* host = argv[1];
  const char* port = argv[2];

  boost::asio::io_context io_context;

  // NOTE:
  // Don't use output parameter |error_code| in this example.
  // Using exception handling could largely simplify the source code.
  try {
    tcp::resolver resolver(io_context);

    // Return type: tcp::resolver::results_type
    auto endpoints = resolver.resolve(tcp::v4(), host, port);

    // Don't use socket.connect() directly.
    // Global function connect() calls socket.connect() internally.
    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    // Get user input.

    char request[BUF_SIZE];
    std::size_t request_length = 0;
    do {
      std::cout << "Enter message: ";
      std::cin.getline(request, BUF_SIZE);
      request_length = strlen(request);
    } while (request_length == 0);

    // Write to the socket.
    boost::asio::write(socket, boost::asio::buffer(request, request_length));

    // Read the response.
    // Use global read() or not (please note the difference).

    std::cout << "Reply is: ";

#if USE_GLOBAL_READ
    // Receive reply with global read().

    char reply[BUF_SIZE];

    // Global read() returns once the specified size of buffer has been
    // fully filled.
    std::size_t reply_length = boost::asio::read(
        socket,
        boost::asio::buffer(reply, request_length));

    std::cout.write(reply, reply_length);

#else
    // Receive reply with socket.read_some().

    std::size_t total_reply_length = 0;
    while (true) {
      std::array<char, BUF_SIZE> reply;
      std::size_t reply_length = socket.read_some(boost::asio::buffer(reply));

      std::cout.write(reply.data(), reply_length);

      total_reply_length += reply_length;
      if (total_reply_length >= request_length) {
        break;
      }
    }

#endif  // USE_GLOBAL_READ

    std::cout << std::endl;

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
