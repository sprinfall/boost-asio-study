#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

// Represents a single connection from a client.
class Connection
  : public boost::enable_shared_from_this<Connection>
  , private boost::noncopyable {
public:
  // Construct a connection with the given io_service.
  explicit Connection(boost::asio::io_service& io_service,
                      RequestHandler& handler);

  // Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket() {
    return socket_;
  }

  // Start the first asynchronous operation for the connection.
  void Start();

private:
  // Handle completion of a read operation.
  void HandleRead(const boost::system::error_code& ec, std::size_t bytes_transferred);

  // Handle completion of a write operation.
  void HandleWrite(const boost::system::error_code& ec);

private:
  // Strand to ensure the connection's handlers are not called concurrently.
  boost::asio::io_service::strand strand_;

  // Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  // The handler used to process the incoming request.
  RequestHandler& request_handler_;

  // Buffer for incoming data.
  boost::array<char, 8192> buffer_;

  // The incoming request.
  Request request_;

  // The parser for the incoming request.
  RequestParser request_parser_;

  // The reply to be sent back to the client.
  Reply reply_;
};

typedef boost::shared_ptr<Connection> connection_ptr;

#endif  // CONNECTION_HPP_
