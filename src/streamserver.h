#ifndef STREAMSERVER_H
#define STREAMSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <QPixmap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QWidget>
#include <QtDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>

#include <QSslError>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

template <class T>
class Propagation {
public:
    Propagation(const QString &message, const QWidget &w, double pixelRatio) :
        message_(message),
        widget_(w),
        x_(0),
        y_(0),
		z_(0),
        adjustedX_(0),
        adjustedY_(0),
        pixelRatio_(pixelRatio)
    {
        QStringList chunks(message.split(" "));
        bool isok = false;
        x_ = chunks.at(1).toLong(&isok);
        if (!isok)
            qDebug() << "Extracting x failed";
        y_ = chunks.at(2).toLong(&isok);
        if (!isok)
            qDebug() << "Extracting y failed";

		if (chunks.length() > 3)
			z_ = chunks.at(3).toLong();
		else
			z_ = 0;
    }

    void exec() {

        QWidget *w = widget_.childAt(QPoint(x_, y_));
        if (!w) {
            qDebug() << "No widget found at " << x_ << " " << y_ << " .. returning";
            return;
        }

        adjustPixelRatio(w);

        QPoint offset(w->mapTo(&widget_, QPoint(x_, y_)));
        adjustedX_ = offset.x();
        adjustedY_ = offset.y();

		return static_cast<T*>(this)->exec(w, adjustedX_, adjustedY_, z_);
    }

    void adjustPixelRatio(const QWidget * const w)
    {
        QPoint offset(w->mapTo(&widget_, QPoint(x_, y_)));

        adjustedX_ -= (offset.x() / pixelRatio_);
        adjustedY_ -= (offset.y() / pixelRatio_);
        adjustedX_ *= pixelRatio_;
        adjustedY_ *= pixelRatio_;
    }

protected:
    const QWidget &widget_;
    const QString &message_;
    long x_;
    long y_;
	long z_;
    long adjustedX_;
    long adjustedY_;
    double pixelRatio_;
};

class MouseMove : public Propagation<MouseMove> {
public:
	void exec(QWidget *w, long x, long y, long z) {
        QApplication::sendEvent(w, new QMouseEvent(QEvent::MouseMove, QPoint(x, y), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    };
};
class MouseDown : public Propagation<MouseDown> {
public:
	void exec(QWidget *w, long x, long y, long z) {
        QApplication::sendEvent(w, new QMouseEvent(QEvent::MouseButtonPress, QPoint(x, y), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    };
};
class MouseUp : public Propagation<MouseUp> {
public:
	void exec(QWidget *w, long x, long y, long z) {
        QApplication::sendEvent(w, new QMouseEvent(QEvent::MouseMove, QPoint(x, y), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
        QApplication::sendEvent(w, new QMouseEvent(QEvent::MouseButtonRelease, QPoint(x, y), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    };
};
class Wheel : public Propagation<Wheel> {
public:
	void exec(QWidget *w, long x, long y, long z) {
		QApplication::sendEvent(w, new QWheelEvent(QPoint(x, y), z * 70, Qt::NoButton, Qt::NoModifier, Qt::Vertical));
	};
};




class StreamServer : public QObject
{
    Q_OBJECT
public:
    explicit StreamServer(quint16 port, bool debug, QWidget &widget, double pixelRatio = 1.0, QObject *parent = Q_NULLPTR);
    ~StreamServer();

signals:
    void closed();

private slots:
    void onNewConnection();
	void onSslErrors(const QList<QSslError> &errors);
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    bool m_debug;

	void sendImage(QWebSocket *client, const char *format, int quality = -1);

    QByteArray previousArray;
    QWidget &widget;
    double pixelRatio;
};

#endif //STREAMSERVER_H
