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
#include "cuteboarddclient.h"
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QClipboard>
#include <QImage>
#include <QApplication>
#include <QSysInfo>

#define DEBUG_CLIENT
#ifdef DEBUG_CLIENT
#include <QDebug>
#define D(a) qDebug() <<"[CuteboarddClient]" <<a
#else
#define D(a)
#endif

#define CLIENT_INFO "Cuteboard/180207"
#ifdef Q_OS_MACOS
#define CLIENT_OS "darwin"
#elif defined Q_OS_WIN
#define CLIENT_OS (QString("%1 %2").arg(QSysInfo::prettyProductName()).arg(QSysInfo::currentCpuArchitecture()))
#else
#endif

CuteboarddClient::CuteboarddClient(QObject *parent) : QObject(parent),s(nullptr),amConnected(false)
{
    QObject::connect(&this->pingTimer,&QTimer::timeout,this,&CuteboarddClient::handlePingTimeout);
    this->pingTimer.setSingleShot(false);
}

void CuteboarddClient::connect(QString host, QString user, QString password)
{
    quint16 port = 19780;
    if (host.contains(':')) {
        int idx = host.indexOf(':');
        bool okay = true;
        port = host.mid(idx+1).toUShort(&okay);
        if (!okay) {
            port = 19780;
        }
        host = host.left(idx);
    }
    connect(host,port,user,password);
}

void CuteboarddClient::connect(QString host, quint16 port,QString user,QString password)
{
    if (this->amConnected) {
        return;
    }
    D("Connect to:"<<host<<port<<"as"<<user);
    if (!this->s) {
        QTcpSocket *s;
        this->s = s = new QTcpSocket(this);

        QObject::connect(this->s,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,&CuteboarddClient::handleError);
        QObject::connect(this->s,&QAbstractSocket::disconnected,this,&CuteboarddClient::handleDisconnected);
        QObject::connect(this->s,&QAbstractSocket::connected,this,&CuteboarddClient::handleConnected);
        QObject::connect(this->s,&QAbstractSocket::readyRead,this,&CuteboarddClient::handleReadyRead);
    }
    this->user = user;
    this->password = password;
    s->connectToHost(host,port);
}

void CuteboarddClient::postClipboard(const QMimeData *data)
{
    if (this->amConnected) {
        D("Preparing clipboard for transmission...");
        QByteArray dataToSend;

        dataToSend.append("CUTE");
        dataToSend.append(encodeMimeData(data));

        D("Sending clipboard.");
        QByteArray encryptedData = this->crypto.encrypt(dataToSend);
        D("Encrpypted data size: "<<encryptedData.size());

        write(QString("Clipboard: %1").arg(QString(encryptedData.toBase64())));
    }
}

QByteArray CuteboarddClient::encodeMimeData(const QMimeData *data)
{
    QStringList formats = data->formats();
    QByteArray dataToSend;
    QImage image = qvariant_cast<QImage>(data->imageData());

    if (image.isNull()) {
        for(int idx=0;idx<formats.size();idx++) {
            QString format = formats.at(idx);
            dataToSend.append(QString("%1=%2"
                                      "\n")
                              .arg(format)
                              .arg(QString(data->data(format).toBase64()))
                              );
        }
    } else {
        dataToSend.append(QString("application/x-qt-image="));
        dataToSend.append(QByteArray::fromRawData((const char*)(image.constBits()),image.byteCount()).toBase64());
        dataToSend.append("\r\n");
    }
    return dataToSend;
}

QMimeData *CuteboarddClient::getNextRemoteClipboard()
{
    D("In get remote cb, "<<this->incomingClipboards.size()<<"entries");
    return this->incomingClipboards.takeFirst();
}

void CuteboarddClient::close()
{
    if (this->amConnected) {
        D("Closing connection.");
        this->s->close();
        this->amConnected = false;
    }
}

void CuteboarddClient::write(QString data)
{
//    D("Write:"<<data);
    if(this->s) {
        this->s->write(QString("%1\r\n").arg(data).toUtf8());
    }
}

void CuteboarddClient::processRemoteClipboard(QString data)
{
//    D("Encoded data size:"<<data.size());
    QByteArray encryptedData = QByteArray::fromBase64(data.toUtf8());
//    D("Encrypted data size:"<<encryptedData.size());
    QByteArray decryptedData = this->crypto.decrypt(encryptedData);

//    D("DATA:"<<decryptedData.left(20));

    if (decryptedData.startsWith("CUTE")) {
        D("Sucesfully decrypted clipboard.");
        decryptedData = decryptedData.mid(4);
        QList<QByteArray> clipboardData = decryptedData.split('\n');
        QMimeData *newData = new QMimeData();
        for(int idx=0;idx<clipboardData.size();idx++) {
            QByteArray line = clipboardData.at(idx);
            if (line.indexOf('=')>-1) {
                int pos = line.indexOf('=');
                QString name = QString(line.left(pos));
                QByteArray value = QByteArray::fromBase64(line.mid(pos+1));
                if (name=="application/x-qt-image=") {
                    newData->setImageData(QVariant(value));
                } else {
//                    D("Data: "<<name<<"="<<value.left(10));
                    newData->setData(name,value);
                }
            }
        }
        this->incomingClipboards.append(newData);
        emit hasRemoteClipboard();
        //this->receivedClipboards.append(newData);
        //QApplication::clipboard()->setMimeData(newData);
    } else {
        D("Decryption failed");
    }
}

QPair<QString, QString> CuteboarddClient::readLine()
{
    int idx = this->readBuffer.indexOf("\r\n");
    QString readLine = QString(this->readBuffer.left(idx));
    this->readBuffer = this->readBuffer.mid(idx+2);

    idx = readLine.indexOf(":");
    if (idx<0) {
        return QPair<QString,QString>("",readLine);
    }
//    D("Read:"<<readLine);
    return QPair<QString,QString>(readLine.left(idx),readLine.mid(idx+2));
}

void CuteboarddClient::handleConnected()
{
    D("In handleConnected, sending procol strings...");
    write("CB 1.0");//Protocol version
    write(QString("User: %1").arg(this->user));// User
    write(QString("Client: %1 (%2)").arg(CLIENT_INFO).arg(CLIENT_OS)); // Client info
    //this->pingTimer.start();
}

void CuteboarddClient::handleDisconnected()
{
    D("Disconnected.");
    this->amConnected = false;
    if (this->s) {
        this->s->close();
    }
    emit disconnected();
}

void CuteboarddClient::handleError(QAbstractSocket::SocketError socketError)
{
    D("An error occured:"<<this->s->errorString());
    this->amConnected = false;
    if (this->s) {
        this->s->close();
    }
    emit disconnected();
}

void CuteboarddClient::handlePingTimeout()
{
    if (!this->amConnected) {
        this->pingTimer.stop();
    } else {
        D("Sending ping...");
        write("Ping: ");
    }
}

void CuteboarddClient::handleReadyRead()
{
//    D("In read: readbuffer size:"<<this->readBuffer.size());
    this->readBuffer.append(this->s->readAll());
//    D("In read: readbuffer size:"<<this->readBuffer.size());

    while (this->readBuffer.contains("\r\n")) {
        QPair<QString,QString> line = readLine();
        QString command = line.first;
        QString value = line.second;
//        D("Got command: "<<command);

        if (command=="Challenge") {
            D("Got challenge: "<<value);
            QByteArray dataToHash = QString("%1%2").arg(this->password).arg(value).toUtf8();

            write(QString("ChallengeResponse: %1").arg(
                      QString(QCryptographicHash::hash(dataToHash,QCryptographicHash::Sha256).toBase64())
                      ));
        } else if (command=="Clipboard") {
            D("Got remote clipboard:"<<value);
            processRemoteClipboard(value);
        } else if (command=="Pong") {
            D("Pong!");
        } else if (command=="Error") {
            if (value=="0/0 OK") {
                D("Connected.");
                this->amConnected = true;
                this->pingTimer.start(60000); // Ping the server every 60 seconds
                this->crypto.setKey(this->password.toUtf8());
                this->password="";
                emit connected();
            } else {
                this->amConnected = false;
                this->error = value;
            }
        }
    }
}
