#include "cuteboardserver.h"

#define DEBUG_SERVER
#ifdef DEBUG_SERVER
#include <QDebug>
#define D(a) qDebug() <<"[CuteboardServer]" <<a
#else
#define D(a)
#endif

CuteboardServer::CuteboardServer(QObject *parent) : QObject(parent)
{
    D("Cuteboard server starting.");
    this->server = new QTcpServer(this);

    connect(this->server,&QTcpServer::newConnection,this,&CuteboardServer::handleIncommingConnection);
}

void CuteboardServer::commence()
{
    this->server->listen(QHostAddress::Any,19780);
    D("Server listening on port:"<<this->server->serverPort()<<this->server->isListening());
}

void CuteboardServer::handleIncommingConnection()
{
    D("In handle incoming connection, with"<<this->connections.size()<<"active connections.");
    QTcpSocket *s = this->server->nextPendingConnection();
    if (s) {
        CuteboardConnection *newConnection = new CuteboardConnection(s,this);
        this->connections << newConnection;
        connect(newConnection,&CuteboardConnection::connectionClosed,this,&CuteboardServer::handleConnectionClosed);
    }
}

void CuteboardServer::handleConnectionClosed(CuteboardConnection *connection)
{
    if (this->connections.contains(connection)) {
        D("Removing closed connection.");
        this->connections.removeOne(connection);
        connection->deleteLater();
    }
}
