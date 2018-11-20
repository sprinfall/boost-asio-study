#ifndef DAYTIME_CLIENT_H_
#define DAYTIME_CLIENT_H_

// Asynchronous daytime client.

#include <array>
#include <memory>
#include <string>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"

class DaytimeClient : public std::enable_shared_from_this<DaytimeClient> {
public:
  DaytimeClient(boost::asio::io_context& io_context,
                const std::string& host);

  void Start();

private:
  void OnConnect(boost::system::error_code ec,
                 boost::asio::ip::tcp::endpoint endpoint);

  void DoRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  boost::asio::ip::tcp::socket socket_;
  std::string host_;

  std::array<char, 512> buf_;
};

#endif  // DAYTIME_CLIENT_H_
