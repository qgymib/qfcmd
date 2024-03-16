#ifndef QFCMD_LOCAL_HPP
#define QFCMD_LOCAL_HPP

#include "filesystem.hpp"

namespace qfcmd {

/**
 * @brief The local file system.
 *
 * The local file system is the default file system for this application.
 *
 * It basically has the same function as the host file system, but with some extended
 * features:
 *
 * 1. (Windows only) Contents of `file:///` is:
 *   + All drivers (C:/, D:/, etc.)
 */
class LocalFS : public FileSystem
{
public:
    LocalFS(QObject* parent = nullptr);
    virtual ~LocalFS();

public:
    static int mount(const QUrl& url, FsPtr& fs);

public:
    virtual int ls(const QUrl& url, FileInfoEntry* info) override;
    virtual int stat(const QUrl& path, qfcmd_fs_stat_t* stat) override;
    virtual int open(uintptr_t* fh, const QUrl& path, uint64_t flags) override;
    virtual void close(uintptr_t fh) override;
    virtual int read(uintptr_t fh, void* buf, size_t size) override;
    virtual int write(uintptr_t fh, const void* buf, size_t size) override;
};

} /* namespace qfcmd */

#endif // LOCAL_HPP
