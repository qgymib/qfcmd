#ifndef QFCMD_MODEL_FILESYSTEMMODEL_H
#define QFCMD_MODEL_FILESYSTEMMODEL_H

#include <functional>
#include <QAbstractItemModel>
#include <QFileIconProvider>
#include <QIcon>
#include <QThread>
#include <QUrl>
#include <QVector>

#include "qfcmd/qfcmd.h"
#include "vfs/filesystem.hpp"

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
    Q_DISABLE_COPY_MOVE(FileSystemModelNode)

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

class FileSystemModel : public QAbstractItemModel
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
    explicit FileSystemModel(QObject *parent = nullptr);
    virtual ~FileSystemModel();

public:
    /**
     * @brief Dump model.
     * @return Dumped model.
     */
    QString dumpModel(void) const;

    /**
     * @see https://doc.qt.io/qt-6/qfilesystemmodel.html#setRootPath
     */
    QModelIndex setRootPath(const QString& path);

    /**
     * @see https://doc.qt.io/qt-6/qfilesystemmodel.html#filePath
     */
    QString filePath(const QModelIndex& index) const;

    /**
     * @see https://doc.qt.io/qt-6/qfilesystemmodel.html#isDir
     */
    bool isDir(const QModelIndex &index) const;

    FileSystemModelNode* getNode(const QUrl& url);
    QModelIndex getIndex(FileSystemModelNode* node);
    QUrl getUrl(const FileSystemModelNode* node) const;
    void clearChildren(FileSystemModelNode* node);

public:
    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void doFetch(const QUrl& url) const;

public slots:
    void handleFetchResult(const QUrl& url, int ret, const FileSystem::FileInfoEntry& entry);

public:
    QVector<TitleEntry>     m_titles;       /**< The column titles. */

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

    IconProvider            m_iconProvider;
    QThread                 m_workerThread;
};

} /* namespace qfcmd */

#endif // FILESYSTEMMODEL_H
