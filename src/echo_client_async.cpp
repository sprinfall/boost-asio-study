#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

// Asynchronous echo client.

using boost::asio::ip::tcp;

#define BUF_SIZE 128

// Print the resolved endpoints.
// NOTE: Endpoint is one word, don't use "end point".
void DumpEndpoints(tcp::resolver::results_type& endpoints) {
  std::cout << "Endpoints: " << endpoints.size() << std::endl;

  tcp::resolver::results_type::iterator it = endpoints.begin();
  for (; it != endpoints.end(); ++it) {
    std::cout << "  - " << it->endpoint();

    if (it->endpoint().protocol() == tcp::v4()) {
      std::cout << ", v4";
    } else if (it->endpoint().protocol() == tcp::v6()) {
      std::cout << ", v6";
    }

    std::cout << std::endl;
  }
}

class Client {
public:
  Client(boost::asio::io_context& ioc,
         const std::string& host,
         const std::string& port)
      : socket_(ioc) {
    resolver_.reset(new tcp::resolver(ioc));

    // TODO: async_resolve

    // If you don't specify tcp::v4() as the first parameter (protocol) of
    // resolve(), the result will have two endpoints, one for v6, one for
    // v4. The first v6 endpoint will fail to connect.
#if 1
    endpoints_ = resolver_->resolve(tcp::v4(), host, port);
    // 127.0.0.1:2017, v4
#else
    endpoints_ = resolver_->resolve(host, port);
    // [::1]:2017, v6
    // 127.0.0.1:2017, v4
#endif

    //DumpEndPoints(endpoints_);

    AsyncConnect(endpoints_.begin());
  }

private:
  void AsyncConnect(tcp::resolver::results_type::iterator endpoint_it) {
    if (endpoint_it != endpoints_.end()) {
      socket_.async_connect(endpoint_it->endpoint(),
                            boost::bind(&Client::HandleConnect, this, _1, endpoint_it));
    }
  }

  void HandleConnect(const boost::system::error_code& ec,
                     tcp::resolver::results_type::iterator endpoint_it) {
    if (ec) {
      // Will be here if the end point is v6.
      std::cout << "Connect error: " << ec.message() << std::endl;

      socket_.close();

      // Try the next available endpoint.
      AsyncConnect(++endpoint_it);
    } else {
      AsyncWrite();
    }
  }

  void HandleWrite(const boost::system::error_code& ec) {
    if (!ec) {
      socket_.async_read_some(boost::asio::buffer(buf_),
                              boost::bind(&Client::HandleRead, this, _1, _2));
    }
  }

  void HandleRead(const boost::system::error_code& ec,
                  size_t bytes_transferred) {
    if (!ec) {
      std::cout.write(buf_.data(), bytes_transferred);
      std::cout << std::endl;

      AsyncWrite();
    }
  }

  void AsyncWrite() {
    size_t len = 0;
    do {
      std::cin.getline(cin_buf_, BUF_SIZE);
      len = strlen(cin_buf_);
    } while (len == 0);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(cin_buf_, len),
                             boost::bind(&Client::HandleWrite, this, _1));
  }

private:
  tcp::socket socket_;

  boost::scoped_ptr<tcp::resolver> resolver_;

  tcp::resolver::results_type endpoints_;

  char cin_buf_[BUF_SIZE];
  boost::array<char, BUF_SIZE> buf_;
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  const char* host = argv[1];
  const char* port = argv[2];

  boost::asio::io_context ioc;
  Client client(ioc, host, port);

  ioc.run();

  return 0;
}
