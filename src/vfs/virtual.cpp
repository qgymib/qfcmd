#include <QAtomicInteger>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QSharedPointer>

#include "virtual.hpp"
#include "vfs.hpp"
#include "file.hpp"
#include "utils/serialization.hpp"

namespace qfcmd {

class VirtualFSInner
{
public:
    VirtualFSInner(const QUrl& url);
    ~VirtualFSInner();

public:
    QUrl    m_url;
};

struct VfsRouterRecord
{
    uint64_t flags;
    VirtualFS::RouterCB cb;
};

typedef QMap<QString, VfsRouterRecord> VfsRouterMap;

struct VfsRouterSession
{
    VfsRouterSession(uintptr_t fh, uint64_t flags);

    uintptr_t               fh;
    uint64_t                flags;
    VfsRouterRecord         rec;

    QByteArray              sendCache;
    bool                    readed;
};

typedef QSharedPointer<VfsRouterSession> VfsRouterSessionPtr;
typedef QMap<uintptr_t, VfsRouterSessionPtr> VfsRouterSessionMap;

class VirtualFSCtx
{
public:
    VirtualFSCtx();
    ~VirtualFSCtx();

public:
    QAtomicInteger<uintptr_t>   m_fhCnt;
    VfsRouterMap                m_routeMap;
    VfsRouterSessionMap         m_sessionMap;
};

} /* namespace qfcmd */

static qfcmd::VirtualFSCtx* s_vfs_ctx = nullptr;

static int _vfs_router_open(uintptr_t* fh,
                            const qfcmd::VfsRouterRecord& rec,
                            uint64_t flags)
{
    *fh = s_vfs_ctx->m_fhCnt++;

    qfcmd::VfsRouterSessionPtr session(new qfcmd::VfsRouterSession(*fh, flags));
    session->rec = rec;
    session->rec.flags = flags;

    s_vfs_ctx->m_sessionMap.insert(session->fh, session);

    return 0;
}

static int _virtual_fs_mount(const QUrl& url, qfcmd::FileSystem::FsPtr& fs)
{
    fs = qfcmd::FileSystem::FsPtr(new qfcmd::VirtualFS(url));
    return 0;
}

static QJsonObject _virtualfs_list(const QJsonObject& msg)
{
    (void)msg;

    QJsonObject ret;
    QJsonArray arr;

    auto it = s_vfs_ctx->m_routeMap.begin();
    for (; it != s_vfs_ctx->m_routeMap.end(); it++)
    {
        arr.append(it.key());
    }

    ret["result"] = arr;
    return ret;
}

static int _virtual_fs_read_once(qfcmd::VfsRouterSessionPtr session, void* buf, size_t bufsz)
{
    if (!session->readed)
    {
        session->readed = true;
        QJsonObject out = session->rec.cb(QJsonObject());
        session->sendCache = qfcmd::Serialize(out);
    }

    const char* data = session->sendCache.data();
    qsizetype data_sz = session->sendCache.size();
    size_t min_copy_sz = qMin((size_t)data_sz, bufsz);

    memcpy(buf, data, min_copy_sz);
    session->sendCache.remove(0, min_copy_sz);

    return min_copy_sz;
}

qfcmd::VirtualFSInner::VirtualFSInner(const QUrl &url)
{
    m_url = url;
}

qfcmd::VirtualFSInner::~VirtualFSInner()
{
}

qfcmd::VfsRouterSession::VfsRouterSession(uintptr_t fh, uint64_t flags)
{
    this->fh = fh;
    this->flags = flags;
    this->readed = false;
}

qfcmd::VirtualFSCtx::VirtualFSCtx()
{
    m_fhCnt = 0;
}

qfcmd::VirtualFSCtx::~VirtualFSCtx()
{
}

qfcmd::VirtualFS::VirtualFS(const QUrl& url)
{
    m_inner = new VirtualFSInner(url);
}

qfcmd::VirtualFS::~VirtualFS()
{
    delete m_inner;
}

void qfcmd::VirtualFS::init()
{
    if (s_vfs_ctx != nullptr)
    {
        return;
    }

    s_vfs_ctx = new VirtualFSCtx;
    VFS::registerVFS("qfcmd", _virtual_fs_mount);

    route(QUrl("qfcmd://virtualfs/route/list"), QFCMD_FS_O_RDONLY, _virtualfs_list);
}

void qfcmd::VirtualFS::exit()
{
    if (s_vfs_ctx == nullptr)
    {
        return;
    }

    delete s_vfs_ctx;
    s_vfs_ctx = nullptr;
}

void qfcmd::VirtualFS::route(const QUrl& url, uint64_t flags, RouterCB cb)
{
    const QString path = url.toString();
    VFS::mount(url, url);

    /* Remove unnecessary flags */
    flags &= QFCMD_FS_O_RDWR;

    VfsRouterRecord rec;
    rec.cb = cb;
    rec.flags = flags;
    s_vfs_ctx->m_routeMap.insert(path, rec);
}

QJsonObject qfcmd::VirtualFS::exchange(const QUrl &url, uint64_t flags, const QJsonObject &msg)
{
    int ret;
    VFile file;
    QJsonObject result;
    QByteArray cache;

    if ((ret = file.open(url, flags)) != 0)
    {
        result["error"] = ret;
        return result;
    }

    if (!msg.isEmpty())
    {
        cache = QJsonDocument(msg).toJson();
        file.write(cache);
    }

    cache.clear();
    file.read(cache);

    QJsonDocument rspDoc = QJsonDocument::fromJson(cache);
    Q_ASSERT(!rspDoc.isNull());

    return rspDoc.object();
}

int qfcmd::VirtualFS::open(uintptr_t *fh, const QUrl &url, uint64_t flags)
{
    (void)url;

    /* Remove unnecessary flags. */
    flags &= QFCMD_FS_O_RDWR;

    const QString path = m_inner->m_url.toString();
    auto it = s_vfs_ctx->m_routeMap.find(path);
    if (it == s_vfs_ctx->m_routeMap.end())
    {
        return -ENOENT;
    }
    VfsRouterRecord record = it.value();

    /*
     * The provider and the consumer must share compatible flags:
     * If provider is #QFCMD_FS_O_RDONLY, the consumer must be #QFCMD_FS_O_RDONLY.
     * If provider is #QFCMD_FS_O_WRONLY, the consumer must be #QFCMD_FS_O_WRONLY.
     * If provider is #QFCMD_FS_O_RDWR, the consumer can be #QFCMD_FS_O_RDWR or #QFCMD_FS_O_WRONLY.
    */
    uint64_t comb_flags = record.flags & flags;
    if (comb_flags == 0 || comb_flags != flags)
    {
        return -EPERM;
    }

    return _vfs_router_open(fh, record, comb_flags);
}

int qfcmd::VirtualFS::close(uintptr_t fh)
{
    auto it = s_vfs_ctx->m_sessionMap.find(fh);
    if (it == s_vfs_ctx->m_sessionMap.end())
    {
        return -ENOENT;
    }

    s_vfs_ctx->m_sessionMap.erase(it);
    return 0;
}

int qfcmd::VirtualFS::read(uintptr_t fh, void *buf, size_t bufsz)
{
    auto it = s_vfs_ctx->m_sessionMap.find(fh);
    if (it == s_vfs_ctx->m_sessionMap.end())
    {
        return -EINVAL;
    }
    VfsRouterSessionPtr session = it.value();

    /* This session is opend as read-only, just call route and read output */
    if (session->rec.flags == QFCMD_FS_O_RDONLY)
    {
        return _virtual_fs_read_once(session, buf, bufsz);
    }

    const char* data = session->sendCache.data();
    qsizetype data_sz = session->sendCache.size();
    if (data == nullptr || data_sz <= 0)
    {
        return 0;
    }

    size_t copy_sz = qMin((size_t)data_sz, bufsz);
    memcpy(buf, data, copy_sz);
    session->sendCache.remove(0, copy_sz);

    return copy_sz;
}

int qfcmd::VirtualFS::write(uintptr_t fh, const void *buf, size_t bufsz)
{
    auto it = s_vfs_ctx->m_sessionMap.find(fh);
    if (it == s_vfs_ctx->m_sessionMap.end())
    {
        return -EINVAL;
    }
    VfsRouterSessionPtr session = it.value();

    /* This session is opend as write-only, no need to save response. */
    if (session->rec.flags == QFCMD_FS_O_WRONLY)
    {
        return bufsz;
    }

    QByteArray jsonData(static_cast<const char*>(buf), bufsz);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull())
    {
        return -EINVAL;
    }
    QJsonObject jsonObj = jsonDoc.object();

    QJsonObject jsonRsp = session->rec.cb(jsonObj);
    if (jsonRsp.isEmpty())
    {
        return bufsz;
    }

    QByteArray writeData = Serialize(jsonRsp);
    session->sendCache.append(writeData);

    return bufsz;
}
