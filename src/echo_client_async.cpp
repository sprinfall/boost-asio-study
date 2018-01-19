#include <iostream>
#include <string>
#include <functional>

#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

// Asynchronous echo client.

using boost::asio::ip::tcp;

enum {
  BUF_SIZE = 1024
};

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

#define ASYNC_RESOLVE 1

class Client {
public:
  Client(boost::asio::io_context& ioc,
         const std::string& host,
         const std::string& port)
      : socket_(ioc) {

#if ASYNC_RESOLVE
    resolver_.reset(new tcp::resolver(ioc));

    auto handler = std::bind(&Client::HandleResolve,
                             this,
                             std::placeholders::_1,
                             std::placeholders::_2);
    resolver_->async_resolve(tcp::v4(), host, port, handler);

#else
    // If you don't specify tcp::v4() as the first parameter (protocol) of
    // resolve(), the result will have two endpoints, one for v6, one for
    // v4. The first v6 endpoint will fail to connect.

    tcp::resolver resolver(ioc);

#if 1
    endpoints_ = resolver.resolve(tcp::v4(), host, port);
    // 127.0.0.1:2017, v4
#else
    endpoints_ = resolver.resolve(host, port);
    // [::1]:2017, v6
    // 127.0.0.1:2017, v4
#endif

    //DumpEndPoints(endpoints_);

    DoConnect(endpoints_.begin());

#endif  // ASYNC_RESOLVE
  }

private:

#if ASYNC_RESOLVE
  void HandleResolve(boost::system::error_code ec,
                     tcp::resolver::results_type results) {
    if (ec) {
      std::cerr << "Resolve: " << ec.message() << std::endl;
    } else {
      endpoints_ = results;
      DoConnect(endpoints_.begin());
    }
  }
#endif  // ASYNC_RESOLVE

  void DoConnect(tcp::resolver::results_type::iterator endpoint_it) {
    if (endpoint_it != endpoints_.end()) {
      socket_.async_connect(endpoint_it->endpoint(),
                            std::bind(&Client::HandleConnect,
                                      this,
                                      std::placeholders::_1,
                                      endpoint_it));
    }
  }

  void HandleConnect(boost::system::error_code ec,
                     tcp::resolver::results_type::iterator endpoint_it) {
    if (ec) {
      // Will be here if the end point is v6.
      std::cout << "Connect error: " << ec.message() << std::endl;

      socket_.close();

      // Try the next available endpoint.
      DoConnect(++endpoint_it);
    } else {
      DoWrite();
    }
  }
 
  void DoWrite() {
    std::size_t len = 0;
    do {
      std::cout << "Enter message: ";
      std::cin.getline(cin_buf_, BUF_SIZE);
      len = strlen(cin_buf_);
    } while (len == 0);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(cin_buf_, len),
                             std::bind(&Client::HandleWrite,
                                       this,
                                       std::placeholders::_1));
  }

  void HandleWrite(boost::system::error_code ec) {
    if (!ec) {
      std::cout << "Reply is: ";

      socket_.async_read_some(boost::asio::buffer(buf_),
                              std::bind(&Client::HandleRead,
                                        this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    }
  }

  void HandleRead(boost::system::error_code ec,
                  std::size_t length) {
    if (!ec) {
      std::cout.write(buf_.data(), length);
      std::cout << std::endl;

      //DoWrite();
    }
  }

private:
  tcp::socket socket_;

#if ASYNC_RESOLVE
  boost::scoped_ptr<tcp::resolver> resolver_;
#endif
  tcp::resolver::results_type endpoints_;

  char cin_buf_[BUF_SIZE];
  std::array<char, BUF_SIZE> buf_;
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
