#ifndef CUTEBOARDDCLIENT_H
#define CUTEBOARDDCLIENT_H

#include <QObject>
#include <QAbstractSocket>
#include <QTimer>

class CuteboarddClient : public QObject
{
    Q_OBJECT
public:
    explicit CuteboarddClient(QObject *parent = nullptr);

    void connect(QString host, quint16 port);
signals:

public slots:

private:
    QString host;
    quint16 port;
    QAbstractSocket *s;
    QTimer pingTimer;

    void write(QString data);
private slots:
    void handleConnected();
    void handleError(QAbstractSocket::SocketError socketError);
    void handlePingTimeout();
    void handleReadyRead();
};

#endif // CUTEBOARDDCLIENT_H
