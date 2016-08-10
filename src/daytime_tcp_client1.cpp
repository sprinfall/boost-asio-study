#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// A synchronous TCP daytime client

// NIST Internet Time Servers:
//   http://tf.nist.gov/tf-cgi/servers.cgi

// Output:
// 57594 16-07-25 05:56:02 50 0 0 610.1 UTC(NIST) *

int main(int argc, char* argv[]) {
  //if (argc != 2) {
  //  std::cerr << "Usage: " << argv[0] << " <host>" << std::endl;
  //  return 1;
  //}

  boost::asio::io_service io_service;

  using boost::asio::ip::tcp;

  // Use resolver to turn the server name into a TCP endpoint.
  tcp::resolver resolver(io_service);

  // Construct a query using the server name, and the service name.
  //tcp::resolver::query query(argv[1], "daytime");
  //tcp::resolver::query query("129.6.15.28", "daytime");
  //tcp::resolver::query query("time-a.nist.gov", "daytime");
  tcp::resolver::query query("localhost", "daytime");

  // The list of endpoints is returned using an iterator.
  tcp::resolver::iterator endpoint_it = resolver.resolve(query);

  // Create and connect the socket.
  tcp::socket socket(io_service);

  // socket.connet(endpoint, error)
  //boost::asio::connect(socket, endpoint_it);  // Exception will be thrown on error.
  boost::system::error_code error;
  boost::asio::connect(socket, endpoint_it, error);  // No exception will be thrown.
  if (error) {
    return 1;
  }

  while (true) {
    boost::array<char, 128> buf;  // TODO: Move outside of while.
    boost::system::error_code error;
    size_t len = socket.read_some(boost::asio::buffer(buf), error);

    if (error == boost::asio::error::eof) {
      break;  // Connection closed cleanly by peer.
    } else if (error) {
      break;  // Some other error.
    }

    std::cout.write(buf.data(), len);
  }

  return 0;
}