#include <QApplication>

#include "MainWindow.h"
#include "streamserver.h"

int main(int argc, char *argv[])
{

	QApplication app(argc, argv);
	MainWindow mainWindow;
	mainWindow.show();

    StreamServer *server = new StreamServer(1234, true, mainWindow, app.devicePixelRatio());
	QObject::connect(server, &StreamServer::closed, &app, &QCoreApplication::quit);
    QObject::connect(&app, &QGuiApplication::lastWindowClosed, server, &StreamServer::closed);

	return app.exec();

}
