#include <QDebug>
#include "crypt.h"
extern "C" {
#include "external/twofish/AES.H"
}

#define KEY_BITS 256

Crypt::Crypt(QObject *parent) : QObject(parent)
{
    qDebug() << "initialising crypto gubbins.";

    this->ki = (void*)new keyInstance();
    this->ci = (void*)new cipherInstance();

    if (!makeKey((keyInstance*)this->ki,DIR_ENCRYPT,KEY_BITS,0)) {
        qDebug() << "Initialising crypto gubbins failed.";
    }
    cipherInit((cipherInstance*)this->ci,MODE_ECB,NULL);
}

void Crypt::setKey(QByteArray key)
{
    int keyBytes = KEY_BITS/8;
    char *data = key.data();
    char *buff = (char*)((keyInstance*)this->ki)->key32;
    int x;
    for(x=0;x<keyBytes&&x<key.size();x++) {
        buff[x]=data[x];
    }
    for(;x<keyBytes;x++) {
        buff[x]=0;
    }
//    qDebug() << "rekeying...";
    reKey((keyInstance*)this->ki);
}

QByteArray Crypt::encrypt(QByteArray input)
{
    QByteArray plainText(input);

    int blockSizeBytes = BLOCK_SIZE/8;
//    qDebug() << "input length: "<< input.length() << "BS: "<<blockSizeBytes;
    if ((input.length()%blockSizeBytes)!=0) {
        int padTo = ((input.length()/blockSizeBytes)+1)*blockSizeBytes;
//        qDebug() << "Padding..." << padTo;
        plainText.resize(padTo);
        for(int x=input.size();x<padTo;x++) {
            plainText.data()[x]=0;
        }
    }
    QByteArray output(plainText.size(),0);
    blockEncrypt((cipherInstance*)this->ci,(keyInstance*)this->ki, (BYTE*)plainText.data(),plainText.length()*8,(BYTE*)output.data());
    return output;
}

QByteArray Crypt::decrypt(QByteArray input)
{
    int blockSizeBytes = BLOCK_SIZE/8;
    if ((input.length()%blockSizeBytes)!=0) {
        qDebug() << "Input buffer size mismatch." << input.length() <<blockSizeBytes << (input.length()%blockSizeBytes);
        return QByteArray();
    }
    QByteArray output(input.length(),0);
    blockDecrypt((cipherInstance*)this->ci,(keyInstance*)this->ki,(BYTE*)input.data(),input.length()*8,(BYTE*)output.data());
    return output;
}

