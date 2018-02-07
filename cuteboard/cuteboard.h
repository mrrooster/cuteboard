#ifndef CUTEBOARD_H
#define CUTEBOARD_H

#include <QObject>
#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
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
    QSystemTrayIcon trayIcon;
    QMenu menu;
    QMenu clipboardMenu;
    QClipboard *clipboard;
    QVector<QMimeData*> history;
    CuteboarddClient client;
    bool ignoreChanges;
    SettingsDialog settings;
#ifdef Q_OS_MACOS
    QTimer checkForChangesTimer;
    QString checkString;
    void setCheckString();
#endif

    void saveClipboard();
    void setupClipboardMenu();
    QMimeData *cloneMimeData(const QMimeData *src);

private slots:
    void handleSettingsChanged();
    void handleMenuQuit();
    void handleMenuSelected();
    void handleClipboardContentsChanged();
    void handleRemoteClipboard();
#ifdef Q_OS_MACOS
    void checkForChanges();
#endif
};

#endif // CUTEBOARD_H
