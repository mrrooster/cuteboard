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
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);    
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    QSettings opts;
    opts.setValue("localOnly",QVariant(!this->ui->localOnly->isChecked()));
    opts.setValue("user",QVariant(this->ui->user->text()));
    opts.setValue("pass",QVariant(this->ui->pass->text()));
    opts.setValue("host",QVariant(this->ui->host->text()));
    close();
    emit settingsChanged();
}

void SettingsDialog::setVisible(bool visible)
{
    if (visible) {
        QSettings opts;
        this->ui->localOnly->setChecked(!opts.value("localOnly",QVariant(true)).toBool());
        this->ui->user->setText(opts.value("user",QVariant("")).toString());
        this->ui->pass->setText(opts.value("pass",QVariant("")).toString());
        this->ui->host->setText(opts.value("host",QVariant("")).toString());
        bool disabled=!this->ui->localOnly->isChecked();
        this->ui->label->setDisabled(disabled);
        this->ui->label_2->setDisabled(disabled);
        this->ui->label_3->setDisabled(disabled);
        this->ui->user->setDisabled(disabled);
        this->ui->pass->setDisabled(disabled);
        this->ui->host->setDisabled(disabled);
    }
    QDialog::setVisible(visible);
}
