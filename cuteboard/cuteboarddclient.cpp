#include "cuteboarddclient.h"
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QClipboard>
#include <QImage>
#include <QApplication>

#define DEBUG_CLIENT
#ifdef DEBUG_CLIENT
#include <QDebug>
#define D(a) qDebug() <<"[CuteboarddClient]" <<a
#else
#define D(a)
#endif

CuteboarddClient::CuteboarddClient(QObject *parent) : QObject(parent),s(nullptr)
{
    QObject::connect(&this->pingTimer,&QTimer::timeout,this,&CuteboarddClient::handlePingTimeout);
    this->pingTimer.setSingleShot(false);
    this->pingTimer.start(60000); // Ping the server every 60 seconds
}

void CuteboarddClient::connect(QString host, quint16 port,QString user,QString password)
{
    if (!this->s) {
        QTcpSocket *s;
        this->s = s = new QTcpSocket(this);

        QObject::connect(this->s,&QAbstractSocket::connected,this,&CuteboarddClient::handleConnected);
        QObject::connect(this->s,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,&CuteboarddClient::handleError);
        QObject::connect(this->s,&QAbstractSocket::readyRead,this,&CuteboarddClient::handleReadyRead);

        this->user = user;
        this->password = password;

        s->connectToHost(host,port);
    }
}

void CuteboarddClient::postClipboard(const QMimeData *data)
{
    if (this->connected) {
        D("Preparing clipboard for transmission...");
        QByteArray dataToSend;

        dataToSend.append("CUTE");
        dataToSend.append(encodeMimeData(data));

        D("Sending clipboard.");
        QByteArray encryptedData = this->crypto.encrypt(dataToSend);
        D("Encrpypted data size: "<<encryptedData.size());

        write(QString("Clipboard: %1").arg(QString(encryptedData.toPercentEncoding())));
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

void CuteboarddClient::write(QString data)
{
    if(this->s) {
        this->s->write(QString("%1\r\n").arg(data).toUtf8());
    }
}

void CuteboarddClient::processRemoteClipboard(QString data)
{
    D("Encoded data size:"<<data.size());
    QByteArray encryptedData = QByteArray::fromPercentEncoding(data.toUtf8());
    D("Encrypted data size:"<<encryptedData.size());
    QByteArray decryptedData = this->crypto.decrypt(encryptedData);

    D("DATA:"<<decryptedData.left(20));

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
                    D("Data: "<<name<<"="<<value.left(10));
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
    D("Read:"<<readLine);
    return QPair<QString,QString>(readLine.left(idx),readLine.mid(idx+2));
}

void CuteboarddClient::handleConnected()
{
    write("CB 1.0");//Protocol version
    write(QString("User: %1").arg(this->user));// User
    //this->pingTimer.start();
}

void CuteboarddClient::handleError(QAbstractSocket::SocketError socketError)
{
    D("An error occured:"<<this->s->errorString());
    this->connected = false;
}

void CuteboarddClient::handlePingTimeout()
{
    D("Sending ping...");
    write("Ping: ");
}

void CuteboarddClient::handleReadyRead()
{
    D("In read: readbuffer size:"<<this->readBuffer.size());
    this->readBuffer.append(this->s->readAll());
    D("In read: readbuffer size:"<<this->readBuffer.size());

    if (this->readBuffer.contains("\r\n")) {
        QPair<QString,QString> line = readLine();
        QString command = line.first;
        QString value = line.second;

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
                this->connected = true;
                this->crypto.setKey(this->password.toUtf8());
                this->password="";
            } else {
                this->connected = false;
                this->error = value;
            }
        }
    }
}
