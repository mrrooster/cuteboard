#ifndef CUTEBOARDSERVER_H
#define CUTEBOARDSERVER_H

#include <QObject>
#include <QTcpServer>
#include "cuteboardconnection.h"

class CuteboardServer : public QObject
{
    Q_OBJECT
public:
    explicit CuteboardServer(QObject *parent = nullptr);

    void setListenAddress(QString address);
    void setListenPort(quint16 port);
signals:

public slots:
    void commence();

private:
    QTcpServer *server;
    QVector<CuteboardConnection*> connections;
    QString hostAddress;
    quint16 port;

private slots:
    void handleIncommingConnection();
    void handleConnectionClosed(CuteboardConnection* connection);
};

#endif // CUTEBOARDSERVER_H
