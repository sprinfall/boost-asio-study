#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// A synchronous UDP daytime client

int main(int argc, char* argv[]) {
  //if (argc != 2) {
  //  std::cerr << "Usage: " << argv[0] << " <host>" << std::endl;
  //  return 1;
  //}

  boost::asio::io_service io_service;

  using boost::asio::ip::udp;

  // Use resolver to find the correct remote endpoint based on the host
  // and service names.
  udp::resolver resolver(io_service);

  // The query is restricted to return only IPv4 endpoints by the
  // ip::udp::v4() argument.
  //udp::resolver::query query(udp::v4(), argv[1], "daytime");
  udp::resolver::query query(udp::v4(), "localhost", "daytime");

  udp::endpoint receiver_endpoint = *resolver.resolve(query);

  // Since UDP is datagram-oriented, we will not use a stream socket.
  // Create a udp::socket and initiate contact with the remote endpoint.
  udp::socket socket(io_service);
  socket.open(udp::v4());
  boost::array<char, 1> send_buf = { { 0 } };
  socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);

  // Be ready to accept the server response.
  boost::array<char, 128> recv_buf;
  udp::endpoint sender_endpoint;
  size_t len = socket.receive_from(
    boost::asio::buffer(recv_buf), sender_endpoint);
 
  std::cout.write(recv_buf.data(), len);

  return 0;
}