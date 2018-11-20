#include "my_window.h"

#include <functional>
#include <memory>
#include <thread>

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "daytime_client.h"

static const std::string kHost = "time.nist.gov";

MyWindow::MyWindow(boost::asio::io_context& io_context)
    : io_context_(io_context) {
  setWindowTitle(tr("Qt Asio Example"));

  QWidget* central_widget = new QWidget();
  setCentralWidget(central_widget);

  button_ = new QPushButton(tr("Get Daytime"));
  button_->setToolTip(QStringLiteral("Use Asio to get current daytime"));

  QVBoxLayout* vlayout = new QVBoxLayout();

  vlayout->addWidget(button_, 0, Qt::AlignCenter);

  central_widget->setLayout(vlayout);

  resize(400, 300);

  QObject::connect(button_, &QPushButton::clicked,
                   this, &MyWindow::GetDaytime);
}

void MyWindow::GetDaytime() {
  // Use std::shared_ptr to ensure the life of DaytimeClient.
  auto client = std::make_shared<DaytimeClient>(io_context_, kHost);

  // Don't execute Start() from the GUI thread because it consists of
  // async operations which should be run in the same thread as io_context.
  // Use post() instead.
  // Note that the lambda function posted doesn't rely on MyWindow at all.
  // The |client| variable is copied so the reference count is increased.
  boost::asio::post(io_context_, [client]() {
    client->Start();
  });
}
