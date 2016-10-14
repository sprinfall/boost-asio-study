#include <cstdlib>
#include <iostream>
#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// NOTE:
// E.g., win_iocp_operation will be allocated and deallocated several times during
// socket.async_receive() on Windows.

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class HandlerAllocator : private boost::noncopyable {
public:
  HandlerAllocator()
    : in_use_(false) {
  }

  void* Allocate(std::size_t size) {
    if (!in_use_ && size < storage_.size) {
      in_use_ = true;
      return storage_.address();
    } else {
      return ::operator new(size);
    }
  }

  void Deallocate(void* p) {
    if (p == storage_.address()) {
      in_use_ = false;
    } else {
      ::operator delete(p);
    }
  }

private:
  // Storage space used for handler-based custom memory allocation.
  boost::aligned_storage<1024> storage_;

  // Whether the handler-based custom allocation storage has been used.
  bool in_use_;
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class CustomAllocHandler {
public:
  CustomAllocHandler(HandlerAllocator& allocator, Handler handler)
    : allocator_(allocator), handler_(handler) {
  }

  template <typename Arg1>
  void operator()(Arg1 arg1) {
    handler_(arg1);
  }

  template <typename Arg1, typename Arg2>
  void operator()(Arg1 arg1, Arg2 arg2) {
    handler_(arg1, arg2);
  }

  friend void* asio_handler_allocate(std::size_t size,
                                     CustomAllocHandler<Handler>* this_handler) {
    std::cout << "asio_handler_allocate: " << size << std::endl;
    return this_handler->allocator_.Allocate(size);
  }

  friend void asio_handler_deallocate(void* p,
                                      std::size_t size,
                                      CustomAllocHandler<Handler>* this_handler) {
    std::cout << "asio_handler_deallocate: " << size << std::endl;
    this_handler->allocator_.Deallocate(p);
  }

private:
  HandlerAllocator& allocator_;
  Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline CustomAllocHandler<Handler> MakeCustomAllocHandler(HandlerAllocator& a, Handler h) {
  return CustomAllocHandler<Handler>(a, h);
}

class Connection : public boost::enable_shared_from_this<Connection> {
public:
  typedef boost::shared_ptr<Connection> Pointer;

  static Pointer Create(boost::asio::io_service& io_service) {
    return Pointer(new Connection(io_service));
  }

  tcp::socket& socket() {
    return socket_;
  }

  void Start() {
    DoRead();
  }

  void HandleRead(const boost::system::error_code& ec, size_t bytes_transferred) {
    if (!ec) {
      DoWrite(bytes_transferred);
    }
  }

  void HandleWrite(const boost::system::error_code& ec) {
    if (!ec) {
      DoRead();
    }
  }

  void DoRead() {
    auto handler = boost::bind(&Connection::HandleRead,
                               shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred);

    socket_.async_read_some(boost::asio::buffer(data_),
                            MakeCustomAllocHandler(allocator_, handler));
  }

  void DoWrite(size_t bytes_transferred) {
    auto handler = boost::bind(&Connection::HandleWrite,
                               shared_from_this(),
                               boost::asio::placeholders::error);

    boost::asio::async_write(socket_,
                             boost::asio::buffer(data_, bytes_transferred),
                             MakeCustomAllocHandler(allocator_, handler));
  }

private:
  Connection(boost::asio::io_service& io_service)
    : socket_(io_service) {
  }

private:
  // The socket used to communicate with the client.
  tcp::socket socket_;

  // Buffer used to store data received from the client.
  boost::array<char, 1024> data_;

  // The allocator to use for handler-based custom memory allocation.
  HandlerAllocator allocator_;
};

class Server {
public:
  Server(boost::asio::io_service& io_service, short port)
      : io_service_(io_service)
      , acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
    StartAccept();
  }

private:
  void StartAccept() {
    Connection::Pointer connection(Connection::Create(io_service_));

    auto handler = boost::bind(&Server::HandleAccept,
                               this,
                               connection,
                               boost::asio::placeholders::error);
    acceptor_.async_accept(connection->socket(), handler);
  }

  void HandleAccept(Connection::Pointer connection, const boost::system::error_code& ec) {
    if (!ec) {
      connection->Start();
    }

    // Accept another connection.
    StartAccept();
  }

private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};

int main() {
  boost::asio::io_service io_service;

  Server server(io_service, 5999);
  io_service.run();

  return 0;
}
