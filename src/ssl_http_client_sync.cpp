// HTTPs client sending a GET request.
// Based on Asio synchronous APIs.
// Adapted from: https://stackoverflow.com/q/28264313/6825348

#include <iostream>
#include <string>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

// Verify the certificate of the peer (remote host).
#define SSL_VERIFY 0

#define VERBOSE_VERIFICATION 1

// -----------------------------------------------------------------------------

#if VERBOSE_VERIFICATION

// Helper class that prints the current certificate's subject name and the
// verification results.
template <typename Verifier>
class VerboseVerification {
public:
  VerboseVerification(Verifier verifier) : verifier_(verifier) {
  }

  bool operator()(bool preverified, ssl::verify_context& ctx) {
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

    bool verified = verifier_(preverified, ctx);
    std::cout << "Verifying: " << subject_name << std::endl;
    std::cout << "Verified: " << verified << std::endl;
    return verified;
  }

private:
  Verifier verifier_;
};

template <typename Verifier>
VerboseVerification<Verifier> MakeVerboseVerification(Verifier verifier) {
  return VerboseVerification<Verifier>(verifier);
}

#endif  // VERBOSE_VERIFICATION

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
    boost::asio::io_context io_context;

    ssl::context ssl_context(ssl::context::sslv23);

    // Use the default paths for finding CA certificates.
    // Trusted certificates are often installed or updated via the OS, browsers,
    // or individual packages. For instance, in the *nix world, the certificates
    // are often available through the ca-certificates package, and the
    // certificates are installed to locations that set_default_verify_paths()
    // will find.
    // Run the following command on Linux for the package information:
    //   $ aptitude search ca-certifi*
    // See also: https://serverfault.com/questions/62496/ssl-certificate-location-on-unix-linux
    // On Windows, you have to set environment variable SSL_CERT_FILE properly.
    ssl_context.set_default_verify_paths();

    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, "https");  // TODO: "https"

    // Try each endpoint until we successfully establish a connection.
    ssl::stream<tcp::socket> ssl_socket(io_context, ssl_context);
    boost::asio::connect(ssl_socket.lowest_layer(), endpoints);

#if SSL_VERIFY
    // Perform SSL handshake and verify the remote host's certificate.
    ssl_socket.set_verify_mode(ssl::verify_peer);

#else
    ssl_socket.set_verify_mode(ssl::verify_none);
#endif  // SSL_VERIFY

#if VERBOSE_VERIFICATION
    ssl_socket.set_verify_callback(
      MakeVerboseVerification(ssl::rfc2818_verification(host)));
#else
    ssl_socket.set_verify_callback(ssl::rfc2818_verification(host));
#endif  // VERBOSE_VERIFICATION

    ssl_socket.handshake(ssl::stream_base::client);

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n\r\n";

    // Send the request.
    boost::asio::write(ssl_socket, request);

    // Read response.

    // Read the status line.
    boost::asio::streambuf response;
    boost::asio::read_until(ssl_socket, response, "\r\n");

    std::istream response_stream(&response);
    std::string status_line;
    std::getline(response_stream, status_line);
    std::cout << status_line << std::endl;

    // TODO: Continue to read until the end.

  } catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
