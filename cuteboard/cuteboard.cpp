#include "cuteboard.h"
#include <QApplication>
#include <QMimeData>
#include <QDebug>
#include <QPointer>

#define DEBUG_CUTEBOARD
#ifdef DEBUG_CUTEBOARD
#include <QDebug>
#define D(a) qDebug() <<"[Cuteboard]" <<a
#else
#define D(a)
#endif


Cuteboard::Cuteboard(QObject *parent) :
    QObject(parent),
    trayIcon(QIcon(":/trayicon.png")),
    historyMaxItems(40),
    ignoreChanges(false)
{
    this->clipboard = QApplication::clipboard();

    this->trayIcon.setContextMenu(&this->menu);

    connect(&this->client,&CuteboarddClient::hasRemoteClipboard,this,&Cuteboard::handleRemoteClipboard);
    // FIXME connect to server
    this->client.connect("tsunami.ohmyno.co.uk",19780,"me","mypassword");

    // Clipboard
    connect(this->clipboard,&QClipboard::dataChanged,this,&Cuteboard::handleClipboardContentsChanged);

    // Menu
    this->clipboardMenu.menuAction()->setText("Clipboard items");
    this->menu.addMenu(&this->clipboardMenu);
    this->menu.addSeparator();
    connect(this->menu.addAction("Quit"),&QAction::triggered,this,&Cuteboard::handleMenuQuit);

#ifdef Q_OS_MACOS
    connect(&this->checkForChangesTimer,&QTimer::timeout,this,&Cuteboard::checkForChanges);
    this->checkForChangesTimer.setSingleShot(false);
    this->checkForChangesTimer.start(2000);
#endif
}

void Cuteboard::start()
{
    saveClipboard();
    this->trayIcon.show();
}

void Cuteboard::saveClipboard()
{
    QMimeData *data = new QMimeData();
    const QMimeData *clipboardData = this->clipboard->mimeData();
#ifdef Q_OS_MACOS
    setCheckString();
#endif
    QStringList formats = clipboardData->formats();
    D("Copying data to clipboard");
    for(int idx=0;idx<formats.size();idx++) {
        QString format = formats.at(idx);
        data->setData(format,clipboardData->data(format));
    }
    D("Storing object in history.");
    this->history.append(data);

    this->clipboardMenu.clear(); // To ensure we don't have any stale pointers around.
    while(this->history.size()>this->historyMaxItems) {
        delete this->history.takeFirst();
    }

    D("History has"<<this->history.size()<<"entries.");
    setupClipboardMenu();
}

void Cuteboard::setupClipboardMenu()
{
    this->clipboardMenu.clear();
    for(int idx=0;idx<this->history.size();idx++) {
        QMimeData *data = this->history.at(idx);
        QString dataEntry = data->text().remove('\n').left(30);

        QAction *menuAct = this->clipboardMenu.addAction(dataEntry);
        menuAct->setData(QVariant::fromValue(QPointer<QMimeData>(data)));
        connect(menuAct,&QAction::triggered,this,&Cuteboard::handleMenuSelected);
    }
}

void Cuteboard::handleMenuQuit()
{
    QApplication::quit();
}

void Cuteboard::handleMenuSelected()
{
    QAction *act = qobject_cast<QAction*>(sender());
    QPointer<QMimeData> data = act->data().value<QPointer<QMimeData>>();

    if (data) {
        this->trayIcon.showMessage("Cuteboard",data->text());
    }
}

void Cuteboard::handleClipboardContentsChanged()
{
    if (this->ignoreChanges) {
        return;
    }
    saveClipboard();
    this->client.postClipboard(this->clipboard->mimeData());
}

void Cuteboard::handleRemoteClipboard()
{
    D("In handle remote clipboard.");
    this->ignoreChanges = true;
    this->clipboard->setMimeData(this->client.getNextRemoteClipboard());
#ifdef Q_OS_MACOS
    setCheckString();
#endif
    QApplication::processEvents();
    this->ignoreChanges = false;
    D("Leaving handle remote clipboard.");
}
#ifdef Q_OS_MACOS

void Cuteboard::checkForChanges()
{
    if (this->checkString != QString(this->client.encodeMimeData(this->clipboard->mimeData()))) {
        D("Contents changed!");
        handleClipboardContentsChanged();
    }
}

void Cuteboard::setCheckString()
{
    this->checkString = QString(this->client.encodeMimeData(this->clipboard->mimeData()));
}
#endif
