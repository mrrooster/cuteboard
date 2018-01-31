#ifndef USERSTATE_H
#define USERSTATE_H

#include <QObject>
#include <QUuid>

class CuteboardConnection;

class UserState : public QObject
{
    Q_OBJECT
public:
    explicit UserState(QObject *parent = nullptr);

    QUuid challenge;
    QString response;
signals:
    void clipboardData(CuteboardConnection *source,QString data);

public slots:
};

#endif // USERSTATE_H
