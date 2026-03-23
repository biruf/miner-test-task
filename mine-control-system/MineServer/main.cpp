#include <QCoreApplication>
#include <QDebug>
#include "mineserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "Starting Mine Server...";

    MineServer server;

    if (!server.start(12345)) {
        qCritical() << "Failed to start server on port 12345";
        return 1;
    }

    qDebug() << "Mine server started on port 12345";
    qDebug() << "Waiting for clients...";

    return app.exec();
}
