#ifndef QFCMD_MODEL_FILESYSTEMMODEL_PRIVATE_H
#define QFCMD_MODEL_FILESYSTEMMODEL_PRIVATE_H

#include <functional>
#include <QFileIconProvider>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QUrl>
#include <QVector>
#include <QIcon>

#include "qfcmd/filesystem.h"
#include "vfs/filesystem.hpp"
#include "filesystem.hpp"

namespace qfcmd {

class IconProvider : public QFileIconProvider
{
    Q_DISABLE_COPY_MOVE(IconProvider)

public:
    IconProvider();

public:
    QIcon icon(const QUrl& url, const qfcmd_fs_stat_t& stat);

private:
    QIcon getNativeIcon(const QUrl& url, const qfcmd_fs_stat_t& stat);
};

class FileSystemModelNode
{
public:
    FileSystemModelNode(FileSystemModelNode* parent);
    ~FileSystemModelNode();

public:
    QString                             m_name;             /**< The name of the node. */
    qfcmd_fs_stat_t                     m_stat;             /**< File stat. */
    QIcon                               m_icon;

    FileSystemModelNode*                m_parent;           /**< The parent node. */
    QMap<QString, FileSystemModelNode*> m_children;         /**< The children nodes. */
    QVector<QString>                    m_visibleChildren;  /**< The visible children nodes. */
};

class FileSystemModelWorker : public QObject
{
    Q_OBJECT

public slots:
    void doFetch(const QUrl& url);

signals:
    void fetchReady(const QUrl& url, int ret, const FileSystem::FileInfoEntry& entry);
};

class FileSystemModelInner : public QObject
{
    Q_OBJECT

public:

    enum TitleType
    {
        TITLE_NAME,
        TITLE_EXT,
        TITLE_SIZE,
        TITLE_DATE,
    };

    struct TitleEntry
    {
        TitleType                                               type;
        QString                                                 name;
        std::function<QVariant(qfcmd::FileSystemModelNode*)>    func;
    };

public:
    FileSystemModelInner(FileSystemModel* parent);
    ~FileSystemModelInner();

public:
    FileSystemModelNode* getNode(const QUrl& url);
    QModelIndex getIndex(FileSystemModelNode* node);
    QUrl getUrl(const FileSystemModelNode* node);

private:
    void clearChildren(FileSystemModelNode* node);

signals:
    void doFetch(const QUrl& url);

private slots:
    void handleFetchResult(const QUrl& url, int ret, const FileSystem::FileInfoEntry& entry);

public:
    QVector<TitleEntry>     m_titles;       /**< The column titles. */
    Qt::SortOrder           m_sort_order;   /**< The sort order. */
    FileSystemModel*        m_parent;       /**< The parent object. */

    /**
     * @brief The root node.
     *
     * Layout:
     * ```
     * root
     * |- scheme
     *   |- authority
     *     |- path (optional)
     * ```
     *
     * Example 1, the URL `file:///foo/bar` and `file:///foo/bar/` is splited into:
     * ```
     * root
     * |- file://
     *   |- (empty)
     *     |- foo
     *       |- bar
     * ```
     *
     * Example 2, the URL `file:///` is splited into:
     * ```
     * root
     * |- file://
     *   |- (empty)
     * ```
     *
     * Example 3, the URL `http://foo.com` and `http://foo.com/` is splited into:
     * ```
     * root
     * |- http://
     *   |- foo.com
     * ```
     * which is no path nodes.
     */
    FileSystemModelNode*    m_root;

private:
    QThread                 m_workerThread;
    IconProvider            m_iconProvider;
};

} /* namespace qfcmd */

#endif
