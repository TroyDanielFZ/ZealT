/****************************************************************************
**
** Copyright (C) 2015-2016 Oleg Shparber
** Copyright (C) 2013-2014 Jerzy Kozera
** Contact: https://go.zealdocs.org/l/contact
**
** This file is part of Zeal.
**
** Zeal is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Zeal is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Zeal. If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <core/application.h>
#include <core/settings.h>

#include <QDir>
#include <QFileDialog>
//#include <QWebSettings>

using namespace Zeal;
using namespace Zeal::WidgetUi;

namespace {
// QFontDatabase::standardSizes() lacks some sizes, like 13, which QtWK uses by default.
//const QWebSettings::FontFamily BasicFontFamilies[] = {QWebSettings::SerifFont,
//                                                      QWebSettings::SansSerifFont,
//                                                      QWebSettings::FixedFont};
}

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog())
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Setup signals & slots
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::saveSettings);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::loadSettings);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton *button) {
        if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
            saveSettings();
    });

//    QWebSettings *webSettings = QWebSettings::globalSettings();


    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::chooseDocsetStoragePath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                     ui->docsetStorageEdit->text());
    if (path.isEmpty()) {
        return;
    }

#ifdef PORTABLE_BUILD
    // Use relative path if selected directory is under the application binary path.
    if (path.startsWith(QCoreApplication::applicationDirPath() + QLatin1String("/"))) {
        const QDir appDirPath(QCoreApplication::applicationDirPath());
        path = appDirPath.relativeFilePath(path);
    }
#endif

    ui->docsetStorageEdit->setText(QDir::toNativeSeparators(path));
}

void SettingsDialog::loadSettings()
{
    const Core::Settings * const settings = Core::Application::instance()->settings();

    // General Tab
    ui->startMinimizedCheckBox->setChecked(settings->startMinimized);
    ui->checkForUpdateCheckBox->setChecked(settings->checkForUpdate);

    ui->systrayGroupBox->setChecked(settings->showSystrayIcon);
    ui->minimizeToSystrayCheckBox->setChecked(settings->minimizeToSystray);
    ui->hideToSystrayCheckBox->setChecked(settings->hideOnClose);

    ui->toolButton->setKeySequence(settings->showShortcut);

    ui->docsetStorageEdit->setText(QDir::toNativeSeparators(settings->docsetPath));


    // Search Tab
    ui->fuzzySearchCheckBox->setChecked(settings->fuzzySearchEnabled);

    // Network Tab
    switch (settings->proxyType) {
    case Core::Settings::ProxyType::None:
        ui->noProxySettings->setChecked(true);
        break;
    case Core::Settings::ProxyType::System:
        ui->systemProxySettings->setChecked(true);
        break;
    case Core::Settings::ProxyType::UserDefined:
        ui->manualProxySettings->setChecked(true);
        break;
    }

    ui->httpProxy->setText(settings->proxyHost);
    ui->httpProxyPort->setValue(settings->proxyPort);
    ui->httpProxyNeedsAuth->setChecked(settings->proxyAuthenticate);
    ui->httpProxyUser->setText(settings->proxyUserName);
    ui->httpProxyPass->setText(settings->proxyPassword);
}

void SettingsDialog::saveSettings()
{
    Core::Settings * const settings = Core::Application::instance()->settings();

    // General Tab
    settings->startMinimized = ui->startMinimizedCheckBox->isChecked();
    settings->checkForUpdate = ui->checkForUpdateCheckBox->isChecked();

    settings->showSystrayIcon = ui->systrayGroupBox->isChecked();
    settings->minimizeToSystray = ui->minimizeToSystrayCheckBox->isChecked();
    settings->hideOnClose = ui->hideToSystrayCheckBox->isChecked();

    settings->showShortcut = ui->toolButton->keySequence();

    settings->docsetPath = QDir::fromNativeSeparators(ui->docsetStorageEdit->text());


    // Search Tab
    settings->fuzzySearchEnabled = ui->fuzzySearchCheckBox->isChecked();


    // Network Tab
    // Proxy settings
    if (ui->noProxySettings->isChecked())
        settings->proxyType = Core::Settings::ProxyType::None;
    else if (ui->systemProxySettings->isChecked())
        settings->proxyType = Core::Settings::ProxyType::System;
    else if (ui->manualProxySettings->isChecked())
        settings->proxyType = Core::Settings::ProxyType::UserDefined;

    settings->proxyHost = ui->httpProxy->text();
    settings->proxyPort = ui->httpProxyPort->text().toUShort();
    settings->proxyAuthenticate = ui->httpProxyNeedsAuth->isChecked();
    settings->proxyUserName = ui->httpProxyUser->text();
    settings->proxyPassword = ui->httpProxyPass->text();

    settings->save();
}
