#include "cuteboard.h"
#include <QApplication>
#include <QMimeData>
#include <QPixmap>
#include <QDebug>
#include <QPointer>
#include <QSettings>
#include <QLabel>
#include <QHBoxLayout>
#include "previewwidget.h"

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
    ignoreChanges(false),
    hoverWidget(0)
{
    this->clipboard = QApplication::clipboard();

    connect(&this->client,&CuteboarddClient::hasRemoteClipboard,this,&Cuteboard::handleRemoteClipboard);
    // FIXME connect to server
    //this->client.connect("tsunami.ohmyno.co.uk",19780,"me","mypassword");
    connect(&this->settings,&SettingsDialog::settingsChanged,this,&Cuteboard::handleSettingsChanged);

    // Hover time
    connect(&this->hoverTimer,&QTimer::timeout,this,&Cuteboard::handleHoverTimeout);
    this->hoverTimer.setSingleShot(true);

    // Clipboard
    connect(this->clipboard,&QClipboard::dataChanged,this,&Cuteboard::handleClipboardContentsChanged);

    // Menu
    this->clipboardMenu.menuAction()->setText("Clipboard items");
    this->menu.addMenu(&this->clipboardMenu);
    this->menu.addSeparator();
    connect(this->menu.addAction("Settings"),&QAction::triggered,&this->settings,&SettingsDialog::open);
    connect(this->menu.addAction("Quit"),&QAction::triggered,this,&Cuteboard::handleMenuQuit);

    //tray icon
    connect(&this->trayIcon,&QSystemTrayIcon::activated,this,&Cuteboard::handleTrayActivation);
    this->trayIcon.setContextMenu(&this->menu);

#ifdef Q_OS_MACOS
    connect(&this->checkForChangesTimer,&QTimer::timeout,this,&Cuteboard::checkForChanges);
    this->checkForChangesTimer.setSingleShot(false);
    this->checkForChangesTimer.start(2000);
#endif

    handleSettingsChanged();
}

void Cuteboard::start()
{
    saveClipboard();
    this->trayIcon.show();
}

void Cuteboard::saveClipboard()
{
    D("In saveClipboard");
    QMimeData *data = cloneMimeData(this->clipboard->mimeData());
#ifdef Q_OS_MACOS
    setCheckString();
#endif
    this->history.prepend(data);

    this->clipboardMenu.clear(); // To ensure we don't have any stale pointers around.
    while(this->history.size()>this->historyMaxItems) {
        delete this->history.takeLast();
    }

    D("History has"<<this->history.size()<<"entries.");
    setupClipboardMenu();
}

void Cuteboard::setupClipboardMenu()
{
    this->clipboardMenu.clear();
    for(int idx=0;idx<this->history.size();idx++) {
        QMimeData *data = this->history.at(idx);
        QAction *menuAct = this->clipboardMenu.addAction("");
        //Shall we draw a picture?
        if (data->hasImage()) {
            QImage image = qvariant_cast<QImage>(data->imageData());

            menuAct->setText(QString("[Image: %1x%2 %3bpp]")
                             .arg(image.width())
                             .arg(image.height())
                             .arg(image.depth())
                             );
            menuAct->setIcon(QIcon(QPixmap::fromImage(image.scaledToWidth(256))));
        } else {
            menuAct->setText(data->text().remove('\n').left(30));
        }

        menuAct->setData(QVariant::fromValue(QPointer<QMimeData>(data)));
        connect(menuAct,&QAction::triggered,this,&Cuteboard::handleMenuSelected);
        connect(menuAct,&QAction::hovered,this,&Cuteboard::handleMenuActionHover);
    }
}

QMimeData *Cuteboard::cloneMimeData(const QMimeData *src)
{
    D("Cloning mime data:"<<src<<", has:"<<src->formats().size()<<"items.");
    QMimeData *data = new QMimeData();
    QStringList formats = src->formats();
    for(int idx=0;idx<formats.size();idx++) {
        QString format = formats.at(idx);
        if (format=="application/x-qt-image") {
            data->setImageData(src->imageData());
        } else {
            data->setData(format,src->data(format));
        }
    }
    return data;
}

QWidget *Cuteboard::createHoverWidget()
{
    QPoint currentPos = QCursor::pos();
    QWidget *w = new PreviewWidget(this->hoverData);
    w->move(currentPos.x()-w->width()-10,currentPos.y()-w->height()-10);
    return w;
}

void Cuteboard::handleSettingsChanged()
{
    QSettings settings;
    this->client.close();
    if (!(settings.value("localOnly",QVariant(true)).toBool())) {
        QString host = settings.value("host",QVariant("")).toString();
        ushort port = 19780;
        D("Connecting to server: "<<host<<port);
        this->client.connect(host,
                             port,
                             settings.value("user",QVariant("")).toString(),
                             settings.value("pass",QVariant("")).toString()
                             );
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
        //this->trayIcon.showMessage("Cuteboard",data->text());
        this->clipboard->setMimeData(cloneMimeData(data));
    }
}

void Cuteboard::handleClipboardContentsChanged()
{
    saveClipboard();
    if (this->ignoreChanges) {
        return;
    }
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

void Cuteboard::handleMenuActionHover()
{
    QAction *act = qobject_cast<QAction*>(sender());
    QPointer<QMimeData> data = act->data().value<QPointer<QMimeData>>();

    if (data) {

            QPoint currentPos = QCursor::pos();
            if (currentPos!=this->lastCursorPos) {
                this->lastCursorPos = currentPos;
                if (this->hoverWidget) {
                    this->hoverWidget->close();
                    this->hoverWidget->deleteLater();
                    this->hoverWidget = 0;
                }
                this->hoverTimer.start(1000);
                this->hoverData = data;
            }
            //this->trayIcon.showMessage("Cuteboard",data->text());
    }

}

void Cuteboard::handleHoverTimeout()
{
    QPoint currentPos = QCursor::pos();
    if (currentPos==this->lastCursorPos) {
        if (!this->hoverWidget) {
            this->hoverWidget=createHoverWidget();
            this->hoverWidget->setVisible(true);
        }
        this->hoverTimer.start(200);
    } else {
        if (this->hoverWidget) {
            this->hoverWidget->close();
            this->hoverWidget->deleteLater();
            this->hoverWidget=nullptr;
        }
    }
}

void Cuteboard::handleTrayActivation(QSystemTrayIcon::ActivationReason reason)
{
    if (reason==QSystemTrayIcon::Trigger) {
        this->clipboardMenu.popup(QCursor::pos(),this->clipboardMenu.actions().last());
    }
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
