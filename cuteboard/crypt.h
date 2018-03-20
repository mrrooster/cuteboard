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
#ifndef CRYPT_H
#define CRYPT_H

#include <QObject>
#include <QByteArray>

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
    void *ki;			/* key information, including tables */
    void *ci;			/* keeps mode (ECB, CBC) and IV */
};

#endif // CRYPT_H
