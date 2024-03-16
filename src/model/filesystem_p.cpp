#include <QApplication>
#include <QDateTime>
#include <QImageReader>
#include <QFileInfo>
#include "vfs/vfs.hpp"
#include "filesystem_p.hpp"

static QVariant _fs_model_node_get_name(qfcmd::FileSystemModelNode* node)
{
    return node->m_name;
}

static QVariant _fs_model_node_get_ext(qfcmd::FileSystemModelNode* node)
{
    if (node->m_stat.st_mode & QFCMD_FS_S_IFDIR)
    {
        return QVariant();
    }

    int dotIndex = node->m_name.lastIndexOf('.');
    if (dotIndex < 0)
    {
        return QVariant();
    }

    return node->m_name.mid(dotIndex + 1);
}

static QVariant _fs_model_node_get_size(qfcmd::FileSystemModelNode* node)
{
    if (node->m_stat.st_mode & QFCMD_FS_S_IFDIR)
    {
        const QString info = QApplication::translate("FileSystemModel", "Dir");
        return "<" + info + ">";
    }

    return node->m_stat.st_size;
}

static QVariant _fs_model_node_get_date(qfcmd::FileSystemModelNode* node)
{
    return QDateTime::fromSecsSinceEpoch(node->m_stat.st_mtime).toString("yyyy-MM-dd HH:mm:ss");
}

static qfcmd::FileSystemModelNode* _fs_model_get_scheme_node(
    qfcmd::FileSystemModelInner* inner, const QUrl& url)
{
    qfcmd::FileSystemModelNode* root = inner->m_root;
    const QString scheme = url.scheme() + "://";

    auto it = root->m_children.find(scheme);
    if (it != root->m_children.end())
    {
        return it.value();
    }

    qfcmd::FileSystemModelNode* node = new qfcmd::FileSystemModelNode(root);
    node->m_name = scheme;
    root->m_children.insert(scheme, node);
    root->m_visibleChildren.append(scheme);
    return node;
}

static bool _fs_model_compare_stat(const qfcmd_fs_stat_t& a, const qfcmd_fs_stat_t& b)
{
    return a.st_mode == b.st_mode && a.st_size == b.st_size && a.st_mtime == b.st_mtime;
}

static QIcon _fs_model_get_local_file_icon_direct_read(const QUrl &url, const qfcmd_fs_stat_t& stat)
{
    const uint64_t maxSize = 131072;
    if (stat.st_size > maxSize)
    {
        return QIcon();
    }

    const QString path = url.toLocalFile();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return QIcon();
    }

    QImageReader imgReader(&file);
    if (!imgReader.canRead())
    {
        return QIcon();
    }

    QImage image;
    if (!imgReader.read(&image))
    {
        return QIcon();
    }

    QPixmap pix = QPixmap::fromImage(image, Qt::ThresholdDither | Qt::AutoColor);
    return QIcon(pix);
}

qfcmd::IconProvider::IconProvider()
{
}

QIcon qfcmd::IconProvider::icon(const QUrl &url, const qfcmd_fs_stat_t& stat)
{
    QIcon icon;
    const QString scheme = url.scheme();
    if (scheme == "file")
    {
        icon = getNativeIcon(url, stat);
    }

    if (!icon.isNull())
    {
        return icon;
    }

    if (stat.st_mode & QFCMD_FS_S_IFDIR)
    {
        return QFileIconProvider::icon(QFileIconProvider::Folder);
    }
    return QFileIconProvider::icon(QFileIconProvider::File);
}

QIcon qfcmd::IconProvider::getNativeIcon(const QUrl &url, const qfcmd_fs_stat_t &stat)
{
    QIcon icon = _fs_model_get_local_file_icon_direct_read(url, stat);
    if (!icon.isNull())
    {
        return icon;
    }

    const QString path = url.toLocalFile();
    const QFileInfo info(path);
    return QFileIconProvider::icon(info);
}

qfcmd::FileSystemModelNode::FileSystemModelNode(FileSystemModelNode* parent)
{
    m_parent = parent;
    memset(&m_stat, 0, sizeof(m_stat));
}

qfcmd::FileSystemModelNode::~FileSystemModelNode()
{
    /* Remove link for parent. */
    if (m_parent != nullptr)
    {
        Q_ASSERT(m_parent->m_children.size() == m_parent->m_visibleChildren.size());

        m_parent->m_children.remove(m_name);
        m_parent->m_visibleChildren.removeOne(m_name);
        m_parent = nullptr;
    }

    Q_ASSERT(m_children.size() == m_visibleChildren.size());

    while (!m_visibleChildren.empty())
    {
        const QString childName = m_visibleChildren.takeLast();

        auto it_child = m_children.find(childName);
        Q_ASSERT(it_child != m_children.end());
        FileSystemModelNode* child = it_child.value();
        m_children.erase(it_child);

        child->m_parent = nullptr;
        delete child;
    }
}

void qfcmd::FileSystemModelWorker::doFetch(const QUrl &url)
{
    FileSystem::FsPtr fs = VFS::accessfs(url);

    FileSystem::FileInfoEntry entry;
    int ret = fs->ls(url, &entry);

    emit fetchReady(url, ret, entry);
}

qfcmd::FileSystemModelInner::FileSystemModelInner(FileSystemModel *parent)
    : QObject(parent)
{
    m_titles = {
        {
            TITLE_NAME,
            QApplication::translate("FileSystemModel", "Name"),
            _fs_model_node_get_name,
        },
        {
            TITLE_EXT,
            QApplication::translate("FileSystemModel", "Ext"),
            _fs_model_node_get_ext,
        },
        {
            TITLE_SIZE,
            QApplication::translate("FileSystemModel", "Size"),
            _fs_model_node_get_size,
        },
        {
            TITLE_DATE,
            QApplication::translate("FileSystemModel", "Last modified"),
            _fs_model_node_get_date,
        },
    };
    m_sort_order = Qt::AscendingOrder;
    m_parent = parent;
    m_root = new FileSystemModelNode(nullptr);

    {
        FileSystemModelWorker* worker = new FileSystemModelWorker;
        worker->moveToThread(&m_workerThread);

        connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &FileSystemModelInner::doFetch, worker, &FileSystemModelWorker::doFetch);
        connect(worker, &FileSystemModelWorker::fetchReady, this, &FileSystemModelInner::handleFetchResult);

        m_workerThread.start();
    }
}

qfcmd::FileSystemModelInner::~FileSystemModelInner()
{
    m_workerThread.quit();
    m_workerThread.wait();

    delete m_root;
}

qfcmd::FileSystemModelNode* qfcmd::FileSystemModelInner::getNode(const QUrl &url)
{
    FileSystem::FsPtr fs = VFS::accessfs(url);
    const QStringList paths = url.path().split('/');

    qfcmd::FileSystemModelNode* node = _fs_model_get_scheme_node(this, url);
    for (qsizetype i = 0; i < paths.size(); i++)
    {
        const QString name = paths[i];
        auto it = node->m_children.find(name);
        if (it != node->m_children.end())
        {
            node = it.value();
            continue;
        }

        qfcmd::FileSystemModelNode* new_node = new qfcmd::FileSystemModelNode(node);
        new_node->m_name = paths[i];
        new_node->m_stat.st_mode |= QFCMD_FS_S_IFDIR;
        node->m_children.insert(name, new_node);
        node->m_visibleChildren.append(name);
        node = new_node;
    }

    Q_ASSERT(node->m_parent != nullptr);
    Q_ASSERT(node->m_parent->m_children.size() == node->m_parent->m_visibleChildren.size());

    return node;
}

QModelIndex qfcmd::FileSystemModelInner::getIndex(FileSystemModelNode *node)
{
    qfcmd::FileSystemModelNode* parentNode = node ? node->m_parent : nullptr;
    if (parentNode == m_root || parentNode == nullptr)
    {
        return QModelIndex();
    }

    int visualRow = parentNode->m_visibleChildren.indexOf(node->m_name);
    return m_parent->createIndex(visualRow, 0, node);
}

QUrl qfcmd::FileSystemModelInner::getUrl(const FileSystemModelNode* node)
{
    QStringList path;

    for (; node != nullptr && node != m_root; node = node->m_parent)
    {
        path.prepend(node->m_name);
    }

    QString fullPath = path[0];
    for (qsizetype i = 1; i < path.size() - 1; i++)
    {
        fullPath += path[i] + "/";
    }
    fullPath += path.last();

    return QUrl(fullPath);
}

void qfcmd::FileSystemModelInner::clearChildren(FileSystemModelNode *node)
{
    QModelIndex nodeIndex = getIndex(node);

    m_parent->beginRemoveRows(nodeIndex, 0, node->m_visibleChildren.size());
    while (node->m_children.size() != 0)
    {
        auto it = node->m_children.begin();
        qfcmd::FileSystemModelNode* child = it.value();
        delete child;
    }
    m_parent->endRemoveRows();
}

void qfcmd::FileSystemModelInner::handleFetchResult(const QUrl& url, int ret, const FileSystem::FileInfoEntry& entry)
{
    FileSystemModelNode* node = getNode(url);
    Q_ASSERT(node->m_children.size() == node->m_visibleChildren.size());

    /* Error occur, clear clildren. */
    if (ret < 0)
    {
        clearChildren(node);
        return;
    }

    Q_ASSERT(node->m_children.size() == node->m_visibleChildren.size());

    FileSystem::FileInfoEntry entryCopy = entry;
    const QModelIndex nodeIndex = getIndex(node);

    for (auto it = node->m_children.begin(); it != node->m_children.end(); )
    {
        const QString childName = it.key();
        FileSystemModelNode* child = node->m_children.value(childName);

        auto entry_it = entryCopy.find(childName);

        /* If name not found in entry, this child should be delete. */
        if (entry_it == entryCopy.end())
        {
            int row = node->m_visibleChildren.indexOf(childName);
            m_parent->beginRemoveRows(nodeIndex, row, row);
            it = node->m_children.erase(it);
            delete child;
            m_parent->endRemoveRows();
            continue;
        }

        /* Child found, check if it is changed. */
        const qfcmd_fs_stat_t info = entry_it.value();
        if (!_fs_model_compare_stat(info, child->m_stat))
        {
            child->m_stat = info;
            child->m_icon = m_iconProvider.icon(getUrl(child), info);
            const QModelIndex childIndex = getIndex(child);
            emit m_parent->dataChanged(childIndex, childIndex, {Qt::DisplayRole});
        }

        /* Remove from entry. */
        entryCopy.erase(entry_it);
        it++;
    }

    Q_ASSERT(node->m_children.size() == node->m_visibleChildren.size());

    if (entryCopy.size() == 0)
    {
        return;
    }

    /* Now everything left should be append. */
    const int beginRow = node->m_children.size();
    const int endRow = beginRow + entryCopy.size() - 1;
    m_parent->beginInsertRows(nodeIndex, beginRow, endRow);
    for (auto it = entryCopy.begin(); it != entryCopy.end(); it++)
    {
        const QString new_node_name = it.key();
        qfcmd::FileSystemModelNode* new_node = new qfcmd::FileSystemModelNode(node);
        node->m_children.insert(new_node_name, new_node);
        node->m_visibleChildren.append(new_node_name);
        new_node->m_name = new_node_name;
        new_node->m_stat = it.value();
        new_node->m_icon = m_iconProvider.icon(getUrl(new_node), new_node->m_stat);
    }
    m_parent->endInsertRows();

    Q_ASSERT(node->m_children.size() == node->m_visibleChildren.size());
}
