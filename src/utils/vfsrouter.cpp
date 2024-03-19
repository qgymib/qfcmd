#include <QAtomicInteger>
#include <QByteArray>
#include <QJsonDocument>
#include <QMap>
#include <QSharedPointer>
#include "vfs/filesystem.hpp"
#include "vfsrouter.hpp"

namespace qfcmd {

typedef QMap<QString, VfsRouter::RouterCB> VfsRouterMap;

struct VfsRouterSession
{
    VfsRouterSession(uintptr_t fh, uint64_t flags);

    uintptr_t               fh;
    uint64_t                flags;
    VfsRouter::RouterCB     cb;

    QByteArray              sendCache;
};

typedef QSharedPointer<VfsRouterSession> VfsRouterSessionPtr;
typedef QMap<uintptr_t, VfsRouterSessionPtr> VfsRouterSessionMap;

class VfsRouterInner : public FileSystem
{
public:
    VfsRouterInner(VfsRouter* parent);
    ~VfsRouterInner();

public:
    virtual int open(uintptr_t* fh, const QUrl& url, uint64_t flags) override;
    virtual int close(uintptr_t fh) override;
    virtual int read(uintptr_t fh, void* buf, size_t bufsz) override;
    virtual int write(uintptr_t fh, const void *buf, size_t bufsz) override;

public:
    VfsRouter*                  m_parent;
    QAtomicInteger<uintptr_t>   m_fhCnt;

    VfsRouterMap                m_routeMap;
    VfsRouterSessionMap         m_sessionMap;
};

} /* namespace qfcmd */

static int _vfs_router_open(qfcmd::VfsRouterInner* inner,
                            uintptr_t* fh,
                            const qfcmd::VfsRouter::RouterCB& cb,
                            uint64_t flags)
{
    qfcmd::VfsRouterSessionPtr session(new qfcmd::VfsRouterSession(inner->m_fhCnt++, flags));
    session->cb = cb;

    *fh = session->fh;
    inner->m_sessionMap.insert(session->fh, session);

    return 0;
}

qfcmd::VfsRouterSession::VfsRouterSession(uintptr_t fh, uint64_t flags)
{
    this->fh = fh;
    this->flags = flags;
}

qfcmd::VfsRouterInner::VfsRouterInner(VfsRouter* parent)
{
    m_parent = parent;
    m_fhCnt = 0;
}

qfcmd::VfsRouterInner::~VfsRouterInner()
{
}

int qfcmd::VfsRouterInner::open(uintptr_t *fh, const QUrl &url, uint64_t flags)
{
    const QString path = url.path();

    auto it = m_routeMap.upperBound(path);
    if (it == m_routeMap.begin())
    {
        return -ENOENT;
    }

    do
    {
        it--;

        if (path.startsWith(it.key()))
        {
            return _vfs_router_open(this, fh, it.value(), flags);
        }
    } while (it != m_routeMap.begin());

    return -ENOENT;
}

int qfcmd::VfsRouterInner::close(uintptr_t fh)
{
    auto it = m_sessionMap.find(fh);
    if (it == m_sessionMap.end())
    {
        return -ENOENT;
    }

    m_sessionMap.erase(it);
    return 0;
}

int qfcmd::VfsRouterInner::read(uintptr_t fh, void* buf, size_t bufsz)
{
    auto it = m_sessionMap.find(fh);
    if (it == m_sessionMap.end())
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

int qfcmd::VfsRouterInner::write(uintptr_t fh, const void* buf, size_t bufsz)
{
    auto it = m_sessionMap.find(fh);
    if (it == m_sessionMap.end())
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

qfcmd::VfsRouter::VfsRouter()
{
    m_inner = new VfsRouterInner(this);
}

qfcmd::VfsRouter::~VfsRouter()
{
    delete m_inner;
}

void qfcmd::VfsRouter::route(const QString& path, RouterCB cb)
{
    m_inner->m_routeMap.insert(path, cb);
}
