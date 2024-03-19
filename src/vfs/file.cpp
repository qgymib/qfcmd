#include "file.hpp"
#include "vfs.hpp"

namespace qfcmd {

class FileInner
{
public:
    FileInner(const QUrl& url, uint64_t flags);
    ~FileInner();

public:
    uintptr_t           m_fh;
    uint64_t            m_flags;
    bool                m_opened;
    int                 m_error;

    QUrl                m_url;
    QUrl                m_relativeUrl;
    FileSystem::FsPtr   m_fs;
};

} /* namespace qfcmd */

qfcmd::FileInner::FileInner(const QUrl& url, uint64_t flags)
{
    m_fh = 0;
    m_flags = flags;
    m_opened = false;
    m_error = 0;

    m_url = url;
    m_fs = VFS::accessfs(url, &m_relativeUrl);
    if (m_fs.isNull())
    {
        return;
    }

    if ((m_error = m_fs->open(&m_fh, m_url, flags)) < 0)
    {
        return;
    }

    m_opened = true;
}

qfcmd::FileInner::~FileInner()
{
    if (m_opened)
    {
        m_fs->close(m_fh);
        m_opened = false;
    }
}

qfcmd::File::File(QObject* parent)
    : QObject(parent)
{
    m_inner = nullptr;
}

qfcmd::File::~File()
{
    close();
}

int qfcmd::File::ls(const QUrl &url, FileSystem::FileInfoEntry *entry)
{
    QUrl relative_path;
    FileSystem::FsPtr fs = VFS::accessfs(url, &relative_path);
    return fs->ls(relative_path, entry);
}

int qfcmd::File::stat(const QUrl &url, qfcmd_fs_stat_t *stat)
{
    QUrl relative_path;
    FileSystem::FsPtr fs = VFS::accessfs(url, &relative_path);
    return fs->stat(relative_path, stat);
}

int qfcmd::File::open(const QUrl& url, uint64_t flags)
{
    int ret;
    if (m_inner != nullptr)
    {
        return -EALREADY;
    }

    m_inner = new FileInner(url, flags);
    if (!m_inner->m_opened)
    {
        ret = m_inner->m_error;
        delete m_inner;
        m_inner = nullptr;
        return ret;
    }

    return 0;
}

void qfcmd::File::close()
{
    if (m_inner == nullptr)
    {
        return;
    }

    delete m_inner;
    m_inner = nullptr;
}

int qfcmd::File::read(void* buf, size_t size)
{
    if (m_inner == nullptr)
    {
        return -ENOENT;
    }

    return m_inner->m_fs->read(m_inner->m_fh, buf, size);
}

int qfcmd::File::write(const void* buf, size_t size)
{
    if (m_inner == nullptr)
    {
        return -ENOENT;
    }

    return m_inner->m_fs->write(m_inner->m_fh, buf, size);
}
