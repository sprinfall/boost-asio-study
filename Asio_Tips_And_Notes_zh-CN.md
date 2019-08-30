# Asio Tips And Notes

本文列举 Asio 各种值得注意的细节。

## No Deprecated

在包含 Asio 头文件之前，定义宏 `BOOST_ASIO_NO_DEPRECATED`，这样在编译时，Asio 就会剔除那些已经过时的接口。

比如在最新的 Boost 1.66 中，`io_service` 已经改名为 `io_context`，如果没有 `BOOST_ASIO_NO_DEPRECATED`，还是可以用 `io_service` 的，虽然那只是 `io_context` 的一个 `typedef`。

`BOOST_ASIO_NO_DEPRECATED` 可以保证你用的是最新修订的 API。长期来看，有便于代码的维护。何况，这些修订正是 Asio 进入标准库的前奏。

```cpp
#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio/io_context.hpp"
#include "boost/asio/steady_timer.hpp"
...
```

## \_WIN32_WINNT Warning

在 Windows 平台，编译时会遇到关于 `_WIN32_WINNT` 的警告。
可以说，这是 Asio 自身的问题。
它应该在某个地方包含 `SDKDDKVer.h`。
不应该让用户自己去定义平台的版本。

如果你用 CMake，可以借助下面这个宏自动检测 `_WIN32_WINNT`：
(详见：https://stackoverflow.com/a/40217291/6825348)
```cmake
if (WIN32)
    macro(get_WIN32_WINNT version)
        if (CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if ("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif ("${verMajor}" MATCHES "10")
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif(CMAKE_SYSTEM_VERSION)
    endmacro(get_WIN32_WINNT)

    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
endif(WIN32)
```

## 尽量少包含头文件

尽量不要直接包含大而全的 `boost/asio.hpp`。
这样做，是为了帮助自己记忆哪个类源于哪个具体的头文件，以及避免包含那些不必要的头文件。

在实际项目中，在你自己的某个「头文件」里简单粗暴的包含 `boost/asio.hpp` 是很不妥的；当然，在你的「源文件」里包含 `boost/asio.hpp` 是可以接受的，毕竟实际项目依赖的东西比较多，很难搞清楚每一个定义源自哪里。

## Handler 签名问题

虽然关于 Handler 的签名，文档里都有说明，但是直接定位到源码，更方便，也更精确。

以 `steady_timer.async_wait()` 为例，在 IDE 里定位到 `async_wait()` 的定义，代码（片段）如下：
```cpp
  template <typename WaitHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WaitHandler,
      void (boost::system::error_code))
  async_wait(BOOST_ASIO_MOVE_ARG(WaitHandler) handler)
  {
```

通过宏 `BOOST_ASIO_INITFN_RESULT_TYPE`，`WaitHandler` 的签名一目了然。


## Handler 的 error_code 参数到底是不是引用？

其实，早期的版本应该是 `const boost::system::error_code&`，现在文档和代码注释里还有这么写的，估计是没来得及更新。
前面在说 Handler 签名时，已经看到 `BOOST_ASIO_INITFN_RESULT_TYPE` 这个宏的提示作用，翻一翻 Asio 源码，`error_code` 其实都已经传值了。

奇怪的是，即使你的 Handler 传 `error_code` 为引用，编译运行也都没有问题。
```cpp
void Print(const boost::system::error_code& ec) {
  std::cout << "Hello, world!" << std::endl;
}

int main() {
  boost::asio::io_context ioc;
  boost::asio::steady_timer timer(ioc, std::chrono::seconds(3));

  timer.async_wait(&Print);

  ioc.run();
  return 0;
}
```

而我发现，当 Handler 是成员函数时，就不行了。下面这个 timer 的例子，如果把 `Print` 的 `error_code` 改成引用，就不能编译了。
```cpp
class Printer {
public:
  ...

  void Start() {
    timer_.async_wait(std::bind(&Printer::Print, this, std::placeholders::_1));
  }

private:
  // 不能用 const boost::system::error_code&
  void Print(boost::system::error_code ec) {
    ...
  }

private:
  boost::asio::steady_timer timer_;
  int count_;
};
```
这个问题在习惯了引用的情况下，害苦了我，真是百思不得其解！也算是 Boost 比较坑的一个地方吧。
2019/08/30: 实测 1.70 没有这个问题，可以用 const reference。也许是 1.66 的 bug 吧。

## Bind 占位符

调用 `bind` 时，使用了占位符（placeholder），其实下面四种写法都可以：
```cpp
boost::bind(Print, boost::asio::placeholders::error, &timer, &count)
boost::bind(Print, boost::placeholders::_1, &timer, &count);
boost::bind(Print, _1, &timer, &count);
std::bind(Print, std::placeholders::_1, &timer, &count);
```
第一种，占位符是 Boost Asio 定义的。
第二种，占位符是 Boost Bind 定义的。
第三种，同第二种，之所以可行，是因为 `boost/bind.hpp` 里有一句 `using namespace boost::placeholders;`。
```cpp
// boost/bind.hpp
#include <boost/bind/bind.hpp>

#ifndef BOOST_BIND_NO_PLACEHOLDERS

using namespace boost::placeholders;
...
```
第四种，STL Bind，类似于 Boost Bind，只是没有声明 `using namespace std::placeholders;`。

四种写法，推荐使用二或四。至于是用 Boost Bind 还是 STL Bind，没那么重要。
此外，数字占位符共有 9 个，`_1` - `_9`。

## Endpoint 是一个单词

不要写成 "end point"。


## Server 也可以用 Resolver

TCP Server 的 acceptor 一般是这样构造的：
```cpp
tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), port))
```
也就是说，指定 protocol (`tcp::v4()`) 和 port 就行了。

但是，Asio 的 http 这个例子，确实用了 resolver，根据 IP 地址 resolve 出 endpoint：
```cpp
  tcp::resolver resolver(io_context_);

  tcp::resolver::results_type endpoints = resolver.resolve(address, port);

  tcp::endpoint endpoint = *endpoints.begin();

  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  acceptor_.async_accept(...);
```

http 这个例子之所以这么写，主要是初始化 `acceptor_` 时，还拿不到 endpoint，否则可以直接用下面这个构造函数：
```cpp
basic_socket_acceptor(boost::asio::io_context& io_context,
      const endpoint_type& endpoint, bool reuse_addr = true)
```

这个构造函数注释说它等价于下面这段代码：
```cpp
basic_socket_acceptor<Protocol> acceptor(io_context);
acceptor.open(endpoint.protocol());
if (reuse_addr)
  acceptor.set_option(socket_base::reuse_address(true));
acceptor.bind(endpoint);
acceptor.listen(listen_backlog);
```

下面是不同的 `address` 对应的 endpoints 结果（假定 port 都是 `8080`）：

- "localhost": [::1]:8080, v6; [127.0.0.1]:8080, v4
- "0.0.0.0": 0.0.0.0:8080, v4
- "0::0": [::]:8080, v6
- 本机实际 IP 地址 (e.g., IPv4 "10.123.164.142"): 10.123.164.142:8080, v4。这时候，本机 client 无法通过 "localhost" 连接到这个 server，通过具体的 IP 地址则可以。
- 一个具体的非本机地址 (e.g., IPv4 "10.123.164.145"): exception: bind: The requested address is not valid in its context


## Move Acceptable Handler

使用 `acceptor.async_accept` 时，发现了 Move Acceptable Handler。

简单来说，`async_accept` 接受两种 AcceptHandler，直接看源码：

```cpp
  template <typename MoveAcceptHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(MoveAcceptHandler,
      void (boost::system::error_code, typename Protocol::socket))
  async_accept(BOOST_ASIO_MOVE_ARG(MoveAcceptHandler) handler)
```

```cpp
  template <typename Protocol1, typename AcceptHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(AcceptHandler,
      void (boost::system::error_code))
  async_accept(basic_socket<Protocol1>& peer,
      BOOST_ASIO_MOVE_ARG(AcceptHandler) handler,
      typename enable_if<is_convertible<Protocol, Protocol1>::value>::type* = 0)
```

第一种是 Move Acceptable Handler，它的第二个参数是新 accept 的 socket。
第二种是普通的 Handler，它的第一个参数是预先构造的 socket。

对于 Move Acceptable Handler，用 bind 行不通。比如给定：
```cpp
void Server::HandleAccept(boost::system::error_code ec,
                          boost::asio::ip::tcp::socket socket) {
}
```

在 VS 2015 下（支持 C++14），`std::bind` 可以编译，`boost::bind` 则不行。
```cpp
  // std::bind 可以，boost::bind 不可以。
  acceptor_.async_accept(std::bind(&Server::HandleAccept,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2));
```
在 VS 2013 下，`std::bind` 和 `boost::bind` 都不行。

结论是，对于 Move Acceptable Handler，不要用 bind，直接用 lambda 表达式：
```cpp
void DoAccept() {
  acceptor_.async_accept(
    [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
    // Check whether the server was stopped by a signal before this
    // completion handler had a chance to run.
    if (!acceptor_.is_open()) {
      return;
    }

    if (!ec) {
      connection_manager_.Start(
        std::make_shared<Connection>(std::move(socket),
        connection_manager_,
        request_handler_));
    }

    DoAccept();
  });
}
