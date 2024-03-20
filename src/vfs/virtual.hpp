#ifndef QFCMD_VFS_VIRTUALFS_HPP
#define QFCMD_VFS_VIRTUALFS_HPP

#include <QJsonObject>
#include <QUrl>
#include "filesystem.hpp"

namespace qfcmd {

class VirtualFSInner;

class VirtualFS : public FileSystem
{
    Q_OBJECT
    friend class VirtualFSInner;

public:
    typedef std::function<QJsonObject(const QJsonObject&)> RouterCB;

public:
    /**
     * @brief Initialize the VirtualFS.
     * @param[in] url - The url mount to.
     */
    VirtualFS(const QUrl& url);
    virtual ~VirtualFS();

public:
    static void init();
    static void exit();

    /**
     * @brief Route a path to a function.
     *
     * @param[in] url - Route url.
     * @param[in] cb   - Callback function.
     */
    static void route(const QUrl& url, RouterCB cb);

    /**
     * @brief Exchange Data from VirtualFS.
     *
     * The exchange procedure obey JsonRPC:
     * + The URL is composed of `model/method`.
     * + The \p msg is the request parameter.
     * + The return value is the response.
     *
     * The response contains:
     * + `result`: This member is REQUIRED on success and not exist when failure.
     * + `error`: This member is REQUIRED on failure and not exist when success.
     *
     * @param[in] url - URL
     * @param[in] msg - Request message.
     * @return Response message.
     */
    static QJsonObject exchange(const QUrl& url, const QJsonObject& msg);

public:
    virtual int open(uintptr_t* fh, const QUrl& url, uint64_t flags) override;
    virtual int close(uintptr_t fh) override;
    virtual int read(uintptr_t fh, void* buf, size_t bufsz) override;
    virtual int write(uintptr_t fh, const void* buf, size_t bufsz) override;

private:
    VirtualFSInner* m_inner;
};

} /* namespace qfcmd */

#endif // QFCMD_VFS_VIRTUALFS_HPP
