/**
  * (c) 2018 Ian Clark
  *
  * This file is part of cuteboard
  *
  * Cuteboard is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * Cuteboard is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * You should have received a copy of the GNU General Public License
  * along with Cuteboard.  If not, see <http://www.gnu.org/licenses/>.
  */
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

    void connect(QString host, QString user, QString password);
    void connect(QString host, quint16 port, QString user, QString password);
    void postClipboard(const QMimeData *data);
    QByteArray encodeMimeData(const QMimeData *data);
    QMimeData *getNextRemoteClipboard();
    void close();
signals:
    void hasRemoteClipboard();
    void disconnected();
    void connected();
public slots:

private:
    QString host;
    quint16 port;
    QString user;
    QString password;
    QAbstractSocket *s;
    QTimer pingTimer;
    bool amConnected;
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
