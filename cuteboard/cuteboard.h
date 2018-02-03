#ifndef CUTEBOARD_H
#define CUTEBOARD_H

#include <QObject>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
#include "cuteboarddclient.h"
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
    QSystemTrayIcon trayIcon;
    QMenu menu;
    QMenu clipboardMenu;
    QClipboard *clipboard;
    QVector<QMimeData*> history;
    CuteboarddClient client;
#ifdef Q_OS_MACOS
    QTimer checkForChangesTimer;
    QString checkString;
    void setCheckString();
#endif

    void saveClipboard();
    void setupClipboardMenu();

private slots:
    void handleMenuQuit();
    void handleMenuSelected();
    void handleClipboardContentsChanged();
#ifdef Q_OS_MACOS
    void checkForChanges();
#endif
};

#endif // CUTEBOARD_H
