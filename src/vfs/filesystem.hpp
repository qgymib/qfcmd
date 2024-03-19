#ifndef QFCMD_VFS_FILESYSTEM_HPP
#define QFCMD_VFS_FILESYSTEM_HPP

#include <functional>
#include <QString>
#include <QSharedPointer>
#include <QUrl>
#include <QObject>
#include <QMap>

#include "qfcmd/filesystem.h"

namespace qfcmd {

class FileSystemInner;

/**
 * @brief File system base class.
 *
 * This is the base class for all file systems, and provides C++ wrapper for
 * #qfcmd_filesystem_t.
 */
class FileSystem : public QObject
{
    Q_OBJECT

    friend class FileSystemInner;

public:
    typedef QMap<QString, qfcmd_fs_stat_t> FileInfoEntry;
    typedef QSharedPointer<FileSystem> FsPtr;

    /**
     * @brief File system mount function.
     * @param[in] url - URL of mount point.
     * @param[out] fs - File system instance.
     * @return 0 on success, or -errno on error.
     */
    typedef std::function<int(const QUrl& url, FsPtr& fs)> MountFn;

    typedef std::function<int(const QString&, const qfcmd_fs_stat_t*)> FillDirFn;

public:
    FileSystem(QObject *parent = nullptr);
    FileSystem(qfcmd_filesystem_t* fs, QObject *parent = nullptr);
    virtual ~FileSystem();

public:
    /**
     * @brief List items in directory.
     * @param[in] url - URL of directory.
     * @param[in] cb - Callback function.
     * @return  0 on success, or -errno on error.
     */
    virtual int ls(const QUrl& url, FileInfoEntry* entry);

    /**
     * @brief Get file status.
     * @param[in] url - URL of file.
     * @param[out] stat - File status.
     * @return 0 on success, or -errno on error.
     */
    virtual int stat(const QUrl& url, qfcmd_fs_stat_t* stat);

    /**
     * @brief Open file.
     * @param[out] fh - File handle.
     * @param[in] path - URL of file.
     * @param[in] flags - Open flags.
     * @return 0 on success, or -errno on error.
     */
    virtual int open(uintptr_t* fh, const QUrl& url, uint64_t flags);

    /**
     * @brief Close file.
     * @param[in] fh - File handle.
     * @return 0 on success, or -errno on error.
     */
    virtual int close(uintptr_t fh);

    /**
     * @brief Read data from file.
     * @param[in] fh - File handle.
     * @param[in] buf - Buffer to store data.
     * @param[in] size - Size of data.
     * @return Number of bytes read on success, or -errno on error.
     */
    virtual int read(uintptr_t fh, void* buf, size_t size);

    /**
     * @brief Write data to file.
     * @param[in] fh - File handle.
     * @param[in] buf - Buffer containing data.
     * @param[in] size - Size of data.
     * @return Number of bytes written on success, or -errno on error.
     */
    virtual int write(uintptr_t fh, const void* buf, size_t size);

private:
    FileSystemInner* m_inner;   /**< Inner object. */
};

} /* namespace qfcmd */

#endif
