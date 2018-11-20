// This example demostrates how to use ASIO in a GUI application.

#include <functional>
#include <iostream>
#include <thread>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"

#include <QApplication>

#include "my_window.h"

int main(int argc, char* argv[]) {
  try {  // For Asio

    QApplication app(argc, argv);

    // Run the io_context off in its own thread so that it operates completely
    // asynchronously with respect to the rest of the program.

    boost::asio::io_context io_context;

    auto work = boost::asio::make_work_guard(io_context);

    std::thread asio_thread([&io_context]() { io_context.run(); });
    // Use bind instead of lambda:
    //   std::bind(&boost::asio::io_context::run, &io_context)

    // Pass a reference of io_context to MyWindow so that async operations
    // could be created from inside.
    MyWindow my_window(io_context);
    my_window.show();

    int code = app.exec();

    // Stop the Asio thread.
    io_context.stop();
    asio_thread.join();

    return code;

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}