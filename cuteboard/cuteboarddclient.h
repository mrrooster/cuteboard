#ifndef CUTEBOARDDCLIENT_H
#define CUTEBOARDDCLIENT_H

#include <QObject>
#include <QAbstractSocket>
#include <QTimer>
#include <QMimeData>
#include "crypt.h"

class CuteboarddClient : public QObject
{
    Q_OBJECT
public:
    explicit CuteboarddClient(QObject *parent = nullptr);

    void connect(QString host, quint16 port, QString user, QString password);
    void postClipboard(const QMimeData *data);
    QByteArray encodeMimeData(const QMimeData *data);
    QMimeData *getNextRemoteClipboard();
    void close();
signals:
    void hasRemoteClipboard();
    void disconnected();
public slots:

private:
    QString host;
    quint16 port;
    QString user;
    QString password;
    QAbstractSocket *s;
    QTimer pingTimer;
    bool connected;
    QString error;
    Crypt crypto;
    QByteArray readBuffer;
    QVector<QMimeData*> incomingClipboards;

    void write(QString data);
    void processRemoteClipboard(QString data);
    QPair<QString,QString> readLine();
private slots:
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError socketError);
    void handlePingTimeout();
    void handleReadyRead();
};

#endif // CUTEBOARDDCLIENT_H
