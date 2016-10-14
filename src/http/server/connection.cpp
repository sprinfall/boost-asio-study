#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"

Connection::Connection(boost::asio::io_service& io_service, RequestHandler& handler)
    : strand_(io_service)
    , socket_(io_service)
    , request_handler_(handler) {
}

void Connection::Start() {
  auto handler = boost::bind(&Connection::HandleRead,
                             shared_from_this(),
                             boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred);
  socket_.async_read_some(boost::asio::buffer(buffer_), strand_.wrap(handler));
}

void Connection::HandleRead(const boost::system::error_code& ec,
                             std::size_t bytes_transferred) {
  if (!ec) {
    boost::tribool result;
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
      request_, buffer_.data(), buffer_.data() + bytes_transferred);

    if (result) {
      request_handler_.HandleRequest(request_, reply_);

      auto handler = boost::bind(&Connection::HandleWrite,
                                 shared_from_this(),
                                 boost::asio::placeholders::error);
      boost::asio::async_write(socket_,
                               reply_.ToBuffers(),
                               strand_.wrap(handler));
    } else if (!result) {
      reply_ = Reply::StockReply(Reply::BAD_REQUEST);

      auto handler = boost::bind(&Connection::HandleWrite,
                                 shared_from_this(),
                                 boost::asio::placeholders::error);
      boost::asio::async_write(socket_,
                               reply_.ToBuffers(),
                               strand_.wrap(handler));
    } else {
      auto handler = boost::bind(&Connection::HandleRead,
                                 shared_from_this(),
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred);
      socket_.async_read_some(boost::asio::buffer(buffer_),
                              strand_.wrap(handler));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

void Connection::HandleWrite(const boost::system::error_code& ec) {
  if (!ec) {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}
