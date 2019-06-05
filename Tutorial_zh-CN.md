# 基于 Boost Asio 的 C++ 网络编程

**环境：** Boost v1.66, VS 2013 & 2015

**说明：**
这篇教程形成于 Boost v1.62 时代，最近（2018/01）针对 v1.66 做了一次大的更新。
此外，在代码风格上，C++11 用得更多了。

----

## 概述

近期学习 Boost Asio，依葫芦画瓢，写了不少例子，对这个「轻量级」的网络库算是有了一定理解。但是秉着理论与实践结合的态度，决定写一篇教程，把脑子里一知半解的东西，试图说清楚。

Asio，即「异步 IO」（Asynchronous Input/Output），本是一个 [独立的 C++ 网络程序库](http://think-async.com/Asio)，似乎并不为人所知，后来因为被 Boost 相中，才声名鹊起。

从设计上来看，Asio 相似且重度依赖于 Boost，与 thread、bind、smart pointers 等结合时，体验顺滑。从使用上来看，依然是重组合而轻继承，一贯的 C++ 标准库风格。

什么是「异步 IO」？

简单来说，就是你发起一个 IO 操作，却不用等它结束，你可以继续做其他事情，当它结束时，你会得到通知。

当然这种表述是不精确的，操作系统并没有直接提供这样的机制。以 Unix 为例，有五种 IO 模型可用：

- 阻塞 I/O
- 非阻塞 I/O
- I/O 多路复用（multiplexing）（`select` 和 `poll`）
- 信号驱动 I/O（`SIGIO`）
- 异步 I/O（POSIX `aio_` 系列函数）

这五种模型的定义和比较，详见「Unix Network Programming, Volume 1: The Sockets Networking API」一书 6.2 节，或者可参考 [这篇笔记](https://segmentfault.com/n/1330000004444307)。

Asio 封装的正是「I/O 多路复用」。具体一点，`epoll` 之于 Linux，`kqueue` 之于 Mac 和 BSD。`epoll` 和 `kqueue` 比 `select` 和 `poll` 更高效。当然在 Windows 上封装的则是 IOCP（完成端口）。

Asio 的「I/O 操作」，主要还是指「网络 IO」，比如 socket 读写。由于网络传输的特性，「网络 IO」相对比较费时，设计良好的服务器，不可能同步等待一个 IO 操作的结束，这太浪费 CPU 了。

对于普通的「文件 IO」，操作系统并没有提供“异步”读写机制，libuv 的做法是用线程模拟异步，为网络和文件提供了一致的接口。Asio 并没有这样做，它专注于网络。提供机制而不是策略，这很符合 C++ 哲学。

下面以示例，由浅到深，由简单到复杂，逐一介绍 Asio 的用法。
简单起见，头文件一律省略。

## I/O Context

每个 Asio 程序都至少有一个 `io_context` 对象，它代表了操作系统的 I/O 服务（`io_context` 在 Boost 1.66 之前一直叫 `io_service`），把你的程序和这些服务链接起来。

下面这个程序空有 `io_context` 对象，却没有任何异步操作，所以它其实什么也没做，也没有任何输出。
```cpp
int main() {
  boost::asio::io_context ioc;
  ioc.run();
  return 0;
}
```

`io_context.run` 是一个阻塞（blocking）调用，姑且把它想象成一个 loop（事件循环），直到所有异步操作完成后，loop 才结束，`run` 才返回。但是这个程序没有任何异步操作，所以 loop 直接就结束了。

## Timer

有了 `io_context` 还不足以完成 I/O 操作，用户一般也不跟 `io_context` 直接交互。

根据 I/O 操作的不同，Asio 提供了不同的 I/O 对象，比如 timer（定时器），socket，等等。
Timer 是最简单的一种 I/O 对象，可以用来实现异步调用的超时机制，下面是最简单的用法：

```cpp
void Print(boost::system::error_code ec) {
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

先创建一个 `steady_timer`，指定时间 3 秒，然后异步等待这个 timer，3 秒后，timer 超时结束，`Print` 被调用。

以下几点需要注意：
- 所有 I/O 对象都依赖 `io_context`，一般在构造时指定。
- `async_wait` 初始化了一个异步操作，但是这个异步操作的执行，要等到 `io_context.run` 时才开始。
- Timer 除了异步等待（`async_wait`），还可以同步等待（`wait`）。同步等待是阻塞的，直到 timer 超时结束。基本上所有 I/O 对象的操作都有同步和异步两个版本，也许是出于设计上的完整性。
- `async_wait` 的参数是一个函数对象，异步操作完成时它会被调用，所以也叫 completion handler，简称 handler，可以理解成回调函数。
- 所有 I/O 对象的 `async_xyz` 函数都有 handler 参数，对于 handler 的签名，不同的异步操作有不同的要求，除了官方文档里的说明，也可以直接查看 Boost 源码。

`async_wait` 的 handler 签名为 `void (boost::system::error_code)`，如果要传递额外的参数，就得用 `bind`。不妨修改一下 `Print`，让它每隔一秒打印一次计数，从 `0` 递增到 `3`。

```cpp
void Print(boost::system::error_code ec,
           boost::asio::steady_timer* timer,
           int* count) {
  if (*count < 3) {
    std::cout << *count << std::endl;
    ++(*count);

    timer->expires_after(std::chrono::seconds(1));
    
    timer->async_wait(std::bind(&Print, std::placeholders::_1, timer, count));
  }
}
```
与前版相比，`Print` 多了两个参数，以便访问当前计数及重启 timer。
```cpp
int main() {
  boost::asio::io_context ioc;
  boost::asio::steady_timer timer(ioc, std::chrono::seconds(1));
  int count = 0;
  timer.async_wait(std::bind(&Print, std::placeholders::_1, &timer, &count));

  ioc.run();
  return 0;
}
```
调用 `bind` 时，使用了占位符（placeholder）`std::placeholders::_1`。数字占位符共有 9 个，`_1` - `_9`。占位符也有很多种写法，这里就不详述了。

## Echo Server

Socket 也是一种 I/O 对象，这一点前面已经提及。相比于 timer，socket 更为常用，毕竟 Asio 是一个网络程序库。

下面以经典的 Echo 程序为例，实现一个 TCP Server。所谓 Echo，就是 Server 把 Client 发来的内容原封不动发回给 Client。

先从同步方式开始，异步太复杂，慢慢来。

### 同步方式

`Session` 代表会话，负责管理一个 client 的连接。参数 `socket` 传的是值，但是会用到 move 语义来避免拷贝。
```cpp
void Session(tcp::socket socket) {
  try {
    while (true) {
      boost::array<char, BUF_SIZE> data;

      boost::system::error_code ec;
      std::size_t length = socket.read_some(boost::asio::buffer(data), ec);

      if (ec == boost::asio::error::eof) {
        std::cout << "连接被 client 妥善的关闭了" << std::endl;
        break;
      } else if (ec) {
        // 其他错误
        throw boost::system::system_error(ec);
      }

      boost::asio::write(socket, boost::asio::buffer(data, length));
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " <<  e.what() << std::endl;
  }
}
```
其中，`tcp` 即 `boost::asio::ip::tcp`；`BUF_SIZE` 定义为 `enum { BUF_SIZE = 1024 };`。这些都是细节，后面的例子不再赘述。
```cpp
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  unsigned short port = std::atoi(argv[1]);

  boost::asio::io_context ioc;

  // 创建 Acceptor 侦听新的连接
  tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), port));

  try {
    // 一次处理一个连接
    while (true) {
      Session(acceptor.accept());
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception: " <<  e.what() << std::endl;
  }

  return 0;
}
```
启动时，通过命令行参数指定端口号，比如：
```
$ echo_server_sync 8080
```
因为 Client 部分还未实现，先用 `netcat` 测试一下：
```
$ nc localhost 8080
hello
hello
```

以下几点需要注意：
- `tcp::acceptor` 也是一种 I/O 对象，用来接收 TCP 连接，连接端口由 `tcp::endpoint` 指定。
- 数据 buffer 以 `boost::array<char, BUF_SIZE>` 表示，也可以用 `char data[BUF_SIZE]`，或 `std::vector<char> data(BUF_SIZE)`。事实上，用 `std::vector` 是最推荐的，因为它不但可以动态调整大小，还支持 [Buffer Debugging](http://blog.think-async.com/2006/11/buffer-debugging.html)。
- 同步方式下，没有调用 `io_context.run`，因为 `accept`、`read_some` 和 `write` 都是阻塞的。这也意味着一次只能处理一个 Client 连接，但是可以连续 echo，除非 Client 断开连接。
- 写回数据时，没有直接调用 `socket.write_some`，因为它不能保证一次写完所有数据，但是 `boost::asio::write` 可以。我觉得这是 Asio 接口设计不周，应该提供 `socket.write`。
- `acceptor.accept` 返回一个新的 socket 对象，利用 move 语义，直接就转移给了 `Session` 的参数，期间并没有拷贝开销。

### 异步方式

异步方式下，困难在于对象的生命周期，可以用 `shared_ptr` 解决。

为了同时处理多个 Client 连接，需要保留每个连接的 socket 对象，于是抽象出一个表示连接会话的类，叫 `Session`：
```cpp
class Session : public std::enable_shared_from_this<Session> {
public:
  Session(tcp::socket socket) : socket_(std::move(socket)) {
  }

  void Start() {
    DoRead();
  }

  void DoRead() {
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            DoWrite(length);
          }
        });
  }

  void DoWrite(std::size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(buffer_, length),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            DoRead();
          }
        });
  }

private:
  tcp::socket socket_;
  std::array<char, BUF_SIZE> buffer_;
};
```
就代码风格来说，有以下几点需要注意：
- 优先使用 STL，比如 `std::enable_shared_from_this`，`std::bind`，`std::array`，等等。
- 定义 handler 时，尽量使用匿名函数（lambda 表达式）。
- 以 C++ `std::size_t` 替 C `size_t`。
刚开始，你可能会不习惯，我也是这样，过了好久才慢慢拥抱 C++11 乃至 C++14。

`Session` 有两个成员变量，`socket_` 与 Client 通信，`buffer_` 是接收 Client 数据的缓存。只要 `Session` 对象在，socket 就在，连接就不断。Socket 对象是构造时传进来的，而且是通过 move 语义转移进来的。

虽然还没看到 `Session` 对象是如何创建的，但可以肯定的是，它必须用 `std::shared_ptr` 进行封装，这样才能保证异步模式下对象的生命周期。

此外，在 `Session::DoRead` 和 `Session::DoWrite` 中，因为读写都是异步的，同样为了防止当前 `Session` 不被销毁（因为超出作用域），所以要增加它的引用计数，即 `auto self(shared_from_this());` 这一句的作用。

至于读写的逻辑，基本上就是把 `read_some` 换成 `async_read_some`，把 `write` 换成 `async_write`，然后以匿名函数作为 completion handler。

接收 Client 连接的代码，提取出来，抽象成一个类 `Server`：
```cpp
class Server {
public:
  Server(boost::asio::io_context& ioc, std::uint16_t port)
      : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
    DoAccept();
  }

private:
  void DoAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec) {
            std::make_shared<Session>(std::move(socket))->Start();
          }
          DoAccept();
        });
  }

private:
  tcp::acceptor acceptor_;
};
```

同样，`async_accept` 替换了 `accept`。`async_accept` 不再阻塞，`DoAccept` 即刻就会返回。
为了保证 `Session` 对象继续存在，使用 `std::shared_ptr` 代替普通的栈对象，同时把新接收的 socket 对象转移过去。

最后是 `main()`：

```cpp
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  std::uint16_t port = std::atoi(argv[1]);

  boost::asio::io_context ioc;
  Server server(ioc, port);

  ioc.run();
  return 0;
}
```

## Echo Client

虽然用 `netcat` 测试 Echo Server 非常方便，但是自己动手写一个 Echo Client 仍然十分必要。
还是先考虑同步方式。

### 同步方式

首先通过 `host` 和 `port` 解析出 endpoints（对，是复数！）：
```cpp
tcp::resolver resolver(ioc);
auto endpoints = resolver.resolve(tcp::v4(), host, port);
```
`resolve` 返回的 endpoints 类型为  `tcp::resolver::results_type`，代之以 `auto` 可以简化代码。类型推导应适当使用，至于连 `int` 都用 `auto` 就没有必要了。
`host` 和 `port` 通过命令行参数指定，比如 `localhost` 和 `8080`。

接着创建 socket，建立连接：
```cpp
tcp::socket socket(ioc);
boost::asio::connect(socket, endpoints);
```

这里没有直接调用 `socket.connect`，因为 `endpoints` 可能会有多个，`boost::asio::connect` 会挨个尝试，逐一调用 `socket.connect` 直到连接成功。

其实这样说不太严谨，根据我的测试，`resolve` 在没有指定 protocol 时，确实会返回多个 endpoints，一个是 IPv6，一个是 IPv4。但是我们已经指定了 protocol 为 `tcp::v4()`：
```cpp
resolver.resolve(tcp::v4(), host, port)
```
所以，应该只有一个 endpoint。

接下来，从标准输入（`std::cin`）读一行数据，然后通过 `boost::asio::write` 发送给 Server：
```cpp
    char request[BUF_SIZE];
    std::size_t request_length = 0;
    do {
      std::cout << "Enter message: ";
      std::cin.getline(request, BUF_SIZE);
      request_length = std::strlen(request);
    } while (request_length == 0);

    boost::asio::write(socket, boost::asio::buffer(request, request_length));
```

`do...while` 是为了防止用户直接 Enter 导致输入为空。`boost::asio::write` 是阻塞调用，发送完才返回。

从 Server 同步接收数据有两种方式：
- 使用 `boost::asio::read`（对应于 `boost::asio::write`）；
- 使用 `socket.read_some`。

两者的差别是，`boost::asio::read` 读到指定长度时，就会返回，你需要知道你想读多少；而 `socket.read_some` 一旦读到一些数据就会返回，所以必须放在循环里，然后手动判断是否已经读到想要的长度，否则无法退出循环。

下面分别是两种实现的代码。

使用 `boost::asio::read`：
```cpp
    char reply[BUF_SIZE];
    std::size_t reply_length = boost::asio::read(
        socket,
        boost::asio::buffer(reply, request_length));

    std::cout.write(reply, reply_length);
```

使用 `socket.read_some`：
```cpp
    std::size_t total_reply_length = 0;
    while (true) {
      std::array<char, BUF_SIZE> reply;
      std::size_t reply_length = socket.read_some(boost::asio::buffer(reply));

      std::cout.write(reply.data(), reply_length);

      total_reply_length += reply_length;
      if (total_reply_length >= request_length) {
        break;
      }
    }
```
不难看出，`socket.read_some` 用起来更为复杂。
Echo 程序的特殊之处就是，你可以假定 Server 会原封不动的把请求发回来，所以你知道 Client 要读多少。
但是很多时候，我们不知道要读多少数据。
所以，`socket.read_some` 反倒更为实用。

此外，在这个例子中，我们没有为各函数指定输出参数 `boost::system::error_code`，而是使用了异常，把整个代码块放在 `try...catch` 中。
```cpp
try {
  // ...
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
}
```

Asio 的 API 基本都通过重载（overload），提供了 `error_code` 和 `exception` 两种错误处理方式。使用异常更易于错误处理，也可以简化代码，但是 `try...catch` 该包含多少代码，并不是那么明显，新手很容易误用，什么都往 `try...catch` 里放。

**一般来说，异步方式下，使用 `error_code` 更方便一些。所以 complete handler 的参数都有 `error_code`。**

### 异步方式

就 Client 来说，异步也许并非必要，除非想同时连接多个 Server。

异步读写前面已经涉及，我们就先看 `async_resolve` 和 `async_connect`。

首先，抽取出一个类 `Client`：
```cpp
class Client {
public:
  Client(boost::asio::io_context& ioc,
         const std::string& host, const std::string& port)
      : socket_(ioc), resolver_(ioc) {
  }

private:
  tcp::socket socket_;
  tcp::resolver resolver_;

  char cin_buf_[BUF_SIZE];
  std::array<char, BUF_SIZE> buf_;
};
```

`resolver_` 是为了 `async_resolve`，作为成员变量，生命周期便得到了保证，不会因为函数结束而失效。

下面来看 `async_resolve` 实现（代码在构造函数中）：
```cpp
Client(...) {
  resolver_.async_resolve(tcp::v4(), host, port,
                          std::bind(&Client::OnResolve, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}
```

`async_resolve` 的 handler：
```cpp
void OnResolve(boost::system::error_code ec,
               tcp::resolver::results_type endpoints) {
  if (ec) {
    std::cerr << "Resolve: " << ec.message() << std::endl;
  } else {
    boost::asio::async_connect(socket_, endpoints,
                               std::bind(&Client::OnConnect, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
  }
}
```
`async_connect` 的 handler：
```cpp
void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint) {
  if (ec) {
    std::cout << "Connect failed: " << ec.message() << std::endl;
    socket_.close();
  } else {
    DoWrite();
  }
}
```
连接成功后，调用 `DoWrite`，从标准输入读取一行数据，然后异步发送给 Server。
下面是异步读写相关的函数，一并给出：
```cpp
void DoWrite() {
  std::size_t len = 0;
  do {
    std::cout << "Enter message: ";
    std::cin.getline(cin_buf_, BUF_SIZE);
    len = strlen(cin_buf_);
  } while (len == 0);

  boost::asio::async_write(socket_,
                           boost::asio::buffer(cin_buf_, len),
                           std::bind(&Client::OnWrite, this,
                                     std::placeholders::_1));
}

void OnWrite(boost::system::error_code ec) {
  if (!ec) {
    std::cout << "Reply is: ";

    socket_.async_read_some(boost::asio::buffer(buf_),
                            std::bind(&Client::OnRead, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  }
}

void OnRead(boost::system::error_code ec, std::size_t length) {
  if (!ec) {
    std::cout.write(buf_.data(), length);
    std::cout << std::endl;
    // 如果想继续下一轮，可以在这里调用 DoWrite()。
  }
}
```
异步读写在异步 Server 那一节已经介绍过，这里就不再赘述了。

最后是 `main()`：
```cpp
int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  const char* host = argv[1];
  const char* port = argv[2];

  boost::asio::io_context ioc;
  Client client(ioc, host, port);

  ioc.run();
  return 0;
}
```

至此，异步方式的 Echo Client 就算实现了。

为了避免文章太长，Asio 的介绍暂时先告一段落。若有补遗，会另行记录。

完整及更加丰富的示例代码，请移步 [GitHub](https://github.com/sprinfall/boost-asio-study)。
