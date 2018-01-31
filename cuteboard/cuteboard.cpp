#include "cuteboard.h"
#include <QApplication>
#include <QMimeData>
#include <QDebug>
#include <QPointer>

Cuteboard::Cuteboard(QObject *parent) :
    QObject(parent),
    trayIcon(QIcon(":/trayicon.png")),
    historyMaxItems(40)
{
    this->clipboard = QApplication::clipboard();

    this->trayIcon.setContextMenu(&this->menu);

    // FIXME connect to server
    this->client.connect("localhost",19780);

    // Clipboard
    connect(this->clipboard,&QClipboard::dataChanged,this,&Cuteboard::handleClipboardContentsChanged);

    // Menu
    this->clipboardMenu.menuAction()->setText("Clipboard items");
    this->menu.addMenu(&this->clipboardMenu);
    this->menu.addSeparator();
    connect(this->menu.addAction("Quit"),&QAction::triggered,this,&Cuteboard::handleMenuQuit);
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

    QStringList formats = clipboardData->formats();
    qDebug()<<"Copying data to clipboard";
    for(int idx=0;idx<formats.size();idx++) {
        QString format = formats.at(idx);
        data->setData(format,clipboardData->data(format));
    }
    qDebug()<<"Storing object in history.";
    this->history.append(data);

    this->clipboardMenu.clear(); // To ensure we don't have any stale pointers around.
    while(this->history.size()>this->historyMaxItems) {
        delete this->history.takeFirst();
    }

    qDebug()<<"History has"<<this->history.size()<<"entries.";
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
    saveClipboard();
}
