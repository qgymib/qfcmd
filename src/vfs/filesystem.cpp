#include "filesystem.hpp"

namespace qfcmd {
class FileSystemInner
{
public:
    FileSystemInner(FileSystem* parent, qfcmd_filesystem_t* fs);
    ~FileSystemInner();

    FileSystem*             parent;
    qfcmd_filesystem_t*     fs;
};
} /* namespace qfcmd */

/**
 * @brief Proxy callback function for ls command.
 * @param[in] name - The name of the file or directory
 * @param[in] stat - Pointer to the file or directory stat structure
 * @param[in] data - Pointer to user data
 * @return the result of the callback function
 */
static int _fs_proxy_ls_cb(const char* name, const qfcmd_fs_stat_t* stat, void* data)
{
    qfcmd::FileSystem::FileInfoEntry* entry = static_cast<qfcmd::FileSystem::FileInfoEntry*>(data);

    entry->insert(QString::fromUtf8(name), *stat);

    return 0;
}

qfcmd::FileSystemInner::FileSystemInner(FileSystem *parent, qfcmd_filesystem_t* fs)
{
    this->parent = parent;
    this->fs = fs;
}

qfcmd::FileSystemInner::~FileSystemInner()
{
    if (this->fs != nullptr)
    {
        this->fs->destroy(this->fs);
    }
    this->fs = nullptr;
}

qfcmd::FileSystem::FileSystem(QObject *parent)
    : FileSystem(nullptr, parent)
{
}

qfcmd::FileSystem::FileSystem(qfcmd_filesystem_t *fs, QObject *parent)
    : QObject(parent)
    , m_inner(new FileSystemInner(this, fs))
{
}

qfcmd::FileSystem::~FileSystem()
{
    delete m_inner;
}

int qfcmd::FileSystem::ls(const QUrl& url, FileInfoEntry* entry)
{
    qfcmd_filesystem_t* fs = m_inner->fs;
    if (fs == nullptr || fs->ls == nullptr)
    {
        return -ENOSYS;
    }

    QByteArray c_path = url.toString().toUtf8();
    void* data = static_cast<void*>(entry);
    return fs->ls(fs, c_path.data(), _fs_proxy_ls_cb, data);
}

int qfcmd::FileSystem::stat(const QUrl& url, qfcmd_fs_stat_t* stat)
{
    qfcmd_filesystem_t* fs = m_inner->fs;
    if (fs == nullptr || fs->stat == nullptr)
    {
        return -ENOSYS;
    }

    QByteArray c_path = url.toString().toUtf8();
    return fs->stat(fs, c_path.data(), stat);
}

int qfcmd::FileSystem::open(uintptr_t* fh, const QUrl& url, uint64_t flags)
{
    qfcmd_filesystem_t* fs = m_inner->fs;
    if (fs == nullptr || fs->open == nullptr)
    {
        return -ENOSYS;
    }

    QByteArray c_path = url.toString().toUtf8();
    return fs->open(fs, fh, c_path.data(), flags);
}

void qfcmd::FileSystem::close(uintptr_t fh)
{
    qfcmd_filesystem_t* fs = m_inner->fs;
    if (fs == nullptr || fs->close == nullptr)
    {
        return;
    }

    fs->close(fs, fh);
}

int qfcmd::FileSystem::read(uintptr_t fh, void* buf, size_t size)
{
    qfcmd_filesystem_t* fs = m_inner->fs;
    if (fs == nullptr || fs->read == nullptr)
    {
        return -ENOSYS;
    }

    return fs->read(fs, fh, buf, size);
}

int qfcmd::FileSystem::write(uintptr_t fh, const void* buf, size_t size)
{
    qfcmd_filesystem_t* fs = m_inner->fs;
    if (fs == nullptr || fs->write == nullptr)
    {
        return -ENOSYS;
    }

    return fs->write(fs, fh, buf, size);
}
