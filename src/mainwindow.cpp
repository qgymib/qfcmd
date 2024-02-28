#include <QFileSystemModel>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "foldertab.hpp"
#include "aboutdialog.hpp"
#include "perferencesdialog.hpp"
#include "keyboardshortcutsform.hpp"
#include "view/settingsdialog.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_shortcut_mgr(this)
{
    ui->setupUi(this);
    ui->centralwidget->setStretchFactor(1, 1);
    setupActions();

    {
        QFileSystemModel* model = new QFileSystemModel;
        model->setRootPath(QDir::currentPath());
        ui->treeView->setModel(model);
    }

    {
        bool visible = settings.value("view/ShowToolbar", true).toBool();
        ui->toolBar->setVisible(visible);
        ui->actionShowToolbar->setChecked(visible);
    }

    FolderTab* tab = new FolderTab(QDir::homePath());
    ui->leftPanel->addTab(tab, tab->windowTitle());

    tab = new FolderTab;
    ui->leftPanel->addTab(tab, tab->windowTitle());

    tab = new FolderTab(QDir::homePath());
    ui->rightPanel->addTab(tab, tab->windowTitle());

    tab = new FolderTab;
    ui->rightPanel->addTab(tab, tab->windowTitle());
}

MainWindow::~MainWindow()
{
    delete ui;
    ui = nullptr;
}

void MainWindow::setupActions()
{
    /* Register all known actions. */
    m_shortcut_mgr.regAction(ui->actionPerferences, this, &MainWindow::actionPerferences);
    m_shortcut_mgr.regAction(ui->actionShowToolbar, this, &MainWindow::actionShowToolbarChange);
    m_shortcut_mgr.regAction(ui->actionAbout, this, &MainWindow::actionAboutDialog);
}

void MainWindow::actionAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::actionShowToolbarChange()
{
    bool need_visible = !ui->toolBar->isVisible();

    settings.setValue("view/ShowToolbar", need_visible);

    ui->actionShowToolbar->setChecked(need_visible);
    ui->toolBar->setVisible(need_visible);
}

void MainWindow::actionPerferences()
{
    //PerferencesDialog dialog(this);
    //dialog.addConfigWidget(new KeyboardShortcutsForm(m_shortcut_mgr.getShortcutMap()));
    //dialog.exec();

    qfcmd::SettingsDialog dialog(tr("Preferences"), this);
    dialog.exec();
}
