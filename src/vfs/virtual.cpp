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

class VirtualFSInner
{
public:
    VirtualFSInner();
    ~VirtualFSInner();

public:
    QAtomicInteger<uintptr_t>   m_fhCnt;
    VfsRouterMap                m_routeMap;
    VfsRouterSessionMap         m_sessionMap;
};

} /* namespace qfcmd */

static qfcmd::VirtualFSInner* s_vfsi = nullptr;

static int _vfs_router_open(uintptr_t* fh,
                            const qfcmd::VirtualFS::RouterCB& cb,
                            uint64_t flags)
{
    *fh = s_vfsi->m_fhCnt++;

    qfcmd::VfsRouterSessionPtr session(new qfcmd::VfsRouterSession(*fh, flags));
    session->cb = cb;

    s_vfsi->m_sessionMap.insert(session->fh, session);

    return 0;
}

static int _vfs_mount(const QUrl& url, qfcmd::FileSystem::FsPtr& fs)
{
    (void)url;
    fs = qfcmd::FileSystem::FsPtr(new qfcmd::VirtualFS);
    return 0;
}

static QJsonObject _vfs_virtualfs_list(const QJsonObject& msg)
{
    (void)msg;

    QJsonObject ret;
    QJsonArray arr;

    auto it = s_vfsi->m_routeMap.begin();
    for (; it != s_vfsi->m_routeMap.end(); it++)
    {
        QJsonObject obj;
        obj["path"] = it.key();
        arr.append(obj);
    }

    ret["result"] = arr;
    return ret;
}

qfcmd::VfsRouterSession::VfsRouterSession(uintptr_t fh, uint64_t flags)
{
    this->fh = fh;
    this->flags = flags;
}

qfcmd::VirtualFSInner::VirtualFSInner()
{
    m_fhCnt = 0;
}

qfcmd::VirtualFSInner::~VirtualFSInner()
{
}

qfcmd::VirtualFS::VirtualFS()
{
}

qfcmd::VirtualFS::~VirtualFS()
{
}

void qfcmd::VirtualFS::init()
{
    if (s_vfsi != nullptr)
    {
        return;
    }

    s_vfsi = new VirtualFSInner;

    VFS::registerVFS("qfcmd", _vfs_mount);
    VFS::mount(QUrl("qfcmd:///"), QUrl("qfcmd:///"));

    route("/vfs/list", _vfs_virtualfs_list);
}

void qfcmd::VirtualFS::exit()
{
    if (s_vfsi == nullptr)
    {
        return;
    }

    delete s_vfsi;
    s_vfsi = nullptr;
}

void qfcmd::VirtualFS::route(const QString &path, RouterCB cb)
{
    s_vfsi->m_routeMap.insert(path, cb);
}

QJsonObject qfcmd::VirtualFS::exchange(const QUrl &url, const QJsonObject &msg)
{
    char buffer[1024];

    QByteArray cache = QJsonDocument(msg).toJson();
    if (msg.isEmpty())
    {
        cache.assign("{}");
    }

    File file;
    QJsonObject result;

    int ret = file.open(url, QFCMD_FS_O_RDWR);
    if (ret != 0)
    {
        result["error"] = ret;
        return result;
    }

    const char* data = cache.data();
    qsizetype data_sz = cache.size();
    file.write(static_cast<const void*>(data), data_sz);

    cache.clear();

    int read_size;
    while ((read_size = file.read(buffer, sizeof(buffer))) > 0)
    {
        cache.append(buffer, read_size);
    }

    QJsonDocument rspDoc = QJsonDocument::fromJson(cache);
    Q_ASSERT(!rspDoc.isNull());

    return rspDoc.object();
}

int qfcmd::VirtualFS::open(uintptr_t *fh, const QUrl &url, uint64_t flags)
{
    const QString path = url.path();

    auto it = s_vfsi->m_routeMap.upperBound(path);
    if (it == s_vfsi->m_routeMap.begin())
    {
        return -ENOENT;
    }

    do
    {
        it--;

        if (path.startsWith(it.key()))
        {
            return _vfs_router_open(fh, it.value(), flags);
        }
    } while (it != s_vfsi->m_routeMap.begin());

    return -ENOENT;
}

int qfcmd::VirtualFS::close(uintptr_t fh)
{
    auto it = s_vfsi->m_sessionMap.find(fh);
    if (it == s_vfsi->m_sessionMap.end())
    {
        return -ENOENT;
    }

    s_vfsi->m_sessionMap.erase(it);
    return 0;
}

int qfcmd::VirtualFS::read(uintptr_t fh, void *buf, size_t bufsz)
{
    auto it = s_vfsi->m_sessionMap.find(fh);
    if (it == s_vfsi->m_sessionMap.end())
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
    auto it = s_vfsi->m_sessionMap.find(fh);
    if (it == s_vfsi->m_sessionMap.end())
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
