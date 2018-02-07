#ifndef CUTEBOARDCONNECTION_H
#define CUTEBOARDCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QUuid>
#include <QTimer>
#include "userstate.h"

class CuteboardConnection : public QObject
{
    Q_OBJECT
public:
    explicit CuteboardConnection(QTcpSocket *socket, QObject *parent = nullptr);

    void close();
signals:
    void clipboardData(CuteboardConnection *source,QString data);
    void connectionClosed(CuteboardConnection *connection);

public slots:

private:
    enum ConnectionState {
        Error,New,PreAuth,AuthChallenge,Connected
    };

    QTcpSocket *s;
    QTimer watchdog;
    ConnectionState state;
    QString user;
    QString client;
    QUuid connectionId;
    QUuid challenge;
    QByteArray readBuffer;
    static QMap<QString,UserState*> userMap;

    void setState(ConnectionState newState);
    void setError(QString message);
    void write(QString message);
    void handleLogin();
    void handleChallengeResponse(QString response);
    void handleClipboardCommand(QString data);
    QPair<QString,QString> readLine();
private slots:
    void handleWatchdogTimeout();
    void handleReadyToRead();
    void handleClipboardData(CuteboardConnection *source,QString data);
};

#endif // CUTEBOARDCONNECTION_H
