#include <QDebug>
#include "crypt.h"
#ifndef Q_OS_OSX
extern "C" {
#include "external/twofish/AES.H"
}
#else
#include "external/twofish/AES.H"
#endif

#define KEY_BITS 256
//#define DEBUG_CRYPT
#ifdef DEBUG_CRYPT
#include <QDebug>
#define D(a) qDebug() <<"[Crypt]" <<a
#else
#define D(a)
#endif

Crypt::Crypt(QObject *parent) : QObject(parent)
{
    D("initialising crypto gubbins."<<sizeof(DWORD));

    this->ki = (void*)new keyInstance();
    this->ci = (void*)new cipherInstance();

    if (!makeKey((keyInstance*)this->ki,DIR_ENCRYPT,KEY_BITS,0)) {
        D("Initialising key failed.");
    }
    if (!cipherInit((cipherInstance*)this->ci,MODE_ECB,NULL)) {
        D("Initialising cypher failed.");
    }
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
    D("rekeying... ");
    reKey((keyInstance*)this->ki);
}

QByteArray Crypt::encrypt(QByteArray input)
{
    D("In encrypt with: "<<input.left(10).toPercentEncoding());
    QByteArray plainText(input);

    int blockSizeBytes = BLOCK_SIZE/8;
    D("input length: "<< input.length() << "BS: "<<blockSizeBytes);
    if ((input.length()%blockSizeBytes)!=0) {
        int padTo = ((input.length()/blockSizeBytes)+1)*blockSizeBytes;
        D("Padding..." << padTo);
        plainText.resize(padTo);
        for(int x=input.size();x<padTo;x++) {
            plainText.data()[x]=0;
        }
    }
    QByteArray output(plainText.size(),0);
    D("PT:"<<plainText.left(10).toPercentEncoding());
    blockEncrypt((cipherInstance*)this->ci,(keyInstance*)this->ki, (BYTE*)plainText.data(),plainText.length()*8,(BYTE*)output.data());
    D("CT:"<<output.left(10).toPercentEncoding());
    return output;
}

QByteArray Crypt::decrypt(QByteArray input)
{
    D("In decrypt with: "<<input.left(10).toPercentEncoding());
    int blockSizeBytes = BLOCK_SIZE/8;
    if ((input.length()%blockSizeBytes)!=0) {
        int padTo = ((input.length()/blockSizeBytes)+1)*blockSizeBytes;
        D("Padding..." << padTo);
        input.resize(padTo);
        for(int x=input.size();x<padTo;x++) {
            input.data()[x]=0;
        }
//        qDebug() << "Input buffer size mismatch." << input.length() <<blockSizeBytes << (input.length()%blockSizeBytes);
//        return QByteArray();
    }
    QByteArray output(input.length(),0);
    D("CT:"<<input.left(10).toPercentEncoding());
    blockDecrypt((cipherInstance*)this->ci,(keyInstance*)this->ki,(BYTE*)input.data(),input.length()*8,(BYTE*)output.data());
    D("PT:"<<output.left(10).toPercentEncoding());
    return output;
}

