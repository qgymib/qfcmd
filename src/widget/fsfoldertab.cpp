#include <QCommonStyle>
#include <QDesktopServices>
#include <QTableWidgetItem>
#include <QMenu>
#include <QVariant>
#include <QApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileSystemModel>

#if defined(_WIN32)
#include <QProcess>
#include "utils/win32.hpp"
#else
#include <QDBusInterface>
#include <QDBusMessage>
#include <QProcess>
#endif

#include "fsfoldertab.hpp"

namespace qfcmd {
struct FolderTabInner
{
    FolderTab*          parent;

    QVBoxLayout*        verticalLayout;
    QHBoxLayout*        horizontalLayout;
    QPushButton*        goBack;
    QPushButton*        goForward;
    QPushButton*        goUp;
    QPlainTextEdit*     url;
    QTableView*         tableView;

    QFileSystemModel*   model;                  /* File system model. */
    qsizetype           cfg_path_max_history;   /* Max number of path history.*/
    qsizetype           path_idx;               /* Current path. */
    QStringList         path_history;           /* Path history. */
};
} /* namespace qfcmd */

#if defined(_WIN32)
static void _show_file_properties(const QString& path)
{
    qfcmd::wchar w_path = qfcmd::wchar::fromQString(path);

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

static void _folder_tabl_setup_inner(qfcmd::FolderTab* parent, qfcmd::FolderTabInner* ui)
{
    ui->parent = parent;

    parent->setWindowTitle(QString::fromUtf8("Form"));
    ui->verticalLayout = new QVBoxLayout(parent);
    ui->horizontalLayout = new QHBoxLayout();
    ui->goBack = new QPushButton(parent);
    QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ui->goBack->sizePolicy().hasHeightForWidth());
    ui->goBack->setSizePolicy(sizePolicy);
    ui->goBack->setMaximumSize(QSize(32, 32));
    ui->goBack->setFlat(true);

    ui->horizontalLayout->addWidget(ui->goBack);

    ui->goForward = new QPushButton(parent);
    sizePolicy.setHeightForWidth(ui->goForward->sizePolicy().hasHeightForWidth());
    ui->goForward->setSizePolicy(sizePolicy);
    ui->goForward->setMaximumSize(QSize(32, 32));
    ui->goForward->setFlat(true);

    ui->horizontalLayout->addWidget(ui->goForward);

    ui->goUp = new QPushButton(parent);
    sizePolicy.setHeightForWidth(ui->goUp->sizePolicy().hasHeightForWidth());
    ui->goUp->setSizePolicy(sizePolicy);
    ui->goUp->setMaximumSize(QSize(32, 32));
    ui->goUp->setFlat(true);

    ui->horizontalLayout->addWidget(ui->goUp);

    ui->url = new QPlainTextEdit(parent);
    QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(ui->url->sizePolicy().hasHeightForWidth());
    ui->url->setSizePolicy(sizePolicy1);
    ui->url->setMaximumSize(QSize(16777215, 32));
    ui->url->setInputMethodHints(Qt::ImhNoAutoUppercase);
    ui->url->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->url->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->url->setLineWrapMode(QPlainTextEdit::NoWrap);

    ui->horizontalLayout->addWidget(ui->url);

    ui->verticalLayout->addLayout(ui->horizontalLayout);

    ui->tableView = new QTableView(parent);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setShowGrid(false);
    ui->tableView->setWordWrap(false);
    ui->tableView->verticalHeader()->setVisible(false);

    ui->verticalLayout->addWidget(ui->tableView);

    QMetaObject::connectSlotsByName(parent);
}

static void _update_ui_visibility(qfcmd::FolderTabInner* inner)
{
    inner->goBack->setEnabled(inner->path_idx > 0);
    inner->goForward->setEnabled(inner->path_idx < inner->path_history.size() - 1);

    QDir dir(inner->path_history[inner->path_idx]);
    inner->goUp->setEnabled(dir.cdUp());
}

/**
 * @brief Set current folder.
 * @param[in] path - path to folder.
 */
static void _folder_tab_cd(qfcmd::FolderTabInner* inner, const QString& path)
{
    inner->url->setPlainText(path);
    inner->tableView->setRootIndex(inner->model->setRootPath(path));

    QDir dir(path);
    inner->parent->setWindowTitle(dir.dirName());

    _update_ui_visibility(inner);
}

/**
 * Change the directory with history tracking.
 *
 * @param inner pointer to the FolderTabInner instance
 * @param path the new directory path
 *
 * @return void
 *
 * @throws None
 */
static void _folder_tab_cd_with_history(qfcmd::FolderTabInner* inner, const QString& path)
{
    if (inner->path_history.size() > 0 && inner->path_idx < inner->path_history.size() - 1)
    {
        inner->path_history.erase(inner->path_history.begin() + inner->path_idx + 1, inner->path_history.end());
    }
    inner->path_history.push_back(path);

    if (inner->path_history.size() > inner->cfg_path_max_history)
    {
        inner->path_history.pop_front();
    }
    inner->path_idx = inner->path_history.size() - 1;

    _folder_tab_cd(inner, path);
}

qfcmd::FolderTab::FolderTab(QWidget *parent)
    : FolderTab(QDir::currentPath(), parent)
{
}

qfcmd::FolderTab::FolderTab(const QString& path, QWidget *parent)
    : QWidget(parent), m_inner(new qfcmd::FolderTabInner)
{
    _folder_tabl_setup_inner(this, m_inner);

    {
        QCommonStyle style;
        m_inner->goBack->setIcon(style.standardIcon(QStyle::SP_ArrowBack));
        m_inner->goForward->setIcon(style.standardIcon(QStyle::SP_ArrowForward));
        m_inner->goUp->setIcon(style.standardIcon(QStyle::SP_ArrowUp));
    }

    {
        m_inner->model = new QFileSystemModel;
        m_inner->tableView->setModel(m_inner->model);

        m_inner->cfg_path_max_history = 1024;
        _folder_tab_cd_with_history(m_inner, path);
    }

    connect(m_inner->goBack, &QPushButton::clicked, this, &FolderTab::onGoBackClicked);
    connect(m_inner->goForward, &QPushButton::clicked, this, &FolderTab::onGoForwardClicked);
    connect(m_inner->goUp, &QPushButton::clicked, this, &FolderTab::onGoUpClicked);
    connect(m_inner->tableView, &QTableView::doubleClicked, this, &FolderTab::slotOpenItem);
    connect(m_inner->tableView, &QTableView::customContextMenuRequested, this, &FolderTab::slotTableViewContextMenuRequested);
}

qfcmd::FolderTab::~FolderTab()
{
    delete m_inner;
}

void qfcmd::FolderTab::onGoBackClicked()
{
    if (m_inner->path_idx > 0)
    {
        m_inner->path_idx--;
        QString path = m_inner->path_history[m_inner->path_idx];
        _folder_tab_cd(m_inner, path);
    }
}

void qfcmd::FolderTab::onGoForwardClicked()
{
    if (m_inner->path_idx < m_inner->path_history.size() - 1)
    {
        m_inner->path_idx++;
        QString path = m_inner->path_history[m_inner->path_idx];
        _folder_tab_cd(m_inner, path);
    }
}

void qfcmd::FolderTab::onGoUpClicked()
{
    QString path = m_inner->path_history[m_inner->path_idx];
    QDir dir(path);
    if (!dir.cdUp())
    {
        return;
    }

    _folder_tab_cd_with_history(m_inner, dir.absolutePath());
}

void qfcmd::FolderTab::slotOpenItem()
{
    QModelIndex index = m_inner->tableView->currentIndex();
    const QString path = m_inner->model->filePath(index);

    if (m_inner->model->isDir(index))
    {/* Open directory. */
        _folder_tab_cd_with_history(m_inner, path);
    }
    else
    {/* Open file. */
        QDesktopServices::openUrl(QUrl(path));
    }
}

void qfcmd::FolderTab::slotTableViewContextMenuRequested(QPoint pos)
{
    const QModelIndex idx = m_inner->tableView->indexAt(pos);
    const QFileInfo info = m_inner->model->fileInfo(idx);
    QMenu* menu = new QMenu(this);
    menu->addAction(tr("Open"), this, &FolderTab::slotOpenItem);
    if (info.isDir())
    {
        menu->addAction(tr("Open in new tab"), this, &FolderTab::slotOpenInNewTab);
    }
    else
    {
        menu->addAction(tr("Open with..."), this, &FolderTab::slotOpenFileWith);
    }

    menu->addAction(tr("Properties"), this, &FolderTab::slotShowProperties);
    menu->exec(m_inner->tableView->viewport()->mapToGlobal(pos));
}

void qfcmd::FolderTab::slotShowProperties()
{
    QModelIndex index = m_inner->tableView->currentIndex();
    const QString path = m_inner->model->filePath(index);
    _show_file_properties(path);
}

void qfcmd::FolderTab::slotOpenFileWith()
{
    QModelIndex index = m_inner->tableView->currentIndex();
    QString path = m_inner->model->filePath(index);
    path = QDir::toNativeSeparators(path);

#if defined(_WIN32)
    QProcess::execute("Rundll32", QStringList() << "Shell32.dll,OpenAs_RunDLL" << path);
#else
    // TODO: Implement on Linux.
#endif
}

void qfcmd::FolderTab::slotOpenInNewTab()
{
    QModelIndex index = m_inner->tableView->currentIndex();
    QString path = m_inner->model->filePath(index);
    emit signalOpenInNewTab(path);
}
