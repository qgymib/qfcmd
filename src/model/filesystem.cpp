#include <QMap>
#include <QUrl>
#include "filesystem.hpp"
#include "filesystem_p.hpp"
#include "vfs/vfs.hpp"

static qfcmd::FileSystemModelNode* _fs_mode_index_to_node(const QModelIndex& index)
{
    return static_cast<qfcmd::FileSystemModelNode*>(index.internalPointer());
}

static QVariant _fs_model_data_display(qfcmd::FileSystemModelInner* inner,
                                       const QModelIndex &index)
{
    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    Q_ASSERT(node != nullptr);

    int col = index.column();
    if (col < 0 || col >= inner->m_titles.size())
    {
        return QVariant();
    }

    return inner->m_titles[col].func(node);
}

qfcmd::FileSystemModel::FileSystemModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_inner(new FileSystemModelInner(this))
{
}

qfcmd::FileSystemModel::~FileSystemModel()
{
    delete m_inner;
}

QModelIndex qfcmd::FileSystemModel::setRootPath(const QString &path)
{
    const QUrl url = QUrl::fromLocalFile(path);
    FileSystemModelNode* node = m_inner->getNode(url);

    emit m_inner->doFetch(url);
    return createIndex(0, 0, node);
}

QString qfcmd::FileSystemModel::filePath(const QModelIndex &index) const
{
    qfcmd::FileSystemModelNode* node = _fs_mode_index_to_node(index);
    const QUrl url = m_inner->getUrl(node);
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

    if (section < 0 || section >= m_inner->m_titles.size())
    {
        return QVariant();
    }

    return m_inner->m_titles[section].name;
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
    if (column >= m_inner->m_titles.size())
    {
        return QModelIndex();
    }

    /* Find the correct parent node. */
    qfcmd::FileSystemModelNode* parentNode = _fs_mode_index_to_node(parent);
    if (parentNode == nullptr)
    {
        parentNode = m_inner->m_root;
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
        return m_inner->m_root->m_visibleChildren.size();
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

    return m_inner->m_titles.size();
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

    const QUrl url = m_inner->getUrl(parentNode);
    emit m_inner->doFetch(url);
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
        return _fs_model_data_display(m_inner, index);

    default:
        break;
    }

    return QVariant();
}
