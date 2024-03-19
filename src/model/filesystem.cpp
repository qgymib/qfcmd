#include <QApplication>
#include <QImageReader>

#include "vfs/file.hpp"
#include "filesystem.hpp"

static qfcmd::FileSystemModelNode* _fs_mode_index_to_node(const QModelIndex& index)
{
    return static_cast<qfcmd::FileSystemModelNode*>(index.internalPointer());
}

static QVariant _fs_model_data_display(const qfcmd::FileSystemModel* thiz,
                                       const QModelIndex &index)
{
    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    Q_ASSERT(node != nullptr);

    int col = index.column();
    if (col < 0 || col >= thiz->m_titles.size())
    {
        return QVariant();
    }

    return thiz->m_titles[col].func(node);
}

static QVariant _fs_model_data_decoration(const qfcmd::FileSystemModel* thiz,
                                          const QModelIndex &index)
{
    int col = index.column();
    if (col < 0 || col >= thiz->m_titles.size())
    {
        return QVariant();
    }
    if (thiz->m_titles[col].type != qfcmd::FileSystemModel::TITLE_NAME)
    {
        return QVariant();
    }

    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    Q_ASSERT(node != nullptr);

    return node->m_icon;
}

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

    return (qsizetype)node->m_stat.st_size;
}

static QVariant _fs_model_node_get_date(qfcmd::FileSystemModelNode* node)
{
    return QDateTime::fromSecsSinceEpoch(node->m_stat.st_mtime).toString("yyyy-MM-dd HH:mm:ss");
}

static qfcmd::FileSystemModelNode* _fs_model_get_scheme_node(
    qfcmd::FileSystemModel* thiz, const QUrl& url)
{
    qfcmd::FileSystemModelNode* root = thiz->m_root;
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

static qfcmd::FileSystemModelNode* _fs_model_get_authority_node(qfcmd::FileSystemModelNode* schemeNode, const QUrl &url)
{
    const QString authority = url.authority();
    auto it = schemeNode->m_children.find(authority);
    if (it != schemeNode->m_children.end())
    {
        return it.value();
    }

    qfcmd::FileSystemModelNode* authorityNode = new qfcmd::FileSystemModelNode(schemeNode);
    authorityNode->m_name = authority;
    schemeNode->m_children.insert(authority, authorityNode);
    schemeNode->m_visibleChildren.append(authority);

    return authorityNode;
}

static QStringList _fs_model_split_path(const QUrl& url)
{
    /*
     * Possible syntax are:
     * + `/`
     * + `/foo`
     * + `/foo/`
     */
    QString path = url.path();
    QStringList paths = path.split('/', Qt::SkipEmptyParts);

    return paths;
}

static QUrl _fs_model_append_path(const QUrl& url, const QString& path)
{
    QString url_path = url.path();
    if (url_path.endsWith('/'))
    {
        url_path.chop(1);
    }

    url_path += "/" + path;

    QUrl new_url = url;
    new_url.setPath(url_path);
    return new_url;
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

        Q_ASSERT(m_parent->m_children.size() == m_parent->m_visibleChildren.size());

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
    FileSystem::FileInfoEntry entry;
    int ret = File::ls(url, &entry);

    FileInfoMap records;
    for (auto it = entry.begin(); it != entry.end(); it++)
    {
        const QString name = it.key();
        const QUrl item_url = _fs_model_append_path(url, name);

        FileInfo info;
        info.info = it.value();
        info.icon = m_iconProvider.icon(item_url, info.info);

        records.insert(name, info);
    }

    emit fetchReady(url, ret, records);
}

qfcmd::FileSystemModelNode* qfcmd::FileSystemModel::getNode(const QUrl &url)
{
    const QStringList paths = _fs_model_split_path(url);

    qfcmd::FileSystemModelNode* schemeNode = _fs_model_get_scheme_node(this, url);
    qfcmd::FileSystemModelNode* authorityNode = _fs_model_get_authority_node(schemeNode, url);

    qfcmd::FileSystemModelNode* node = authorityNode;
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

QModelIndex qfcmd::FileSystemModel::getIndex(FileSystemModelNode *node)
{
    qfcmd::FileSystemModelNode* parentNode = node ? node->m_parent : nullptr;
    if (parentNode == m_root || parentNode == nullptr)
    {
        return QModelIndex();
    }

    int visualRow = parentNode->m_visibleChildren.indexOf(node->m_name);
    Q_ASSERT(visualRow >= 0);

    return createIndex(visualRow, 0, node);
}

QUrl qfcmd::FileSystemModel::getUrl(const FileSystemModelNode* node) const
{
    QList<const FileSystemModelNode*> nodeChain;

    const FileSystemModelNode* tmpNode = node;
    for (; tmpNode->m_parent != nullptr; tmpNode = tmpNode->m_parent)
    {
        nodeChain.prepend(tmpNode);
    }

    /*
     * [0]: scheme
     * [1]: authority
     */
    Q_ASSERT(nodeChain.size() >= 2);

    QString fullPath = nodeChain.takeFirst()->m_name;
    fullPath += nodeChain.takeFirst()->m_name + "/";

    size_t cnt = 0;
    while (!nodeChain.isEmpty())
    {
        fullPath += nodeChain.takeFirst()->m_name + "/";
        cnt++;
    }

    /* If path empty, the last slash should not remove */
    if (cnt != 0 && !(node->m_stat.st_mode & QFCMD_FS_S_IFDIR))
    {
        fullPath.chop(1);
    }

    return QUrl(fullPath);
}

void qfcmd::FileSystemModel::clearChildren(FileSystemModelNode *node)
{
    QModelIndex nodeIndex = getIndex(node);

    beginRemoveRows(nodeIndex, 0, node->m_visibleChildren.size());
    while (node->m_children.size() != 0)
    {
        auto it = node->m_children.begin();
        qfcmd::FileSystemModelNode* child = it.value();
        delete child;
    }
    endRemoveRows();
}

void qfcmd::FileSystemModel::handleFetchResult(const QUrl& url, int ret, const FileSystemModelWorker::FileInfoMap& entry)
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

    FileSystemModelWorker::FileInfoMap entryCopy = entry;
    const QModelIndex nodeIndex = getIndex(node);

    for (auto it = node->m_children.begin(); it != node->m_children.end(); )
    {
        const QString childName = it.key();
        FileSystemModelNode* child = it.value();

        auto entry_it = entryCopy.find(childName);

        /* If name not found in entry, this child should be delete. */
        if (entry_it == entryCopy.end())
        {
            int row = node->m_visibleChildren.indexOf(childName);
            Q_ASSERT(row >= 0);

            beginRemoveRows(nodeIndex, row, row);
            {
                /*
                 * The following delete step automatically remove child from tree,
                 * so we need to move iterator to next node, without delete it.
                 */
                it++;
                delete child;
            }
            endRemoveRows();
            Q_ASSERT(node->m_children.size() == node->m_visibleChildren.size());

            continue;
        }

        /* Child found, check if it is changed. */
        const qfcmd_fs_stat_t info = entry_it.value().info;
        if (!_fs_model_compare_stat(info, child->m_stat))
        {
            child->m_stat = info;
            child->m_icon = entry_it.value().icon;
            const QModelIndex childIndex = getIndex(child);
            emit dataChanged(childIndex, childIndex, {Qt::DisplayRole});
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
    beginInsertRows(nodeIndex, beginRow, endRow);
    for (auto it = entryCopy.begin(); it != entryCopy.end(); it++)
    {
        const QString new_node_name = it.key();
        qfcmd::FileSystemModelNode* new_node = new qfcmd::FileSystemModelNode(node);
        node->m_children.insert(new_node_name, new_node);
        node->m_visibleChildren.append(new_node_name);
        new_node->m_name = new_node_name;
        new_node->m_stat = it.value().info;
        new_node->m_icon = it.value().icon;
    }
    endInsertRows();

    Q_ASSERT(node->m_children.size() == node->m_visibleChildren.size());
}

qfcmd::FileSystemModel::FileSystemModel(QObject *parent)
    : QAbstractItemModel(parent)
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

    m_root = new FileSystemModelNode(nullptr);

    {
        FileSystemModelWorker* worker = new FileSystemModelWorker;
        worker->moveToThread(&m_workerThread);

        connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &FileSystemModel::doFetch, worker, &FileSystemModelWorker::doFetch);
        connect(worker, &FileSystemModelWorker::fetchReady, this, &FileSystemModel::handleFetchResult);

        m_workerThread.start();
    }
}

qfcmd::FileSystemModel::~FileSystemModel()
{
    m_workerThread.quit();
    m_workerThread.wait();

    delete m_root;
}

QModelIndex qfcmd::FileSystemModel::setRootPath(const QString &path)
{
    const QUrl url = QUrl::fromLocalFile(path);
    FileSystemModelNode* node = getNode(url);

    emit doFetch(url);
    return createIndex(0, 0, node);
}

QString qfcmd::FileSystemModel::filePath(const QModelIndex &index) const
{
    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    const QUrl url = getUrl(node);
    return url.toLocalFile();
}

bool qfcmd::FileSystemModel::isDir(const QModelIndex &index) const
{
    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    return node->m_stat.st_mode & QFCMD_FS_S_IFDIR;
}

QVariant qfcmd::FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation != Qt::Horizontal)
    {
        return QVariant();
    }

    if (section < 0 || section >= m_titles.size())
    {
        return QVariant();
    }

    return m_titles[section].name;
}

static qfcmd::FileSystemModelNode* _fs_model_get_child(qfcmd::FileSystemModelNode* parent, const QString& name)
{
    auto it = parent->m_children.find(name);
    if (it == parent->m_children.end())
    {
        return nullptr;
    }
    return it.value();
}

QModelIndex qfcmd::FileSystemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column >= m_titles.size())
    {
        return QModelIndex();
    }

    /* Find the correct parent node. */
    qfcmd::FileSystemModelNode* parentNode = _fs_mode_index_to_node(parent);
    if (parentNode == nullptr)
    {
        parentNode = m_root;
    }
    if (row >= parentNode->m_visibleChildren.size())
    {
        return QModelIndex();
    }

    const QString childName = parentNode->m_visibleChildren[row];
    FileSystemModelNode* node = _fs_model_get_child(parentNode, childName);
    Q_ASSERT(node != nullptr);

    return createIndex(row, column, static_cast<void*>(node));
}

QModelIndex qfcmd::FileSystemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    Q_ASSERT(node != nullptr);
    qfcmd::FileSystemModelNode* parentNode = node->m_parent;
    if (parentNode == nullptr)
    {
        return QModelIndex();
    }

    Q_ASSERT(parentNode->m_children.size() == parentNode->m_visibleChildren.size());
    int visualRow = parentNode->m_visibleChildren.indexOf(node->m_name);
    Q_ASSERT(visualRow >= 0);

    return createIndex(visualRow, 0, static_cast<void*>(parentNode));
}

int qfcmd::FileSystemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        return m_root->m_visibleChildren.size();
    }

    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(parent);
    Q_ASSERT(node != nullptr);

    return node->m_visibleChildren.size();
}

int qfcmd::FileSystemModel::columnCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    return m_titles.size();
}

bool qfcmd::FileSystemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return true;
    }

    if (parent.column() > 0)
    {
        return false;
    }

    qfcmd::FileSystemModelNode* parentNode = _fs_mode_index_to_node(parent);
    Q_ASSERT(parentNode != nullptr);

    if (!(parentNode->m_stat.st_mode & QFCMD_FS_S_IFDIR))
    {
        return false;
    }

    const QUrl url = getUrl(parentNode);
    emit doFetch(url);
    return true;
}

QVariant qfcmd::FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    switch(role)
    {
    case Qt::DisplayRole:
        return _fs_model_data_display(this, index);

    case Qt::DecorationRole:
        return _fs_model_data_decoration(this, index);

    default:
        break;
    }

    return QVariant();
}
