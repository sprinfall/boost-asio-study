// HTTPs client sending a GET request.
// Based on Asio asynchronous APIs but run in blocking mode.
// A deadline timer is added for timeout control.

#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"

// -----------------------------------------------------------------------------

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

typedef ssl::stream<tcp::socket> ssl_socket;

// Verify the certificate of the peer (remote host).
#define SSL_VERIFY 0

// Timeout seconds.
const int kMaxConnectSeconds = 10;
const int kMaxHandshakeSeconds = 10;
const int kMaxSendSeconds = 30;
const int kMaxReceiveSeconds = 30;

// -----------------------------------------------------------------------------

class Client {
public:
  Client(const std::string& host, const std::string& path);

  bool Request();

  void set_timeout_seconds(int timeout_seconds) {
    assert(timeout_seconds > 0);
    timeout_seconds_ = timeout_seconds;
  }

  bool timed_out() const { return timed_out_; }

  void Stop();

private:
  bool Connect();

  bool Handshake();

  bool SendRequest();

  bool ReadResponse();

  void CheckDeadline();

  boost::asio::io_context io_context_;

  std::string host_;
  std::string path_;

  ssl::context ssl_context_;
  ssl::stream<tcp::socket> ssl_socket_;

  boost::asio::streambuf request_;

  std::vector<char> buffer_;

  boost::asio::deadline_timer deadline_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  bool stopped_;

  // If the error was caused by timeout or not.
  bool timed_out_;
};

// -----------------------------------------------------------------------------

Client::Client(const std::string& host, const std::string& path)
    : host_(host), path_(path),
      ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context_, ssl_context_),
      buffer_(1024),
      deadline_(io_context_),
      timeout_seconds_(30),
      stopped_(false),
      timed_out_(false) {
  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
}

bool Client::Request() {
  stopped_ = false;
  timed_out_ = false;

  // Start the persistent actor that checks for deadline expiry.
  deadline_.expires_at(boost::posix_time::pos_infin);
  CheckDeadline();

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

void Client::Stop() {
  stopped_ = true;

  boost::system::error_code ignored_ec;
  ssl_socket_.lowest_layer().close(ignored_ec);

  deadline_.cancel();
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

  deadline_.expires_from_now(boost::posix_time::seconds(kMaxConnectSeconds));

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

  // Determine whether a connection was successfully established. The
  // deadline actor may have had a chance to run and close our socket, even
  // though the connect operation notionally succeeded. Therefore we must
  // check whether the socket is still open before deciding if we succeeded
  // or failed.
  if (ec) {
    std::cerr << "Connect failed: " << ec.message() << std::endl;
    return false;
  }

  // The deadline actor may have had a chance to run and close our socket, even
  // though the connect operation notionally succeeded.
  if (stopped_) {
    // |timed_out_| should be true in this case.
    std::cerr << "Connect timed out." << std::endl;
    return false;
  }

  return true;
}

bool Client::Handshake() {
  deadline_.expires_from_now(boost::posix_time::seconds(kMaxHandshakeSeconds));

  boost::system::error_code ec = boost::asio::error::would_block;

#if SSL_VERIFY
  ssl_socket_.set_verify_mode(ssl::verify_peer);
#else
  ssl_socket_.set_verify_mode(ssl::verify_none);
#endif  // SSL_VERIFY

  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host_));

  // HandshakeHandler: void (boost::system::error_code)
  ssl_socket_.async_handshake(ssl::stream_base::client,
                              boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    Stop();
    std::cerr << "Handshake failed: " << ec.message() << std::endl;
    return false;
  }

  if (stopped_) {
    // |timed_out_| should be true in this case.
    std::cerr << "Handshake timed out." << std::endl;
    return false;
  }

  return true;
}

bool Client::SendRequest() {
  std::ostream request_stream(&request_);
  request_stream << "GET " << path_ << " HTTP/1.1\r\n";
  request_stream << "Host: " << host_ << "\r\n\r\n";

  deadline_.expires_from_now(boost::posix_time::seconds(kMaxSendSeconds));

  boost::system::error_code ec = boost::asio::error::would_block;

  // WriteHandler: void (boost::system::error_code, std::size_t)
  boost::asio::async_write(ssl_socket_, request_,
                           boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    Stop();
    std::cerr << "Write failed: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

bool Client::ReadResponse() {
  deadline_.expires_from_now(boost::posix_time::seconds(kMaxSendSeconds));

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

void Client::CheckDeadline() {
  if (stopped_) {
    return;
  }

  if (deadline_.expires_at() <=
      boost::asio::deadline_timer::traits_type::now()) {
    // The deadline has passed.
    // The socket is closed so that any outstanding asynchronous operations
    // are canceled.
    std::cout << "HTTP client timed out." << std::endl;
    Stop();
    timed_out_ = true;
  }

  // Put the actor back to sleep.
  deadline_.async_wait(std::bind(&Client::CheckDeadline, this));
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
