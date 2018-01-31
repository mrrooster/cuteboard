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

signals:

public slots:
    void commence();

private:
    QTcpServer *server;
    QVector<CuteboardConnection*> connections;

private slots:
    void handleIncommingConnection();
    void handleConnectionClosed(CuteboardConnection* connection);
};

#endif // CUTEBOARDSERVER_H
