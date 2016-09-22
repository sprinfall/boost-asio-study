#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// A synchronous TCP daytime client

// NIST Internet Time Servers:
//   http://tf.nist.gov/tf-cgi/servers.cgi

// Output:
// 57594 16-07-25 05:56:02 50 0 0 610.1 UTC(NIST) *

int main(int argc, char* argv[]) {
  const char* host = NULL;
  if (argc == 2) {
    host = argv[1];
  } else {
    host = "time-a.nist.gov";  // or IP 129.6.15.28
    std::cout << "Host not provided, use '" << host << "' by default." << std::endl;
  }

  boost::asio::io_service io_service;

  using boost::asio::ip::tcp;

  // Use resolver to turn the server name into a TCP endpoint.
  tcp::resolver resolver(io_service);

  // Construct a query using the server name, and the service name.
  tcp::resolver::query query(host, "daytime");

  // The list of endpoints is returned using an iterator.
  tcp::resolver::iterator endpoint_it = resolver.resolve(query);

  // Create and connect the socket.
  tcp::socket socket(io_service);

  // With an error code parameter, no exception will be thrown.
  boost::system::error_code ec;
  boost::asio::connect(socket, endpoint_it, ec);
  if (ec) {
    return 1;
  }

  boost::array<char, 128> buf;

  while (true) {
    boost::system::error_code ec;
    size_t len = socket.read_some(boost::asio::buffer(buf), ec);

    if (ec == boost::asio::error::eof) {
      break;  // Connection closed cleanly by peer.
    } else if (ec) {
      break;  // Some other error.
    }

    std::cout.write(buf.data(), len);
  }

  return 0;
}