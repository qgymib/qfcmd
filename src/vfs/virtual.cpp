#include <QAtomicInteger>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QSharedPointer>

#include "virtual.hpp"
#include "vfs.hpp"
#include "file.hpp"

namespace qfcmd {

class VirtualFSInner
{
public:
    VirtualFSInner(const QUrl& url);
    ~VirtualFSInner();

public:
    QUrl    m_url;
};

typedef QMap<QString, VirtualFS::RouterCB> VfsRouterMap;

struct VfsRouterSession
{
    VfsRouterSession(uintptr_t fh, uint64_t flags);

    uintptr_t               fh;
    uint64_t                flags;
    VirtualFS::RouterCB     cb;

    QByteArray              sendCache;
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
                            const qfcmd::VirtualFS::RouterCB& cb,
                            uint64_t flags)
{
    *fh = s_vfs_ctx->m_fhCnt++;

    qfcmd::VfsRouterSessionPtr session(new qfcmd::VfsRouterSession(*fh, flags));
    session->cb = cb;

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
        QJsonObject obj;
        obj["path"] = it.key();
        arr.append(obj);
    }

    ret["result"] = arr;
    return ret;
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

    route(QUrl("qfcmd://virtualfs/route/list"), _virtualfs_list);
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

void qfcmd::VirtualFS::route(const QUrl& url, RouterCB cb)
{
    const QString path = url.toString();
    VFS::mount(url, url);
    s_vfs_ctx->m_routeMap.insert(path, cb);
}

QJsonObject qfcmd::VirtualFS::exchange(const QUrl &url, const QJsonObject &msg)
{
    int ret;
    VFile file;
    QJsonObject result;
    QByteArray cache;

    if (msg.isEmpty())
    {
        goto read_data;
    }
    cache = QJsonDocument(msg).toJson();

    if ((ret = file.open(url, QFCMD_FS_O_RDWR)) != 0)
    {
        result["error"] = ret;
        return result;
    }

    file.write(cache);

read_data:
    cache.clear();
    file.read(cache);

    QJsonDocument rspDoc = QJsonDocument::fromJson(cache);
    Q_ASSERT(!rspDoc.isNull());

    return rspDoc.object();
}

int qfcmd::VirtualFS::open(uintptr_t *fh, const QUrl &url, uint64_t flags)
{
    (void)url;

    const QString path = m_inner->m_url.toString();
    auto it = s_vfs_ctx->m_routeMap.find(path);
    if (it == s_vfs_ctx->m_routeMap.end())
    {
        return -ENOENT;
    }

    return _vfs_router_open(fh, it.value(), flags);
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

    QByteArray jsonData(static_cast<const char*>(buf), bufsz);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull())
    {
        return -EINVAL;
    }
    QJsonObject jsonObj = jsonDoc.object();

    QJsonObject jsonRsp = session->cb(jsonObj);
    if (jsonRsp.isEmpty())
    {
        return bufsz;
    }

    QJsonDocument writeDoc(jsonRsp);
    QByteArray writeData = writeDoc.toJson();
    session->sendCache.append(writeData);

    return bufsz;
}
