#ifndef REPLY_HPP_
#define REPLY_HPP_

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "header.hpp"

// A reply to be sent to a client.
struct Reply {
  // The status of the reply.
  enum Status {
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    MOVED_TEMPORARILY = 302,
    NOT_MODIFIED = 304,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503
  };
  
  Status status;

  // The headers to be included in the reply.
  std::vector<Header> headers;

  // The content to be sent in the reply.
  std::string content;

  // Convert the reply into a vector of buffers. The buffers do not own the
  // underlying memory blocks, therefore the reply object must remain valid and
  // not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> ToBuffers();

  // Get a stock reply.
  static Reply StockReply(Status status);
};

#endif  // REPLY_HPP_
