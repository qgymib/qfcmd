#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "foldertab.hpp"
#include "aboutdialog.hpp"
#include <QFileSystemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralwidget->setStretchFactor(1, 1);

    connect(ui->actionShowToolbar, &QAction::triggered, this, &MainWindow::onViewShowToolbarChange);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

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
}

void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onViewShowToolbarChange()
{
    bool need_visible = !ui->toolBar->isVisible();

    settings.setValue("view/ShowToolbar", need_visible);

    ui->actionShowToolbar->setChecked(need_visible);
    ui->toolBar->setVisible(need_visible);
}
