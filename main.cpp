#include <QApplication>
#include "FloatingButton.h"

int main(int argc, char *argv[])
{
    // High-DPI support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QApplication app(argc, argv);
    app.setApplicationName("MiniTasks");

    FloatingButton trigger;
    trigger.show();

    return app.exec();
}
