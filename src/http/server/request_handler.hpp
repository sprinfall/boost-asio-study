#ifndef REQUEST_HANDLER_HPP_
#define REQUEST_HANDLER_HPP_

#include <string>
#include <boost/noncopyable.hpp>

struct Reply;
struct Request;

// The common handler for all incoming requests.
class RequestHandler : private boost::noncopyable {
public:
  // Construct with a directory containing files to be served.
  explicit RequestHandler(const std::string& doc_root);

  // Handle a request and produce a reply.
  void HandleRequest(const Request& req, Reply& rep);

private:
  // The directory containing the files to be served.
  std::string doc_root_;

  // Perform URL-decoding on a string. Returns false if the encoding was invalid.
  static bool UrlDecode(const std::string& in, std::string& out);
};

#endif  // REQUEST_HANDLER_HPP_
