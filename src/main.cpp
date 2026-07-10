#include <QApplication>
#include <QIcon>

#include "app/ApplicationContext.h"
#include "app/AppSettings.h"
#include "app/AppVersion.h"
#include "ui/mainwindow/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    ApplicationContext& context = ApplicationContext::instance();
    context.setAppSettings(AppSettingsStore::load());

    MainWindow window;
    window.setWindowTitle(AppVersion::displayName());
    window.setWindowIcon(app.windowIcon());
    window.show();

    return app.exec();
}
