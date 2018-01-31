#ifndef CRYPT_H
#define CRYPT_H

#include <QObject>
#include <QByteArray>
extern "C" {
#include "external/twofish/AES.H"
}

class Crypt : public QObject
{
    Q_OBJECT
public:
    explicit Crypt(QObject *parent = 0);

    void setKey(QByteArray key);
    QByteArray encrypt(QByteArray input);
    QByteArray decrypt(QByteArray input);

signals:

public slots:

private:
    keyInstance    ki;			/* key information, including tables */
    cipherInstance ci;			/* keeps mode (ECB, CBC) and IV */
};

#endif // CRYPT_H
