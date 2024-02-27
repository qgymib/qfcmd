#include "foldertab.hpp"
#include "ui_foldertab.h"
#include <QCommonStyle>

FolderTab::FolderTab(QWidget *parent)
    : FolderTab(QDir::currentPath(), parent)
{
}

FolderTab::FolderTab(const QString& path, QWidget *parent)
    : QWidget(parent), ui(new Ui::FolderTab)
{
    ui->setupUi(this);

    {
        QCommonStyle style;
        ui->goBack->setIcon(style.standardIcon(QStyle::SP_ArrowBack));
        ui->goForward->setIcon(style.standardIcon(QStyle::SP_ArrowForward));
        ui->goUp->setIcon(style.standardIcon(QStyle::SP_ArrowUp));
    }

    {
        m_model = new QFileSystemModel;
        ui->tableView->setModel(m_model);

        m_cfg_path_max_history = 1024;
        cd_with_history(path);
    }
    update_ui_visibility();

    connect(ui->goBack, &QPushButton::clicked, this, &FolderTab::onGoBackClicked);
    connect(ui->goForward, &QPushButton::clicked, this, &FolderTab::onGoForwardClicked);
    connect(ui->goUp, &QPushButton::clicked, this, &FolderTab::onGoUpClicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &FolderTab::onTableViewDoubleClicked);
}

FolderTab::~FolderTab()
{
    delete ui;
}

void FolderTab::onGoBackClicked()
{
    if (m_path_idx > 0)
    {
        m_path_idx--;
        QString path = m_path_history[m_path_idx];
        cd(path);
    }
    update_ui_visibility();
}

void FolderTab::onGoForwardClicked()
{
    if (m_path_idx < m_path_history.size() - 1)
    {
        m_path_idx++;
        QString path = m_path_history[m_path_idx];
        cd(path);
    }
    update_ui_visibility();
}

void FolderTab::onGoUpClicked()
{
    QString path = m_path_history[m_path_idx];
    QDir dir(path);
    if (!dir.cdUp())
    {
        return;
    }

    cd_with_history(dir.absolutePath());
    update_ui_visibility();
}

void FolderTab::onTableViewDoubleClicked(const QModelIndex &index)
{
    /*
     * If it is a directory, CD to it.
     */
    if (m_model->isDir(index))
    {
        QString path = m_model->filePath(index);
        cd_with_history(path);
    }

    update_ui_visibility();
}

void FolderTab::update_ui_visibility()
{
    ui->goBack->setEnabled(m_path_idx > 0);
    ui->goForward->setEnabled(m_path_idx < m_path_history.size() - 1);

    QDir dir(m_path_history[m_path_idx]);
    ui->goUp->setEnabled(dir.cdUp());
}

void FolderTab::cd(const QString& path)
{
    ui->url->setPlainText(path);
    ui->tableView->setRootIndex(m_model->setRootPath(path));

    QDir dir(path);
    setWindowTitle(dir.dirName());
}

void FolderTab::cd_with_history(const QString& path)
{
    if (m_path_history.size() > 0 && m_path_idx < m_path_history.size() - 1)
    {
        m_path_history.erase(m_path_history.begin() + m_path_idx + 1, m_path_history.end());
    }
    m_path_history.push_back(path);

    if (m_path_history.size() > m_cfg_path_max_history)
    {
        m_path_history.pop_front();
    }
    m_path_idx = m_path_history.size() - 1;

    cd(path);
}
