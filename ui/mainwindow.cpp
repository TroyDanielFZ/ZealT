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

#include <QProcess>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "docsetsdialog.h"
#include "searchitemdelegate.h"
#include "settingsdialog.h"
#include "webbridge.h"
#include <QDebug>
#include "qxtglobalshortcut/qxtglobalshortcut.h"
//#include "widgets/webviewtab.h"

#include <core/application.h>
#include <core/settings.h>
#include <registry/docset.h>
#include <registry/docsetregistry.h>
#include <registry/itemdatarole.h>
#include <registry/listmodel.h>
#include <registry/searchmodel.h>
#include <registry/searchquery.h>

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include <QScrollBar>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QTabBar>
#include <QTimer>

using namespace Zeal;
using namespace Zeal::WidgetUi;

namespace {
const char DarkModeCssUrl[] = ":/browser/assets/css/darkmode.css";
const char HighlightOnNavigateCssUrl[] = ":/browser/assets/css/highlight.css";
}

MainWindow::MainWindow(Core::Application *app, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_application(app),
    m_settings(app->settings()),
    m_zealListModel(new Registry::ListModel(app->docsetRegistry(), this)),
    m_globalShortcut(new QxtGlobalShortcut(m_settings->showShortcut, this)),
    m_searchModel(new Registry::SearchModel()),
    m_openDocsetTimer(new QTimer(this))
{
    ui->setupUi(this);

    // initialise key grabber
    connect(m_globalShortcut, &QxtGlobalShortcut::activated, this, &MainWindow::toggleWindow);

    // Setup application wide shortcuts.
    // Focus search bar.
    QShortcut *shortcut = new QShortcut(QStringLiteral("Ctrl+K"), this);
    connect(shortcut, &QShortcut::activated,
            ui->lineEdit, static_cast<void (SearchEdit::*)()>(&SearchEdit::setFocus));

    shortcut = new QShortcut(QStringLiteral("Ctrl+L"), this);
    connect(shortcut, &QShortcut::activated,
            ui->lineEdit, static_cast<void (SearchEdit::*)()>(&SearchEdit::setFocus));

    for(int i = 0; i< 37; ++ i){ // 1-90a-z
        auto st = i<10? QStringLiteral("Alt+%1").arg(i ):
                        QStringLiteral("Alt+%1").arg(QChar(i-10 + 97));
        shortcut = new QShortcut(st, this);
        connect(shortcut, &QShortcut::activated,
                this, [this,i]{
            int val = i < 10 ? (i+9) % 10 : i;
            if(val >= m_searchModel->rowCount())
                return;
            this->openDocset(m_searchModel->index(val,0,QModelIndex()));
        });
    }

    restoreGeometry(m_settings->windowGeometry);

    // Menu
    // Some platform plugins do not define QKeySequence::Quit.
    if (QKeySequence(QKeySequence::Quit).isEmpty())
        ui->actionQuit->setShortcut(QStringLiteral("Ctrl+Q"));
    else
        ui->actionQuit->setShortcut(QKeySequence::Quit);

    // Follow Windows HIG.
#ifdef Q_OS_WIN32
    ui->actionQuit->setText(tr("E&xit"));
#endif

    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);


    if (QKeySequence(QKeySequence::Preferences).isEmpty()) {
        ui->actionPreferences->setShortcut(QStringLiteral("Ctrl+,"));
    } else {
        ui->actionPreferences->setShortcut(QKeySequence::Preferences);
    }

    connect(ui->actionPreferences, &QAction::triggered, [this]() {
        m_globalShortcut->setEnabled(false);
        QScopedPointer<SettingsDialog> dialog(new SettingsDialog(this));
        dialog->exec();
        m_globalShortcut->setEnabled(true);
    });

    connect(ui->actionDocsets, &QAction::triggered, [this]() {
        QScopedPointer<DocsetsDialog> dialog(new DocsetsDialog(m_application, this));
        dialog->exec();
    });

    connect(ui->actionAboutZeal, &QAction::triggered, [this]() {
        QScopedPointer<AboutDialog> dialog(new AboutDialog(this));
        dialog->exec();
    });

    // treeView and lineEdit
    ui->lineEdit->setTreeView(ui->treeView);
    ui->lineEdit->setFocus();
    setupSearchBoxCompletions();
    SearchItemDelegate *delegate = new SearchItemDelegate(ui->treeView);
    delegate->setDecorationRoles({Registry::ItemDataRole::DocsetIconRole, Qt::DecorationRole});
    connect(ui->lineEdit, &QLineEdit::textChanged, [delegate](const QString &text) {
        delegate->setHighlight(Registry::SearchQuery::fromString(text).query());
    });
    ui->treeView->setItemDelegate(delegate);

    m_webBridge = new WebBridge(this);
    connect(m_webBridge, &WebBridge::actionTriggered, this, [this](const QString &action) {
        // TODO: In the future connect directly to the ActionManager.
        if (action == "openDocsetManager") {
            ui->actionDocsets->trigger();
        } else if (action == "openPreferences") {
            ui->actionPreferences->trigger();
        }
    });


    ui->treeView->setModel(m_zealListModel);
    ui->treeView->setRootIsDecorated(true);

    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::openDocset);
    connect(ui->treeView, &QTreeView::activated, this, &MainWindow::openDocset);

    connect(m_application->docsetRegistry(), &Registry::DocsetRegistry::searchCompleted,
            this, [this](const QList<Registry::SearchResult> &results) {
        if(ui->lineEdit->text().length()){ // non-empty query
            m_searchModel->setResults(results);
            ui->treeView->setModel(m_searchModel);
            ui->treeView->setCurrentIndex(m_searchModel->index(0, 0, QModelIndex()));
        }else{ // empty query
            ui->treeView->setModel(m_zealListModel);
        }
    });



    connect(m_application->docsetRegistry(), &Registry::DocsetRegistry::docsetLoaded,
            this, [this](const QString &) {
        setupSearchBoxCompletions();
    });

    connect(ui->lineEdit, &QLineEdit::textChanged, [this](const QString &text) {
        m_application->docsetRegistry()->search(text);
    });

    // Setup delayed navigation to a page until user makes a pause in typing a search query.
    m_openDocsetTimer->setInterval(400);
    m_openDocsetTimer->setSingleShot(true);
    connect(m_openDocsetTimer, &QTimer::timeout, this, [this]() {
        QModelIndex index = m_openDocsetTimer->property("index").toModelIndex();
        if (!index.isValid())
            return;

        openDocset(index);

        ui->lineEdit->setFocus(Qt::MouseFocusReason);
    });

    connect(m_settings, &Core::Settings::updated, this, &MainWindow::applySettings);
    applySettings();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::search(const Registry::SearchQuery &query)
{
    if (query.isEmpty())
        return;

    ui->lineEdit->setText(query.toString());
    emit ui->treeView->activated(ui->treeView->currentIndex());
}

QString MainWindow::parseAssociationCommand(QString strFilepath){

//    auto& settings = m_settings->mapFileAssociations;
    auto& settings = m_settings->lstFileAssociations;

    // QRegExp reHTML("\\.html?$|\\.html?#");
    // QRegExp rePDF("\\.pdf$|\\.pdf%7C");
    QRegExp reGroup("\\{\\$\\d\\}");
    // rePDF.setCaseSensitivity(Qt::CaseInsensitive);

    using Zeal::Core::AssociationInfo;
    qDebug() << "strFilepath = " << strFilepath;
    QString cmd = "start {%}";
    AssociationInfo currentAssoc;
    foreach(const AssociationInfo & assoc, settings){
        if(assoc.regex.length()){
            if(QRegExp(assoc.regex).indexIn(strFilepath, 0) != -1){
                cmd = assoc.command;
                currentAssoc = assoc;
                break;
            }
        }else if(assoc.match.length()){
            QRegExp rx(assoc.match);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if(rx.exactMatch(strFilepath)){
                cmd = assoc.command;
                currentAssoc = assoc;
                break;
            }
        }

    }

    cmd = cmd.replace("{%}", strFilepath);
//    qDebug() << reGroup.indexIn(cmd, 0);
    if(reGroup.indexIn(cmd) != -1){
        QStringList arr= strFilepath.split(currentAssoc.spliter, QString::SkipEmptyParts);
        int length = arr.length() > 9 ? 9 : arr.length();
        for(int i = 0; i < length; ++i){
            cmd.replace(QString("{$%1}").arg(i + 1), arr[i]);
        }
        for(int i = length; i < 9; ++i){
            cmd.replace(QString("{$%1}").arg(i + 1), "");
        }
        cmd.replace("{0}", strFilepath);
    }else{
        qDebug() << "Match group failed: " + cmd;
    }
    return cmd;
}
void MainWindow::openDocset(const QModelIndex &index)
{
    //toggle expanding state
    ui->treeView->isExpanded(index)? ui->treeView->collapse(index) : ui->treeView->expand(index);

    const QVariant url = index.data(Registry::ItemDataRole::UrlRole);
    if (url.isNull())
        return;
    QString cmd = parseAssociationCommand(url.toString());

    qDebug() << "Open the following document:\n\t" + cmd;
    if(!QProcess::startDetached(cmd)){
        QMessageBox::information(nullptr, "Error", "Error open the following document:\r\n" + cmd, QMessageBox::Warning);
    }
}

QString MainWindow::docsetName(const QUrl &url) const
{
    const QRegExp docsetRegex(QStringLiteral("/([^/]+)[.]docset"));
    return docsetRegex.indexIn(url.path()) != -1 ? docsetRegex.cap(1) : QString();
}

QIcon MainWindow::docsetIcon(const QString &docsetName) const
{
    Registry::Docset *docset = m_application->docsetRegistry()->docset(docsetName);
    return docset ? docset->icon() : QIcon(QStringLiteral(":/icons/logo/icon.png"));
}

void MainWindow::queryCompleted()
{
    m_openDocsetTimer->stop();
    m_openDocsetTimer->setProperty("index", ui->treeView->currentIndex());
    m_openDocsetTimer->start();
}

// Sets up the search box autocompletions.
void MainWindow::setupSearchBoxCompletions()
{
    QStringList completions;
    for (const Registry::Docset * const docset: m_application->docsetRegistry()->docsets()) {
        if (docset->keywords().isEmpty())
            continue;

        completions << docset->keywords().first() + QLatin1Char(':');
    }

    ui->lineEdit->setCompletions(completions);
}

void MainWindow::createTrayIcon()
{
    if (m_trayIcon)
        return;

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon::fromTheme(QStringLiteral("zeal-tray"), windowIcon()));
    m_trayIcon->setToolTip(QStringLiteral("Zeal"));

    connect(m_trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::Trigger && reason != QSystemTrayIcon::DoubleClick)
            return;

        toggleWindow();
    });

    QMenu *trayIconMenu = new QMenu(this);
#if QT_VERSION >= 0x050600
    QAction *toggleAction = trayIconMenu->addAction(tr("Show Zeal"),
                                                    this, &MainWindow::toggleWindow);
#else
    QAction *toggleAction = trayIconMenu->addAction(tr("Show Zeal"));
    connect(toggleAction, &QAction::triggered, this, &MainWindow::toggleWindow);
#endif

    connect(trayIconMenu, &QMenu::aboutToShow, this, [this, toggleAction]() {
        toggleAction->setText(isVisible() ? tr("Minimize to Tray") : tr("Show Zeal"));
    });

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionQuit);
    m_trayIcon->setContextMenu(trayIconMenu);

    m_trayIcon->show();
}

void MainWindow::removeTrayIcon()
{
    if (!m_trayIcon)
        return;

    QMenu *trayIconMenu = m_trayIcon->contextMenu();
    delete m_trayIcon;
    m_trayIcon = nullptr;
    delete trayIconMenu;
}

void MainWindow::bringToFront()
{
    show();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    activateWindow();

    ui->lineEdit->setFocus();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (m_settings->showSystrayIcon && m_settings->minimizeToSystray
            && event->type() == QEvent::WindowStateChange && isMinimized()) {
        hide();
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_settings->showSystrayIcon && m_settings->hideOnClose) {
        event->ignore();
        toggleWindow();
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_tabBar) {
        switch (event->type()) {
//        case QEvent::MouseButtonRelease: {
//            QMouseEvent *e = static_cast<QMouseEvent *>(event);
//            if (e->button() == Qt::MiddleButton) {
//                const int index = m_tabBar->tabAt(e->pos());
//                if (index != -1) {
////                    closeTab(index);
//                    return true;
//                }
//            }
//            break;
//        }
        case QEvent::Wheel:
            // TODO: Remove in case QTBUG-8428 is fixed on all platforms
            return true;
        default:
            break;
        }
    }

    return QMainWindow::eventFilter(object, event);
}

// Captures global events in order to pass them to the search bar.
void MainWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_Escape:
        ui->lineEdit->setFocus();
        ui->lineEdit->clearQuery();
        break;
    case Qt::Key_Question:
        ui->lineEdit->setFocus();
        ui->lineEdit->selectQuery();
        break;
    default:
        QMainWindow::keyPressEvent(keyEvent);
        break;
    }
}

void MainWindow::applySettings()
{
    m_globalShortcut->setShortcut(m_settings->showShortcut);

    if (m_settings->showSystrayIcon)
        createTrayIcon();
    else
        removeTrayIcon();

    // Content
    QByteArray ba = QByteArrayLiteral("body { background-color: white; }");
    if (m_settings->darkModeEnabled) {
        QScopedPointer<QFile> file(new QFile(DarkModeCssUrl));
        if (file->open(QIODevice::ReadOnly)) {
            ba += file->readAll();
        }
    }

    if (m_settings->highlightOnNavigateEnabled) {
        QScopedPointer<QFile> file(new QFile(HighlightOnNavigateCssUrl));
        if (file->open(QIODevice::ReadOnly)) {
            ba += file->readAll();
        }
    }

    if (QFileInfo::exists(m_settings->customCssFile)) {
        QScopedPointer<QFile> file(new QFile(m_settings->customCssFile));
        if (file->open(QIODevice::ReadOnly)) {
            ba += file->readAll();
        }
    }

    const QString cssUrl = QLatin1String("data:text/css;charset=utf-8;base64,") + ba.toBase64();
}

void MainWindow::toggleWindow()
{
    const bool checkActive = sender() == m_globalShortcut;

    if (!isVisible() || (checkActive && !isActiveWindow())) {
        bringToFront();
    } else {
        if (m_trayIcon) {
            hide();
        } else {
            showMinimized();
        }
    }
}
