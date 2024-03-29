
#include <QMap>

#include "filesystem.hpp"
#include "vfs.hpp"
#include "local.hpp"

namespace qfcmd {

/**
 * @brief Map of file system.
 * The key is scheme of file system. The value is file system.
 */
typedef QMap<QString, FileSystem::MountFn> VfsProviderMap;

/**
 * @brief Map of mount point.
 * The mount point contains scheme and path, e.g. file:///foo/bar.
 * It should be safe to construct a QUrl object from this string.
 */
typedef QMap<QString, FileSystem::FsPtr> VfsMountMaps;

struct VfsFileHandle
{
    VfsFileHandle()
    {
        this->wrap = 0;
        this->real = 0;
    }
    uintptr_t           wrap;
    uintptr_t           real;
    FileSystem::FsPtr   fs;
};

typedef QMap<uintptr_t, VfsFileHandle> VfsFileHandleMap;

struct VfsInner
{
    VfsInner();
    ~VfsInner();

    VfsProviderMap      fsMap;      /**< Map of file system provider. */

    /**
     * @brief Map of mount point.
     *
     * Suppose we have mount point list:
     * 0. file:///
     * 1. file:///foo
     * 2. file:///foo/bar
     * 3. ftp://127.0.0.1:21
     *
     * The access to `file///foo/bar/1` should return index 2.
     */
    VfsMountMaps        mountMap;

    /**
     * @brief Record all open file handle.
     */
    VfsFileHandleMap    fhMap;

    uintptr_t           fhCnt;
};

} /* namespace qfcmd */

static qfcmd::VfsInner* s_vfs = nullptr;

static qfcmd::FileSystem::MountFn _vfs_find_mount_fn_by_url(const QUrl& path, const QString& scheme)
{
    QString path_scheme = scheme;
    if (path_scheme.size() == 0)
    {
        path_scheme = path.scheme();
    }
    qfcmd::VfsProviderMap::iterator it = s_vfs->fsMap.find(scheme);
    if (it == s_vfs->fsMap.end())
    {
        return qfcmd::LocalFS::mount;
    }
    return it.value();
}

/**
 * @brief Get strip path from \p url
 * @param[in] url - URL
 * @return path without trailing '/'.
 */
static QString _vfs_strip_path(const QUrl& url)
{
    QString path = url.path();
    while (path.size() > 1 && path.endsWith('/'))
    {
        path.chop(1);
    }

    return path;
}

/**
 * @brief Get strip url from \p url
 * @param[in] url - URL
 * @return url without trailing '/'.
 */
static QString _vfs_strip_url(const QUrl& url)
{
    QString path = url.scheme() + "://" + url.authority() + _vfs_strip_path(url);
    return path;
}

static qfcmd::FileSystem::FsPtr _vfs_accessfs(const QUrl& path, QUrl* mount)
{
    const QString file_path = _vfs_strip_url(path);

    auto it = s_vfs->mountMap.upperBound(file_path);
    if (it == s_vfs->mountMap.begin())
    {
        return qfcmd::FileSystem::FsPtr();
    }

    do
    {
        it--;

        if (file_path.startsWith(it.key()))
        {
            if (mount != nullptr)
            {
                const QString mount_path = it.key();
                *mount = QUrl(mount_path);
            }

            return it.value();
        }
    } while (it != s_vfs->mountMap.begin());

    return qfcmd::FileSystem::FsPtr();
}

static QUrl _vfs_get_relative_url(const QUrl& url, const QUrl& mount)
{
    QUrl urlCopy = url;
    const QString urlPath = _vfs_strip_path(url);
    const QString mountPath = _vfs_strip_path(mount);
    Q_ASSERT(urlPath.startsWith(mountPath));

    QString newPath = urlPath.mid(mountPath.size());
    if (newPath.startsWith('/'))
    {
        newPath = newPath.mid(1);
    }

    urlCopy.setPath(newPath);
    return urlCopy;
}

static qfcmd::FileSystem::FsPtr _vfs_op(const QUrl& url, QUrl& relative)
{
    QUrl mount_point;
    qfcmd::FileSystem::FsPtr fs = _vfs_accessfs(url, &mount_point);
    relative = _vfs_get_relative_url(url, mount_point);
    return fs;
}

qfcmd::VfsInner::VfsInner()
{
    this->fhCnt = 0;
}

qfcmd::VfsInner::~VfsInner()
{
}

void qfcmd::VFS::init()
{
    if (s_vfs != nullptr)
    {
        return;
    }

    s_vfs = new qfcmd::VfsInner;

    /*
     * Register a local file system as default, and mount to root path.
     */
    VFS::registerVFS("file", qfcmd::LocalFS::mount);
    VFS::mount(QUrl("file:///"), QUrl("file:///"));
}

void qfcmd::VFS::exit()
{
    if (s_vfs == nullptr)
    {
        return;
    }
    delete s_vfs;
    s_vfs = nullptr;
}

void qfcmd::VFS::registerVFS(const QString& scheme, const FileSystem::MountFn& fn)
{
    s_vfs->fsMap.insert(scheme, fn);
}

int qfcmd::VFS::mount(const QUrl& path, const QUrl& src, const QString& scheme)
{
    /* Search for mount point. */
    const QString mount_path = _vfs_strip_url(path);
    VfsMountMaps::iterator it = s_vfs->mountMap.find(mount_path);
    if (it != s_vfs->mountMap.end())
    {
        return -EALREADY;
    }

    qfcmd::FileSystem::FsPtr fs;
    qfcmd::FileSystem::MountFn fn = _vfs_find_mount_fn_by_url(src, scheme);

    int ret;
    if ((ret = fn(src, fs)) < 0)
    {
        return ret;
    }
    else if (fs.isNull())
    {
        return -EINVAL;
    }

    s_vfs->mountMap.insert(mount_path, fs);
    return 0;
}

int qfcmd::VFS::unmount(const QUrl& path)
{
    const QString mount_path = _vfs_strip_url(path);
    VfsMountMaps::iterator it = s_vfs->mountMap.find(mount_path);
    if (it == s_vfs->mountMap.end())
    {
        return -ENOENT;
    }

    s_vfs->mountMap.erase(it);
    return 0;
}

qfcmd::VFS::VFS(QObject* parent)
    : FileSystem(parent)
{
}

qfcmd::VFS::~VFS()
{
}

int qfcmd::VFS::ls(const QUrl &url, FileInfoEntry *entry)
{
    QUrl relative_path;
    FileSystem::FsPtr fs = _vfs_op(url, relative_path);
    return fs->ls(relative_path, entry);
}

int qfcmd::VFS::stat(const QUrl &url, qfcmd_fs_stat_t *stat)
{
    QUrl relative_path;
    FileSystem::FsPtr fs = _vfs_op(url, relative_path);
    return fs->stat(relative_path, stat);
}

int qfcmd::VFS::open(uintptr_t *fh, const QUrl &url, uint64_t flags)
{
    QUrl relative_path;
    qfcmd::VfsFileHandle handle;
    handle.fs = _vfs_op(url, relative_path);

    int ret;
    if ((ret = handle.fs->open(&handle.real, relative_path, flags)) < 0)
    {
        return ret;
    }
    handle.wrap = s_vfs->fhCnt++;

    s_vfs->fhMap.insert(handle.wrap, handle);
    *fh = handle.wrap;

    return 0;
}

int qfcmd::VFS::close(uintptr_t fh)
{
    auto it = s_vfs->fhMap.find(fh);
    if (it == s_vfs->fhMap.end())
    {
        return -ENOENT;
    }

    qfcmd::VfsFileHandle handle = it.value();
    s_vfs->fhMap.erase(it);

    return handle.fs->close(handle.real);
}

int qfcmd::VFS::read(uintptr_t fh, void *buf, size_t size)
{
    auto it = s_vfs->fhMap.find(fh);
    if (it == s_vfs->fhMap.end())
    {
        return -ENOENT;
    }

    qfcmd::VfsFileHandle handle = it.value();
    return handle.fs->read(handle.real, buf, size);
}

int qfcmd::VFS::write(uintptr_t fh, const void *buf, size_t size)
{
    auto it = s_vfs->fhMap.find(fh);
    if (it == s_vfs->fhMap.end())
    {
        return -ENOENT;
    }

    qfcmd::VfsFileHandle handle = it.value();
    return handle.fs->write(handle.real, buf, size);
}
