// This example demostrates how to use ASIO in a GUI application.

#include <functional>
#include <iostream>
#include <thread>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio/io_context.hpp"
#include "boost/asio/executor_work_guard.hpp"

#include <QApplication>
#include <QDebug>

#include "my_window.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  int code = 0;

  // Exception handling is for Asio.
  try {
    // Run the io_context off in its own thread so that it operates completely
    // asynchronously with respect to the rest of the program.

    boost::asio::io_context io_context;

    // Make sure the io_context won't end without work to do.
    // The variable |work| is not used but necessary for the guard to work.
    auto work = boost::asio::make_work_guard(io_context);

    // You can also use bind instead of lambda as below:
    //   std::bind(&boost::asio::io_context::run, &io_context)
    std::thread asio_thread([&io_context]() { io_context.run(); });

    // Pass a reference of io_context to MyWindow so that async operations
    // could be created from inside.
    MyWindow my_window(io_context);
    my_window.show();

    // The GUI blocks here.
    code = app.exec();

    // The GUI ends, stop the Asio thread.
    // Pending async operations will be canceled.
    io_context.stop();
    // Wait for the thread to exit.
    asio_thread.join();

  } catch (const std::exception& e) {
    qDebug() << "Exception:" << e.what();
    code = 1;
  }

  return code;
}