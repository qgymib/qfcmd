#include <QDir>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "aboutdialog.hpp"
#include "perferencesdialog.hpp"
#include "keyboardshortcutsform.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_shortcut_mgr(this)
{
    ui->setupUi(this);
    ui->centralwidget->setStretchFactor(1, 1);
    setupActions();

    {
        ui->treeView->slotChangeDirectory(QDir::currentPath());
    }

    {
        bool visible = settings.value("view/ShowToolbar", true).toBool();
        ui->toolBar->setVisible(visible);
        ui->actionShowToolbar->setChecked(visible);
    }
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
    PerferencesDialog dialog(this);

    dialog.addConfigWidget(new KeyboardShortcutsForm(m_shortcut_mgr.getShortcutMap()));

    dialog.exec();
}
