#include "previewwidget.h"
#include <QPointer>
#include <QPainter>
#include <QImage>
#include <QFontMetrics>

PreviewWidget::PreviewWidget(QMimeData *data,QWidget *parent) : QWidget(parent)
{
    this->data = QPointer<QMimeData>(data);
    setWindowFlag(Qt::Popup);
    setMaximumHeight(230);
    setMaximumWidth(230);
    setinitialSize();
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    if (this->data == nullptr) {
        return;
    }
    QPainter painter(this);
    painter.setPen(Qt::black);
    if (this->data->hasImage()) {
        QImage image = qvariant_cast<QImage>(this->data->imageData()).scaledToWidth(this->maximumWidth());
        painter.drawImage(0,0,image);
    } else if (this->data->hasText()) {
        painter.drawText(this->rect().marginsRemoved(QMargins(4,4,4,4)),textString());
    } else {
        painter.drawText(this->rect().marginsRemoved(QMargins(4,4,4,4)),tr("No preview available"));
    }
}

void PreviewWidget::setinitialSize()
{
    if (this->data == nullptr) {
        return;
    }
    QRect rect = this->rect();
    if (this->data->hasImage()) {
        QImage image = qvariant_cast<QImage>(this->data->imageData()).scaledToWidth(this->maximumWidth());
        rect = image.rect();
    } else if (this->data->hasText()) {
        QFontMetrics m(this->font());
        rect = m.boundingRect(this->rect(),Qt::AlignLeft,textString()).marginsAdded(QMargins(4,4,6,4));
    } else {
        QFontMetrics m(this->font());

        rect = m.boundingRect(tr("No preview available")).marginsAdded(QMargins(4,4,6,4));
    }
    setGeometry(rect);

}

QString PreviewWidget::textString()
{
    QString text;
    if (this->data && this->data->hasText()) {
        text = this->data->text();
        if (text.length()>150) {
            text = text.left(150) + "[...]";
        }
    }
    return text;
}
