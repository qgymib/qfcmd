#ifndef QFCMD_VFS_FILE_HPP
#define QFCMD_VFS_FILE_HPP

#include <QByteArray>
#include <QObject>
#include <QUrl>
#include "filesystem.hpp"

namespace qfcmd {

class VFileInner;

/**
 * @brief File class for access to VFS.
 */
class VFile : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(VFile)

    friend class VFileInner;

public:
    VFile(QObject* parent = nullptr);
    virtual ~VFile();

public:

    /**
     * @brief List files.
     * @param[in] url - URL of directory.
     * @param[out] entry - File information.
     * @return 0 on success, or -errno on error.
     */
    static int ls(const QUrl &url, FileSystem::FileInfoEntry *entry);

    /**
     * @brief Get file information.
     * @param[in] url - URL of file.
     * @param[out] stat - File information.
     * @return 0 on success, or -errno on error.
     */
    static int stat(const QUrl &url, qfcmd_fs_stat_t *stat);

public:

    /**
     * @brief Open the file.
     * @param[in] url - URL of file.
     * @param[in] flags - Open flags. See #qfcmd_fs_open_flag_t.
     * @return 0 on success, or -errno on error.
     */
    virtual int open(const QUrl& url, uint64_t flags);

    /**
     * @brief Close the file.
     *
     * The file will be closed automatically when the object is destroyed.
     */
    virtual void close();

    /**
     * @brief Read data from file.
     * @param[out] buf - Buffer to store data.
     * @param[in] size - Buffer size.
     * @return Number of bytes read on success, or -errno on error.
     */
    virtual int read(void* buf, size_t size);

    /**
     * @brief Read data from file.
     * @param[out] data - Buffer to store data.
     * @return Number of bytes read on success, or -errno on error.
     */
    virtual int read(QByteArray& data);

    /**
     * @brief Write data to file.
     * @param[in] buf - Buffer containing data.
     * @param[in] size - Size of data.
     * @return Number of bytes written on success, or -errno on error.
     */
    virtual int write(const void* buf, size_t size);

    /**
     * @brief Write data to file.
     * @param[in] data - Buffer containing data.
     * @return Number of bytes written on success, or -errno on error.
     */
    virtual int write(const QByteArray& data);

private:
    VFileInner* m_inner;
};

} /* namespace qfcmd */

#endif
