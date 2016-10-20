#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// Synchronous echo client.

using boost::asio::ip::tcp;

static const size_t MAX_SIZE = 1024;

int main() {
  try {
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "localhost", "2016");
    tcp::resolver::iterator it = resolver.resolve(query);

    tcp::socket socket(io_service);
    // NOTE: Don't call socket.connect(*it).
    // boost::asio::connect might try several endpoints 
    boost::asio::connect(socket, it);

    while (true) {
      char req_buf[MAX_SIZE];
      std::cin.getline(req_buf, MAX_SIZE);
      size_t req_len = strlen(req_buf);
      boost::asio::write(socket, boost::asio::buffer(req_buf, req_len));

      boost::array<char, MAX_SIZE> res_buf;
      // NOTE: Can't call boost::asio::read(socket, boost::asio::buffer(res));
      size_t res_len = socket.read_some(boost::asio::buffer(res_buf));
      std::cout.write(res_buf.data(), res_len);
      std::cout << std::endl;
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
