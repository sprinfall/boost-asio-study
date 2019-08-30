# Asio Example Notes

## timer2_async

Use `const reference` for error code:

```cpp
timer.async_wait([](const boost::system::error_code& ec) {
  std::cout << "Hello, World!" << std::endl;
});
```

## timer3_lambda

Use lambda expression:

```cpp
timer.async_wait([](boost::system::error_code ec) {
  std::cout << "Hello, World!" << std::endl;
});
```

## timer4_multi

Print thread ID to explain the handlers are executed in the same thread as `io_context.run`.

```cpp
std::cout << std::this_thread::get_id() << std::endl;
```

## timer5_threaded

The handlers are guaranteed to be called from the same thread as `io_context.run`.

Explain why `g_io_mutex` is needed by trying to comment the lock.

## timer6_args

Explain why it's difficult to use lambda expression.

```cpp
timer.async_wait([&timer, &count](boost::system::error_code ec) {
  if (count < 3) {
    std::cout << count++ << std::endl;

    timer.expires_after(std::chrono::seconds(1));

    timer.async_wait(???);
  }
});
```

Introduce how to know the signature of a handler. E.g.,

```cpp
  template <typename WaitHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WaitHandler,
      void (boost::system::error_code))
  async_wait(BOOST_ASIO_MOVE_ARG(WaitHandler) handler)
  {
    return async_initiate<WaitHandler, void (boost::system::error_code)>(
        initiate_async_wait(), handler, this);
  }

```

About `std::bind`:

```cpp
void f(int a, int b, int c) {
  std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
}

auto fx = std::bind(&f, 1, std::placeholders::_1, std::placeholders::_2);

fx(2, 3);
```

## timer7_memfunc

Explain the usage of `std::bind` for member functions.

```cpp
timer_.async_wait(std::bind(&Printer::Print, this, std::placeholders::_1));
```

Explain why async waiting `Print` from inside `Print` is not a recursive call. Create the following logger at the beginning of `Print`.

```cpp
struct Logger {
  Logger() {
    std::cout << "enter." << std::endl;
  }

  ~Logger() {
    std::cout << "leave." << std::endl;
  }
};
```

## echo_client_sync

Introduce `boost::asio::buffer`: `char[]`, `std::array`, `std::vector`.

## echo_client_async

Don't exit after read the reply. Go to next round instead.

## Others

Avoid including `boost/asio.hpp` (in your header).
