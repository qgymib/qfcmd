
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

struct VfsInner
{
    VfsInner();
    ~VfsInner();

    VfsProviderMap  fsMap;      /**< Map of file system provider. */

    /**
     * @brief Map of mount point.
     *
     * Suppose we have mount point list:
     * 0. file:///foo
     * 1. file:///foo/bar
     * 2. file:///foo/bar0
     * 3. file:///foo/bar0/1
     * 4. ftp://127.0.0.1:21
     *
     * The access to `file///foo/bar/1` should return index 1.
     */
    VfsMountMaps    mountMap;
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

qfcmd::VfsInner::VfsInner()
{
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
    const QString mount_path = path.path();
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
    const QString mount_path = path.toString();
    VfsMountMaps::iterator it = s_vfs->mountMap.find(mount_path);
    if (it == s_vfs->mountMap.end())
    {
        return -ENOENT;
    }

    s_vfs->mountMap.erase(it);
    return 0;
}

qfcmd::FileSystem::FsPtr qfcmd::VFS::accessfs(const QUrl& path)
{
    const QString file_path = path.path();

    VfsMountMaps::iterator it = s_vfs->mountMap.upperBound(file_path);
    if (it == s_vfs->mountMap.begin())
    {
        return qfcmd::FileSystem::FsPtr();
    }

    do
    {
        it--;

        if (file_path.startsWith(it.key()))
        {
            return it.value();
        }
    } while (it != s_vfs->mountMap.begin());

    return qfcmd::FileSystem::FsPtr();
}
