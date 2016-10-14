#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "request_handler.hpp"

// The top-level class of the HTTP server.
class Server : private boost::noncopyable {
public:
  // Construct the server to listen on the specified TCP address and port, and
  // serve up files from the given directory.
  Server(const std::string& address,
         const std::string& port,
         const std::string& doc_root,
         std::size_t thread_pool_size);

  // Run the server's io_service loop.
  void Run();

private:
  // Initiate an asynchronous accept operation.
  void StartAccept();

  // Handle completion of an asynchronous accept operation.
  void HandleAccept(const boost::system::error_code& ec);

  // Handle a request to stop the server.
  void HandleStop();

private:
  // The number of threads that will call io_service::run().
  std::size_t thread_pool_size_;

  // The io_service used to perform asynchronous operations.
  boost::asio::io_service io_service_;

  // The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  // Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  // The next connection to be accepted.
  connection_ptr new_connection_;

  // The handler for all incoming requests.
  RequestHandler request_handler_;
};

#endif  // SERVER_HPP_
