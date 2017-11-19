#include "streamserver.h"

#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QFileInfo>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>

#include "MainWindow.h"

QT_USE_NAMESPACE

StreamServer::StreamServer(quint16 port, bool debug, QWidget &widget, double pixelRatio, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("RTVis1 Server"), QWebSocketServer::NonSecureMode, this)),
    m_clients(),
    m_debug(debug),
    widget(widget),
    pixelRatio(pixelRatio)
{
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "Streamserver listening on port" << port;
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &StreamServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &StreamServer::closed);
		connect(m_pWebSocketServer, &QWebSocketServer::sslErrors, this, &StreamServer::onSslErrors);
	}
}

StreamServer::~StreamServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void StreamServer::onSslErrors(const QList<QSslError> &errors)
{
    foreach (QSslError error, errors) {
		qDebug() << "SSL ERROR: " << error.errorString();
	}
}

void StreamServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &StreamServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &StreamServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &StreamServer::socketDisconnected);

    m_clients << pSocket;

	if (m_debug)
        qDebug() << "New client!" << pSocket->resourceName();
}

void StreamServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Message received:" << message;
    if (pClient) {

        if (message == "req_image") {
			sendImage(pClient, "JPG", 100);
        }
        else if (message.startsWith("req_move_mouse_xy")) {
            Propagation<MouseMove> prop(message, widget, pixelRatio);
            prop.exec();
			sendImage(pClient, "JPG", 50);
        }
        else if (message.startsWith("req_down_mouse_xy")) {
            Propagation<MouseDown> prop(message, widget, pixelRatio);
            prop.exec();
			sendImage(pClient, "JPG", 100);
        }
        else if (message.startsWith("req_up_mouse_xy")) {
			Propagation<MouseUp> prop(message, widget, pixelRatio);
            prop.exec();
			sendImage(pClient, "JPG", 100);
        }
		else if (message.startsWith("req_wheel")) {
			Propagation<Wheel> prop(message, widget, pixelRatio);
			prop.exec();
			sendImage(pClient, "JPG", 100);
		}
	}
}

void StreamServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void StreamServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void StreamServer::sendImage(QWebSocket *client, const char *format, int quality)
{
	GLWidget *canvas = dynamic_cast<MainWindow*> (&widget)->getGLWidget();

	QImage image = canvas->getImage();

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
	image.save(&buffer, format, quality);

    if (ba != previousArray || true) {
		client->sendBinaryMessage(ba);
    }

    previousArray = QByteArray(ba);
}
