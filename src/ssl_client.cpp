#include <cstdlib>
#include <iostream>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

// -----------------------------------------------------------------------------

class Client {
public:
  Client(boost::asio::io_context& io_context,
         ssl::context& ssl_context,
         tcp::resolver::results_type endpoints)
      : socket_(io_context, ssl_context) {
    socket_.set_verify_mode(ssl::verify_peer);

    socket_.set_verify_callback(std::bind(&Client::VerifyCertificate, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2));

    boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                               std::bind(&Client::HandleConnect, this,
                                         std::placeholders::_1));
  }

  bool VerifyCertificate(bool preverified, ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }

  void HandleConnect(boost::system::error_code ec) {
    if (!ec) {
      socket_.async_handshake(ssl::stream_base::client,
                              std::bind(&Client::HandleHandshake, this,
                                        std::placeholders::_1));
    } else {
      std::cout << "Connect failed: " << ec.message() << "\n";
    }
  }

  void HandleHandshake(boost::system::error_code ec) {
    if (!ec) {
      std::cout << "Enter message: ";
      std::cin.getline(request_, kMaxLength);
      size_t request_length = strlen(request_);

      boost::asio::async_write(socket_,
                               boost::asio::buffer(request_, request_length),
                               std::bind(&Client::HandleWrite, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
    } else {
      std::cout << "Handshake failed: " << ec.message() << "\n";
    }
  }

  void HandleWrite(boost::system::error_code ec, size_t length) {
    if (!ec) {
      boost::asio::async_read(socket_,
                              boost::asio::buffer(reply_, length),
                              std::bind(&Client::HandleRead, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    } else {
      std::cout << "Write failed: " << ec.message() << "\n";
    }
  }

  void HandleRead(boost::system::error_code ec, size_t length) {
    if (!ec) {
      std::cout << "Reply: ";
      std::cout.write(reply_, length);
      std::cout << "\n";
    } else {
      std::cout << "Read failed: " << ec.message() << "\n";
    }
  }

private:
  ssl::stream<tcp::socket> socket_;

  enum { kMaxLength = 1024 };
  char request_[kMaxLength];
  char reply_[kMaxLength];
};

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  try {
    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);

    ssl::context ssl_context(ssl::context::sslv23);

    // Use the default paths for finding CA certificates.
    //ssl_context.set_default_verify_paths();
    // TODO
    ssl_context.load_verify_file("D:\\github\\boost-asio-study\\src\\ssl\\ca.pem");

    Client client(io_context, ssl_context, endpoints);

    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
