// HTTPs client sending a GET request.
// Based on Asio asynchronous APIs but run in blocking mode.
// Why not just use synchronous APIs?
// Because you might want to add a deadline timer for timeout control.
// See |ssl_http_client_async_blocking_timeout| for the details.

#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"

// -----------------------------------------------------------------------------

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

typedef ssl::stream<tcp::socket> ssl_socket;

// Verify the certificate of the peer (remote host).
#define SSL_VERIFY 0

// -----------------------------------------------------------------------------

class Client {
public:
  Client(const std::string& host, const std::string& path);

  bool Request();

private:
  bool Connect();

  bool Handshake();

  bool SendRequest();

  bool ReadResponse();

  boost::asio::io_context io_context_;

  std::string host_;
  std::string path_;

  ssl::context ssl_context_;
  ssl::stream<tcp::socket> ssl_socket_;

  boost::asio::streambuf request_;

  std::vector<char> buffer_;
};

// -----------------------------------------------------------------------------

Client::Client(const std::string& host, const std::string& path)
    : host_(host), path_(path),
      ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context_, ssl_context_),
      buffer_(1024) {

  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
}

bool Client::Request() {
  if (!Connect()) {
    return false;
  }

  if (!Handshake()) {
    return false;
  }

  if (!SendRequest()) {
    return false;
  }

  return ReadResponse();
}

bool Client::Connect() {
  boost::system::error_code ec;

  // Get a list of endpoints corresponding to the server name.
  tcp::resolver resolver(io_context_);
  auto endpoints = resolver.resolve(host_, "https", ec);

  if (ec) {
    std::cerr << "Resolve failed: " << ec.message() << std::endl;
    return false;
  }

  ec = boost::asio::error::would_block;

  // ConnectHandler: void (boost::system::error_code, tcp::endpoint)
  // Using |boost::lambda::var()| is identical to:
  //   boost::asio::async_connect(
  //       ssl_socket_.lowest_layer(), endpoints,
  //       [this, &ec](boost::system::error_code inner_ec, tcp::endpoint) {
  //         ec = inner_ec;
  //       });
  boost::asio::async_connect(ssl_socket_.lowest_layer(), endpoints,
                             boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    std::cerr << "Connect failed: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

bool Client::Handshake() {
  boost::system::error_code ec = boost::asio::error::would_block;

#if SSL_VERIFY
  ssl_socket_.set_verify_mode(ssl::verify_peer);
#else
  ssl_socket_.set_verify_mode(ssl::verify_none);
#endif  // SSL_VERIFY

  // ssl::host_name_verification has been added since Boost 1.73 to replace
  // ssl::rfc2818_verification.
#if BOOST_VERSION < 107300
  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host_));
#else
  ssl_socket_.set_verify_callback(ssl::host_name_verification(host_));
#endif  // BOOST_VERSION < 107300

  // HandshakeHandler: void (boost::system::error_code)
  ssl_socket_.async_handshake(ssl::stream_base::client,
                              boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    std::cerr << "Handshake failed: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

bool Client::SendRequest() {
  std::ostream request_stream(&request_);
  request_stream << "GET " << path_ << " HTTP/1.1\r\n";
  request_stream << "Host: " << host_ << "\r\n\r\n";

  boost::system::error_code ec = boost::asio::error::would_block;

  // WriteHandler: void (boost::system::error_code, std::size_t)
  boost::asio::async_write(ssl_socket_, request_,
                           boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    std::cerr << "Write failed: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

bool Client::ReadResponse() {
  boost::system::error_code ec = boost::asio::error::would_block;

  ssl_socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, &ec](boost::system::error_code inner_ec, std::size_t length) {
        ec = inner_ec;

        if (inner_ec || length == 0) {
          std::cout << "Socket read error." << std::endl;
          return;
        }

        std::cout.write(buffer_.data(), length);

        // TODO: Call ReadResponse() to read until the end.
      });

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    std::cout << "Read failed: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <host> <path>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " www.boost.org /LICENSE_1_0.txt" << std::endl;
  std::cout << "    " << argv0 << " www.google.com /" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    Help(argv[0]);
    return 1;
  }

  std::string host = argv[1];
  std::string path = argv[2];

  try {
    Client client(host, path);

    client.Request();

  } catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
