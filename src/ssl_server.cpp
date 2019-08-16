#include <cstdlib>
#include <iostream>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

typedef ssl::stream<tcp::socket> ssl_socket;

// -----------------------------------------------------------------------------

class Session {
public:
  Session(boost::asio::io_context& io_context, ssl::context& ssl_context)
      : socket_(io_context, ssl_context) {
  }

  ssl_socket::lowest_layer_type& socket() {
    return socket_.lowest_layer();
  }

  void Start() {
    socket_.async_handshake(ssl::stream_base::server,
                            std::bind(&Session::HandleHandshake, this,
                                      std::placeholders::_1));
  }

  void HandleHandshake(boost::system::error_code error) {
    if (!error) {
      socket_.async_read_some(boost::asio::buffer(data_, kMaxLength),
                              std::bind(&Session::HandleRead, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    } else {
      delete this;
    }
  }

  void HandleRead(boost::system::error_code ec, std::size_t length) {
    if (!ec) {
      boost::asio::async_write(socket_,
                               boost::asio::buffer(data_, length),
                               std::bind(&Session::HandleWrite, this,
                                         std::placeholders::_1));
    } else {
      delete this;
    }
  }

  void HandleWrite(boost::system::error_code ec) {
    if (!ec) {
      socket_.async_read_some(boost::asio::buffer(data_, kMaxLength),
                              std::bind(&Session::HandleRead, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    } else {
      delete this;
    }
  }

private:
  ssl_socket socket_;

  enum { kMaxLength = 1024 };
  char data_[kMaxLength];
};

// -----------------------------------------------------------------------------

class Server {
public:
  Server(boost::asio::io_context& io_context, unsigned short port)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        ssl_context_(ssl::context::sslv23) {
    ssl_context_.set_options(ssl::context::default_workarounds |
                             ssl::context::no_sslv2 |
                             ssl::context::single_dh_use);

    // See:
    //   - http://blog.think-async.com/2006/09/ssl-password-callbacks.html
    //   - https://stackoverflow.com/a/13986949/6825348
    ssl_context_.set_password_callback(std::bind(&Server::GetPassword, this));

    // TODO
    std::string key = "D:\\github\\boost-asio-study\\src\\ssl\\server.pem";
    std::string dh_file = "D:\\github\\boost-asio-study\\src\\ssl\\dh2048.pem";

    ssl_context_.use_certificate_chain_file(key);
    ssl_context_.use_private_key_file(key, ssl::context::pem);
    ssl_context_.use_tmp_dh_file(dh_file);

    StartAccept();
  }

  std::string GetPassword() const {
    return "test";
  }

  void StartAccept() {
    Session* new_session = new Session(io_context_, ssl_context_);
    acceptor_.async_accept(new_session->socket(),
                           std::bind(&Server::HandleAccept, this, new_session,
                                     std::placeholders::_1));
  }

  void HandleAccept(Session* new_session, boost::system::error_code ec) {
    if (!ec) {
      new_session->Start();
    } else {
      delete new_session;
    }

    StartAccept();
  }

private:
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  ssl::context ssl_context_;
};

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <host>" << std::endl;
    return 1;
  }

  try {
    boost::asio::io_context io_context;

    Server server(io_context, std::atoi(argv[1]));

    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
