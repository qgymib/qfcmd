#ifndef QFCMD_MODEL_FILESYSTEMMODEL_H
#define QFCMD_MODEL_FILESYSTEMMODEL_H

#include <QAbstractItemModel>

namespace qfcmd {

class FileSystemModelInner;

class FileSystemModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class FileSystemModelInner;

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

private:
    FileSystemModelInner* m_inner;
};

} /* namespace qfcmd */

#endif // FILESYSTEMMODEL_H
