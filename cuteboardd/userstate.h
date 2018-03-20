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
