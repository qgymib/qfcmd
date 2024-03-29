#ifndef QFCMD_VFS_HPP
#define QFCMD_VFS_HPP

#include <QSharedPointer>
#include "filesystem.hpp"

namespace qfcmd {

/**
 * @breif In application Virtual File System(VFS).
 *
 * A file system have a scheme and can be registered to VFS via registerVFS().
 * The scheme is used to identify a file system and the file system can be mapped
 * to mount point. If user input a URL like `scheme://path` then the file system
 * will be used to open the file. The scheme `file` is always reserved for local
 * file system.
 */
class VFS : public FileSystem
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(VFS)

public:
    static void init();
    static void exit();
    static void registerVFS(const QString& scheme, const FileSystem::MountFn& fn);

    /**
     * @brief Mount a file system to a mount point.
     *
     * For example, to mount `ftp://127.0.0.1:21` to `file:///mnt/ftp`, use:
     *
     * ```cpp
     * VFS::mount(QUrl("file:///mnt/ftp"), QUrl("ftp://foo:bar@127.0.0.1:21"));
     * ```
     *
     * So access to `file:///mnt/ftp/` will be actually access `ftp://foo:bar@127.0.0.1:21/`.
     *
     * Another example is to mount .zip file:
     *
     * ```cpp
     * VFS::mount(QUrl("file:///foo/bar.zip"), QUrl("file:///foo/bar.zip", "qfcmd_archive"));
     * ```
     *
     * Which mount file `/foo/bar.zip` to directory `/foo/bar.zip` with scheme `archive`.
     *
     * @param[in] path - URL of mount point.
     * @param[in] src - URL of file system.
     * @param[in] scheme - Scheme of file system. If empty, use scheme from \p src.
     * @return 0 on success, or -errno on error.
     */
    static int mount(const QUrl& path, const QUrl& src, const QString& scheme = QString());

    /**
     * @brief Unmount a file system.
     * @note This operation is not sync. A successful operation does not means
     *   no reference to the file system.
     */
    static int unmount(const QUrl& path);

public:
    VFS(QObject* parent = nullptr);
    virtual ~VFS();

public:
    virtual int ls(const QUrl &url, FileInfoEntry *entry) override;
    virtual int stat(const QUrl &url, qfcmd_fs_stat_t *stat) override;
    virtual int open(uintptr_t *fh, const QUrl &url, uint64_t flags) override;
    virtual int close(uintptr_t fh) override;
    virtual int read(uintptr_t fh, void *buf, size_t size) override;
    virtual int write(uintptr_t fh, const void *buf, size_t size) override;
};

} /* namespace qfcmd */

#endif // VFS_HPP
