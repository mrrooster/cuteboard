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
#ifndef CUTEBOARD_H
#define CUTEBOARD_H

#include <QObject>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include "cuteboarddclient.h"
#include "settingsdialog.h"
#ifdef Q_OS_MACOS
#include <QTimer>
#endif

class Cuteboard : public QObject
{
    Q_OBJECT
public:
    explicit Cuteboard(QObject *parent = nullptr);

signals:

public slots:
    void start();

private:
    int historyMaxItems;
    int retryMSecs;
    int retryMaxMSecs;
    QSystemTrayIcon trayIcon;
    QMenu menu;
    QMenu clipboardMenu;
    QClipboard *clipboard;
    QVector<QMimeData*> history;
    CuteboarddClient client;
    bool ignoreChanges;
    SettingsDialog settings;
    QPoint lastCursorPos;
    QMimeData *hoverData;
    QTimer hoverTimer;
    QWidget *hoverWidget;
    QTimer reconnectTimer;
#ifdef Q_OS_MACOS
    QTimer checkForChangesTimer;
    QString checkString;
    void setCheckString();
#endif

    void saveClipboard();
    void setupClipboardMenu();
    QMimeData *cloneMimeData(const QMimeData *src);
    QWidget *createHoverWidget();
    void connectToServer();

private slots:
    void handleSettingsChanged();
    void handleMenuQuit();
    void handleMenuSelected();
    void handleClipboardContentsChanged();
    void handleRemoteClipboard();
    void handleMenuActionHover();
    void handleHoverTimeout();
    void handleTrayActivation(QSystemTrayIcon::ActivationReason reason);
    void handleReconnectTimerTimeout();
    void handleDisconnect();
    void handleConnected();
#ifdef Q_OS_MACOS
    void checkForChanges();
#endif
};

#endif // CUTEBOARD_H
