#ifndef CUTEBOARD_H
#define CUTEBOARD_H

#include <QObject>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
#include "cuteboarddclient.h"

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

    void saveClipboard();
    void setupClipboardMenu();

private slots:
    void handleMenuQuit();
    void handleMenuSelected();
    void handleClipboardContentsChanged();
};

#endif // CUTEBOARD_H
