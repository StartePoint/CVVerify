#include <QApplication>
#include <QIcon>

#include "ui/mainwindow/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    MainWindow window;
    window.setWindowIcon(app.windowIcon());
    window.show();

    return app.exec();
}
