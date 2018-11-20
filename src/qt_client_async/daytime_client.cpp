#include "daytime_client.h"

#include <functional>
#include <iostream>

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

using boost::asio::ip::tcp;

DaytimeClient::DaytimeClient(boost::asio::io_context& io_context,
                             const std::string& host)
    : socket_(io_context), host_(host) {
}

void DaytimeClient::Start() {
  tcp::resolver resolver(socket_.get_executor().context());
  auto endpoints = resolver.resolve(host_, "daytime");

  // ConnectHandler: void(boost::system::error_code, tcp::endpoint)
  boost::asio::async_connect(socket_, endpoints,
                             std::bind(&DaytimeClient::OnConnect,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
}

void DaytimeClient::OnConnect(boost::system::error_code ec,
                              tcp::endpoint endpoint) {
  if (ec) {
    std::cout << "Connect failed: " << ec.message() << std::endl;
    socket_.close();
  } else {
    DoRead();
  }
}

void DaytimeClient::DoRead() {
  socket_.async_read_some(boost::asio::buffer(buf_),
                          std::bind(&DaytimeClient::OnRead,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void DaytimeClient::OnRead(boost::system::error_code ec, std::size_t length) {
  if (!ec) {
    // This will not print anything except when launched from Linux termimal.
    // If you want to display something in the GUI to indicate the status (a
    // progress bar, maybe), remember this is executed in a separate thread,
    // so you have to "notify" the GUI instead of call it directly. To notify
    // the GUI, Qt's signal machanism should work.
    std::cout.write(buf_.data(), length);
    std::cout << std::endl;
  }
}
