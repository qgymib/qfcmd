#include "file.hpp"
#include "vfs.hpp"

namespace qfcmd {

class VFileInner
{
public:
    VFileInner(const QUrl& url, uint64_t flags);
    ~VFileInner();

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

qfcmd::VFileInner::VFileInner(const QUrl& url, uint64_t flags)
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

qfcmd::VFileInner::~VFileInner()
{
    if (m_opened)
    {
        m_fs->close(m_fh);
        m_opened = false;
    }
}

qfcmd::VFile::VFile(QObject* parent)
    : QObject(parent)
{
    m_inner = nullptr;
}

qfcmd::VFile::~VFile()
{
    close();
}

int qfcmd::VFile::ls(const QUrl &url, FileSystem::FileInfoEntry *entry)
{
    QUrl relative_path;
    FileSystem::FsPtr fs = VFS::accessfs(url, &relative_path);
    return fs->ls(relative_path, entry);
}

int qfcmd::VFile::stat(const QUrl &url, qfcmd_fs_stat_t *stat)
{
    QUrl relative_path;
    FileSystem::FsPtr fs = VFS::accessfs(url, &relative_path);
    return fs->stat(relative_path, stat);
}

int qfcmd::VFile::open(const QUrl& url, uint64_t flags)
{
    int ret;
    if (m_inner != nullptr)
    {
        return -EALREADY;
    }

    m_inner = new VFileInner(url, flags);
    if (m_inner->m_opened)
    {
        return 0;
    }

    ret = m_inner->m_error;
    delete m_inner;
    m_inner = nullptr;

    return ret;
}

void qfcmd::VFile::close()
{
    if (m_inner == nullptr)
    {
        return;
    }

    delete m_inner;
    m_inner = nullptr;
}

int qfcmd::VFile::read(void* buf, size_t size)
{
    if (m_inner == nullptr)
    {
        return -ENOENT;
    }

    return m_inner->m_fs->read(m_inner->m_fh, buf, size);
}

int qfcmd::VFile::read(QByteArray& data)
{
    int read_sz = 0;
    char buf[1024];

    int ret;
    while ((ret = read(buf, sizeof(buf))) > 0)
    {
        data.append(buf, ret);
        read_sz += ret;
    }

    if (ret < 0)
    {
        return ret;
    }

    return read_sz;
}

int qfcmd::VFile::write(const void* buf, size_t size)
{
    if (m_inner == nullptr)
    {
        return -ENOENT;
    }

    return m_inner->m_fs->write(m_inner->m_fh, buf, size);
}

int qfcmd::VFile::write(const QByteArray& data)
{
    const char* buf = data.data();
    size_t size = data.size();
    return write(buf, size);
}
