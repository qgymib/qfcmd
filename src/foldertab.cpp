#include <QCommonStyle>
#include <QDesktopServices>
#include <QTableWidgetItem>
#include <QMenu>

#if defined(_WIN32)
#include "utils/win32.hpp"
#else
#include <QDBusInterface>
#include <QDBusMessage>
#include <QProcess>
#endif

#include "foldertab.hpp"
#include "ui_foldertab.h"

#if defined(_WIN32)
static void _show_file_properties(const QString& path)
{
    qfcmd::wchar w_path = qfcmd::Win32::StringToWide(path);

    SHELLEXECUTEINFOW aShExecInfo;
    ZeroMemory(&aShExecInfo, sizeof(SHELLEXECUTEINFOW));
    aShExecInfo.cbSize = sizeof(aShExecInfo);
    aShExecInfo.fMask = SEE_MASK_INVOKEIDLIST;
    aShExecInfo.lpVerb = L"properties";
	aShExecInfo.lpFile = w_path.data();
	ShellExecuteExW(&aShExecInfo);
}
#else
static void _show_file_properties(const QString& path)
{
	QDBusInterface iface("org.freedesktop.FileManager1",
		"/org/freedesktop/FileManager1",
		"org.freedesktop.FileManager1",
		QDBusConnection::sessionBus());

	if (!iface.isValid())
	{
		return;
	}

	QDBusMessage rsp = iface.call("ShowItemProperties", QStringList() << QUrl::fromLocalFile(path).toString());
	if (rsp.type() == QDBusMessage::ErrorMessage)
	{
		qWarning() << "Error: " << rsp.errorMessage();
	}
}
#endif

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

    connect(ui->goBack, &QPushButton::clicked, this, &FolderTab::onGoBackClicked);
    connect(ui->goForward, &QPushButton::clicked, this, &FolderTab::onGoForwardClicked);
    connect(ui->goUp, &QPushButton::clicked, this, &FolderTab::onGoUpClicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &FolderTab::onTableViewDoubleClicked);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &FolderTab::onTableViewContextMenuRequested);
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
}

void FolderTab::onGoForwardClicked()
{
    if (m_path_idx < m_path_history.size() - 1)
    {
        m_path_idx++;
        QString path = m_path_history[m_path_idx];
        cd(path);
    }
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
}

void FolderTab::onTableViewDoubleClicked(const QModelIndex &index)
{
    const QString path = m_model->filePath(index);

    if (m_model->isDir(index))
    {/* Open directory. */
        cd_with_history(path);
    }
    else
    {/* Open file. */
        QDesktopServices::openUrl(QUrl(path));
    }
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

    update_ui_visibility();
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

void FolderTab::onTableViewContextMenuRequested(QPoint pos)
{
    QMenu* menu = new QMenu(this);
    menu->addAction(tr("Properties"), this, &FolderTab::slotShowProperties);
	menu->exec(ui->tableView->viewport()->mapToGlobal(pos));
}

void FolderTab::slotShowProperties()
{
    QModelIndex index = ui->tableView->currentIndex();
    const QString path = m_model->filePath(index);
    _show_file_properties(path);
}
