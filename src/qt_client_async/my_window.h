#ifndef MY_WINDOW_H_
#define MY_WINDOW_H_

#include <QMainWindow>

#include "boost/asio/io_context.hpp"

QT_FORWARD_DECLARE_CLASS(QPushButton)

class MyWindow : public QMainWindow {
  Q_OBJECT

public:
  MyWindow(boost::asio::io_context& io_context);

private slots:
  void GetDaytime();

private:
  boost::asio::io_context& io_context_;

  QPushButton* button_;
};

#endif  // MY_WINDOW_H_
